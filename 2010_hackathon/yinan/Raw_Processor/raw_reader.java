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
        OpenDialog od = new OpenDialog("Open raw Image...", arg);
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
          int nread = fid.read(by);
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
          int unitSize = datatype;
          by = null;
  
          
          //read the data size info (the data size is stored in either 2-byte or 4-byte space)
          int tmpn = 0;         
          int[] sz = new int[4];
          int totalUnit = 1;
          
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
         
          
          //construct img into ImageJ ImageStack
          int layerOffset = sz[0]*sz[1];
          int colorOffset = layerOffset*sz[2];
          ImageStack imStack = new ImageStack(sz[0],sz[1]);
          
          switch (unitSize) {
          case 1:
        	  if (sz[3]==1)
        	  {
        		  for (int layer=0;layer<sz[2];layer++)
        		  {
        			  ByteProcessor cF = new ByteProcessor(sz[0],sz[1]);
            		  byte[] imtmp = new byte[layerOffset];
            		  for (int i=0;i<layerOffset;i++)
        				 imtmp[i] = img[layer*layerOffset+i];
            		  cF.setPixels(imtmp);
            		  imStack.addSlice(null,cF);
        		  }
        	  }
        	  else {
		          for (int layer = 0;layer<sz[2];layer++)
		          {
		        	  ColorProcessor currFrame = new ColorProcessor(sz[0],sz[1]);
		        	  byte[] r,g,b;
		        	  r = new byte[layerOffset];
		        	  g = new byte[layerOffset];
		        	  b = new byte[layerOffset];
		        	  for (int colorChannel = 0; colorChannel<sz[3];colorChannel++)
		              {
		        		  switch (colorChannel)
		        		  {
			        		  case 0:
			        			  for (int i=0;i<layerOffset;i++)
			        				  r[i] = img[colorChannel*colorOffset+layer*layerOffset+i];
			        			  break;
			        		  case 1:
			        			  for (int i=0;i<layerOffset;i++)
			        				  g[i] = img[colorChannel*colorOffset+layer*layerOffset+i];
			        			  break;
			        		  case 2:
			        			  for (int i=0;i<layerOffset;i++)
			        				  b[i] = img[colorChannel*colorOffset+layer*layerOffset+i];
			        			  break;
			        		  default:
			        			  IJ.showMessage("raw format currently supports images with up to 3 channels. Show the first 3 channels only.");
			        			  colorChannel = sz[3];
		        		  }    		  
		        		  currFrame.setRGB(r,g,b);
		              }
		        	imStack.addSlice(null,currFrame);
		        	r = null;
		        	g = null;
		        	b = null;
		          }
        	  }
        	  break;
          case 2:
        	  if (sz[3]!=1)
        		  throw new Exception("now the plugin can only read grayscale 16-bit images.");
    		  byte[] bytmp = new byte[2];
    		  for (int layer=0;layer<sz[2];layer++)
    		  {
    			 short[] im16 = new short[layerOffset];
    			 ShortProcessor cF16 = new ShortProcessor(sz[0],sz[1]);
    			 for (int i=0;i<layerOffset;i++){
    				 bytmp[0] = img[layer*layerOffset*2+i*2];
    				 bytmp[1] = img[layer*layerOffset*2+i*2+1];
    				 im16[i] = (short)bytes2int(bytmp,isBig);
    			 }
    			 cF16.setPixels(im16);
    			 imStack.addSlice(null,cF16);
    		  }
    		  bytmp = null;
        	  break;
          case 4:
        	  if (sz[3]!=1)
        		  throw new Exception("now the plugin can only read grayscale 32-bit images.");
    		  bytmp = new byte[4];
    		  for (int layer=0;layer<sz[2];layer++)
    		  {
    			  float[] im32 = new float[layerOffset];
    			  FloatProcessor cF32 = new FloatProcessor(sz[0],sz[1]);
    			  for (int i=0;i<layerOffset;i++){
    				 bytmp[0] = img[layer*layerOffset*4+i*4];
    				 bytmp[1] = img[layer*layerOffset*4+i*4+1];
    				 bytmp[2] = img[layer*layerOffset*4+i*4+2];
    				 bytmp[3] = img[layer*layerOffset*4+i*4+3];
    				 im32[i] = bytes2int(bytmp,isBig);
    			  }
    			 cF32.setPixels(im32);
    			 imStack.addSlice(null,cF32);
    		  }
    		  bytmp = null;
    		  break;
          default:
        	  throw new Exception("format not supported by raw");
          }
          ImagePlus imPlus = new ImagePlus(fileName,imStack);
          imPlus.show();
          
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
            for (int i=b.length-1;i>=0;i--) {
            	retVal = (retVal<<8) + (b[i] & 0xff);
            }
        else
            for (int i=0;i<b.length;i++) {
                retVal = (retVal<<8) + (b[i] & 0xff);
            }
       
        return retVal;
    }
}
