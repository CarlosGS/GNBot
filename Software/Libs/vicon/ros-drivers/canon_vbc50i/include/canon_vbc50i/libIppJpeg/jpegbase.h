/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//    Copyright (c) 2001-2004 Intel Corporation. All Rights Reserved.
//
//  Intel® Integrated Performance Primitives JPEG Viewer Sample for Windows*
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel® Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ipplic.htm located in the root directory of your Intel® IPP product
//  installation for more information.
//
//  JPEG is an international standard promoted by ISO/IEC and other organizations.
//  Implementations of these standards, or the standard enabled platforms may
//  require licenses from various entities, including Intel Corporation.
//
//
*/

#ifndef __JPEGBASE_H__
#define __JPEGBASE_H__

//#include <iostream.h>
#ifndef __IPPDEFS_H__
#include "ippdefs.h"
#endif
#ifndef __IPPCORE_H__
#include "ippcore.h"
#endif
#ifndef __IPPS_H__
#include "ipps.h"
#endif
#ifndef __IPPI_H__
#include "ippi.h"
#endif
#ifndef __IPPJ_H__
#include "ippj.h"
#endif


#ifdef _DEBUG
#define ENABLE_TRACING
#endif
#undef ENABLE_ERROR_LOGGING
//#define ENABLE_ERROR_LOGGING


#define OWN_ALIGN_PTR(ptr,align) \
  ippAlignPtr(ptr,align)


#define DIB_ALIGN (sizeof(int) - 1)

#define DIB_UWIDTH(width,nchannels) \
  ((width) * (nchannels))

#define DIB_AWIDTH(width,nchannels) \
  ( ((DIB_UWIDTH(width,nchannels) + DIB_ALIGN) & (~DIB_ALIGN)) )

#define DIB_PAD_BYTES(width,nchannels) \
  ( DIB_AWIDTH(width,nchannels) - DIB_UWIDTH(width,nchannels) )



#undef ENABLE_TRACING
#ifdef ENABLE_TRACING

#define TRC(msg) \
  cout << (msg)

#define TRC0(msg) \
  cout << (msg) << endl

#define TRC1(msg,p1) \
  cout << (msg) << (p1) << endl

#else

#define TRC(msg)
#define TRC0(msg)
#define TRC1(msg,p1)

#endif


#ifdef ENABLE_ERROR_LOGGING

#define LOG(msg) \
  cout << (msg)

#define LOG0(msg) \
  cout << (msg) << endl

#define LOG1(msg,p1) \
  cout << (msg) << (p1) << endl

#else

#define LOG(msg)
#define LOG0(msg)
#define LOG1(msg,p1)

#endif


typedef enum _JPEG_MODE
{
  JPEG_BASELINE    = 0,
  JPEG_PROGRESSIVE = 1,
  JPEG_LOSSLESS    = 2

} JMODE;


typedef enum _JPEG_OPERATION
{
  JO_READ_HEADER = 0,
  JO_READ_DATA   = 1

} JOPERATION;


typedef enum _JPEG_COLOR
{
  JC_UNKNOWN = 0,
  JC_GRAY    = 1,
  JC_RGB     = 2,
  JC_BGR     = 3,
  JC_YCBCR   = 4,
  JC_CMYK    = 5,
  JC_YCCK    = 6

} JCOLOR;


typedef enum _JPEG_SAMPLING
{
  JS_444   = 0,
  JS_422   = 1,
  JS_411   = 2,
  JS_OTHER = 3

} JSS;


typedef enum _JPEG_MARKER
{
  JM_NONE  = 0,
  JM_SOI   = 0xd8, /* start of image */
  JM_EOI   = 0xd9, /* end of image */

  /* start of frame */
  JM_SOF0  = 0xc0, /* Nondifferential Huffman-coding Baseline DCT */
  JM_SOF1  = 0xc1, /* Nondifferential Huffman-coding Extended sequental DCT */
  JM_SOF2  = 0xc2, /* Nondifferential Huffman-coding Progressive DCT */
  JM_SOF3  = 0xc3, /* Nondifferential Huffman-coding Lossless (sequental) */

  JM_SOF5  = 0xc5, /* Differential Huffman-coding Sequental DCT */
  JM_SOF6  = 0xc6, /* Differential Huffman-coding Progressive DCT */
  JM_SOF7  = 0xc7, /* Differential Lossless */

  JM_SOF9  = 0xc9, /* Nondifferential arithmetic-coding Extended sequental DCT */
  JM_SOFA  = 0xca, /* Nondifferential arithmetic-coding Progressive DCT */
  JM_SOFB  = 0xcb, /* Nondifferential arithmetic-coding Lossless (sequental) */

  JM_SOFD  = 0xcd, /* Differential arithmetic-coding Sequental DCT */
  JM_SOFE  = 0xce, /* Differential arithmetic-coding Progressive DCT */
  JM_SOFF  = 0xcf, /* Differential arithmetic-coding Lossless */

  JM_SOS   = 0xda, /* start of scan */
  JM_DQT   = 0xdb, /* define quantization table(s) */
  JM_DHT   = 0xc4, /* define Huffman table(s) */
  JM_APP0  = 0xe0, /* APP0 */
  JM_APP1  = 0xe1,
  JM_APP2  = 0xe2,
  JM_APP3  = 0xe3,
  JM_APP4  = 0xe4,
  JM_APP5  = 0xe5,
  JM_APP6  = 0xe6,
  JM_APP7  = 0xe7,
  JM_APP8  = 0xe8,
  JM_APP9  = 0xe9,
  JM_APP10 = 0xea,
  JM_APP11 = 0xeb,
  JM_APP12 = 0xec,
  JM_APP13 = 0xed,
  JM_APP14 = 0xee, /* APP14 */
  JM_APP15 = 0xef,
  JM_RST0  = 0xd0, /* restart with modulo 8 counter 0 */
  JM_RST1  = 0xd1, /* restart with modulo 8 counter 1 */
  JM_RST2  = 0xd2, /* restart with modulo 8 counter 2 */
  JM_RST3  = 0xd3, /* restart with modulo 8 counter 3 */
  JM_RST4  = 0xd4, /* restart with modulo 8 counter 4 */
  JM_RST5  = 0xd5, /* restart with modulo 8 counter 5 */
  JM_RST6  = 0xd6, /* restart with modulo 8 counter 6 */
  JM_RST7  = 0xd7, /* restart with modulo 8 counter 7 */
  JM_DRI   = 0xdd, /* define restart interval */
  JM_COM   = 0xfe  /* comment */

} JMARKER;


typedef enum _JPEG_ERROR
{
  JPEG_OK                 =   0,
  JPEG_NOT_IMPLEMENTED    =  -1,
  JPEG_INTERNAL_ERROR     =  -2,
  JPEG_BUFF_TOO_SMALL     =  -3,
  JPEG_OUT_OF_MEMORY      =  -4,
  JPEG_BAD_SEGMENT_LENGTH =  -5,
  JPEG_BAD_HUFF_TBL       =  -6,
  JPEG_BAD_QUANT_SEGMENT  =  -7,
  JPEG_BAD_SCAN_SEGMENT   =  -8,
  JPEG_BAD_FRAME_SEGMENT  =  -9,
  JPEG_BAD_COMPONENT_ID   = -10,
  JPEG_BAD_SAMPLING       = -11,
  JPEG_BAD_RESTART        = -12

} JERRCODE;


typedef struct _image
{
  union
  {
    Ipp8u*  Data8u;
    Ipp16s* Data16s;
  } p;

  int    width;
  int    height;
  int    lineStep;
  int    precision;
  int    nChannels;
  JCOLOR color;
} IMAGE;


typedef struct _bitstream
{
  Ipp8u* pData;
  int    DataLen;
  int    currPos;

  JERRCODE _WRITE_BYTE(int byte);
  JERRCODE _WRITE_WORD(int word);
  JERRCODE _READ_BYTE(int* byte);
  JERRCODE _READ_WORD(int* word);
} BITSTREAM;


const int CPU_CACHE_LINE      = 32;
const int DCTSIZE2            = 64;
const int MAX_QUANT_TABLES    = 4;
const int MAX_HUFF_TABLES     = 4;
const int MAX_COMPS_PER_SCAN  = 4;
const int MAX_COMPS_PER_FRAME = 255;
const int MAX_HUFF_BITS       = 16;
const int MAX_HUFF_VALS       = 256;
const int MAX_BLOCKS_PER_MCU  = 10;
const int MAX_BYTES_PER_MCU   = DCTSIZE2*sizeof(Ipp16s)*MAX_BLOCKS_PER_MCU;


extern const Ipp8u DefaultLuminanceQuant[64];
extern const Ipp8u DefaultChrominanceQuant[64];

extern const Ipp8u DefaultLuminanceDCBits[];
extern const Ipp8u DefaultLuminanceDCValues[];
extern const Ipp8u DefaultChrominanceDCBits[];
extern const Ipp8u DefaultChrominanceDCValues[];
extern const Ipp8u DefaultLuminanceACBits[];
extern const Ipp8u DefaultLuminanceACValues[];
extern const Ipp8u DefaultChrominanceACBits[];
extern const Ipp8u DefaultChrominanceACValues[];

const char* GetErrorStr(JERRCODE code);


#endif // __JPEGBASE_H__


