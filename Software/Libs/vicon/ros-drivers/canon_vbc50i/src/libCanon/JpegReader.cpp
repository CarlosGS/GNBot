#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
extern "C" {
#include <jpeglib.h>
}
#include <string.h>
#include <setjmp.h>

#include "canon_vbc50i/libCanon/JpegReader.h"

JpegReader::JpegReader()
{
	width = height = 0;
	outputColorSpace = cmAuto;
	reqColorSpace = cmAuto;
	externaloutput = false;
	buffer = NULL;
	timestamp = 0;
}

JpegReader::~JpegReader()
{
	//printf("~JpegReader\n");
	free(buffer);
	buffer = NULL;
	
}

void JpegReader::setTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	timestamp = tv.tv_sec + 1e-6*tv.tv_usec;
}

void JpegReader::setTimeStamp(double ts)
{
	timestamp = ts;
}

struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */

	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

void my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}


bool JpegReader::load(char * filename)
{
	//printf("Reading\n");
	/* This struct contains the JPEG decompression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 */
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	 * Note that this struct must live as long as the main JPEG parameter
	 * struct, to avoid dangling-pointer problems.
	 */
	struct my_error_mgr jerr;
	/* More stuff */
	FILE * infile;		/* source file */
	int row_stride;		/* physical row width in output buffer */
	JSAMPARRAY lbuffer;		/* Output row buffer */

	/* In this example we want to open the input file before doing anything else,
	 * so that the setjmp() error recovery below can assume the file is open.
	 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	 * requires it in order to read binary files.
	 */

	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return false;
	}
	//printf("File %s opened\n",filename);

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return 0;
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	int res = jpeg_read_header(&cinfo, TRUE);
	//printf("Header : %d\n",res);
	/* We can ignore the return value from jpeg_read_header since
	 *   (a) suspension is not possible with the stdio data source, and
	 *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	 * See libjpeg.doc for more info.
	 */


	width = cinfo.image_width;
	height = cinfo.image_height;
	size = 0;
	switch (reqColorSpace) {
		case cmAuto :
			switch (cinfo.jpeg_color_space) {
				case JCS_GRAYSCALE:
					outputColorSpace = cmGray;
					cinfo.out_color_space = JCS_GRAYSCALE;
					size = width*height*sizeof(char);
					break;
				default :
					outputColorSpace = cmRGB;
					cinfo.out_color_space = JCS_RGB;
					size = width*height*sizeof(char)*3;
					break;
			}
			break;
		case cmRGB:
			outputColorSpace = cmRGB;
			cinfo.out_color_space = JCS_RGB;
			size = width*height*sizeof(char)*3;
			break;
		case cmYUV:
			outputColorSpace = cmYUV;
			cinfo.out_color_space = JCS_YCbCr;
			size = width*height*sizeof(char)*3;
			break;
		case cmGray:
		default :
			outputColorSpace = cmGray;
			cinfo.out_color_space = JCS_GRAYSCALE;
			size = width*height*sizeof(char);
			break;
			
	}
		
	if (!externaloutput) {
		buffer = (unsigned char*)(realloc(buffer,size));
		assert(buffer != NULL);
	}

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	 * jpeg_read_header(), so we do nothing here.
	 */

	/* Step 5: Start decompressor */

	(void) jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */

	/* We may need to do some setup of our own at this point before reading
	 * the data.  After jpeg_start_decompress() we have the correct scaled
	 * output image dimensions available, as well as the output colormap
	 * if we asked for color quantization.
	 * In this example, we need to make an output work buffer of the right size.
	 */ 
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;
	//printf("Output components: %d stride %d\n",cinfo.output_components,
	//		row_stride);
	/* Make a one-row-high sample array that will go away when done with image */
	lbuffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	/* Here we use the library's state variable cinfo.output_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 */
	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient.
		 */
		res = jpeg_read_scanlines(&cinfo, lbuffer, 1);
		//printf("scanline : %d %d\n",res,cinfo.output_scanline);
		//assert((cinfo.output_scanline+1)*cinfo.output_width <= width*height);
		memcpy(buffer + (cinfo.output_scanline-1)*row_stride, 
				lbuffer[0], row_stride);
	}

	/* Step 7: Finish decompression */

	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* After finish_decompress, we can close the input file.
	 * Here we postpone it until after no more JPEG errors are possible,
	 * so as to simplify the setjmp error logic above.  (Actually, I don't
	 * think that jpeg_destroy can do an error exit, but why assume anything...)
	 */
	fclose(infile);

	/* At this point you may want to check to see whether any corrupt-data
	 * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	 */

	/* And we're done! */
	return true;
}


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

static void init_source (j_decompress_ptr cinfo)
{
}



static boolean fill_input_buffer (j_decompress_ptr cinfo)
{
  return FALSE;
}


static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  cinfo->src->next_input_byte += (size_t) num_bytes;
  cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
}


static void term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

static void jpeg_mem_src (j_decompress_ptr cinfo, 
		const unsigned char * src, unsigned int srcsize)
{
  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  sizeof(jpeg_source_mgr));
	cinfo->src->next_input_byte = src;
	cinfo->src->bytes_in_buffer = srcsize;
  }

  cinfo->src->init_source = init_source;
  cinfo->src->fill_input_buffer = fill_input_buffer;
  cinfo->src->skip_input_data = skip_input_data;
  cinfo->src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
  cinfo->src->term_source = term_source;
}



bool JpegReader::load(const unsigned char * src, unsigned int srcsize)
{
	//printf("Reading\n");
	/* This struct contains the JPEG decompression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 */
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	 * Note that this struct must live as long as the main JPEG parameter
	 * struct, to avoid dangling-pointer problems.
	 */
	struct my_error_mgr jerr;
	/* More stuff */
	int row_stride;		/* physical row width in output buffer */
	JSAMPARRAY lbuffer;		/* Output row buffer */

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_mem_src(&cinfo, src, srcsize);

	/* Step 3: read file parameters with jpeg_read_header() */

	int res = jpeg_read_header(&cinfo, TRUE);
	//printf("Header : %d\n",res);
	/* We can ignore the return value from jpeg_read_header since
	 *   (a) suspension is not possible with the stdio data source, and
	 *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	 * See libjpeg.doc for more info.
	 */


	width = cinfo.image_width;
	height = cinfo.image_height;
	size = 0;
	switch (reqColorSpace) {
		case cmAuto :
			switch (cinfo.jpeg_color_space) {
				case JCS_GRAYSCALE:
					outputColorSpace = cmGray;
					cinfo.out_color_space = JCS_GRAYSCALE;
					size = width*height*sizeof(char);
					break;
				case JCS_YCbCr:
					outputColorSpace = cmYUV;
					cinfo.out_color_space = JCS_YCbCr;
					size = width*height*sizeof(char)*3;
					break;
				default :
					outputColorSpace = cmRGB;
					cinfo.out_color_space = JCS_RGB;
					size = width*height*sizeof(char)*3;
					break;
			}
			break;
		case cmRGB:
			outputColorSpace = cmRGB;
			cinfo.out_color_space = JCS_RGB;
			size = width*height*sizeof(char)*3;
			break;
		case cmYUV:
			outputColorSpace = cmYUV;
			cinfo.out_color_space = JCS_YCbCr;
			size = width*height*sizeof(char)*3;
			break;
		case cmGray:
		default :
			outputColorSpace = cmGray;
			cinfo.out_color_space = JCS_GRAYSCALE;
			size = width*height*sizeof(char);
			break;
			
	}
	if (!externaloutput) {
		buffer = (unsigned char*)(realloc(buffer,size));
		assert(buffer != NULL);
	}
#if 0
	printf("Data stream: %dx%d, %s : %d bytes\n",width,height,
			rgb?"RGB":"GRAY", size);
#endif

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	 * jpeg_read_header(), so we do nothing here.
	 */

	/* Step 5: Start decompressor */

	(void) jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */

	/* We may need to do some setup of our own at this point before reading
	 * the data.  After jpeg_start_decompress() we have the correct scaled
	 * output image dimensions available, as well as the output colormap
	 * if we asked for color quantization.
	 * In this example, we need to make an output work buffer of the right size.
	 */ 
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	lbuffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	/* Here we use the library's state variable cinfo.output_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 */
	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient.
		 */
		res = jpeg_read_scanlines(&cinfo, lbuffer, 1);
		//printf("scanline : %d %d\n",res,cinfo.output_scanline);
		//assert((cinfo.output_scanline+1)*cinfo.output_width <= width*height);
		memcpy(buffer + (cinfo.output_scanline-1)*row_stride, 
				lbuffer[0], row_stride);
	}

	/* Step 7: Finish decompression */

	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* At this point you may want to check to see whether any corrupt-data
	 * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	 */

	/* And we're done! */
	return true;
}

void JpegReader::setOutputColorSpace(ColorSpace cspace)
{
	reqColorSpace = cspace;
}

