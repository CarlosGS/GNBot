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

#ifndef __COLORCOMP_H__
#define __COLORCOMP_H__

#ifndef __IPPJ_H__
#include "ippj.h"
#endif
#ifndef __JPEGBASE_H__
#include "jpegbase.h"
#endif




class CJPEGColorComponent
{
public:
  int m_id;
  int m_comp_no;
  int m_hsampling;
  int m_vsampling;
  int m_h_factor;
  int m_v_factor;
  int m_nblocks;
  int m_q_selector;
  int m_dc_selector;
  int m_ac_selector;
  int m_ac_scan_completed;
  Ipp16s m_lastDC;

  Ipp8u* m_cc_buffer;
  Ipp8u* m_ss_buffer;
  Ipp8u* m_top_row;
  Ipp8u* m_bottom_row;

  Ipp16s* m_curr_row;
  Ipp16s* m_prev_row;

  CJPEGColorComponent(void);
  virtual ~CJPEGColorComponent(void);

};


#endif // __COLORCOMP_H__

