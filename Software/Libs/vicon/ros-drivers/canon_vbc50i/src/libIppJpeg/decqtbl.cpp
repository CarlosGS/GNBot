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

#include "precomp.h"

#ifndef __DECQTBL_H__
#include "decqtbl.h"
#endif




CJPEGDecoderQuantTable::CJPEGDecoderQuantTable(void)
{
  m_id          = 0;
  m_precision   = 0;
  m_initialized = 0;

  m_raw = (Ipp8u*)OWN_ALIGN_PTR(m_rbf,CPU_CACHE_LINE);
  m_qnt = (Ipp16u*)OWN_ALIGN_PTR(m_qbf,CPU_CACHE_LINE); // align for max performance

  ippsZero_8u(m_raw,sizeof(m_raw));
  ippsZero_8u((Ipp8u*)m_qnt,sizeof(m_qnt));

  return;
} // ctor


CJPEGDecoderQuantTable::~CJPEGDecoderQuantTable(void)
{
  m_initialized = 0;

  ippsZero_8u(m_raw,sizeof(m_raw));
  ippsZero_8u((Ipp8u*)m_qnt,sizeof(m_qnt));

  return;
} // dtor


JERRCODE CJPEGDecoderQuantTable::Init(int id,Ipp8u raw[64])
{
  IppStatus status;

  m_id        = id & 0x0f;
  m_precision = 0; // 8-bit hardcoded

  ippsCopy_8u(raw,m_raw,DCTSIZE2);

  status = ippiQuantInvTableInit_JPEG_8u16u(m_raw,m_qnt);
  if(ippStsNoErr != status)
  {
    LOG1("IPP Error: ippiQuantInvTableInit_JPEG_8u16u() failed - ",status);
    return JPEG_INTERNAL_ERROR;
  }

  m_initialized = 1;

  return JPEG_OK;
} // CJPEGDecoderQuantTable::Init()

