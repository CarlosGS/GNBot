#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../libIppJpeg/decoder.h"

#include "JpegReader-ipp.h"

JpegReaderIPP::JpegReaderIPP() : JpegReader()
{
	decoder = new CJPEGDecoder;
}

JpegReaderIPP::~JpegReaderIPP()
{
	//printf("~JpegReaderIPP\n");
	delete decoder;
	decoder = NULL;
}


bool JpegReaderIPP::load(char * filename)
{
	struct stat s;
	if (!stat(filename,&s)) {
		char tmp[1024];
		sprintf(tmp,"Could not load '%s'",filename);
		perror(tmp);
		return false;
	}

	FILE * infile;
	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return false;
	}

	unsigned char * fbuffer;
	unsigned int size;

	size = s.st_size;
	fbuffer = (unsigned char*)malloc(size);
	assert(fbuffer != NULL);
	fread(fbuffer,size,1,infile);

	bool res = this->load(fbuffer,size);
	free(fbuffer);

	/* And we're done! */
	return res;
}


bool JpegReaderIPP::load(const unsigned char * src, unsigned int srcsize)
{
	JERRCODE     jerr;
	JCOLOR       jpeg_color;
	JSS          jpeg_sampling;
	int          jpeg_nChannels;
	int          jpeg_precision;
	JCOLOR       outcolor;
	int	         outchan;
	unsigned int step, size;

	//printf("SRC  : ");for (unsigned int i=0;i<20;i++) printf("%02X ",src[i]); printf("\n");
	jerr = decoder->SetSource((Ipp8u*)src,srcsize);
	if(JPEG_OK != jerr) {
		fprintf(stderr,"decoder.SetSource() failed\n");
		return false;
	}

	jerr = decoder->ReadHeader((int*)&width,(int*)&height,
			&jpeg_nChannels, &jpeg_color, &jpeg_sampling, &jpeg_precision);	
	if(JPEG_OK != jerr) {
		fprintf(stderr,"decoder.ReadHeader() failed\n");
		return false;
	}
	//printf("Header: %d chan cs %d\n",jpeg_nChannels,jpeg_color);

	switch (reqColorSpace) {
		case cmAuto :
			//printf("Req Auto: ");
			switch (jpeg_color) {
				case JC_GRAY:
					//printf("Out Gray\n");
					outputColorSpace = cmGray;
					step = width * sizeof(Ipp8u);
					size = height * step;
					outcolor = JC_GRAY;
					outchan  = 1;
					break;
				case JC_YCBCR:
					//printf("Out YUV\n");
					outputColorSpace = cmYUV;
					step = width*sizeof(Ipp8u)*3;
					size = height * step;
					outcolor = JC_YCBCR;
					outchan  = 3;
					break;
				default :
					//printf("Out RGB\n");
					outputColorSpace = cmRGB;
					step = width*sizeof(Ipp8u)*3;
					size = height * step;
					outcolor = JC_RGB;
					outchan  = 3;
					break;
			}
			break;
		case cmRGB:
			//printf("Out RGB\n");
			outputColorSpace = cmRGB;
			step = width*sizeof(Ipp8u)*3;
			size = height * step;
			outcolor = JC_RGB;
			break;
		case cmYUV:
			//printf("Out YUV\n");
			outputColorSpace = cmYUV;
			step = width*sizeof(Ipp8u)*3;
			size = height * step;
			outcolor = JC_YCBCR;
			outchan  = 3;
			break;
		case cmGray:
		default :
			//printf("Out Gray\n");
			outputColorSpace = cmGray;
			step = width * sizeof(Ipp8u);
			size = height * step;
			outcolor = JC_GRAY;
			outchan  = 1;
			break;

	}

	if (!externaloutput) {
		buffer = (unsigned char*)(realloc(buffer,size));
		assert(buffer != NULL);
	}

	IppiSize dim = {width,height};
	jerr = decoder->SetDestination((Ipp8u*)buffer,
			step, dim, outchan, outcolor);
	if(JPEG_OK != jerr) {
		fprintf(stderr,"decoder.SetDestination() failed\n");
		return false;
	}

	jerr = decoder->ReadData();
	if(JPEG_OK != jerr) {
		fprintf(stderr,"decoder.ReadData() failed\n");
		return false;
	}
	//printf("OUT  : ");for (unsigned int i=0;i<20;i++) printf("%02X ",buffer[i]); printf("\n");

	return true;
}

void JpegReaderIPP::setOutputColorSpace(ColorSpace cspace)
{
	reqColorSpace = cspace;
}

