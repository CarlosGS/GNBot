#ifndef JPEG_READER_H
#define JPEG_READER_H

#include <stdlib.h>
#include <string.h>

extern "C" {
#include <jpeglib.h>
}
    
class JpegReader
{
	public :
		typedef enum {cmAuto, cmGray, cmRGB, cmYUV} ColorSpace;
		unsigned int size;
		unsigned int width,height;
		ColorSpace reqColorSpace, outputColorSpace;
		bool externaloutput;
		unsigned char * buffer;
		double timestamp;

		JpegReader();

		virtual ~JpegReader();

		virtual bool load(char * filename);
		virtual bool load(const unsigned char * src, unsigned int srcsize);

		void setTimeStamp();
		void setTimeStamp(double ts);
		ColorSpace getOutputColorSpace() const {return outputColorSpace;}
		virtual void setOutputColorSpace(ColorSpace cspace);

		void setExternalOutput(unsigned char * dest)
		{
			free(buffer);
			externaloutput = false;
			buffer = dest;
		}
		void resetOutput() {
			externaloutput = true;
			buffer = NULL;
		}

};




#endif // JPEG_READER_H
