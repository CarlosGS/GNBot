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

#ifndef __JPEGBASE_H__
#include "jpegbase.h"
#endif
#ifndef __ENCHTBL_H__
#include "enchtbl.h"
#endif




CJPEGEncoderHuffmanTable::CJPEGEncoderHuffmanTable(void)
{
  m_id     = 0;
  m_hclass = 0;
  m_table  = 0;

  ippsZero_8u(m_bits,sizeof(m_bits));
  ippsZero_8u(m_vals,sizeof(m_vals));

  return;
} // ctor


CJPEGEncoderHuffmanTable::~CJPEGEncoderHuffmanTable(void)
{
  Destroy();
  return;
} // dtor


JERRCODE CJPEGEncoderHuffmanTable::Create(void)
{
  int       size;
  IppStatus status;

  status = ippiEncodeHuffmanSpecGetBufSize_JPEG_8u(&size);
  if(ippStsNoErr != status)
  {
    LOG1("IPP Error: ippiEncodeHuffmanSpecGetBufSize_JPEG_8u() failed - ",status);
    return JPEG_INTERNAL_ERROR;
  }

  m_table = (IppiEncodeHuffmanSpec*)ippMalloc(size);
  if(0 == m_table)
  {
    LOG0("IPP Error: ippMalloc() failed");
    return JPEG_OUT_OF_MEMORY;
  }

  return JPEG_OK;
} // CJPEGEncoderHuffmanTable::Create()


JERRCODE CJPEGEncoderHuffmanTable::Destroy(void)
{
  m_id     = 0;
  m_hclass = 0;
  m_table  = 0;

  ippsZero_8u(m_bits,sizeof(m_bits));
  ippsZero_8u(m_vals,sizeof(m_vals));

  if(0 != m_table)
  {
    ippFree(m_table);
	m_table = 0;
  }

  return JPEG_OK;
} // CJPEGEncoderHuffmanTable::Destroy()


JERRCODE CJPEGEncoderHuffmanTable::Init(int id,int hclass,Ipp8u* bits,Ipp8u* vals)
{
  IppStatus status;

  m_id     = id     & 0x0f;
  m_hclass = hclass & 0x0f;

  ippsCopy_8u(bits,m_bits,16);
  ippsCopy_8u(vals,m_vals,256);

  status = ippiEncodeHuffmanSpecInit_JPEG_8u(m_bits,m_vals,m_table);
  if(ippStsNoErr != status)
  {
    LOG1("IPP Error: ippiEncodeHuffmanSpecInit_JPEG_8u() failes - ", status);
    return JPEG_INTERNAL_ERROR;
  }

  return JPEG_OK;
} // CJPEGEncoderHuffmanTable::Init()




CJPEGEncoderHuffmanState::CJPEGEncoderHuffmanState(void)
{
  m_state = 0;
  return;
} // ctor


CJPEGEncoderHuffmanState::~CJPEGEncoderHuffmanState(void)
{
  Destroy();
  return;
} // dtor


JERRCODE CJPEGEncoderHuffmanState::Create(void)
{
  int       size;
  IppStatus status;

  status = ippiEncodeHuffmanStateGetBufSize_JPEG_8u(&size);
  if(ippStsNoErr != status)
  {
    LOG1("IPP Error: ippiEncodeHuffmanStateGetBufSize_JPEG_8u() failed - ",status);
    return JPEG_INTERNAL_ERROR;
  }

  m_state = (IppiEncodeHuffmanState*)ippMalloc(size);
  if(0 == m_state)
  {
    LOG0("IPP Error: ippMalloc() failed");
    return JPEG_OUT_OF_MEMORY;
  }

  return JPEG_OK;
} // CJPEGEncoderHuffmanState::Create()


JERRCODE CJPEGEncoderHuffmanState::Destroy(void)
{
  if(0 != m_state)
  {
    ippFree(m_state);
    m_state = 0;
  }

  return JPEG_OK;
} // CJPEGEncoderHuffmanState::Destroy()


JERRCODE CJPEGEncoderHuffmanState::Init(void)
{
  IppStatus status;

  status = ippiEncodeHuffmanStateInit_JPEG_8u(m_state);
  if(ippStsNoErr != status)
  {
    LOG1("IPP Error: ippiEncodeHuffmanStateInit_JPEG_8u() failed - ",status);
    return JPEG_INTERNAL_ERROR;
  }

  return JPEG_OK;
} // CJPEGEncoderHuffmanState::Init()

