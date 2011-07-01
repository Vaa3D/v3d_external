import ij.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;
import ij.plugin.*;
import ij.plugin.frame.*;
import java.io.*;
import ij.io.*;
import java.awt.image.*;
import java.io.*;

public class raw_reader implements PlugIn {

    private String formatkey = "raw_image_stack_by_hpeng";
    
    public void run(String arg) {
        OpenDialog od = new OpenDialog("Open V3D's Raw Image...", arg);
        String fileName = od.getFileName();
        if (fileName==null)       return;
        String directory = od.getDirectory();
        IJ.showStatus("Opening: " + directory + fileName);
       
        // Read in the header values...
        try {
          FileInputStream fid = new FileInputStream(directory + fileName);
          int lenkey = formatkey.length();
          long fileSize = fid.available();
         
         
          //read the format key
          if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte.
              throw new Exception("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.");
          byte[] by = new byte[lenkey];
          long nread = fid.read(by);
          String keyread = new String(by);
          if (nread!=lenkey)
              throw new Exception("File unrecognized or corrupted file.");
          if (!keyread.equals(formatkey))
              throw new Exception("Unrecognized file format.");
          by = null;
         
         
          //read the endianness
          by = new byte[1];
          fid.read(by);
          if (by[0]!='B' && by[0]!='L')
              throw new Exception("This program only supports big- or little- endian but not other format. Check your endian.");
        
          boolean isBig = (by[0]=='B');
          by = null;
         
         
          //read the data type info
          by = new byte[2];
          fid.read(by);
          short dcode = (short)bytes2int(by,isBig);
         
          int datatype;
          switch (dcode){
          case 1:
              datatype = 1;
              break;
          case 2:
              datatype = 2;
              break;
          case 4:
              datatype = 4;
              break;
          default:
              throw new Exception("Unrecognized datatype code"+dcode+". The file is incorrect or this code is not supported in this version");
          }
          long unitSize = datatype;
          by = null;
  
          
          //read the data size info (the data size is stored in either 2-byte or 4-byte space)
          long tmpn = 0;         
          long[] sz = new long[4];
          long totalUnit = 1;
          
          //first assume this is a 2-byte file
          by = new byte[2];
          for (int i=0;i<4;i++)
          {
              tmpn += fid.read(by);
              sz[i] = bytes2int(by,isBig);
              totalUnit *= sz[i];
          }         
          by = null;
          
          if ((totalUnit*unitSize+4*2+2+1+lenkey) != fileSize)
          {
        	  //see if this is a 4-byte file
        	  if (isBig)  {
        		sz[0] = sz[0]*64+sz[1];
        		sz[1] = sz[2]*64+sz[3];
        	  }
        	  else {
        		sz[0] = sz[1]*64+sz[0];
        		sz[1] = sz[3]*64+sz[2];
        	  }
        	  by = new byte[4];
        	  for (int i=2;i<4;i++)
        	  {
        		  fid.read(by);
        		  sz[i] = bytes2int(by,isBig);
        	  }
        	  totalUnit = sz[0]*sz[1]*sz[2]*sz[3];
        	  if ((totalUnit*unitSize+4*4+2+1+lenkey) != fileSize)
        		  throw new Exception("The input file has a size different from what specified in the header. Exit.");
        	}
          //IJ.showMessage("image size: w="+sz[0]+" h="+sz[1]+" s="+sz[2]+" c="+sz[3]);
         
          
          
          //read the pixel info
          
          
          byte[] img = new byte[totalUnit*unitSize];
          fid.read(img);
         
          
          //construct img into an array of ImageStacks, the length of array equals number of color channels.
          long layerOffset = sz[0]*sz[1];
          long colorOffset = layerOffset*sz[2];
          
          ImagePlus[] imps = new ImagePlus[sz[3]];
          
          for (long colorChannel=0;colorChannel<sz[3];colorChannel++)
          {
        	  ImageStack imStack = new ImageStack(sz[0],sz[1]);
          
	          switch (unitSize) {
	          case 1:
        		  for (long layer=0;layer<sz[2];layer++)
        		  {
        			  ByteProcessor cF = new ByteProcessor(sz[0],sz[1]);
            		  byte[] imtmp = new byte[layerOffset];
            		  for (long i=0;i<layerOffset;i++)
        				 imtmp[i] = img[colorChannel*colorOffset+layer*layerOffset+i];
            		  cF.setPixels(imtmp);
            		  imStack.addSlice(null,cF);
        		  }
	        	  break;
	          case 2:
	    		  byte[] bytmp = new byte[2];
	    		  for (long layer=0;layer<sz[2];layer++)
	    		  {
	    			 short[] im16 = new short[layerOffset];
	    			 ShortProcessor cF16 = new ShortProcessor(sz[0],sz[1]);
	    			 for (long i=0;i<layerOffset;i++){
	    				 bytmp[0] = img[colorChannel*colorOffset*2+layer*layerOffset*2+i*2];
	    				 bytmp[1] = img[colorChannel*colorOffset*2+layer*layerOffset*2+i*2+1];
	    				 im16[i] = (short)bytes2int(bytmp,isBig);
	    			 }
	    			 cF16.setPixels(im16);
	    			 imStack.addSlice(null,cF16);
	    		  }
	    		  bytmp = null;
	        	  break;
	          case 4:
	    		  bytmp = new byte[4];
	    		  for (long layer=0;layer<sz[2];layer++)
	    		  {
	    			  float[] im32 = new float[layerOffset];
	    			  FloatProcessor cF32 = new FloatProcessor(sz[0],sz[1]);
	    			  for (long i=0;i<layerOffset;i++){
	    				 bytmp[0] = img[colorChannel*colorOffset*4+layer*layerOffset*4+i*4];
	    				 bytmp[1] = img[colorChannel*colorOffset*4+layer*layerOffset*4+i*4+1];
	    				 bytmp[2] = img[colorChannel*colorOffset*4+layer*layerOffset*4+i*4+2];
	    				 bytmp[3] = img[colorChannel*colorOffset*4+layer*layerOffset*4+i*4+3];
	    				 im32[i] = Float.intBitsToFloat(bytes2int(bytmp,isBig));
	    			  }
	    			 cF32.setPixels(im32);
	    			 imStack.addSlice(null,cF32);
	    		  }
	    		  bytmp = null;
	    		  break;
	          default:
	        	  throw new Exception("format not supported by raw");
	          }
	          imps[colorChannel] = new ImagePlus(null,imStack);
          }
          
          if (sz[3]>1){
	          ImagePlus imPlus = RGBStackMerge.mergeChannels(imps,true);
	          imPlus.show();
          }
          else imps[0].show();
          
          fid.close();
        } catch ( Exception e ) {
          IJ.error("Error:" + e.toString());
          return;
        }
    }
   
    public static final int bytes2int(byte[] b,boolean isBig)
    {
        int retVal = 0;
        if (!isBig)
            for (long i=b.length-1;i>=0;i--) {
            	retVal = (retVal<<8) + (b[i] & 0xff);
            }
        else
            for (long i=0;i<b.length;i++) {
                retVal = (retVal<<8) + (b[i] & 0xff);
            }
       
        return retVal;
    }
}
