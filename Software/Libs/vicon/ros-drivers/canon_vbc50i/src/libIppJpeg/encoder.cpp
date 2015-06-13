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

#include <stdio.h>
#include <string.h>

#ifndef __JPEGBASE_H__
#include "jpegbase.h"
#endif
#ifndef __ENCODER_H__
#include "encoder.h"
#endif




CJPEGEncoder::CJPEGEncoder(void)
{
  m_src.p.Data8u   = 0;
  m_src.width      = 0;
  m_src.height     = 0;
  m_src.nChannels  = 0;
  m_src.lineStep   = 0;

  m_dst.pData      = 0;
  m_dst.DataLen    = 0;
  m_dst.currPos    = 0;

  m_jpeg_ncomp            = 0;
  m_jpeg_precision        = 8;
  m_jpeg_sampling         = JS_444;
  m_jpeg_color            = JC_UNKNOWN;
  m_jpeg_quality          = 100;
  m_jpeg_restart_interval = 0;
  m_jpeg_mode             = JPEG_BASELINE;

  m_numxMCU   = 0;
  m_numyMCU   = 0;
  m_mcuWidth  = 0;
  m_mcuHeight = 0;

  m_ccWidth  = 0;
  m_ccHeight = 0;
  m_xPadding = 0;
  m_yPadding = 0;

  m_restarts_to_go   = 0;
  m_next_restart_num = 0;

  m_scan_count  = 0;
  m_scan_script = 0;
  m_coefbuf     = 0;

  m_ss = 0;
  m_se = 63;
  m_ah = 0;
  m_al = 0;

  m_predictor = 1;
  m_pt = 0;

  m_ccomp[0] = 0;
  m_ccomp[1] = 0;
  m_ccomp[2] = 0;
  m_ccomp[3] = 0;

  return;
} // ctor


CJPEGEncoder::~CJPEGEncoder(void)
{
  Clean();
  return;
} // dtor


JERRCODE CJPEGEncoder::Clean(void)
{
  for(int i = 0; i < m_jpeg_ncomp; i++)
  {
    delete m_ccomp[i];
    m_ccomp[i] = 0;
  }

  if(0 != m_scan_script)
  {
    delete[] m_scan_script;
    m_scan_script = 0;
  }

  if(0 != m_coefbuf)
  {
    ippFree(m_coefbuf);
    m_coefbuf = 0;
  }

  m_src.p.Data8u  = 0;
  m_src.width     = 0;
  m_src.height    = 0;
  m_src.nChannels = 0;
  m_src.lineStep  = 0;

  m_dst.pData     = 0;
  m_dst.DataLen   = 0;
  m_dst.currPos   = 0;

  m_jpeg_ncomp            = 0;
  m_jpeg_sampling         = JS_444;
  m_jpeg_color            = JC_UNKNOWN;
  m_jpeg_quality          = 100;
  m_jpeg_restart_interval = 0;
  m_jpeg_mode             = JPEG_BASELINE;

  m_numxMCU = 0;
  m_numyMCU = 0;
  m_mcuWidth  = 0;
  m_mcuHeight = 0;

  m_ccWidth  = 0;
  m_ccHeight = 0;
  m_xPadding = 0;
  m_yPadding = 0;

  m_restarts_to_go   = 0;
  m_next_restart_num = 0;

  m_scan_count = 0;

  return JPEG_OK;
} // CJPEGEncoder::Clean()


JERRCODE CJPEGEncoder::SetSource(
  Ipp8u*   pSrc,
  int      srcStep,
  IppiSize srcSize,
  int      srcChannels,
  JCOLOR   srcColor)
{
  m_src.p.Data8u  = pSrc;
  m_src.lineStep  = srcStep;
  m_src.width     = srcSize.width;
  m_src.height    = srcSize.height;
  m_src.nChannels = srcChannels;
  m_src.color     = srcColor;

  return JPEG_OK;
} // CJPEGEncoder::SetSource()


JERRCODE CJPEGEncoder::SetDestination(
  Ipp8u* pDst,
  int    dstSize,
  int    dstQuality,
  JSS    dstSampling,
  JCOLOR dstColor,
  JMODE  dstMode,
  int    dstRestartInt)
{
  m_dst.pData   = pDst;
  m_dst.DataLen = dstSize;
  m_dst.currPos = 0;

  m_jpeg_quality          = dstQuality;
  m_jpeg_sampling         = dstSampling;
  m_jpeg_color            = dstColor;
  m_jpeg_mode             = dstMode;
  m_jpeg_restart_interval = dstRestartInt;

  m_restarts_to_go = m_jpeg_restart_interval;

  if(JPEG_LOSSLESS == m_jpeg_mode)
  {
    m_mcuWidth  = 1;
    m_mcuHeight = 1;
  }
  else
  {
    m_mcuWidth  = (m_jpeg_sampling == JS_444) ?  8 : 16;
    m_mcuHeight = (m_jpeg_sampling == JS_411) ? 16 :  8;
  }

  m_numxMCU = (m_src.width  + (m_mcuWidth  - 1)) / m_mcuWidth;
  m_numyMCU = (m_src.height + (m_mcuHeight - 1)) / m_mcuHeight;

  return JPEG_OK;
} // CJPEGEncoder::SetDestination()


JERRCODE CJPEGEncoder::WriteSOI(void)
{
  TRC0("-> WriteSOI");

  if(m_dst.currPos + 2 >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_SOI);

  m_dst._WRITE_WORD(0xff00 | JM_SOI);

  return JPEG_OK;
} // CJPEGEncoder::WriteSOI()


JERRCODE CJPEGEncoder::WriteEOI(void)
{
  TRC0("-> WriteEOI");

  if(m_dst.currPos + 2 >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("emit marker ",JM_EOI);

  m_dst._WRITE_WORD(0xff00 | JM_EOI);

  return JPEG_OK;
} // CJPEGEncoder::WriteEOI()


JERRCODE CJPEGEncoder::WriteAPP0(void)
{
  int len;

  TRC0("-> WriteAPP0");

  len = 2 + 5 + 2 + 1 + 2 + 2 + 1 + 1;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_APP0);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_APP0);
  m_dst._WRITE_WORD(len);

  // identificator JFIF
  m_dst._WRITE_BYTE('J');
  m_dst._WRITE_BYTE('F');
  m_dst._WRITE_BYTE('I');
  m_dst._WRITE_BYTE('F');
  m_dst._WRITE_BYTE(0);

  // version
  m_dst._WRITE_WORD(0x0102);

  // units: 0 - none, 1 - dot per inch, 2 - dot per cm
  m_dst._WRITE_BYTE(0);

  // xDensity
  m_dst._WRITE_WORD(1);

  // yDensity
  m_dst._WRITE_WORD(1);

  // xThumbnails, yThumbnails
  m_dst._WRITE_BYTE(0);
  m_dst._WRITE_BYTE(0);

  return JPEG_OK;
} // CJPEGEncoder::WriteAPP0()


JERRCODE CJPEGEncoder::WriteAPP14(void)
{
  int len;

  TRC0("-> WriteAPP14");

  len = 2 + 5 + 2 + 2 + 2 + 1;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_APP14);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_APP14);
  m_dst._WRITE_WORD(len);

  // identificator Adobe
  m_dst._WRITE_BYTE('A');
  m_dst._WRITE_BYTE('d');
  m_dst._WRITE_BYTE('o');
  m_dst._WRITE_BYTE('b');
  m_dst._WRITE_BYTE('e');

  // version
  m_dst._WRITE_WORD(100);

  // Flags 0
  m_dst._WRITE_WORD(0);

  // Flags 1
  m_dst._WRITE_WORD(0);

  switch(m_jpeg_color)
  {
  case JC_YCBCR:
    m_dst._WRITE_BYTE(1);
    break;
  case JC_YCCK:
    m_dst._WRITE_BYTE(2);
    break;
  default:
    m_dst._WRITE_BYTE(0);
    break;
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteAPP14()


JERRCODE CJPEGEncoder::WriteCOM(
  char*  comment)
{
  int   len;
  char* ptr;

  TRC0("-> WriteCOM");

  if(comment != 0)
  {
    len = (int)strlen(comment) + 1;
    ptr = comment;
  }
  else
  {
    char buf[64];
    ptr = &buf[0];
    const IppLibraryVersion* jv = ippjGetLibVersion();
    sprintf(ptr,"JPEG encoder based on ippJP [%d.%d.%d] - %s",
      jv->major,
      jv->minor,
      jv->build,
      jv->BuildDate);

    len = (int)strlen(ptr) + 1;
  }

  len += 2;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_COM);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_COM);
  m_dst._WRITE_WORD(len);

  for(int i = 0; i < len - 2; i++)
  {
    m_dst._WRITE_BYTE(ptr[i]);
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteCOM()


JERRCODE CJPEGEncoder::WriteDQT(
  CJPEGEncoderQuantTable* qtbl)
{
  int i;
  int len;

  TRC0("-> WriteDQT");

  len = DCTSIZE2 + 2 + 1;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_DQT);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_DQT);
  m_dst._WRITE_WORD(len);

  // precision/id
  m_dst._WRITE_BYTE((qtbl->m_precision << 4) | qtbl->m_id);

  TRC1("  id ",qtbl->m_id);
  TRC1("  precision ",qtbl->m_precision);

  TRC(endl);
  for(i = 0; i < DCTSIZE2; i++)
  {
    TRC(" ");
    TRC((int)qtbl->m_raw[i]);
    if(i % 8 == 7)
    {
      TRC(endl);
    }
  }
  TRC(endl);

  for(i = 0; i < DCTSIZE2; i++)
  {
    m_dst._WRITE_BYTE(qtbl->m_raw[i]);
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteDQT()


JERRCODE CJPEGEncoder::WriteDHT(
  CJPEGEncoderHuffmanTable* htbl)
{
  int i;
  int len;
  int htbl_len;

  TRC0("-> WriteDHT");

  for(htbl_len = 0, i = 0; i < 16; i++)
  {
    htbl_len += htbl->m_bits[i];
  }

  len = 16 + htbl_len + 2 + 1;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_DHT);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_DHT);
  m_dst._WRITE_WORD(len);

  m_dst._WRITE_BYTE((htbl->m_hclass << 4) | htbl->m_id);

  TRC1("  id ",htbl->m_id);
  TRC1("  class ",htbl->m_hclass);

  for(i = 0; i < 16; i++)
  {
    m_dst._WRITE_BYTE(htbl->m_bits[i]);
  }

  for(i = 0; i < htbl_len; i++)
  {
    m_dst._WRITE_BYTE(htbl->m_vals[i]);
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteDHT()


JERRCODE CJPEGEncoder::WriteSOF0(void)
{
  int len;

  TRC0("-> WriteSOF0");

  len = 8 + m_jpeg_ncomp * 3;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_SOF0);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_SOF0);
  m_dst._WRITE_WORD(len);

  // sample precision
  m_dst._WRITE_BYTE(8);

  m_dst._WRITE_WORD(m_src.height);
  m_dst._WRITE_WORD(m_src.width);

  m_dst._WRITE_BYTE(m_jpeg_ncomp);

  for(int i = 0; i < m_jpeg_ncomp; i++)
  {
    m_dst._WRITE_BYTE(i);
    m_dst._WRITE_BYTE((m_ccomp[i]->m_hsampling << 4) | m_ccomp[i]->m_vsampling);
    m_dst._WRITE_BYTE(m_ccomp[i]->m_q_selector);
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteSOF0()


JERRCODE CJPEGEncoder::WriteSOF1(void)
{
  int len;

  TRC0("-> WriteSOF1");

  len = 8 + m_jpeg_ncomp * 3;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_SOF1);
  TRC1("    length ",sof1_len);

  m_dst._WRITE_WORD(0xff00 | JM_SOF1);
  m_dst._WRITE_WORD(len);

  // sample precision
  m_dst._WRITE_BYTE(8);

  m_dst._WRITE_WORD(m_src.height);
  m_dst._WRITE_WORD(m_src.width);

  m_dst._WRITE_BYTE(m_jpeg_ncomp);

  for(int i = 0; i < m_jpeg_ncomp; i++)
  {
    m_dst._WRITE_BYTE(i);
    m_dst._WRITE_BYTE((m_ccomp[i]->m_hsampling << 4) | m_ccomp[i]->m_vsampling);
    m_dst._WRITE_BYTE(m_ccomp[i]->m_q_selector);
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteSOF1()


JERRCODE CJPEGEncoder::WriteSOF2(void)
{
  int len;

  TRC0("-> WriteSOF2");

  len = 8 + m_jpeg_ncomp * 3;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_SOF2);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_SOF2);
  m_dst._WRITE_WORD(len);

  // sample precision
  m_dst._WRITE_BYTE(8);

  m_dst._WRITE_WORD(m_src.height);
  m_dst._WRITE_WORD(m_src.width);

  TRC1("  height ",m_src.height);
  TRC1("  width  ",m_src.width);

  m_dst._WRITE_BYTE(m_jpeg_ncomp);

  for(int i = 0; i < m_jpeg_ncomp; i++)
  {
    m_dst._WRITE_BYTE(i);
    m_dst._WRITE_BYTE((m_ccomp[i]->m_hsampling << 4) | m_ccomp[i]->m_vsampling);
    m_dst._WRITE_BYTE(m_ccomp[i]->m_q_selector);

    TRC1("    id ",i);
    TRC1("      h_sampling ",m_ccomp[i]->m_hsampling);
    TRC1("      v_sampling ",m_ccomp[i]->m_vsampling);
    TRC1("      q_selector ",m_ccomp[i]->m_q_selector);
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteSOF2()


JERRCODE CJPEGEncoder::WriteSOF3(void)
{
  int len;

  TRC0("-> WriteSOF3");

  len = 8 + m_jpeg_ncomp * 3;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_SOF3);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_SOF3);
  m_dst._WRITE_WORD(len);

  // sample precision
  m_dst._WRITE_BYTE(m_jpeg_precision);

  m_dst._WRITE_WORD(m_src.height);
  m_dst._WRITE_WORD(m_src.width);

  m_dst._WRITE_BYTE(m_jpeg_ncomp);

  for(int i = 0; i < m_jpeg_ncomp; i++)
  {
    m_dst._WRITE_BYTE(i);
    m_dst._WRITE_BYTE((m_ccomp[i]->m_hsampling << 4) | m_ccomp[i]->m_vsampling);
    m_dst._WRITE_BYTE(m_ccomp[i]->m_q_selector);
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteSOF3()


JERRCODE CJPEGEncoder::WriteDRI(
  int    restart_interval)
{
  int len;

  TRC0("-> WriteDRI");

  len = 2 + 2;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_DRI);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_DRI);
  m_dst._WRITE_WORD(len);

  // emit restart interval
  m_dst._WRITE_WORD(restart_interval);

  m_restarts_to_go = m_jpeg_restart_interval;
  m_next_restart_num = 0;

  TRC1("  restart ",restart_interval);

  return JPEG_OK;
} // CJPEGEncoder::WriteDRI()


JERRCODE CJPEGEncoder::WriteRST(
  int    next_restart_num)
{
  int len;

  TRC0("-> WriteRST");

  len = 2;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_RST0 + next_restart_num);
  TRC1("    RST ",0xfff0 | (JM_RST0 + next_restart_num));

  // emit restart interval
  m_dst._WRITE_BYTE(0xff);
  m_dst._WRITE_BYTE(0xff00 | (JM_RST0 + next_restart_num));

  // Update next-restart state
  m_next_restart_num = (next_restart_num + 1) & 7;

  return JPEG_OK;
} // CJPEGEncoder::WriteRST()


JERRCODE CJPEGEncoder::ProcessRestart(
  int    id[MAX_COMPS_PER_SCAN],
  int    Ss,
  int    Se,
  int    Ah,
  int    Al)
{
  JERRCODE  jerr;
  IppStatus status = ippStsNoErr;

  TRC0("-> ProcessRestart");

  // flush IppiEncodeHuffmanState
  switch(m_jpeg_mode)
  {
  case JPEG_BASELINE:
    status = ippiEncodeHuffman8x8_JPEG_16s1u_C1(
               0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,0,0,0,m_state,1);
    break;

  case JPEG_PROGRESSIVE:
    if(Ss == 0 && Se == 0)
    {
      // DC scan
      if(Ah == 0)
      {
        status = ippiEncodeHuffman8x8_DCFirst_JPEG_16s1u_C1(
                   0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,0,0,0,m_state,1);
      }
      else
      {
        status = ippiEncodeHuffman8x8_DCRefine_JPEG_16s1u_C1(
                   0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,0,m_state,1);
      }
    }
    else
    {
      // AC scan
      IppiEncodeHuffmanSpec* actbl = m_actbl[m_ccomp[id[0]]->m_ac_selector];

      if(Ah == 0)
      {
        status = ippiEncodeHuffman8x8_ACFirst_JPEG_16s1u_C1(
                   0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,Ss,Se,Al,actbl,m_state,1);
      }
      else
      {
        status = ippiEncodeHuffman8x8_ACRefine_JPEG_16s1u_C1(
                   0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,Ss,Se,Al,actbl,m_state,1);
      }
    }
    break;

  case JPEG_LOSSLESS:
    status = ippiEncodeHuffmanOne_JPEG_16s1u_C1(
               0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,0,m_state,1);
    break;
  }

  if(ippStsNoErr > status)
  {
    LOG1("IPP Error: ippiEncodeHuffman8x8_JPEG_16s1u_C1() failed - ",status);
    return JPEG_INTERNAL_ERROR;
  }

  status = ippiEncodeHuffmanStateInit_JPEG_8u(m_state);
  if(ippStsNoErr != status)
  {
    return JPEG_INTERNAL_ERROR;
  }

  jerr = WriteRST(m_next_restart_num);
  if(JPEG_OK != jerr)
  {
    LOG1("IPP Error: WriteRST() failed - ",jerr);
    return JPEG_INTERNAL_ERROR;
  }

  for(int c = 0; c < m_jpeg_ncomp; c++)
  {
    m_ccomp[c]->m_lastDC = 0;
  }

  m_restarts_to_go = m_jpeg_restart_interval;

  return JPEG_OK;
} // CJPEGEncoder::ProcessRestart()


JERRCODE CJPEGEncoder::ProcessRestart(
  int    stat[2][256],
  int    id[MAX_COMPS_PER_SCAN],
  int    Ss,
  int    Se,
  int    Ah,
  int    Al)
{
  IppStatus status;

  TRC0("-> ProcessRestart");

  // flush IppiEncodeHuffmanState
  if(JPEG_PROGRESSIVE == m_jpeg_mode)
  {
    if(Ss == 0 && Se == 0)
    {
      // DC scan
      // nothing to do
    }
    else
    {
      // AC scan

      if(Ah == 0)
      {
        status = ippiGetHuffmanStatistics8x8_ACFirst_JPEG_16s_C1(
                   0,&stat[m_ccomp[id[0]]->m_ac_selector][0],Ss,Se,Al,m_state,1);

        if(ippStsNoErr > status)
        {
          LOG1("IPP Error: ippiGetHuffmanStatistics8x8_ACFirst_JPEG_16s_C1() failed - ",status);
          return JPEG_INTERNAL_ERROR;
        }
      }
      else
      {
        status = ippiGetHuffmanStatistics8x8_ACRefine_JPEG_16s_C1(
                   0,&stat[m_ccomp[id[0]]->m_ac_selector][0],Ss,Se,Al,m_state,1);

        if(ippStsNoErr > status)
        {
          LOG1("IPP Error: ippiGetHuffmanStatistics8x8_ACRefine_JPEG_16s_C1() failed - ",status);
          return JPEG_INTERNAL_ERROR;
        }
      }
    }
  }

  status = ippiEncodeHuffmanStateInit_JPEG_8u(m_state);
  if(ippStsNoErr != status)
  {
    return JPEG_INTERNAL_ERROR;
  }

  for(int c = 0; c < m_jpeg_ncomp; c++)
  {
    m_ccomp[c]->m_lastDC = 0;
  }

  m_restarts_to_go = m_jpeg_restart_interval;

  return JPEG_OK;
} // CJPEGEncoder::ProcessRestart()


JERRCODE CJPEGEncoder::WriteSOS(void)
{
  int len;

  TRC0("-> WriteSOS");

  len = 3 + m_jpeg_ncomp*2 + 3;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_SOS);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_SOS);
  m_dst._WRITE_WORD(len);

  m_dst._WRITE_BYTE(m_jpeg_ncomp);

  TRC1("  ncomp ",m_jpeg_ncomp);

  for(int i = 0; i < m_jpeg_ncomp; i++)
  {
    m_dst._WRITE_BYTE(i);
    m_dst._WRITE_BYTE((m_ccomp[i]->m_dc_selector << 4) | m_ccomp[i]->m_ac_selector);

    TRC1("    id ",i);
    TRC1("      dc_selector ",m_ccomp[i]->m_dc_selector);
    TRC1("      ac_selector ",m_ccomp[i]->m_ac_selector);
  }

  m_dst._WRITE_BYTE(m_ss); // Ss
  m_dst._WRITE_BYTE(m_se); // Se
  m_dst._WRITE_BYTE(((m_ah << 4) | m_al));  // Ah/Al

  TRC1("  Ss ",m_ss);
  TRC1("  Se ",m_se);
  TRC1("  Ah ",m_ah);
  TRC1("  Al ",m_al);

  return JPEG_OK;
} // CJPEGEncoder::WriteSOS()


JERRCODE CJPEGEncoder::WriteSOS(
  int    ncomp,
  int    id[MAX_COMPS_PER_SCAN],
  int    Ss,
  int    Se,
  int    Ah,
  int    Al)
{
  int len;

  TRC0("-> WriteSOS");

  len = 3 + ncomp*2 + 3;

  if(m_dst.currPos + len >= m_dst.DataLen)
  {
    LOG0("Error: buffer too small");
    return JPEG_BUFF_TOO_SMALL;
  }

  TRC1("  emit marker ",JM_SOS);
  TRC1("    length ",len);

  m_dst._WRITE_WORD(0xff00 | JM_SOS);
  m_dst._WRITE_WORD(len);

  m_dst._WRITE_BYTE(ncomp);

  TRC1("  ncomp ",ncomp);

  for(int i = 0; i < ncomp; i++)
  {
    m_dst._WRITE_BYTE(id[i]);
    m_dst._WRITE_BYTE((m_ccomp[id[i]]->m_dc_selector << 4) | m_ccomp[id[i]]->m_ac_selector);

    TRC1("    id ",id[i]);
    TRC1("    dc_selector ",m_ccomp[id[i]]->m_dc_selector);
    TRC1("    ac_selector ",m_ccomp[id[i]]->m_ac_selector);
  }

  m_dst._WRITE_BYTE(Ss);       // Ss
  m_dst._WRITE_BYTE(Se);       // Se
  m_dst._WRITE_BYTE(((Ah & 0x0f) << 4) | (Al & 0x0f));  // Ah/Al

  TRC1("  Ss ",Ss);
  TRC1("  Se ",Se);
  TRC1("  Ah ",Ah);
  TRC1("  Al ",Al);

  return JPEG_OK;
} // CJPEGEncoder::WriteSOS()


JERRCODE CJPEGEncoder::SelectScanScripts(void)
{
  switch(m_jpeg_ncomp)
  {
  case 1:
    m_scan_count = 6;
    m_scan_script = new JPEG_SCAN [m_scan_count];

    // 1 DC scan, def
    m_scan_script[0].ncomp = 1;
    m_scan_script[0].id[0] = 0;
    m_scan_script[0].Ss    = 0;
    m_scan_script[0].Se    = 0;
    m_scan_script[0].Ah    = 0;
    m_scan_script[0].Al    = 1;
    // 2 AC scan, def(luma)
    m_scan_script[1].ncomp = 1;
    m_scan_script[1].id[0] = 0;
    m_scan_script[1].Ss    = 1;
    m_scan_script[1].Se    = 5;
    m_scan_script[1].Ah    = 0;
    m_scan_script[1].Al    = 2;
    // 3 AC scan, def(luma)
    m_scan_script[2].ncomp = 1;
    m_scan_script[2].id[0] = 0;
    m_scan_script[2].Ss    = 6;
    m_scan_script[2].Se    = 63;
    m_scan_script[2].Ah    = 0;
    m_scan_script[2].Al    = 2;
    // 4 AC scan, ref(luma)
    m_scan_script[3].ncomp = 1;
    m_scan_script[3].id[0] = 0;
    m_scan_script[3].Ss    = 1;
    m_scan_script[3].Se    = 63;
    m_scan_script[3].Ah    = 2;
    m_scan_script[3].Al    = 1;
    // 5 DC scan, ref
    m_scan_script[4].ncomp = 1;
    m_scan_script[4].id[0] = 0;
    m_scan_script[4].Ss    = 0;
    m_scan_script[4].Se    = 0;
    m_scan_script[4].Ah    = 1;
    m_scan_script[4].Al    = 0;
    // 6 AC scan, ref(luma)
    m_scan_script[5].ncomp = 1;
    m_scan_script[5].id[0] = 0;
    m_scan_script[5].Ss    = 1;
    m_scan_script[5].Se    = 63;
    m_scan_script[5].Ah    = 1;
    m_scan_script[5].Al    = 0;
    break;

  case 3:
    m_scan_count = 10;
    m_scan_script = new JPEG_SCAN [m_scan_count];

    // 1 DC scan, def
    m_scan_script[0].ncomp = 3;
    m_scan_script[0].id[0] = 0;
    m_scan_script[0].id[1] = 1;
    m_scan_script[0].id[2] = 2;
    m_scan_script[0].Ss    = 0;
    m_scan_script[0].Se    = 0;
    m_scan_script[0].Ah    = 0;
    m_scan_script[0].Al    = 1;
    // 2 AC scan, def(luma)
    m_scan_script[1].ncomp = 1;
    m_scan_script[1].id[0] = 0;
    m_scan_script[1].Ss    = 1;
    m_scan_script[1].Se    = 5;
    m_scan_script[1].Ah    = 0;
    m_scan_script[1].Al    = 2;
    // 3 AC scan, def(cr)
    m_scan_script[2].ncomp = 1;
    m_scan_script[2].id[0] = 2;
    m_scan_script[2].Ss    = 1;
    m_scan_script[2].Se    = 63;
    m_scan_script[2].Ah    = 0;
    m_scan_script[2].Al    = 1;
    // 4 AC scan, def(cb)
    m_scan_script[3].ncomp = 1;
    m_scan_script[3].id[0] = 1;
    m_scan_script[3].Ss    = 1;
    m_scan_script[3].Se    = 63;
    m_scan_script[3].Ah    = 0;
    m_scan_script[3].Al    = 1;
    // 5 AC scan, def(luma)
    m_scan_script[4].ncomp = 1;
    m_scan_script[4].id[0] = 0;
    m_scan_script[4].Ss    = 6;
    m_scan_script[4].Se    = 63;
    m_scan_script[4].Ah    = 0;
    m_scan_script[4].Al    = 2;
    // 7 AC scan, ref(luma)
    m_scan_script[5].ncomp = 1;
    m_scan_script[5].id[0] = 0;
    m_scan_script[5].Ss    = 1;
    m_scan_script[5].Se    = 63;
    m_scan_script[5].Ah    = 2;
    m_scan_script[5].Al    = 1;
    // 6 DC scan, ref
    m_scan_script[6].ncomp = 3;
    m_scan_script[6].id[0] = 0;
    m_scan_script[6].id[1] = 1;
    m_scan_script[6].id[2] = 2;
    m_scan_script[6].Ss    = 0;
    m_scan_script[6].Se    = 0;
    m_scan_script[6].Ah    = 1;
    m_scan_script[6].Al    = 0;
    // 8 AC scan, ref(cr)
    m_scan_script[7].ncomp = 1;
    m_scan_script[7].id[0] = 2;
    m_scan_script[7].Ss    = 1;
    m_scan_script[7].Se    = 63;
    m_scan_script[7].Ah    = 1;
    m_scan_script[7].Al    = 0;
    // 9 AC scan, ref(cb)
    m_scan_script[8].ncomp = 1;
    m_scan_script[8].id[0] = 1;
    m_scan_script[8].Ss    = 1;
    m_scan_script[8].Se    = 63;
    m_scan_script[8].Ah    = 1;
    m_scan_script[8].Al    = 0;
    // 10 AC scan, ref(luma)
    m_scan_script[9].ncomp = 1;
    m_scan_script[9].id[0] = 0;
    m_scan_script[9].Ss    = 1;
    m_scan_script[9].Se    = 63;
    m_scan_script[9].Ah    = 1;
    m_scan_script[9].Al    = 0;
    break;

  case 4:
    m_scan_count = 18;
    m_scan_script = new JPEG_SCAN [m_scan_count];

    // 1 DC scan, def
    m_scan_script[0].ncomp = 4;
    m_scan_script[0].id[0] = 0;
    m_scan_script[0].id[1] = 1;
    m_scan_script[0].id[2] = 2;
    m_scan_script[0].id[3] = 3;
    m_scan_script[0].Ss    = 0;
    m_scan_script[0].Se    = 0;
    m_scan_script[0].Ah    = 0;
    m_scan_script[0].Al    = 1;
    // 2 AC scan, def(0)
    m_scan_script[1].ncomp = 1;
    m_scan_script[1].id[0] = 0;
    m_scan_script[1].Ss    = 1;
    m_scan_script[1].Se    = 5;
    m_scan_script[1].Ah    = 0;
    m_scan_script[1].Al    = 2;
    // 3 AC scan, def(1)
    m_scan_script[2].ncomp = 1;
    m_scan_script[2].id[0] = 1;
    m_scan_script[2].Ss    = 1;
    m_scan_script[2].Se    = 5;
    m_scan_script[2].Ah    = 0;
    m_scan_script[2].Al    = 2;
    // 4 AC scan, def(2)
    m_scan_script[3].ncomp = 1;
    m_scan_script[3].id[0] = 2;
    m_scan_script[3].Ss    = 1;
    m_scan_script[3].Se    = 5;
    m_scan_script[3].Ah    = 0;
    m_scan_script[3].Al    = 2;
    // 5 AC scan, def(3)
    m_scan_script[4].ncomp = 1;
    m_scan_script[4].id[0] = 3;
    m_scan_script[4].Ss    = 1;
    m_scan_script[4].Se    = 5;
    m_scan_script[4].Ah    = 0;
    m_scan_script[4].Al    = 2;
    // 6 AC scan, def(0)
    m_scan_script[5].ncomp = 1;
    m_scan_script[5].id[0] = 0;
    m_scan_script[5].Ss    = 6;
    m_scan_script[5].Se    = 63;
    m_scan_script[5].Ah    = 0;
    m_scan_script[5].Al    = 2;
    // 7 AC scan, def(1)
    m_scan_script[6].ncomp = 1;
    m_scan_script[6].id[0] = 1;
    m_scan_script[6].Ss    = 6;
    m_scan_script[6].Se    = 63;
    m_scan_script[6].Ah    = 0;
    m_scan_script[6].Al    = 2;
    // 8 AC scan, def(2)
    m_scan_script[7].ncomp = 1;
    m_scan_script[7].id[0] = 2;
    m_scan_script[7].Ss    = 6;
    m_scan_script[7].Se    = 63;
    m_scan_script[7].Ah    = 0;
    m_scan_script[7].Al    = 2;
    // 9 AC scan, def(3)
    m_scan_script[8].ncomp = 1;
    m_scan_script[8].id[0] = 3;
    m_scan_script[8].Ss    = 6;
    m_scan_script[8].Se    = 63;
    m_scan_script[8].Ah    = 0;
    m_scan_script[8].Al    = 2;
    // 10 AC scan, ref(0)
    m_scan_script[9].ncomp = 1;
    m_scan_script[9].id[0] = 0;
    m_scan_script[9].Ss    = 1;
    m_scan_script[9].Se    = 63;
    m_scan_script[9].Ah    = 2;
    m_scan_script[9].Al    = 1;
    // 11 AC scan, ref(1)
    m_scan_script[10].ncomp = 1;
    m_scan_script[10].id[0] = 1;
    m_scan_script[10].Ss    = 1;
    m_scan_script[10].Se    = 63;
    m_scan_script[10].Ah    = 2;
    m_scan_script[10].Al    = 1;
    // 12 AC scan, ref(2)
    m_scan_script[11].ncomp = 1;
    m_scan_script[11].id[0] = 2;
    m_scan_script[11].Ss    = 1;
    m_scan_script[11].Se    = 63;
    m_scan_script[11].Ah    = 2;
    m_scan_script[11].Al    = 1;
    // 13 AC scan, ref(3)
    m_scan_script[12].ncomp = 1;
    m_scan_script[12].id[0] = 3;
    m_scan_script[12].Ss    = 1;
    m_scan_script[12].Se    = 63;
    m_scan_script[12].Ah    = 2;
    m_scan_script[12].Al    = 1;
    // 14 DC scan, ref
    m_scan_script[13].ncomp = 4;
    m_scan_script[13].id[0] = 0;
    m_scan_script[13].id[1] = 1;
    m_scan_script[13].id[2] = 2;
    m_scan_script[13].id[3] = 3;
    m_scan_script[13].Ss    = 0;
    m_scan_script[13].Se    = 0;
    m_scan_script[13].Ah    = 1;
    m_scan_script[13].Al    = 0;
    // 15 AC scan, ref(0)
    m_scan_script[14].ncomp = 1;
    m_scan_script[14].id[0] = 0;
    m_scan_script[14].Ss    = 1;
    m_scan_script[14].Se    = 63;
    m_scan_script[14].Ah    = 1;
    m_scan_script[14].Al    = 0;
    // 16 AC scan, ref(1)
    m_scan_script[15].ncomp = 1;
    m_scan_script[15].id[0] = 1;
    m_scan_script[15].Ss    = 1;
    m_scan_script[15].Se    = 63;
    m_scan_script[15].Ah    = 1;
    m_scan_script[15].Al    = 0;
    // 17 AC scan, ref(2)
    m_scan_script[16].ncomp = 1;
    m_scan_script[16].id[0] = 2;
    m_scan_script[16].Ss    = 1;
    m_scan_script[16].Se    = 63;
    m_scan_script[16].Ah    = 1;
    m_scan_script[16].Al    = 0;
    // 18 AC scan, ref(3)
    m_scan_script[17].ncomp = 1;
    m_scan_script[17].id[0] = 3;
    m_scan_script[17].Ss    = 1;
    m_scan_script[17].Se    = 63;
    m_scan_script[17].Ah    = 1;
    m_scan_script[17].Al    = 0;
    break;

  default:
    return JPEG_NOT_IMPLEMENTED;
  }

  return JPEG_OK;
} // CJPEGEncoder::SelectScanScripts()


JERRCODE CJPEGEncoder::Init(void)
{
  JERRCODE  jerr;

  switch(m_jpeg_color)
  {
  case JC_GRAY:  m_jpeg_ncomp = 1; break;
  case JC_RGB:   m_jpeg_ncomp = 3; break;
  case JC_YCBCR: m_jpeg_ncomp = 3; break;
  case JC_CMYK:  m_jpeg_ncomp = 4; break;
  case JC_YCCK:  m_jpeg_ncomp = 4; break;
  default:
    // let to user selects the number of component
    break;
  }

  m_xPadding = m_numxMCU * m_mcuWidth  - m_src.width;
  m_yPadding = m_numyMCU * m_mcuHeight - m_src.height;

  m_ccWidth  = m_mcuWidth * m_numxMCU;
  m_ccHeight = m_mcuHeight;

  for(int i = 0; i < m_jpeg_ncomp; i++)
  {
    int cc_buf_size;
    int ss_buf_size;

    switch(m_jpeg_mode)
    {
    case JPEG_BASELINE:
      cc_buf_size = m_ccWidth*m_ccHeight;
      ss_buf_size = m_ccWidth*m_ccHeight;
      break;

    case JPEG_PROGRESSIVE:
      cc_buf_size = (m_mcuWidth*m_mcuHeight)*(m_numxMCU*m_numyMCU);
      ss_buf_size = (m_mcuWidth*m_mcuHeight)*(m_numxMCU*m_numyMCU);
      break;

    case JPEG_LOSSLESS:
      cc_buf_size = m_ccWidth*sizeof(Ipp16s);
      ss_buf_size = m_ccWidth*sizeof(Ipp16s);
      break;

    default:
      return JPEG_INTERNAL_ERROR;
    }

    if(0 != m_ccomp[i])
    {
      if(0 != m_ccomp[i]->m_cc_buffer)
      {
        ippFree(m_ccomp[i]->m_cc_buffer);
        m_ccomp[i]->m_cc_buffer = 0;
      }

      if(0 != m_ccomp[i]->m_ss_buffer)
      {
        ippFree(m_ccomp[i]->m_ss_buffer);
        m_ccomp[i]->m_ss_buffer = 0;
      }

      delete m_ccomp[i];
    }

    m_ccomp[i] = new CJPEGColorComponent;
    if(0 == m_ccomp[i])
    {
      return JPEG_OUT_OF_MEMORY;
    }

    m_ccomp[i]->m_id          = i;
    m_ccomp[i]->m_comp_no     = i;
    m_ccomp[i]->m_hsampling   = (m_jpeg_sampling == JS_444) ? 1 : (i == 0 || i == 3 ? 2 : 1);
    m_ccomp[i]->m_vsampling   = (m_jpeg_sampling == JS_411) ? (i == 0 || i == 3 ? 2 : 1) : 1;
    m_ccomp[i]->m_h_factor    = (m_jpeg_sampling == JS_444) ? 1 : (i == 0 || i == 3? 1 : 2);
    m_ccomp[i]->m_v_factor    = (m_jpeg_sampling == JS_411) ? (i == 0 || i == 3 ? 1 : 2) : 1;
    m_ccomp[i]->m_nblocks     = m_ccomp[i]->m_hsampling * m_ccomp[i]->m_vsampling;
    m_ccomp[i]->m_q_selector  = (i == 0 || i == 3) ? 0 : (m_jpeg_color == JC_YCBCR || m_jpeg_color == JC_YCCK ? 1 : 0);
    m_ccomp[i]->m_dc_selector = (i == 0 || i == 3) ? 0 : 1;
    m_ccomp[i]->m_ac_selector = (i == 0 || i == 3) ? 0 : 1;

    // color convert intermediate buffer
    m_ccomp[i]->m_cc_buffer = (Ipp8u*)ippMalloc(cc_buf_size);
    if(0 == m_ccomp[i]->m_cc_buffer)
    {
      return JPEG_OUT_OF_MEMORY;
    }

    // subsampling buffer
    m_ccomp[i]->m_ss_buffer = (Ipp8u*)ippMalloc(ss_buf_size);
    if(0 == m_ccomp[i]->m_ss_buffer)
    {
      return JPEG_OUT_OF_MEMORY;
    }

    // huffman buffer
    m_ccomp[i]->m_top_row = (Ipp8u*)ippMalloc(ss_buf_size);
    if(0 == m_ccomp[i]->m_top_row)
    {
      return JPEG_OUT_OF_MEMORY;
    }

    m_ccomp[i]->m_curr_row = (Ipp16s*)m_ccomp[i]->m_cc_buffer;
    m_ccomp[i]->m_prev_row = (Ipp16s*)m_ccomp[i]->m_ss_buffer;
  }

  if(JPEG_PROGRESSIVE == m_jpeg_mode)
  {
    SelectScanScripts();
  }

  if(JPEG_PROGRESSIVE == m_jpeg_mode/* || JPEG_LOSSLESS == m_jpeg_mode*/)
  {
    if(0 == m_coefbuf)
    {
//      int sz = m_numxMCU*m_numyMCU*sizeof(Ipp16s);
      int sz = m_numxMCU*m_numyMCU*MAX_BYTES_PER_MCU*sizeof(Ipp16s);

      m_coefbuf = (Ipp16s*)ippMalloc(sz);
      if(0 == m_coefbuf)
      {
        return JPEG_OUT_OF_MEMORY;
      }

      ippsZero_8u((Ipp8u*)m_coefbuf,sz);
    }
  }

  if(JPEG_LOSSLESS != m_jpeg_mode)
  {
//    m_qntbl[0] = new CJPEGEncoderQuantTable;
//    m_qntbl[1] = new CJPEGEncoderQuantTable;
  }

  m_dctbl[0].Create();
  m_dctbl[1].Create();
  m_actbl[0].Create();
  m_actbl[1].Create();

  if(JPEG_LOSSLESS != m_jpeg_mode)
  {
    jerr = m_qntbl[0].Init(0,m_jpeg_quality,(Ipp8u*)DefaultLuminanceQuant);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: can't init quant table");
      return jerr;
    }

    jerr = m_qntbl[1].Init(1,m_jpeg_quality,(Ipp8u*)DefaultChrominanceQuant);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: can't init quant table");
      return jerr;
    }
  }

  jerr = m_dctbl[0].Init(0,0,(Ipp8u*)DefaultLuminanceDCBits,(Ipp8u*)DefaultLuminanceDCValues);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: can't init huffman table");
    return jerr;
  }

  jerr = m_dctbl[1].Init(1,0,(Ipp8u*)DefaultChrominanceDCBits,(Ipp8u*)DefaultChrominanceDCValues);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: can't init huffman table");
    return jerr;
  }

  jerr = m_actbl[0].Init(0,1,(Ipp8u*)DefaultLuminanceACBits,(Ipp8u*)DefaultLuminanceACValues);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: can't init huffman table");
    return jerr;
  }

  jerr = m_actbl[1].Init(1,1,(Ipp8u*)DefaultChrominanceACBits,(Ipp8u*)DefaultChrominanceACValues);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: can't init huffman table");
    return jerr;
  }

  m_state.Create();

  return JPEG_OK;
} // CJPEGEncoder::Init()


JERRCODE CJPEGEncoder::ColorConvert(void)
{
  IppStatus status;

  IppiSize roi = { m_src.width, m_src.height };

  Ipp8u* src = m_src.p.Data8u;

  if(m_jpeg_color == JC_UNKNOWN && m_src.color == JC_UNKNOWN)
  {
    switch(m_jpeg_ncomp)
    {
    case 1:
      {
        Ipp8u* dst = m_ccomp[0]->m_cc_buffer;

        status = ippiCopy_8u_C1R(src,m_src.lineStep,dst,m_ccWidth,roi);
        if(ippStsNoErr != status)
        {
          LOG1("IPP Error: ippiCopy_8u_C1R() failed - ",status);
          return JPEG_INTERNAL_ERROR;
        }
      }
      break;

    case 3:
      {
        Ipp8u* dst[3];
        dst[0] = m_ccomp[0]->m_cc_buffer;
        dst[1] = m_ccomp[1]->m_cc_buffer;
        dst[2] = m_ccomp[2]->m_cc_buffer;

        status = ippiCopy_8u_C3P3R(src,m_src.lineStep,dst,m_ccWidth,roi);

        if(ippStsNoErr != status)
        {
          LOG1("IPP Error: ippiCopy_8u_C3P3R() failed - ",status);
          return JPEG_INTERNAL_ERROR;
        }
      }
      break;

    case 4:
      {
        Ipp8u* dst[4];
        dst[0] = m_ccomp[0]->m_cc_buffer;
        dst[1] = m_ccomp[1]->m_cc_buffer;
        dst[2] = m_ccomp[2]->m_cc_buffer;
        dst[3] = m_ccomp[3]->m_cc_buffer;

        status = ippiCopy_8u_C4P4R(src,m_src.lineStep,dst,m_ccWidth,roi);

        if(ippStsNoErr != status)
        {
          LOG1("IPP Error: ippiCopy_8u_C4P4R() failed - ",status);
          return JPEG_INTERNAL_ERROR;
        }
      }
      break;

    default:
      return JPEG_NOT_IMPLEMENTED;
    }
  }

  // Gray to Gray
  if(m_src.color == JC_GRAY && m_jpeg_color == JC_GRAY)
  {
    Ipp8u* dst = m_ccomp[0]->m_cc_buffer;

    status = ippiCopy_8u_C1R(src,m_src.lineStep,dst,m_ccWidth,roi);
    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiCopy_8u_C1R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // RGB to Gray
  if(m_src.color == JC_RGB && m_jpeg_color == JC_GRAY)
  {
    Ipp8u* dst = m_ccomp[0]->m_cc_buffer;

    status = ippiRGBToY_JPEG_8u_C3C1R(src,m_src.lineStep,dst,m_ccWidth,roi);
    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiRGBToY_JPEG_8u_C3C1R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // RGB to RGB
  if(m_src.color == JC_RGB && m_jpeg_color == JC_RGB)
  {
    Ipp8u* dst[3];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;

    status = ippiCopy_8u_C3P3R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiCopy_8u_C3P3R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // RGB to YCbCr
  if(m_src.color == JC_RGB && m_jpeg_color == JC_YCBCR)
  {
    Ipp8u* dst[3];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;

    status = ippiRGBToYCbCr_JPEG_8u_C3P3R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiRGBToYCbCr_JPEG_8u_C3P3R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // BGR to YCbCr
  if(m_src.color == JC_BGR && m_jpeg_color == JC_YCBCR)
  {
    Ipp8u* dst[3];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;

    status = ippiBGRToYCbCr_JPEG_8u_C3P3R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiBGRToYCbCr_JPEG_8u_C3P3R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // CMYK to CMYK
  if(m_src.color == JC_CMYK && m_jpeg_color == JC_CMYK)
  {
    Ipp8u* dst[4];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;
    dst[3] = m_ccomp[3]->m_cc_buffer;

    status = ippiCopy_8u_C4P4R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiCopy_8u_C4P4R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // CMYK to YCCK
  if(m_src.color == JC_CMYK && m_jpeg_color == JC_YCCK)
  {
    Ipp8u* dst[4];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;
    dst[3] = m_ccomp[3]->m_cc_buffer;

    status = ippiCMYKToYCCK_JPEG_8u_C4P4R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiCMYKToYCCK_JPEG_8u_C4P4R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  return JPEG_OK;
} // CJPEGEncoder::ColorConvert()


JERRCODE CJPEGEncoder::ColorConvert(int nMCURow)
{
  IppStatus status;

  if(nMCURow == m_numyMCU - 1)
  {
    m_ccHeight = m_mcuHeight - m_yPadding;
  }

  IppiSize roi = { m_src.width, m_ccHeight };

  Ipp8u* src = m_src.p.Data8u + nMCURow*m_mcuHeight*m_src.lineStep;

  if(m_jpeg_color == JC_UNKNOWN && m_src.color == JC_UNKNOWN)
  {
    switch(m_jpeg_ncomp)
    {
    case 1:
      {
        Ipp8u* dst = m_ccomp[0]->m_cc_buffer;

        status = ippiCopy_8u_C1R(src,m_src.lineStep,dst,m_ccWidth,roi);
        if(ippStsNoErr != status)
        {
          LOG1("IPP Error: ippiCopy_8u_C1R() failed - ",status);
          return JPEG_INTERNAL_ERROR;
        }
      }
      break;

    case 3:
      {
        Ipp8u* dst[3];
        dst[0] = m_ccomp[0]->m_cc_buffer;
        dst[1] = m_ccomp[1]->m_cc_buffer;
        dst[2] = m_ccomp[2]->m_cc_buffer;

        status = ippiCopy_8u_C3P3R(src,m_src.lineStep,dst,m_ccWidth,roi);

        if(ippStsNoErr != status)
        {
          LOG1("IPP Error: ippiCopy_8u_C3P3R() failed - ",status);
          return JPEG_INTERNAL_ERROR;
        }
      }
      break;

    case 4:
      {
        Ipp8u* dst[4];
        dst[0] = m_ccomp[0]->m_cc_buffer;
        dst[1] = m_ccomp[1]->m_cc_buffer;
        dst[2] = m_ccomp[2]->m_cc_buffer;
        dst[3] = m_ccomp[3]->m_cc_buffer;

        status = ippiCopy_8u_C4P4R(src,m_src.lineStep,dst,m_ccWidth,roi);

        if(ippStsNoErr != status)
        {
          LOG1("IPP Error: ippiCopy_8u_C4P4R() failed - ",status);
          return JPEG_INTERNAL_ERROR;
        }
      }
      break;

    default:
      return JPEG_NOT_IMPLEMENTED;
    }
  }

  // Gray to Gray
  if(m_src.color == JC_GRAY && m_jpeg_color == JC_GRAY)
  {
    Ipp8u* dst = m_ccomp[0]->m_cc_buffer;

    status = ippiCopy_8u_C1R(src,m_src.lineStep,dst,m_ccWidth,roi);
    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiCopy_8u_C1R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // RGB to Gray
  if(m_src.color == JC_RGB && m_jpeg_color == JC_GRAY)
  {
    Ipp8u* dst = m_ccomp[0]->m_cc_buffer;

    status = ippiRGBToY_JPEG_8u_C3C1R(src,m_src.lineStep,dst,m_ccWidth,roi);
    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiRGBToY_JPEG_8u_C3C1R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // RGB to RGB
  if(m_src.color == JC_RGB && m_jpeg_color == JC_RGB)
  {
    Ipp8u* dst[3];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;

    status = ippiCopy_8u_C3P3R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiCopy_8u_C3P3R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // RGB to YCbCr
  if(m_src.color == JC_RGB && m_jpeg_color == JC_YCBCR)
  {
    Ipp8u* dst[3];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;

    status = ippiRGBToYCbCr_JPEG_8u_C3P3R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiRGBToYCbCr_JPEG_8u_C3P3R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // BGR to YCbCr
  if(m_src.color == JC_BGR && m_jpeg_color == JC_YCBCR)
  {
    Ipp8u* dst[3];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;

    status = ippiBGRToYCbCr_JPEG_8u_C3P3R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiBGRToYCbCr_JPEG_8u_C3P3R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // CMYK to CMYK
  if(m_src.color == JC_CMYK && m_jpeg_color == JC_CMYK)
  {
    Ipp8u* dst[4];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;
    dst[3] = m_ccomp[3]->m_cc_buffer;

    status = ippiCopy_8u_C4P4R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiCopy_8u_C4P4R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  // CMYK to YCCK
  if(m_src.color == JC_CMYK && m_jpeg_color == JC_YCCK)
  {
    Ipp8u* dst[4];
    dst[0] = m_ccomp[0]->m_cc_buffer;
    dst[1] = m_ccomp[1]->m_cc_buffer;
    dst[2] = m_ccomp[2]->m_cc_buffer;
    dst[3] = m_ccomp[3]->m_cc_buffer;

    status = ippiCMYKToYCCK_JPEG_8u_C4P4R(src,m_src.lineStep,dst,m_ccWidth,roi);

    if(ippStsNoErr != status)
    {
      LOG1("IPP Error: ippiCMYKToYCCK_JPEG_8u_C4P4R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }
  }

  return JPEG_OK;
} // CJPEGEncoder::ColorConvert()


JERRCODE CJPEGEncoder::DownSampling(void)
{
  int i, j, k;

  Ipp8u val;
  Ipp8u* p;
  Ipp8u* p1;

  for(k = 0; k < m_jpeg_ncomp; k++)
  {
    // expand right edge
    if(m_xPadding)
    {
      for(i = 0; i < m_src.height; i++)
      {
        p = m_ccomp[k]->m_cc_buffer + i*m_mcuWidth*m_numxMCU;
        val = p[m_src.width - 1];
        for(j = 0; j < m_xPadding; j++)
        {
          p[m_src.width + j] = val;
        }
      }
    }

    // expand bottom edge only for last MCU row

    if(m_yPadding)
    {
      p = m_ccomp[k]->m_cc_buffer + (m_src.height-1)*m_mcuWidth*m_numxMCU;

      for(i = 0; i < m_yPadding; i++)
      {
        p1 = m_ccomp[k]->m_cc_buffer + m_src.height*m_mcuWidth*m_numxMCU + i*m_mcuWidth*m_numxMCU;
        ippsCopy_8u(p,p1,m_mcuWidth*m_numxMCU);
      }
    }

    // sampling 444
    if(m_ccomp[k]->m_h_factor == 1 && m_ccomp[k]->m_v_factor == 1)
    {
      Ipp8u* src = m_ccomp[k]->m_cc_buffer;
      Ipp8u* dst = m_ccomp[k]->m_ss_buffer;

      ippsCopy_8u(src,dst,m_ccWidth*m_mcuHeight*m_numyMCU);
    }

    IppStatus status;

    // sampling 422
    if(m_ccomp[k]->m_h_factor == 2 && m_ccomp[k]->m_v_factor == 1)
    {
      Ipp8u* src = m_ccomp[k]->m_cc_buffer;
      Ipp8u* dst = m_ccomp[k]->m_ss_buffer;
      IppiSize srcRoi = { m_ccWidth,    m_mcuHeight*m_numyMCU };
      IppiSize dstRoi = { m_ccWidth>>1, m_mcuHeight*m_numyMCU };

      status = ippiSampleDownH2V1_JPEG_8u_C1R(src,m_ccWidth,srcRoi,dst,m_ccWidth,dstRoi);

      if(ippStsNoErr != status)
      {
        LOG1("IPP Error: ippiSampleDownH2V1_JPEG_8u_C1R() failed - ",status);
        return JPEG_INTERNAL_ERROR;
      }
    }

    // sampling 411
    if(m_ccomp[k]->m_h_factor == 2 && m_ccomp[k]->m_v_factor == 2)
    {
      Ipp8u* src = m_ccomp[k]->m_cc_buffer;
      Ipp8u* dst = m_ccomp[k]->m_ss_buffer;
      IppiSize srcRoi = { m_ccWidth,    m_mcuHeight*m_numyMCU    };
      IppiSize dstRoi = { m_ccWidth>>1, m_mcuHeight*m_numyMCU>>1 };

      status = ippiSampleDownH2V2_JPEG_8u_C1R(src,m_ccWidth,srcRoi,dst,m_ccWidth,dstRoi);

      if(ippStsNoErr != status)
      {
        LOG1("IPP Error: ippiSampleDownH2V2_JPEG_8u_C1R() failed - ",status);
        return JPEG_INTERNAL_ERROR;
      }
    }
  } // for m_jpeg_ncomp

  return JPEG_OK;
} // CJPEGEncoder::DownSampling()


JERRCODE CJPEGEncoder::DownSampling(int nMCURow)
{
  int i, j, k;

  Ipp8u val;
  Ipp8u* p;
  Ipp8u* p1;

  for(k = 0; k < m_jpeg_ncomp; k++)
  {
    // expand right edge
    if(m_xPadding)
    {
      for(i = 0; i < m_ccHeight; i++)
      {
        p = m_ccomp[k]->m_cc_buffer + i*m_ccWidth;
        val = p[m_src.width - 1];
        for(j = 0; j < m_xPadding; j++)
        {
          p[m_src.width + j] = val;
        }
      }
    }

    // expand bottom edge only for last MCU row
    if(nMCURow == m_numyMCU - 1)
    {
      p = m_ccomp[k]->m_cc_buffer + (m_ccHeight-1)*m_ccWidth;

      for(i = 0; i < m_yPadding; i++)
      {
        p1 = m_ccomp[k]->m_cc_buffer + m_ccHeight*m_ccWidth + i*m_ccWidth;
        ippsCopy_8u(p,p1,m_ccWidth);
      }
    }

    // sampling 444
    if(m_ccomp[k]->m_h_factor == 1 && m_ccomp[k]->m_v_factor == 1)
    {
      Ipp8u* src = m_ccomp[k]->m_cc_buffer;
      Ipp8u* dst = m_ccomp[k]->m_ss_buffer;

      ippsCopy_8u(src,dst,m_ccWidth*m_mcuHeight);
    }

    IppStatus status;

    // sampling 422
    if(m_ccomp[k]->m_h_factor == 2 && m_ccomp[k]->m_v_factor == 1)
    {
      Ipp8u* src = m_ccomp[k]->m_cc_buffer;
      Ipp8u* dst = m_ccomp[k]->m_ss_buffer;
      IppiSize srcRoi = { m_ccWidth,    m_mcuHeight };
      IppiSize dstRoi = { m_ccWidth>>1, m_mcuHeight };

      ippsZero_8u(dst,m_ccWidth*m_mcuHeight);

      status = ippiSampleDownH2V1_JPEG_8u_C1R(src,m_ccWidth,srcRoi,dst,m_ccWidth,dstRoi);

      if(ippStsNoErr != status)
      {
        LOG1("IPP Error: ippiSampleDownH2V1_JPEG_8u_C1R() failed - ",status);
        return JPEG_INTERNAL_ERROR;
      }
    }

    // sampling 411
    if(m_ccomp[k]->m_h_factor == 2 && m_ccomp[k]->m_v_factor == 2)
    {
      Ipp8u* src = m_ccomp[k]->m_cc_buffer;
      Ipp8u* dst = m_ccomp[k]->m_ss_buffer;
      IppiSize srcRoi = { m_ccWidth,    m_mcuHeight    };
      IppiSize dstRoi = { m_ccWidth>>1, m_mcuHeight>>1 };

      status = ippiSampleDownH2V2_JPEG_8u_C1R(src,m_ccWidth,srcRoi,dst,m_ccWidth,dstRoi);

      if(ippStsNoErr != status)
      {
        LOG1("IPP Error: ippiSampleDownH2V2_JPEG_8u_C1R() failed - ",status);
        return JPEG_INTERNAL_ERROR;
      }
    }
  } // for m_jpeg_ncomp

  return JPEG_OK;
} // CJPEGEncoder::DownSampling()


JERRCODE CJPEGEncoder::PerformDCT(void)
{
  int i;
  int j;
  int n;
  int k;
  int l;
  int size;
  int src_step;
  Ipp8u* src;
  Ipp16u* qtbl;
  Ipp16s* block;
  IppStatus status;

  src_step = m_mcuWidth*m_numxMCU;

  for(size = 0, n = 0; n < m_jpeg_ncomp; n++)
  {
    size += (m_ccomp[n]->m_hsampling * m_ccomp[n]->m_vsampling);
  }


  for(i = 0; i < m_numyMCU; i++)
  {
    for(j = 0; j < m_numxMCU; j++)
    {
      block = m_coefbuf + (DCTSIZE2*size*(j+(i*m_numxMCU)));
      for(n = 0; n < m_jpeg_ncomp; n++)
      {
        for(k = 0; k < m_ccomp[n]->m_vsampling; k++)
        {
          for(l = 0; l < m_ccomp[n]->m_hsampling; l++)
          {
            qtbl = m_qntbl[m_ccomp[n]->m_q_selector];

            src  = m_ccomp[n]->m_ss_buffer +
                   i*8*m_ccomp[n]->m_vsampling*m_ccWidth +
                   j*8*m_ccomp[n]->m_hsampling +
                   k*8*m_ccWidth;

            src += l*8;

            status = ippiDCTQuantFwd8x8LS_JPEG_8u16s_C1R(
              src,
              src_step,
              block,
              qtbl);

            if(ippStsNoErr != status)
            {
              LOG0("Error: ippiDCTQuantFwd8x8LS_JPEG_8u16s_C1R() failed!");
              return JPEG_INTERNAL_ERROR;
            }

            block += DCTSIZE2;
          } // for m_hsampling
        } // for m_vsampling
      }
    }
  }

  return JPEG_OK;
} // CJPEGEncoder::PerformDCT()


JERRCODE CJPEGEncoder::GenerateHuffmanTables(
  int ncomp,
  int id[MAX_COMPS_PER_SCAN],
  int Ss,
  int Se,
  int Ah,
  int Al)
{
  int  i;
  int  j;
  int  k;
  int  n;
  int  l;
  int  c;
  int  size;
  int  dc_statistics[2][256];
  int  ac_statistics[2][256];
  JERRCODE jerr;
  IppStatus status;

  ippsZero_8u((Ipp8u*)dc_statistics,sizeof(dc_statistics));
  ippsZero_8u((Ipp8u*)ac_statistics,sizeof(ac_statistics));

  for(n = 0; n < m_jpeg_ncomp; n++)
  {
    m_ccomp[n]->m_lastDC = 0;
  }

  status = ippiEncodeHuffmanStateInit_JPEG_8u(m_state);
  if(ippStsNoErr != status)
  {
    return JPEG_INTERNAL_ERROR;
  }

  for(size = 0, k = 0; k < m_jpeg_ncomp; k++)
  {
    size += (m_ccomp[k]->m_hsampling * m_ccomp[k]->m_vsampling);
  }

  Ipp16s* block;

  if(Ss != 0 && Se != 0)
  {
    // AC scan
    for(i = 0; i < m_numyMCU; i++)
    {
      for(k = 0; k < m_ccomp[id[0]]->m_vsampling; k++)
      {
        if(i*m_ccomp[id[0]]->m_vsampling*8 + k*8 >= m_src.height)
          break;

        for(j = 0; j < m_numxMCU; j++)
        {
          block = m_coefbuf + (DCTSIZE2*size*(j+(i*m_numxMCU)));

          // skip any relevant components
          for(c = 0; c < m_ccomp[id[0]]->m_comp_no; c++)
          {
            block += (DCTSIZE2*m_ccomp[c]->m_hsampling*
                               m_ccomp[c]->m_vsampling);
          }

          // Skip over relevant 8x8 blocks from this component.
          block += (k * DCTSIZE2 * m_ccomp[id[0]]->m_hsampling);

          for(l = 0; l < m_ccomp[id[0]]->m_hsampling; l++)
          {
            if(m_jpeg_restart_interval)
            {
              if(m_restarts_to_go == 0)
              {
                jerr = ProcessRestart(ac_statistics,id,Ss,Se,Ah,Al);
                if(JPEG_OK != jerr)
                {
                  LOG0("Error: ProcessRestart() failed!");
                  return jerr;
                }
              }
            }

            // Ignore the last column(s) of the image.
            if(((j*m_ccomp[id[0]]->m_hsampling*8) + (l*8)) >= m_src.width)
              break;

            if(Ah == 0)
            {
              status = ippiGetHuffmanStatistics8x8_ACFirst_JPEG_16s_C1(
                block,
                &ac_statistics[m_ccomp[id[0]]->m_ac_selector][0],
                Ss,
                Se,
                Al,
                m_state,
                0);

              if(ippStsNoErr > status)
              {
                LOG0("Error: ippiGetHuffmanStatistics8x8_ACFirst_JPEG_16s_C1() failed!");
                return JPEG_INTERNAL_ERROR;
              }
            }
            else
            {
              status = ippiGetHuffmanStatistics8x8_ACRefine_JPEG_16s_C1(
                block,
                &ac_statistics[m_ccomp[id[0]]->m_ac_selector][0],
                Ss,
                Se,
                Al,
                m_state,
                0);

              if(ippStsNoErr > status)
              {
                LOG0("Error: ippiGetHuffmanStatistics8x8_ACRefine_JPEG_16s_C1() failed!");
                return JPEG_INTERNAL_ERROR;
              }
            }

            block += DCTSIZE2;
            m_restarts_to_go --;
          } // for m_hsampling
        } // for m_numxMCU
      } // for m_vsampling
    } // for m_numyMCU

    if(Ah == 0)
    {
      status = ippiGetHuffmanStatistics8x8_ACFirst_JPEG_16s_C1(
        0,
        ac_statistics[m_ccomp[id[0]]->m_ac_selector],
        Ss,
        Se,
        Al,
        m_state,
        1);

      if(ippStsNoErr > status)
      {
        LOG0("Error: ippiGetHuffmanStatistics8x8_ACFirst_JPEG_16s_C1() failed!");
        return JPEG_INTERNAL_ERROR;
      }
    }
    else
    {
      status = ippiGetHuffmanStatistics8x8_ACRefine_JPEG_16s_C1(
        0,
        ac_statistics[m_ccomp[id[0]]->m_ac_selector],
        Ss,
        Se,
        Al,
        m_state,
        1);

      if(ippStsNoErr > status)
      {
        LOG0("Error: ippiGetHuffmanStatistics8x8_ACRefine_JPEG_16s_C1() failed!");
        return JPEG_INTERNAL_ERROR;
      }
    }

    Ipp8u bits[16];
    Ipp8u vals[256];

    ippsZero_8u(bits,sizeof(bits));
    ippsZero_8u(vals,sizeof(vals));

    status = ippiEncodeHuffmanRawTableInit_JPEG_8u(
               &ac_statistics[m_ccomp[id[0]]->m_ac_selector][0],
               bits,
               vals);

    if(ippStsNoErr > status)
    {
      LOG0("Error: ippiEncodeHuffmanRawTableInit_JPEG_8u() failed!");
      return JPEG_INTERNAL_ERROR;
    }

    jerr = m_actbl[m_ccomp[id[0]]->m_ac_selector].Init(m_ccomp[id[0]]->m_ac_selector,1,bits,vals);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: can't init huffman table");
      return jerr;
    }
  }
  else
  {
    // DC scan
    if(Ah == 0)
    {
      for(i = 0; i < m_numyMCU; i++)
      {
        for(j = 0; j < m_numxMCU; j++)
        {
          if(m_jpeg_restart_interval)
          {
            if(m_restarts_to_go == 0)
            {
              jerr = ProcessRestart(dc_statistics,id,Ss,Se,Ah,Al);
              if(JPEG_OK != jerr)
              {
                LOG0("Error: ProcessRestart() failed!");
                return jerr;
              }
            }
          }

          block = m_coefbuf + (DCTSIZE2*size*(j+(i*m_numxMCU)));

          // first DC scan
          for(n = 0; n < m_jpeg_ncomp; n++)
          {
            Ipp16s* lastDC = &m_ccomp[n]->m_lastDC;

            for(k = 0; k < m_ccomp[n]->m_vsampling; k++)
            {
              for(l = 0; l < m_ccomp[n]->m_hsampling; l++)
              {
                status = ippiGetHuffmanStatistics8x8_DCFirst_JPEG_16s_C1(
                  block,
                  dc_statistics[m_ccomp[n]->m_dc_selector],
                  lastDC,
                  Al);

                if(ippStsNoErr > status)
                {
                  LOG0("Error: ippiGetHuffmanStatistics8x8_DCFirst_JPEG_16s_C1() failed!");
                  return JPEG_INTERNAL_ERROR;
                }

                block += DCTSIZE2;
              } // for m_hsampling
            } // for m_vsampling
          } // for m_jpeg_ncomp
          m_restarts_to_go --;
        } // for m_numxMCU
      } // for m_numyMCU

      for(n = 0; n < ncomp; n++)
      {
        Ipp8u bits[16];
        Ipp8u vals[256];

        ippsZero_8u(bits,sizeof(bits));
        ippsZero_8u(vals,sizeof(vals));

        status = ippiEncodeHuffmanRawTableInit_JPEG_8u(
                   dc_statistics[m_ccomp[n]->m_dc_selector],
                   bits,
                   vals);

        if(ippStsNoErr > status)
        {
          LOG0("Error: ippiEncodeHuffmanRawTableInit_JPEG_8u() failed!");
          return JPEG_INTERNAL_ERROR;
        }

        jerr = m_dctbl[m_ccomp[n]->m_dc_selector].Init(m_ccomp[n]->m_dc_selector,0,bits,vals);
        if(JPEG_OK != jerr)
        {
          LOG0("Error: can't init huffman table");
          return jerr;
        }
      }
    } // Ah == 0
  }

  return JPEG_OK;
} // CJPEGEncoder::GenerateHuffmanTables()


JERRCODE CJPEGEncoder::EncodeScan(
  int ncomp,
  int id[MAX_COMPS_PER_SCAN],
  int Ss,
  int Se,
  int Ah,
  int Al)
{
  int  i;
  int  j;
  int  k;
  int  n;
  int  l;
  int  c;
  int  size;
  Ipp16s* block;
  JERRCODE jerr;
  IppStatus status;

  GenerateHuffmanTables(ncomp,id,Ss,Se,Ah,Al);

  for(n = 0; n < m_jpeg_ncomp; n++)
  {
    m_ccomp[n]->m_lastDC = 0;
  }

  m_restarts_to_go = m_jpeg_restart_interval;

  status = ippiEncodeHuffmanStateInit_JPEG_8u(m_state);
  if(ippStsNoErr != status)
  {
    return JPEG_INTERNAL_ERROR;
  }

  for(size = 0, k = 0; k < m_jpeg_ncomp; k++)
  {
    size += (m_ccomp[k]->m_hsampling * m_ccomp[k]->m_vsampling);
  }

  if(Ss != 0 && Se != 0)
  {
    jerr = WriteDHT(&m_actbl[m_ccomp[id[0]]->m_ac_selector]);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDHT() failed");
      return jerr;
    }

    jerr = WriteSOS(ncomp,id,Ss,Se,Ah,Al);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteSOS() failed");
      return jerr;
    }

    // AC scan
    for(i = 0; i < m_numyMCU; i++)
    {
      for(k = 0; k < m_ccomp[id[0]]->m_vsampling; k++)
      {
        if(i*m_ccomp[id[0]]->m_vsampling*8 + k*8 >= m_src.height)
          break;

        for(j = 0; j < m_numxMCU; j++)
        {
          block = m_coefbuf + (DCTSIZE2*size*(j+(i*m_numxMCU)));

          // skip any relevant components
          for(c = 0; c < m_ccomp[id[0]]->m_comp_no; c++)
          {
            block += (DCTSIZE2*m_ccomp[c]->m_hsampling*
                               m_ccomp[c]->m_vsampling);
          }

          // Skip over relevant 8x8 blocks from this component.
          block += (k * DCTSIZE2 * m_ccomp[id[0]]->m_hsampling);

          for(l = 0; l < m_ccomp[id[0]]->m_hsampling; l++)
          {
            // Ignore the last column(s) of the image.
            if(((j*m_ccomp[id[0]]->m_hsampling*8) + (l*8)) >= m_src.width)
              break;

            if(m_jpeg_restart_interval)
            {
              if(m_restarts_to_go == 0)
              {
                jerr = ProcessRestart(id,Ss,Se,Ah,Al);
                if(JPEG_OK != jerr)
                {
                  LOG0("Error: ProcessRestart() failed!");
                  return jerr;
                }
              }
            }

            IppiEncodeHuffmanSpec* actbl = m_actbl[m_ccomp[id[0]]->m_ac_selector];

            if(Ah == 0)
            {
              status = ippiEncodeHuffman8x8_ACFirst_JPEG_16s1u_C1(
                block,
                m_dst.pData,
                m_dst.DataLen,
                &m_dst.currPos,
                Ss,
                Se,
                Al,
                actbl,
                m_state,
                0);

              if(ippStsNoErr > status)
              {
                LOG1("Error: ippiEncodeHuffman8x8_ACFirst_JPEG_16s1u_C1() failed!",ippGetStatusString(status));
                return JPEG_INTERNAL_ERROR;
              }
            }
            else
            {
              status = ippiEncodeHuffman8x8_ACRefine_JPEG_16s1u_C1(
                block,
                m_dst.pData,
                m_dst.DataLen,
                &m_dst.currPos,
                Ss,
                Se,
                Al,
                actbl,
                m_state,
                0);

              if(ippStsNoErr > status)
              {
                LOG1("Error: ippiEncodeHuffman8x8_ACRefine_JPEG_16s1u_C1() failed!",ippGetStatusString(status));
                return JPEG_INTERNAL_ERROR;
              }
            }

            block += DCTSIZE2;

            m_restarts_to_go --;
          } // for m_hsampling
        } // for m_numxMCU
      } // for m_vsampling
    } // for m_numyMCU

    IppiEncodeHuffmanSpec* actbl = m_actbl[m_ccomp[id[0]]->m_ac_selector];

    if(Ah == 0)
    {
      status = ippiEncodeHuffman8x8_ACFirst_JPEG_16s1u_C1(
        0,
        m_dst.pData,
        m_dst.DataLen,
        &m_dst.currPos,
        Ss,
        Se,
        Al,
        actbl,
        m_state,
        1);

      if(ippStsNoErr > status)
      {
        LOG0("Error: ippiEncodeHuffman8x8_ACFirst_JPEG_16s1u_C1() failed!");
        return JPEG_INTERNAL_ERROR;
      }
    }
    else
    {
      status = ippiEncodeHuffman8x8_ACRefine_JPEG_16s1u_C1(
        0,
        m_dst.pData,
        m_dst.DataLen,
        &m_dst.currPos,
        Ss,
        Se,
        Al,
        actbl,
        m_state,
        1);

      if(ippStsNoErr > status)
      {
        LOG0("Error: ippiEncodeHuffman8x8_ACRefine_JPEG_16s1u_C1() failed!");
        return JPEG_INTERNAL_ERROR;
      }
    }
  }
  else
  {
    if(Ah == 0)
    {
      jerr = WriteDHT(&m_dctbl[0]);
      if(JPEG_OK != jerr)
      {
        LOG0("Error: WriteDHT() failed");
        return jerr;
      }

      if(m_jpeg_ncomp != 1)
      {
        jerr = WriteDHT(&m_dctbl[1]);
        if(JPEG_OK != jerr)
        {
          LOG0("Error: WriteDHT() failed");
          return jerr;
        }
      }
    }

    jerr = WriteSOS(ncomp,id,Ss,Se,Ah,Al);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteSOS() failed");
      return jerr;
    }

    // DC scan
    for(i = 0; i < m_numyMCU; i++)
    {
      for(j = 0; j < m_numxMCU; j++)
      {
        if(m_jpeg_restart_interval)
        {
          if(m_restarts_to_go == 0)
          {
            jerr = ProcessRestart(id,Ss,Se,Ah,Al);
            if(JPEG_OK != jerr)
            {
              LOG0("Error: ProcessRestart() failed!");
              return jerr;
            }
          }
        }

        block = m_coefbuf + (DCTSIZE2*size*(j+(i*m_numxMCU)));

        if(Ah == 0)
        {
          // first DC scan
          for(n = 0; n < m_jpeg_ncomp; n++)
          {
            Ipp16s* lastDC = &m_ccomp[n]->m_lastDC;
            IppiEncodeHuffmanSpec* dctbl = m_dctbl[m_ccomp[n]->m_dc_selector];

            for(k = 0; k < m_ccomp[n]->m_vsampling; k++)
            {
              for(l = 0; l < m_ccomp[n]->m_hsampling; l++)
              {
                status = ippiEncodeHuffman8x8_DCFirst_JPEG_16s1u_C1(
                  block,
                  m_dst.pData,
                  m_dst.DataLen,
                  &m_dst.currPos,
                  lastDC,
                  Al,
                  dctbl,
                  m_state,
                  0);

                if(ippStsNoErr > status)
                {
                  LOG1("Error: ippiEncodeHuffman8x8_DCFirst_JPEG_16s1u_C1() failed!",ippGetStatusString(status));
                  return JPEG_INTERNAL_ERROR;
                }

                block += DCTSIZE2;
              } // for m_hsampling
            } // for m_vsampling
          } // for m_jpeg_ncomp
        }
        else
        {
          // refine DC scan
          for(n = 0; n < m_jpeg_ncomp; n++)
          {
            for(k = 0; k < m_ccomp[n]->m_vsampling; k++)
            {
              for(l = 0; l < m_ccomp[n]->m_hsampling; l++)
              {
                status = ippiEncodeHuffman8x8_DCRefine_JPEG_16s1u_C1(
                  block,
                  m_dst.pData,
                  m_dst.DataLen,
                  &m_dst.currPos,
                  Al,
                  m_state,
                  0);

                if(ippStsNoErr > status)
                {
                  LOG0("Error: ippiEncodeHuffman8x8_DCRefine_JPEG_16s1u_C1() failed!");
                  return JPEG_INTERNAL_ERROR;
                }

                block += DCTSIZE2;
              } // for m_hsampling
            } // for m_vsampling
          } // for m_jpeg_ncomp
        }
        m_restarts_to_go --;
      } // for m_numxMCU
    } // for m_numyMCU

    if(Ah == 0)
    {
      status = ippiEncodeHuffman8x8_DCFirst_JPEG_16s1u_C1(
                 0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,0,0,0,m_state,1);

      if(ippStsNoErr > status)
      {
        LOG0("Error: ippiEncodeHuffman8x8_DCFirst_JPEG_16s1u_C1() failed!");
        return JPEG_INTERNAL_ERROR;
      }
    }
    else
    {
      status = ippiEncodeHuffman8x8_DCRefine_JPEG_16s1u_C1(
                 0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,0,m_state,1);

      if(ippStsNoErr > status)
      {
        LOG0("Error: ippiEncodeHuffman8x8_DCRefine_JPEG_16s1u_C1() failed!");
        return JPEG_INTERNAL_ERROR;
      }
    }
  }

  return JPEG_OK;
} // CJPEGEncoder::EncodeScan()


JERRCODE CJPEGEncoder::WriteImageBaseline(void)
{
  JERRCODE jerr;

  jerr = Init();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: can't init encoder");
    return jerr;
  }

  jerr = WriteSOI();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOI() failed");
    return jerr;
  }

  if(m_jpeg_color == JC_GRAY || m_jpeg_color == JC_YCBCR)
  {
    jerr = WriteAPP0();
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteAPP0() failed");
      return jerr;
    }
  }

  if(m_jpeg_color == JC_RGB || m_jpeg_color == JC_CMYK || m_jpeg_color == JC_YCCK)
  {
    jerr = WriteAPP14();
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteAPP14() failed");
      return jerr;
    }
  }

  jerr = WriteCOM();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteCOM() failed");
    return jerr;
  }

  jerr = WriteDQT(&m_qntbl[0]);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteDQT() failed");
    return jerr;
  }

  if(m_jpeg_ncomp != 1)
  {
    jerr = WriteDQT(&m_qntbl[1]);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDQT() failed");
      return jerr;
    }
  }

  jerr = WriteSOF0();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOF0() failed");
    return jerr;
  }

  jerr = WriteDHT(&m_dctbl[0]);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteDHT() failed");
    return jerr;
  }

  if(m_jpeg_ncomp != 1)
  {
    jerr = WriteDHT(&m_dctbl[1]);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDHT() failed");
      return jerr;
    }
  }

  jerr = WriteDHT(&m_actbl[0]);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteDHT() failed");
    return jerr;
  }

  if(m_jpeg_ncomp != 1)
  {
    jerr = WriteDHT(&m_actbl[1]);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDHT() failed");
      return jerr;
    }
  }

  if(m_jpeg_restart_interval)
  {
    jerr = WriteDRI(m_jpeg_restart_interval);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDRI() failed");
      return jerr;
    }
  }

  jerr = WriteSOS();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOS() failed");
    return jerr;
  }

  IppStatus status;
  IppiEncodeHuffmanSpec* pDCTbl = 0;
  IppiEncodeHuffmanSpec* pACTbl = 0;
  Ipp8u   tmp[DCTSIZE2*sizeof(Ipp16s)+(CPU_CACHE_LINE-1)];
  Ipp16s* block = (Ipp16s*)OWN_ALIGN_PTR(&tmp[0],CPU_CACHE_LINE);

  status = ippiEncodeHuffmanStateInit_JPEG_8u(m_state);
  if(ippStsNoErr != status)
  {
    return JPEG_INTERNAL_ERROR;
  }

  for(int i = 0; i < m_numyMCU; i++)
  {
    jerr = ColorConvert(i);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: ColorConvert() failed");
      return jerr;
    }

    jerr = DownSampling(i);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: DownSampling() failed");
      return jerr;
    }

    Ipp8u* src = 0;
    int src_step = m_ccWidth;

    for(int j = 0; j < m_numxMCU; j++)
    {
      // process restart interval, if any
      if(m_jpeg_restart_interval)
      {
        if(m_restarts_to_go == 0)
        {
          ProcessRestart(0,0,63,0,0);
        }
      }

      for(int n = 0; n < m_jpeg_ncomp; n++)
      {
        Ipp16u* qtbl = m_qntbl[m_ccomp[n]->m_q_selector];

        pDCTbl = m_dctbl[m_ccomp[n]->m_dc_selector];
        pACTbl = m_actbl[m_ccomp[n]->m_ac_selector];

        for(int k = 0; k < m_ccomp[n]->m_vsampling; k++)
        {
          src = m_ccomp[n]->m_ss_buffer + j*8*m_ccomp[n]->m_hsampling + k*8*m_ccWidth;

          for(int l = 0; l < m_ccomp[n]->m_hsampling; l++)
          {
            src += l*8;
            status = ippiDCTQuantFwd8x8LS_JPEG_8u16s_C1R(
              src,
              src_step,
              block,
              qtbl);

            if(ippStsNoErr > status)
            {
              LOG1("IPP Error: ippiDCTQuantFwd8x8LS_JPEG_8u16s_C1R() failed - ",status);
              return JPEG_INTERNAL_ERROR;
            }

            status = ippiEncodeHuffman8x8_JPEG_16s1u_C1(
              block,
              m_dst.pData,
              m_dst.DataLen,
              &m_dst.currPos,
              &m_ccomp[n]->m_lastDC,
              pDCTbl,
              pACTbl,
              m_state,
              0);

            if(ippStsNoErr > status)
            {
              LOG1("IPP Error: ippiEncodeHuffman8x8_JPEG_16s1u_C1() failed - ",status);
              return JPEG_INTERNAL_ERROR;
            }
          } // for m_hsampling
        } // for m_vsampling
      } // for m_jpeg_ncomp

      if(m_jpeg_restart_interval)
      {
        if(m_restarts_to_go == 0)
        {
          m_restarts_to_go = m_jpeg_restart_interval;
        }
        m_restarts_to_go --;
      }
    } // for numxMCU
  } // for numyMCU

  // flush IppiEncodeHuffmanState
  status = ippiEncodeHuffman8x8_JPEG_16s1u_C1(
             0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,0,0,0,m_state,1);

  if(ippStsNoErr > status)
  {
    LOG1("IPP Error: ippiEncodeHuffman8x8_JPEG_16s1u_C1() failed - ",status);
    return JPEG_INTERNAL_ERROR;
  }

  jerr = WriteEOI();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteEOI() failed");
    return jerr;
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteImageBaseline()


JERRCODE CJPEGEncoder::WriteImageProgressive(void)
{
  int i;
  JERRCODE jerr;

  jerr = Init();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: can't init encoder");
    return jerr;
  }

  jerr = WriteSOI();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOI() failed");
    return jerr;
  }

  if(m_jpeg_color == JC_GRAY || m_jpeg_color == JC_YCBCR)
  {
    jerr = WriteAPP0();
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteAPP0() failed");
      return jerr;
    }
  }

  if(m_jpeg_color == JC_RGB || m_jpeg_color == JC_CMYK || m_jpeg_color == JC_YCCK)
  {
    jerr = WriteAPP14();
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteAPP14() failed");
      return jerr;
    }
  }

  jerr = WriteCOM();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteCOM() failed");
    return jerr;
  }

  jerr = WriteDQT(&m_qntbl[0]);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteDQT() failed");
    return jerr;
  }

  if(m_jpeg_ncomp != 1 && m_jpeg_color != JC_RGB && m_jpeg_color != JC_CMYK && m_jpeg_color != JC_UNKNOWN)
  {
    jerr = WriteDQT(&m_qntbl[1]);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDQT() failed");
      return jerr;
    }
  }

  jerr = WriteSOF2();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOF2() failed");
    return jerr;
  }


  jerr = ColorConvert();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: ColorConvert() failed");
    return jerr;
  }

  jerr = DownSampling();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: DownSampling() failed");
    return jerr;
  }

  jerr = PerformDCT();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: PerformDCT() failed");
    return jerr;
  }

  if(m_jpeg_restart_interval)
  {
    jerr = WriteDRI(m_jpeg_restart_interval);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDRI() failed");
      return jerr;
    }
  }

  for(i = 0; i < m_scan_count; i++)
  {
    m_next_restart_num = 0;
    m_restarts_to_go = m_jpeg_restart_interval;

    jerr = EncodeScan(
      m_scan_script[i].ncomp,
      m_scan_script[i].id,
      m_scan_script[i].Ss,
      m_scan_script[i].Se,
      m_scan_script[i].Ah,
      m_scan_script[i].Al);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: EncodeScan() failed");
      return jerr;
    }
  }

  jerr = WriteEOI();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteEOI() failed");
    return jerr;
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteImageProgressive()


JERRCODE CJPEGEncoder::WriteImageLossless(void)
{
  int      i, j;
  JERRCODE jerr;

  jerr = Init();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: can't init encoder");
    return jerr;
  }

  if(m_jpeg_ncomp != 1)
  {
    return JPEG_NOT_IMPLEMENTED;
  }

  m_ss = m_predictor;
  m_se = 0;
  m_ah = 0;
  m_al = m_pt;

  jerr = WriteSOI();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOI() failed");
    return jerr;
  }

  if(m_jpeg_color == JC_GRAY)
  {
    jerr = WriteAPP0();
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteAPP0() failed");
      return jerr;
    }
  }

  jerr = WriteCOM();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteCOM() failed");
    return jerr;
  }

  jerr = WriteSOF3();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOF3() failed");
    return jerr;
  }

  jerr = WriteDHT(&m_dctbl[0]);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteDHT() failed");
    return jerr;
  }

  if(m_jpeg_restart_interval)
  {
    return JPEG_NOT_IMPLEMENTED;
  }

  jerr = WriteSOS();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOS() failed");
    return jerr;
  }

  Ipp8u*                 pSrc;
  Ipp16s*                pMCUBuf;
  Ipp16s*                pCurrRow;
  Ipp16s*                pPrevRow;
  IppiEncodeHuffmanSpec* pDCTbl;
  IppiSize               roi;
  IppStatus              status;
  int                    stat[256];
  Ipp8u                  bits[16];
  Ipp8u                  vals[256];

  roi.width  = m_src.width;
  roi.height = 1;

  ippsZero_8u((Ipp8u*)stat,256*sizeof(int));
  ippsZero_8u(bits,16);
  ippsZero_8u(vals,256);

  status = ippiEncodeHuffmanStateInit_JPEG_8u(m_state);
  if(ippStsNoErr != status)
  {
    return JPEG_INTERNAL_ERROR;
  }

  for(i = 0; i < m_numyMCU; i++)
  {
    pSrc     = m_src.p.Data8u + i*m_src.lineStep;
    pMCUBuf  = (Ipp16s*)m_ccomp[0]->m_top_row;

    pCurrRow = m_ccomp[0]->m_curr_row;
    pPrevRow = m_ccomp[0]->m_prev_row;

    status = ippiConvert_8u16s_C1R(pSrc,m_src.lineStep,pCurrRow,m_numxMCU,roi);
    if(ippStsNoErr > status)
    {
      LOG1("IPP Error: ippiConvert_8u16s_C1R() failed - ",status);
      return JPEG_INTERNAL_ERROR;
    }

    if(m_al)
    {
      // do point transform
      ippsRShiftC_16s_I(m_al,pCurrRow,m_numxMCU);
    }

    if(i != 0)
    {
      m_al = 0;
      m_ss = 1;
      status = ippiDiffPredRow_JPEG_16s_C1(pCurrRow,pPrevRow,pMCUBuf,m_src.width,m_ss);
      if(ippStsNoErr > status)
      {
        LOG1("IPP Error: ippiDiffPredRow_JPEG_16s_C1() failed - ",status);
        return JPEG_INTERNAL_ERROR;
      }
    }
    else
    {
      m_al = 0;
      status = ippiDiffPredFirstRow_JPEG_16s_C1(pCurrRow,pMCUBuf,m_src.width,m_jpeg_precision,m_al);
      if(ippStsNoErr > status)
      {
        LOG1("IPP Error: ippiDiffPredFirstRow_JPEG_16s_C1() failed - ",status);
        return JPEG_INTERNAL_ERROR;
      }
    }

    m_ccomp[0]->m_curr_row = pPrevRow;
    m_ccomp[0]->m_prev_row = pCurrRow;

    for(j = 0; j < m_numxMCU; j++)
    {
      pDCTbl = m_dctbl[m_ccomp[0]->m_dc_selector];

      status = ippiEncodeHuffmanOne_JPEG_16s1u_C1(
        pMCUBuf,
        m_dst.pData,
        m_dst.DataLen,
        &m_dst.currPos,
        pDCTbl,
        m_state,
        0);

      if(ippStsNoErr > status)
      {
        LOG1("IPP Error: ippiEncodeHuffmanOne_JPEG_16s1u_C1() failed - ",status);
        return JPEG_INTERNAL_ERROR;
      }

      pMCUBuf++;
    } // for numxMCU
  }

  // flush IppiEncodeHuffmanState
  status = ippiEncodeHuffmanOne_JPEG_16s1u_C1(
             0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,0,m_state,1);

  if(ippStsNoErr > status)
  {
    LOG1("IPP Error: ippiEncodeHuffmanOne_JPEG_16s1u_C1() failed - ",status);
    return JPEG_INTERNAL_ERROR;
  }

  jerr = WriteEOI();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteEOI() failed");
    return jerr;
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteImageLossless()

/*
JERRCODE CJPEGEncoder::WriteHeader(void)
{
  JERRCODE jerr;

  jerr = Init();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: can't init encoder");
    return jerr;
  }

  jerr = WriteSOI();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOI() failed");
    return jerr;
  }

  if(m_jpeg_color == JC_GRAY || m_jpeg_color == JC_YCBCR)
  {
    jerr = WriteAPP0();
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteAPP0() failed");
      return jerr;
    }
  }

  if(m_jpeg_color == JC_RGB || m_jpeg_color == JC_CMYK || m_jpeg_color == JC_YCCK)
  {
    jerr = WriteAPP14();
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteAPP14() failed");
      return jerr;
    }
  }

  jerr = WriteCOM();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteCOM() failed");
    return jerr;
  }

  jerr = WriteDQT(&m_qntbl[0]);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteDQT() failed");
    return jerr;
  }

  if(m_jpeg_ncomp != 1)
  {
    jerr = WriteDQT(&m_qntbl[1]);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDQT() failed");
      return jerr;
    }
  }

  switch(m_jpeg_mode)
  {
  case JPEG_BASELINE:
    jerr = WriteSOF0();
    break;

  case JPEG_PROGRESSIVE:
    jerr = WriteSOF2();
    break;

  case JPEG_LOSSLESS:
    jerr = WriteSOF3();
    break;
  }

  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOFx() failed");
    return jerr;
  }

  jerr = WriteDHT(&m_dctbl[0]);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteDHT() failed");
    return jerr;
  }

  if(m_jpeg_ncomp != 1)
  {
    jerr = WriteDHT(&m_dctbl[1]);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDHT() failed");
      return jerr;
    }
  }

  jerr = WriteDHT(&m_actbl[0]);
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteDHT() failed");
    return jerr;
  }

  if(m_jpeg_ncomp != 1)
  {
    jerr = WriteDHT(&m_actbl[1]);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDHT() failed");
      return jerr;
    }
  }

  if(m_jpeg_restart_interval)
  {
    jerr = WriteDRI(m_jpeg_restart_interval);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: WriteDRI() failed");
      return jerr;
    }
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteHeader()


JERRCODE CJPEGEncoder::WriteData(void)
{
  JERRCODE  jerr;
  IppStatus status;

  jerr = WriteSOS();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteSOS() failed");
    return jerr;
  }

  IppiEncodeHuffmanSpec* dctbl = 0;
  IppiEncodeHuffmanSpec* actbl = 0;
  Ipp8u   tmp[DCTSIZE2*sizeof(Ipp16s)+(CPU_CACHE_LINE-1)];
  Ipp16s* block = (Ipp16s*)OWN_ALIGN_PTR(&tmp[0],CPU_CACHE_LINE);

  for(int i = 0; i < m_numyMCU; i++)
  {
    jerr = ColorConvert(i);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: ColorConvert() failed");
      return jerr;
    }

    jerr = DownSampling(i);
    if(JPEG_OK != jerr)
    {
      LOG0("Error: DownSampling() failed");
      return jerr;
    }

    Ipp8u* src = 0;
    int src_step = m_ccWidth;

    for(int j = 0; j < m_numxMCU; j++)
    {
      // process restart interval, if any
      if(m_jpeg_restart_interval)
      {
        if(m_restarts_to_go == 0)
        {
          ProcessRestart(0,0,63,0,0);
        }
      }

      for(int n = 0; n < m_jpeg_ncomp; n++)
      {
        Ipp16u* qtbl = m_qntbl[m_ccomp[n]->m_q_selector];

        dctbl = m_dctbl[m_ccomp[n]->m_dc_selector];
        actbl = m_actbl[m_ccomp[n]->m_ac_selector];

        for(int k = 0; k < m_ccomp[n]->m_vsampling; k++)
        {
          src = m_ccomp[n]->m_ss_buffer + j*8*m_ccomp[n]->m_hsampling + k*8*m_ccWidth;

          for(int l = 0; l < m_ccomp[n]->m_hsampling; l++)
          {
            src += l*8;
            status = ippiDCTQuantFwd8x8LS_JPEG_8u16s_C1R(
              src,
              src_step,
              block,
              qtbl);

            if(ippStsNoErr > status)
            {
              LOG1("IPP Error: ippiDCTQuantFwd8x8LS_JPEG_8u16s_C1R() failed - ",status);
              return JPEG_INTERNAL_ERROR;
            }

            status = ippiEncodeHuffman8x8_JPEG_16s1u_C1(
              block,
              m_dst.pData,
              m_dst.DataLen,
              &m_dst.currPos,
              &m_ccomp[n]->m_lastDC,
              dctbl,
              actbl,
              m_state,
              0);

            if(ippStsNoErr > status)
            {
              LOG1("IPP Error: ippiEncodeHuffman8x8_JPEG_16s1u_C1() failed - ",status);
              return JPEG_INTERNAL_ERROR;
            }
          } // for m_hsampling
        } // for m_vsampling
      } // for m_jpeg_ncomp

      if(m_jpeg_restart_interval)
      {
        if(m_restarts_to_go == 0)
        {
          m_restarts_to_go = m_jpeg_restart_interval;
        }
        m_restarts_to_go --;
      }
    } // for numxMCU
  } // for numyMCU

  // flush IppiEncodeHuffmanState
  status = ippiEncodeHuffman8x8_JPEG_16s1u_C1(
             0,m_dst.pData,m_dst.DataLen,&m_dst.currPos,0,0,0,m_state,1);

  if(ippStsNoErr > status)
  {
    LOG1("IPP Error: ippiEncodeHuffman8x8_JPEG_16s1u_C1() failed - ",status);
    return JPEG_INTERNAL_ERROR;
  }

  jerr = WriteEOI();
  if(JPEG_OK != jerr)
  {
    LOG0("Error: WriteEOI() failed");
    return jerr;
  }

  return JPEG_OK;
} // CJPEGEncoder::WriteData()
*/
