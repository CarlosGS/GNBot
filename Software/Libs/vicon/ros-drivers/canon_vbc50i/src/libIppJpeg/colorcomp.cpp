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

#ifndef __COLORCOMP_H__
#include "colorcomp.h"
#endif


CJPEGColorComponent::CJPEGColorComponent(void)
{
  m_id          = 0;
  m_comp_no     = 0;
  m_hsampling   = 0;
  m_vsampling   = 0;
  m_h_factor    = 0;
  m_v_factor    = 0;
  m_nblocks     = 0;
  m_q_selector  = 0;
  m_dc_selector = 0;
  m_ac_selector = 0;
  m_cc_buffer   = 0;
  m_ss_buffer   = 0;
  m_top_row     = 0;
  m_bottom_row  = 0;
  m_lastDC      = 0;
  m_ac_scan_completed = 0;
  return;
} // ctor


CJPEGColorComponent::~CJPEGColorComponent(void)
{
  if(0 != m_cc_buffer)
  {
    ippFree(m_cc_buffer);
    m_cc_buffer = 0;
  }

  if(0 != m_ss_buffer)
  {
    ippFree(m_ss_buffer);
    m_ss_buffer = 0;
  }

  if(0 != m_top_row)
  {
    ippFree(m_top_row);
    m_top_row = 0;
  }

  if(0 != m_bottom_row)
  {
    ippFree(m_bottom_row);
    m_bottom_row = 0;
  }

  return;
} // dtor;

