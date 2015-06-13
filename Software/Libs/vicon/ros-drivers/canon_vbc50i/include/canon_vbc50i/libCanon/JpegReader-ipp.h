#ifndef JPEG_READER_IPP_H
#define JPEG_READER_IPP_H

#include "JpegReader.h"

class CJPEGDecoder;

class JpegReaderIPP : public JpegReader
{
    protected :
	CJPEGDecoder * decoder;

    public :
	JpegReaderIPP();

	virtual ~JpegReaderIPP();

	virtual bool load(char * filename);
	virtual bool load(const unsigned char * src, unsigned int srcsize);

	virtual void setOutputColorSpace(ColorSpace cspace);
};




#endif // JPEG_READER_H
