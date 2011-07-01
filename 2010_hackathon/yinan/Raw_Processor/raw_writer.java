import ij.*;
import ij.process.*;
import ij.gui.*;
import ij.plugin.*;
import ij.plugin.frame.*;
import ij.io.*;
import ij.plugin.filter.PlugInFilter;
import java.awt.*;
import java.lang.*;
import java.io.*;
import java.util.*;

public class raw_writer implements PlugInFilter {

	private String formatkey = "raw_image_stack_by_hpeng";
	private ImagePlus imp;
	
	public int setup(String arg, ImagePlus imp)
	{
		this.imp = imp;
		return DOES_ALL+NO_CHANGES;
	}
	
	public void run(ImageProcessor ip) {
		SaveDialog sd = new SaveDialog("Save V3D's Raw Image...",imp.getTitle(),".v3draw");
		String directory = sd.getDirectory();
		String fileName = sd.getFileName();
		IJ.showStatus("Saving: " + directory + fileName);
		
		try{
			FileOutputStream out = new FileOutputStream(directory+fileName);
			
			//format key
			out.write(formatkey.getBytes());
			
			//endianness, big-endian is default
			out.write("B".getBytes());
			
			int imtype = imp.getType();
			int[] dim = new int[5];
			dim = imp.getDimensions();
			if (dim[4]!=1)
				throw new Exception("Currently the plugin does not support 5d hyperstack. Please check your data.");
			int[] sz = new int[4];
			sz[0] = dim[0];
			sz[1] = dim[1];
			sz[2] = dim[3];
			sz[3] = dim[2];
			int unitSize;
			
			
			//unitSize & image size depends on the type of ImagePlus
			//IJ.showMessage("image type:"+imtype);
			switch (imtype) {
			case ImagePlus.COLOR_RGB:case ImagePlus.GRAY8:
				unitSize = 1;
				break;
			case ImagePlus.GRAY16:
				unitSize = 2;
				sz[3] = 1;
				break;
			case ImagePlus.GRAY32:
				unitSize = 4;
				sz[3] = 1;
				break;
			default:
				unitSize = imp.getBitDepth()/8;
			}
			
			out.write(int2byte(unitSize,2));
			for (int i=0;i<4;i++) 
				out.write(int2byte(sz[i],4));
			
			
			//pixel info
			int totalUnit = sz[0]*sz[1]*sz[2]*sz[3];
			int layerOffset = sz[0]*sz[1];
			int colorOffset = layerOffset*sz[2];
			ImageStack stack = imp.getStack();
			byte[] img = new byte[totalUnit*unitSize];
			
			switch (imtype) {
			case ImagePlus.COLOR_RGB:
				byte[] r = new byte[layerOffset];
				byte[] g = new byte[layerOffset];
				byte[] b = new byte[layerOffset];
				for (int layer=1;layer<=sz[2];layer++)
				{
					ColorProcessor cp = (ColorProcessor)stack.getProcessor(layer);
					cp.getRGB(r,g,b);
					for (int i=0;i<layerOffset;i++)
					{
						img[(layer-1)*layerOffset+i] = r[i];
						img[colorOffset+(layer-1)*layerOffset+i] = g[i];
						img[2*colorOffset+(layer-1)*layerOffset+i] = b[i];
					}
				}
				r = null;
				g = null;
				b = null;
				break;
			case ImagePlus.GRAY8:
				byte[] imtmp = new byte[layerOffset];
				for (int layer=1;layer<=sz[2];layer++)
				{
					for (int colorChannel=0;colorChannel<sz[3];colorChannel++)
					{
						imtmp = (byte[])stack.getPixels((layer-1)*sz[3]+colorChannel+1);
						for (int i=0;i<layerOffset;i++)
							img[colorChannel*colorOffset+(layer-1)*layerOffset+i] = imtmp[i];
					}
				}
				break;
			case ImagePlus.GRAY16:
				short[] im16 = new short[layerOffset];
				for (int layer=1;layer<=sz[2];layer++)
				{
					for (int colorChannel=0;colorChannel<sz[3];colorChannel++)
					{
						im16 = (short[])stack.getPixels((layer-1)*sz[3]+colorChannel+1);
						for (int i=0;i<layerOffset;i++)
						{
							byte[] tmp = int2byte(im16[i],2);
							img[2*colorChannel*colorOffset+(layer-1)*layerOffset*2+2*i] = tmp[0];
							img[2*colorChannel*colorOffset+(layer-1)*layerOffset*2+2*i+1] = tmp[1];
						}
					}
				}
				im16 = null;
				break;
			case ImagePlus.GRAY32:
				float[] im32 = new float[layerOffset];
				for (int layer=1;layer<=sz[2];layer++)
				{
					for (int colorChannel=0;colorChannel<sz[3];colorChannel++)
					{
						im32 = (float[])stack.getPixels((layer-1)*sz[3]+colorChannel+1);
						for (int i=0;i<layerOffset;i++)
						{
							byte[] tmp = int2byte(Float.floatToIntBits(im32[i]),4);
							img[4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i] = tmp[0];
							img[4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i+1] = tmp[1];
							img[4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i+2] = tmp[2];
							img[4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i+3] = tmp[3];
						}
					}
				}
				im32 = null;
				break;
			default:
				throw new Exception("Image type not supported by this plugin.");
			}
			
			out.write(img);
			
			
			out.close();
		} catch ( Exception e ) {
	          IJ.error("Error:" + e.toString());
	          return;
	        }
	}
	
	byte[] int2byte(int num, int len)
	{
		byte[] by = new byte[len];
		for (int i=len-1;i>=0;i--)
		{
			by[i] = (byte)(num & 0xFF);
			num = num >> 8;
		}
		return(by);
	}

}
