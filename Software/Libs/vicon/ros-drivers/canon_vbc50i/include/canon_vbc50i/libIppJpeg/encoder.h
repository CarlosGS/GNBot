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

#ifndef __ENCODER_H__
#define __ENCODER_H__

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
#ifndef __ENCQTBL_H__
#include "encqtbl.h"
#endif
#ifndef __ENCHTBL_H__
#include "enchtbl.h"
#endif
#ifndef __COLORCOMP_H__
#include "colorcomp.h"
#endif




typedef struct _JPEG_SCAN
{
  int ncomp;
  int id[MAX_COMPS_PER_SCAN];
  int Ss;
  int Se;
  int Ah;
  int Al;
} JPEG_SCAN;


class CJPEGEncoder
{
public:

  CJPEGEncoder(void);
  virtual ~CJPEGEncoder(void);

  JERRCODE SetSource(
    Ipp8u*   pSrc,
    int      srcStep,
    IppiSize srcSize,
    int      srcChannels,
    JCOLOR   srcColor);

  JERRCODE SetDestination(
    Ipp8u*   pDst,
    int      dstSize,
    int      dstQuality,
    JSS      dstSampling,
    JCOLOR   dstColor,
    JMODE    dstMode = JPEG_BASELINE,
    int      dstRestartInt = 0);

//  JERRCODE WriteHeader();

//  JERRCODE WriteData();

  JERRCODE WriteImageBaseline();

  JERRCODE WriteImageProgressive();

  JERRCODE WriteImageLossless();

  IMAGE      m_src;
  BITSTREAM  m_dst;
  
  int        m_jpeg_ncomp;
  int        m_jpeg_precision;
  JSS        m_jpeg_sampling;
  JCOLOR     m_jpeg_color;
  int        m_jpeg_quality;
  int        m_jpeg_restart_interval;
  JMODE      m_jpeg_mode;
  
  int        m_numxMCU;
  int        m_numyMCU;
  int        m_mcuWidth;
  int        m_mcuHeight;
  int        m_ccWidth;
  int        m_ccHeight;
  int        m_xPadding;
  int        m_yPadding;
  int        m_restarts_to_go;
  int        m_next_restart_num;
  int        m_scan_count;
  int        m_ss;
  int        m_se;
  int        m_al;
  int        m_ah;
  int        m_predictor;
  int        m_pt;
  JPEG_SCAN* m_scan_script;
  Ipp16s*    m_coefbuf;

  CJPEGColorComponent*       m_ccomp[MAX_COMPS_PER_SCAN];
  CJPEGEncoderQuantTable     m_qntbl[MAX_QUANT_TABLES];
  CJPEGEncoderHuffmanTable   m_dctbl[MAX_HUFF_TABLES];
  CJPEGEncoderHuffmanTable   m_actbl[MAX_HUFF_TABLES];
  CJPEGEncoderHuffmanState   m_state;

  JERRCODE Init(void);
  JERRCODE Clean(void);
  JERRCODE ColorConvert(void);
  JERRCODE ColorConvert(int nMCURow);
  JERRCODE DownSampling(void);
  JERRCODE DownSampling(int nMCURow);
  JERRCODE PerformDCT(void);
  JERRCODE EncodeScan(int ncomp,int id[MAX_COMPS_PER_SCAN],int Ss,int Se,int Ah,int Al);
  JERRCODE SelectScanScripts(void);
  JERRCODE GenerateHuffmanTables(int ncomp,int id[MAX_COMPS_PER_SCAN],int Ss,int Se,int Ah,int Al);

  JERRCODE WriteSOI(void);
  JERRCODE WriteEOI(void);
  JERRCODE WriteAPP0(void);
  JERRCODE WriteAPP14(void);
  JERRCODE WriteSOF0(void);
  JERRCODE WriteSOF1(void);
  JERRCODE WriteSOF2(void);
  JERRCODE WriteSOF3(void);
  JERRCODE WriteDRI(int restart_interval);
  JERRCODE WriteRST(int next_restart_num);
  JERRCODE WriteSOS(void);
  JERRCODE WriteSOS(int ncomp,int id[MAX_COMPS_PER_SCAN],int Ss,int Se,int Ah,int Al);
  JERRCODE WriteDQT(CJPEGEncoderQuantTable* tbl);
  JERRCODE WriteDHT(CJPEGEncoderHuffmanTable* tbl);
  JERRCODE WriteCOM(char* comment = 0);

  JERRCODE ProcessRestart(int id[MAX_COMPS_PER_SCAN],int Ss,int Se,int Ah,int Al);
  JERRCODE ProcessRestart(int stat[2][256],int id[MAX_COMPS_PER_SCAN],int Ss,int Se,int Ah,int Al);

};


#endif // __ENCODER_H__

