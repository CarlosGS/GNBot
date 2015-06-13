/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//     Copyright (c) 2001-2004 Intel Corporation. All Rights Reserved.
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

#ifndef __ENCHTBL_H__
#define __ENCHTBL_H__

#ifndef __IPPJ_H__
#include "ippj.h"
#endif
#ifndef __JPEGBASE_H__
#include "jpegbase.h"
#endif




class CJPEGEncoderHuffmanTable
{
private:
  IppiEncodeHuffmanSpec* m_table;

public:
  int                    m_id;
  int                    m_hclass;
  Ipp8u                  m_bits[16];
  Ipp8u                  m_vals[256];

  CJPEGEncoderHuffmanTable(void);
  virtual ~CJPEGEncoderHuffmanTable(void);

  JERRCODE Create(void);
  JERRCODE Destroy(void);
  JERRCODE Init(int id,int hclass,Ipp8u* bits,Ipp8u* vals);

  operator IppiEncodeHuffmanSpec*(void) { return m_table; }
};


class CJPEGEncoderHuffmanState
{
private:
  IppiEncodeHuffmanState* m_state;

public:
  CJPEGEncoderHuffmanState(void);
  virtual ~CJPEGEncoderHuffmanState(void);

  JERRCODE Create(void);
  JERRCODE Destroy(void);
  JERRCODE Init(void);

  operator IppiEncodeHuffmanState*(void) { return m_state; }
};


#endif // __ENCHTBL_H__


