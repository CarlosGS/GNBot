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

#ifndef __DECODER_H__
#define __DECODER_H__

#ifndef __IPPS_H__
#include "ipps.h"
#endif
#ifndef __IPPI_H__
#include "ippi.h"
#endif
#ifndef __IPPJ_H__
#include "ippj.h"
#endif
#ifndef __JPEGBASE_H__
#include "jpegbase.h"
#endif
#ifndef __DECQTBL_H__
#include "decqtbl.h"
#endif
#ifndef __DECHTBL_H__
#include "dechtbl.h"
#endif
#ifndef __COLORCOMP_H__
#include "colorcomp.h"
#endif




class CJPEGDecoder
{
public:

  CJPEGDecoder(void);
  virtual ~CJPEGDecoder(void);

  void Reset(void);

  JERRCODE SetSource(
    Ipp8u*   pSrc,
    int      srcSize);

  JERRCODE SetDestination(
    Ipp8u*   pDst,
    int      dstStep,
    IppiSize dstSize,
    int      dstChannels,
    JCOLOR   dstColor,
    int      dstPrecision = 8);

  JERRCODE SetDestination(
    Ipp16s*  pDst,
    int      dstStep,
    IppiSize dstSize,
    int      dstChannels,
    JCOLOR   dstColor,
    int      dstPrecision = 16);

  JERRCODE ReadHeader(
    int*     width,
    int*     height,
    int*     nchannels,
    JCOLOR*  color,
    JSS*     sampling,
    int*     precision);

  JERRCODE ReadData(void);

  BITSTREAM m_src;
  IMAGE     m_dst;
  int       m_jpeg_width;
  int       m_jpeg_height;
  int       m_jpeg_ncomp;
  int       m_jpeg_precision;
  JSS       m_jpeg_sampling;
  JCOLOR    m_jpeg_color;
  int       m_jpeg_quality;
  int       m_jpeg_restart_interval;
  JMODE     m_jpeg_mode; 

  // JPEG embedded comments variables
  int      m_jpeg_comment_detected;
  int      m_jpeg_comment_size;
  Ipp8u*   m_jpeg_comment;

  // JFIF APP0 related varibales
  int      m_jfif_app0_detected;
  int      m_jfif_app0_major;
  int      m_jfif_app0_minor;
  int      m_jfif_app0_units;
  int      m_jfif_app0_xDensity;
  int      m_jfif_app0_yDensity;
  int      m_jfif_app0_thumb_width;
  int      m_jfif_app0_thumb_height;

  // JFXX APP0 related variables
  int      m_jfxx_app0_detected;
  int      m_jfxx_thumbnails_type;

  // Adobe APP14 related variables
  int      m_adobe_app14_detected;
  int      m_adobe_app14_version;
  int      m_adobe_app14_flags0;
  int      m_adobe_app14_flags1;
  int      m_adobe_app14_transform;

  int      m_precision;
  int      m_numxMCU;
  int      m_numyMCU;
  int      m_mcuWidth;
  int      m_mcuHeight;
  int      m_ccWidth;
  int      m_ccHeight;
  int      m_xPadding;
  int      m_yPadding;
  int      m_restarts_to_go;
  int      m_next_restart_num;
  int      m_sos_len;
  int      m_curr_comp_no;
  int      m_ss;
  int      m_se;
  int      m_al;
  int      m_ah;
  int      m_dc_scan_completed;
  int      m_ac_scans_completed;
  JMARKER  m_marker;
  Ipp16s*  m_coefbuf;

  int      m_scan_count;

  Ipp16s*  m_block_buffer;//OMP
  int      m_num_threads; //OMP
  int      m_nblock;      //OMP

#ifdef __TIMING__
  Ipp64u   m_clk_diff;
  Ipp64u   m_clk_huff;
#endif

  CJPEGColorComponent*        m_ccomp[MAX_COMPS_PER_SCAN];
  CJPEGDecoderQuantTable      m_qntbl[MAX_QUANT_TABLES];
  CJPEGDecoderHuffmanTable    m_dctbl[MAX_HUFF_TABLES];
  CJPEGDecoderHuffmanTable    m_actbl[MAX_HUFF_TABLES];
  CJPEGDecoderHuffmanState    m_state;

  JERRCODE Init(void);
  JERRCODE Clean(void);
  JERRCODE ColorConvert(void);
  JERRCODE ColorConvert(int nMCURow,int idThread);
  JERRCODE UpSampling(void);
  JERRCODE UpSampling(int nMCURow,int idThread);
  JERRCODE PerformDCT(void);

  JERRCODE ParseJPEGBitStream(JOPERATION op);
  JERRCODE ParseSOI(void);
  JERRCODE ParseEOI(void);
  JERRCODE ParseAPP0(void);
  JERRCODE ParseAPP14(void);
  JERRCODE ParseSOF0(void);
  JERRCODE ParseSOF2(void);
  JERRCODE ParseSOF3(void);
  JERRCODE ParseDRI(void);
  JERRCODE ParseRST(void);
  JERRCODE ParseSOS(void);
  JERRCODE ParseDQT(void);
  JERRCODE ParseDHT(void);
  JERRCODE ParseCOM(void);
  JERRCODE DecodeScanBaseline(void);
  JERRCODE DecodeScanLossless(void);
  JERRCODE DecodeScanProgressive(void);

  JERRCODE ProcessRestart(void);

  JERRCODE NextMarker(JMARKER* marker);
  JERRCODE SkipMarker(void);

  // huffman decode mcu row lossless process
  JERRCODE DecodeHuffmanMCURowLS(Ipp16s* pMCUBuf);
  // huffman decode mcu row baseline process
  JERRCODE DecodeHuffmanMCURowBL(Ipp16s* pMCUBuf);
  // DCT/QNT/SS/CC  mcu row baseline process
  JERRCODE DCT_QNT_SSCC_MCURowBL(Ipp16s* pMCUBuf,int idThread,int mcu_row);
  // reconstruct mcu row lossless process
  JERRCODE ReconstructMCURowLS(Ipp16s* pMCUBuf,int idThread,int mcu_row);

  JERRCODE _set_sampling(void);
};

#endif // __DECODER_H__

