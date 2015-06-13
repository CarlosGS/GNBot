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
#ifndef __DECODER_H__
#include "decoder.h"
#endif

#ifdef _OPENMP
#include <omp.h>
#endif




static int get_num_threads(void)//OMP
{
	int maxThreads = 1;
#ifdef _OPENMP
	kmp_set_blocktime(0);
#pragma omp parallel shared(maxThreads)
	{
#pragma omp master
		{
			maxThreads = omp_get_num_threads();
		}
	}
#endif
	return maxThreads;
}


static void set_num_threads(int maxThreads)//OMP
{
	maxThreads = maxThreads;
#ifdef _OPENMP
	omp_set_num_threads(maxThreads);
#endif
	return;
}


CJPEGDecoder::CJPEGDecoder(void)
{
	Reset();
	return;
} // ctor


CJPEGDecoder::~CJPEGDecoder(void)
{
	//printf("~CJPEGDecoder\n");
	Clean();
	return;
} // dtor


void CJPEGDecoder::Reset(void)
{
	m_src.pData      = 0;
	m_src.DataLen    = 0;

	m_jpeg_width     = 0;
	m_jpeg_height    = 0;
	m_jpeg_ncomp     = 0;
	m_jpeg_precision = 8;
	m_jpeg_sampling  = JS_444;
	m_jpeg_color     = JC_UNKNOWN;
	m_jpeg_quality   = 100;
	m_jpeg_restart_interval = 0;

	m_jpeg_comment_detected = 0;
	m_jpeg_comment          = 0;
	m_jpeg_comment_size     = 0;

	m_precision = 8;
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

	m_sos_len  = 0;

	m_curr_comp_no = 0;

	m_ss = 0;
	m_se = 0;
	m_al = 0;
	m_ah = 0;

	m_jpeg_mode = JPEG_BASELINE;

	m_dc_scan_completed  = 0;
	m_ac_scans_completed = 0;
	m_scan_count         = 0;

	m_coefbuf = 0;

	m_marker = JM_NONE;

	m_ccomp[0] = 0;
	m_ccomp[1] = 0;
	m_ccomp[2] = 0;
	m_ccomp[3] = 0;

	m_jfif_app0_detected     = 0;
	m_jfif_app0_major        = 0;
	m_jfif_app0_minor        = 0;
	m_jfif_app0_units        = 0;
	m_jfif_app0_xDensity     = 0;
	m_jfif_app0_yDensity     = 0;
	m_jfif_app0_thumb_width  = 0;
	m_jfif_app0_thumb_height = 0;

	m_jfxx_app0_detected   = 0;
	m_jfxx_thumbnails_type = 0;

	m_adobe_app14_detected  = 0;
	m_adobe_app14_version   = 0;
	m_adobe_app14_flags0    = 0;
	m_adobe_app14_flags1    = 0;
	m_adobe_app14_transform = 0;

	m_block_buffer = NULL; //OMP
	m_nblock       = 1;    //OMP

#ifdef __TIMING__
	m_clk_diff = 0;
	m_clk_huff = 0;
#endif

	return;
} // CJPEGDecoder::Reset(void)


JERRCODE CJPEGDecoder::Clean(void)
{
	int i;

	for(i = 0; i < MAX_COMPS_PER_SCAN; i++)
	{
		if(0 != m_ccomp[i])
		{
			delete m_ccomp[i];
			m_ccomp[i] = 0;
		}
	}

	if(JPEG_PROGRESSIVE == m_jpeg_mode)
	{
		if(0 != m_coefbuf)
		{
			ippFree(m_coefbuf);
			m_coefbuf = 0;
		}
	}

	if(0 != m_jpeg_comment)
	{
		delete [] m_jpeg_comment;
		m_jpeg_comment = 0;
	}

	if(0 != m_block_buffer)
	{
		ippFree(m_block_buffer);
		m_block_buffer = NULL;
	}

	return JPEG_OK;
} // CJPEGDecoder::Clean()


JERRCODE CJPEGDecoder::SetSource(
		Ipp8u* pSrc,
		int    srcSize)
{
	m_src.pData    = pSrc;
	m_src.DataLen  = srcSize;
	m_src.currPos  = 0;

	return JPEG_OK;
} // CJPEGDecoder::SetSource()


JERRCODE CJPEGDecoder::SetDestination(
		Ipp8u*   pDst,
		int      dstStep,
		IppiSize dstSize,
		int      dstChannels,
		JCOLOR   dstColor,
		int      dstPrecision)
{
	m_dst.p.Data8u    = pDst;
	m_dst.lineStep    = dstStep;
	m_dst.width       = dstSize.width;
	m_dst.height      = dstSize.height;
	m_dst.nChannels   = dstChannels;
	m_dst.color       = dstColor;
	m_dst.precision   = dstPrecision;

	return JPEG_OK;
} // CJPEGDecoder::SetDestination()


JERRCODE CJPEGDecoder::SetDestination(
		Ipp16s*  pDst,
		int      dstStep,
		IppiSize dstSize,
		int      dstChannels,
		JCOLOR   dstColor,
		int      dstPrecision)
{
	m_dst.p.Data16s    = pDst;
	m_dst.lineStep     = dstStep;
	m_dst.width        = dstSize.width;
	m_dst.height       = dstSize.height;
	m_dst.nChannels    = dstChannels;
	m_dst.color        = dstColor;
	m_dst.precision    = dstPrecision;

	return JPEG_OK;
} // CJPEGDecoder::SetDestination()




JERRCODE CJPEGDecoder::_set_sampling(void)
{
	switch(m_jpeg_ncomp)
	{
		case 1:
			if(m_ccomp[0]->m_hsampling == 1 && m_ccomp[0]->m_vsampling == 1)
			{
				m_jpeg_sampling = JS_444;
			}
			else
			{
				return JPEG_BAD_SAMPLING;
			}
			break;

		case 3:
			if(m_ccomp[0]->m_hsampling == 1 && m_ccomp[0]->m_vsampling == 1 &&
					m_ccomp[1]->m_hsampling == 1 && m_ccomp[1]->m_vsampling == 1 &&
					m_ccomp[2]->m_hsampling == 1 && m_ccomp[2]->m_vsampling == 1)
			{
				m_jpeg_sampling = JS_444;
			}
			else if(m_ccomp[0]->m_hsampling == 2 && m_ccomp[0]->m_vsampling == 1 &&
					m_ccomp[1]->m_hsampling == 1 && m_ccomp[1]->m_vsampling == 1 &&
					m_ccomp[2]->m_hsampling == 1 && m_ccomp[2]->m_vsampling == 1)
			{
				m_jpeg_sampling = JS_422;
			}
			else if(m_ccomp[0]->m_hsampling == 2 && m_ccomp[0]->m_vsampling == 2 &&
					m_ccomp[1]->m_hsampling == 1 && m_ccomp[1]->m_vsampling == 1 &&
					m_ccomp[2]->m_hsampling == 1 && m_ccomp[2]->m_vsampling == 1)
			{
				m_jpeg_sampling = JS_411;
			}
			else
			{
				m_jpeg_sampling = JS_OTHER;
			}
			break;

		case 4:
			if(m_ccomp[0]->m_hsampling == 1 && m_ccomp[0]->m_vsampling == 1 &&
					m_ccomp[1]->m_hsampling == 1 && m_ccomp[1]->m_vsampling == 1 &&
					m_ccomp[2]->m_hsampling == 1 && m_ccomp[2]->m_vsampling == 1 &&
					m_ccomp[3]->m_hsampling == 1 && m_ccomp[3]->m_vsampling == 1)
			{
				m_jpeg_sampling = JS_444;
			}
			else if(m_ccomp[0]->m_hsampling == 2 && m_ccomp[0]->m_vsampling == 1 &&
					m_ccomp[1]->m_hsampling == 1 && m_ccomp[1]->m_vsampling == 1 &&
					m_ccomp[2]->m_hsampling == 1 && m_ccomp[2]->m_vsampling == 1 &&
					m_ccomp[3]->m_hsampling == 2 && m_ccomp[3]->m_vsampling == 1)
			{
				m_jpeg_sampling = JS_422;
			}
			else if(m_ccomp[0]->m_hsampling == 2 && m_ccomp[0]->m_vsampling == 2 &&
					m_ccomp[1]->m_hsampling == 1 && m_ccomp[1]->m_vsampling == 1 &&
					m_ccomp[2]->m_hsampling == 1 && m_ccomp[2]->m_vsampling == 1 &&
					m_ccomp[3]->m_hsampling == 2 && m_ccomp[3]->m_vsampling == 2)
			{
				m_jpeg_sampling = JS_411;
			}
			else
			{
				m_jpeg_sampling = JS_OTHER;
			}
			break;
	}

	return JPEG_OK;
} // CJPEGDecoder::_set_sampling()


JERRCODE CJPEGDecoder::NextMarker(JMARKER* marker)
{
	int c;
	int n = 0;

	for(;;)
	{
		if(m_src.currPos >= m_src.DataLen)
		{
			LOG0("Error: buffer too small");
			return JPEG_BUFF_TOO_SMALL;
		}

		m_src._READ_BYTE(&c);

		if(c != 0xff)
		{
			do
			{
				if(m_src.currPos >= m_src.DataLen)
				{
					LOG0("Error: buffer too small");
					return JPEG_BUFF_TOO_SMALL;
				}
				n++;
				m_src._READ_BYTE(&c);
			} while(c != 0xff);
		}

		do
		{
			if(m_src.currPos >= m_src.DataLen)
			{
				LOG0("Error: buffer too small");
				return JPEG_BUFF_TOO_SMALL;
			}

			m_src._READ_BYTE(&c);
		} while(c == 0xff);

		if(c != 0)
		{
			*marker = (JMARKER)c;
			break;
		}
	}

	if(n != 0)
	{
		TRC1("  skip enormous bytes - ",n);
	}

	return JPEG_OK;
} // CJPEGDecoder::NextMarker()


JERRCODE CJPEGDecoder::SkipMarker(void)
{
	int len;

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);

	m_src.currPos += len - 2;

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::SkipMarker()


JERRCODE CJPEGDecoder::ProcessRestart(void)
{
	JERRCODE  jerr;
	IppStatus status;

	status = ippiDecodeHuffmanStateInit_JPEG_8u(m_state);
	if(ippStsNoErr != status)
	{
		LOG0("Error: ippiDecodeHuffmanStateInit_JPEG_8u() failed");
		return JPEG_INTERNAL_ERROR;
	}

	for(int n = 0; n < m_jpeg_ncomp; n++)
	{
		m_ccomp[n]->m_lastDC = 0;
	}

	jerr = ParseRST();
	if(JPEG_OK != jerr)
	{
		LOG0("Error: ParseRST() failed");
		//    return jerr;
	}

	m_restarts_to_go = m_jpeg_restart_interval;

	return JPEG_OK;
} // CJPEGDecoder::ProcessRestart()


JERRCODE CJPEGDecoder::ParseSOI(void)
{
	TRC0("-> SOI");
	m_marker = JM_NONE;
	return JPEG_OK;
} // CJPEGDecoder::ParseSOI()


JERRCODE CJPEGDecoder::ParseEOI(void)
{
	TRC0("-> EOI");
	m_marker = JM_NONE;
	return JPEG_OK;
} // CJPEGDecoder::ParseEOI()


const int APP0_JFIF_LENGTH = 14;
const int APP0_JFXX_LENGTH = 6;

JERRCODE CJPEGDecoder::ParseAPP0(void)
{
	int len;

	TRC0("-> APP0");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);
	len -= 2;

	if(len >= APP0_JFIF_LENGTH &&
			m_src.pData[m_src.currPos + 0] == 0x4a &&
			m_src.pData[m_src.currPos + 1] == 0x46 &&
			m_src.pData[m_src.currPos + 2] == 0x49 &&
			m_src.pData[m_src.currPos + 3] == 0x46 &&
			m_src.pData[m_src.currPos + 4] == 0)
	{
		// we've found JFIF APP0 marker
		len -= 5;
		m_src.currPos += 5;
		m_jfif_app0_detected     = 1;

		m_src._READ_BYTE(&m_jfif_app0_major);
		m_src._READ_BYTE(&m_jfif_app0_minor);
		m_src._READ_BYTE(&m_jfif_app0_units);
		m_src._READ_WORD(&m_jfif_app0_xDensity);
		m_src._READ_WORD(&m_jfif_app0_yDensity);
		m_src._READ_BYTE(&m_jfif_app0_thumb_width);
		m_src._READ_BYTE(&m_jfif_app0_thumb_height);
		len -= 9;
	}

	if(len >= APP0_JFXX_LENGTH &&
			m_src.pData[m_src.currPos + 0] == 0x4a &&
			m_src.pData[m_src.currPos + 1] == 0x46 &&
			m_src.pData[m_src.currPos + 2] == 0x58 &&
			m_src.pData[m_src.currPos + 3] == 0x58 &&
			m_src.pData[m_src.currPos + 4] == 0)
	{
		// we've found JFXX APP0 extension marker
		len -= 5;
		m_src.currPos += 5;
		m_jfxx_app0_detected = 1;

		m_src._READ_BYTE(&m_jfxx_thumbnails_type);

		switch(m_jfxx_thumbnails_type)
		{
			case 0x10: break;
			case 0x11: break;
			case 0x13: break;
			default:   break;
		}
		len -= 1;
	}

	m_src.currPos += len;

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseAPP0()


const int APP14_ADOBE_LENGTH = 12;

JERRCODE CJPEGDecoder::ParseAPP14(void)
{
	int len;

	TRC0("-> APP14");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);
	len -= 2;

	if(len >= APP14_ADOBE_LENGTH &&
			m_src.pData[m_src.currPos + 0] == 0x41  &&
			m_src.pData[m_src.currPos + 1] == 0x64  &&
			m_src.pData[m_src.currPos + 2] == 0x6f  &&
			m_src.pData[m_src.currPos + 3] == 0x62  &&
			m_src.pData[m_src.currPos + 4] == 0x65)
	{
		// we've found Adobe APP14 marker
		len -= 5;
		m_src.currPos += 5;
		m_adobe_app14_detected   = 1;

		m_src._READ_WORD(&m_adobe_app14_version);
		m_src._READ_WORD(&m_adobe_app14_flags0);
		m_src._READ_WORD(&m_adobe_app14_flags1);
		m_src._READ_BYTE(&m_adobe_app14_transform);

		TRC1("  adobe_app14_version   - ",m_adobe_app14_version);
		TRC1("  adobe_app14_flags0    - ",m_adobe_app14_flags0);
		TRC1("  adobe_app14_flags1    - ",m_adobe_app14_flags1);
		TRC1("  adobe_app14_transform - ",m_adobe_app14_transform);

		len -= 7;
	}

	m_src.currPos += len;

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseAPP14()


JERRCODE CJPEGDecoder::ParseCOM(void)
{
	int i;
	int c;
	int len;

	TRC0("-> COM");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);
	len -= 2;

	TRC1("  bytes for comment - ",len);

	m_jpeg_comment_detected = 1;
	m_jpeg_comment_size     = len;

	if(m_jpeg_comment != 0)
	{
		delete [] m_jpeg_comment;
	}

	m_jpeg_comment = new Ipp8u [len+1];

	for(i = 0; i < len; i++)
	{
		m_src._READ_BYTE(&c);
		m_jpeg_comment[i] = (Ipp8u)c;
	}

	m_jpeg_comment[len] = 0;

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseCOM()


JERRCODE CJPEGDecoder::ParseDQT(void)
{
	int id;
	int len;
	JERRCODE jerr;

	TRC0("-> DQT");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);
	len -= 2;

	while(len > 0)
	{
		m_src._READ_BYTE(&id);

		int precision = (id & 0xf0) >> 4;

		TRC1("  id        - ",(id & 0x0f));
		TRC1("  precision - ",precision);

		if((id & 0x0f) > MAX_QUANT_TABLES)
		{
			return JPEG_BAD_QUANT_SEGMENT;
		}

		int q;
		Ipp8u qnt[DCTSIZE2];

		for(int i = 0; i < DCTSIZE2; i++)
		{
			if(precision)
			{
				m_src._READ_WORD(&q);
			}
			else
			{
				m_src._READ_BYTE(&q);
			}

			qnt[i] = (Ipp8u)q;
		}

		jerr = m_qntbl[id & 0x0f].Init(id,qnt);
		if(JPEG_OK != jerr)
		{
			return jerr;
		}

		len -= DCTSIZE2 + DCTSIZE2*precision + 1;
	}

	if(len != 0)
	{
		return JPEG_BAD_SEGMENT_LENGTH;
	}

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseDQT()


JERRCODE CJPEGDecoder::ParseDHT(void)
{
	int i;
	int len;
	int index;
	int count;
	JERRCODE jerr;

	TRC0("-> DHT");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);
	len -= 2;

	int v;
	Ipp8u bits[MAX_HUFF_BITS];
	Ipp8u vals[MAX_HUFF_VALS];

	while(len > 16)
	{
		m_src._READ_BYTE(&index);
		count = 0;
		for(i = 0; i < MAX_HUFF_BITS; i++)
		{
			m_src._READ_BYTE(&v);
			bits[i] = (Ipp8u)v;
			count += bits[i];
		}

		len -= 16 + 1;

		if(count > MAX_HUFF_VALS || count > len)
		{
			return JPEG_BAD_HUFF_TBL;
		}

		for(i = 0; i < count; i++)
		{
			m_src._READ_BYTE(&v);
			vals[i] = (Ipp8u)v;
		}

		len -= count;

		if(index >> 4)
		{
			// make AC Huffman table
			if(m_actbl[index & 0x0f].IsEmpty())
			{
				jerr = m_actbl[index & 0x0f].Create();
				if(JPEG_OK != jerr)
				{
					LOG0("    Can't create AC huffman table");
					return JPEG_INTERNAL_ERROR;
				}
			}

			TRC1("    AC Huffman Table - ",index & 0x0f);
			jerr = m_actbl[index & 0x0f].Init(index & 0x0f,index >> 4,bits,vals);
			if(JPEG_OK != jerr)
			{
				LOG0("    Can't build AC huffman table");
				return JPEG_INTERNAL_ERROR;
			}
		}
		else
		{
			// make DC Huffman table
			if(m_dctbl[index & 0x0f].IsEmpty())
			{
				jerr = m_dctbl[index & 0x0f].Create();
				if(JPEG_OK != jerr)
				{
					LOG0("    Can't create DC huffman table");
					return JPEG_INTERNAL_ERROR;
				}
			}

			TRC1("    DC Huffman Table - ",index & 0x0f);
			jerr = m_dctbl[index & 0x0f].Init(index & 0x0f,index >> 4,bits,vals);
			if(JPEG_OK != jerr)
			{
				LOG0("    Can't build DC huffman table");
				return JPEG_INTERNAL_ERROR;
			}
		}
	}

	if(len != 0)
	{
		return JPEG_BAD_SEGMENT_LENGTH;
	}

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseDHT()


JERRCODE CJPEGDecoder::ParseSOF0(void)
{
	int i;
	int len;
	JERRCODE jerr;

	TRC0("-> SOF0");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);
	len -= 2;

	m_src._READ_BYTE(&m_jpeg_precision);

	if(m_jpeg_precision != 8)
	{
		return JPEG_NOT_IMPLEMENTED;
	}

	m_src._READ_WORD(&m_jpeg_height);
	m_src._READ_WORD(&m_jpeg_width);
	m_src._READ_BYTE(&m_jpeg_ncomp);

	TRC1("  height    - ",m_jpeg_height);
	TRC1("  width     - ",m_jpeg_width);
	TRC1("  nchannels - ",m_jpeg_ncomp);

	if(m_jpeg_ncomp < 0 || m_jpeg_ncomp > MAX_COMPS_PER_SCAN)
	{
		return JPEG_BAD_FRAME_SEGMENT;
	}

	len -= 6;

	if(len != m_jpeg_ncomp * 3)
	{
		return JPEG_BAD_SEGMENT_LENGTH;
	}

	for(i = 0; i < m_jpeg_ncomp; i++)
	{
		if(NULL != m_ccomp[i])
		{
			delete m_ccomp[i];
			m_ccomp[i] = 0;
		}

		m_ccomp[i] = new CJPEGColorComponent;

		m_src._READ_BYTE(&m_ccomp[i]->m_id);

		int ss;

		m_src._READ_BYTE(&ss);

		m_ccomp[i]->m_hsampling  = (ss >> 4) & 0x0f;
		m_ccomp[i]->m_vsampling  = (ss     ) & 0x0f;

		m_src._READ_BYTE(&m_ccomp[i]->m_q_selector);

		if(m_ccomp[i]->m_hsampling <= 0 || m_ccomp[i]->m_vsampling <= 0)
		{
			return JPEG_BAD_FRAME_SEGMENT;
		}

		TRC1("    id ",m_ccomp[i]->m_id);
		TRC1("      hsampling - ",m_ccomp[i]->m_hsampling);
		TRC1("      vsampling - ",m_ccomp[i]->m_vsampling);
		TRC1("      qselector - ",m_ccomp[i]->m_q_selector);
	}

	jerr = _set_sampling();
	if(JPEG_OK != jerr)
	{
		return jerr;
	}

	for(i = 0; i < m_jpeg_ncomp; i++)
	{
		m_ccomp[i]->m_h_factor = (m_jpeg_sampling == JS_444) ? 1 : (i == 0 || i == 3 ? 1 : 2);
		m_ccomp[i]->m_v_factor = (m_jpeg_sampling == JS_411) ? (i == 0 || i == 3 ? 1 : 2) : 1;
	}

	m_jpeg_mode = JPEG_BASELINE;

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseSOF0()


JERRCODE CJPEGDecoder::ParseSOF2(void)
{
	int i;
	int len;
	JERRCODE jerr;

	TRC0("-> SOF2");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);
	len -= 2;

	m_src._READ_BYTE(&m_jpeg_precision);

	if(m_jpeg_precision != 8)
	{
		return JPEG_NOT_IMPLEMENTED;
	}

	m_src._READ_WORD(&m_jpeg_height);
	m_src._READ_WORD(&m_jpeg_width);
	m_src._READ_BYTE(&m_jpeg_ncomp);

	TRC1("  height    - ",m_jpeg_height);
	TRC1("  width     - ",m_jpeg_width);
	TRC1("  nchannels - ",m_jpeg_ncomp);

	if(m_jpeg_ncomp < 0 || m_jpeg_ncomp > MAX_COMPS_PER_SCAN)
	{
		return JPEG_BAD_FRAME_SEGMENT;
	}

	len -= 6;

	if(len != m_jpeg_ncomp * 3)
	{
		return JPEG_BAD_SEGMENT_LENGTH;
	}

	for(i = 0; i < m_jpeg_ncomp; i++)
	{
		if(0 != m_ccomp[i])
		{
			delete m_ccomp[i];
			m_ccomp[i] = 0;
		}

		m_ccomp[i] = new CJPEGColorComponent;

		m_src._READ_BYTE(&m_ccomp[i]->m_id);
		m_ccomp[i]->m_comp_no = i;

		int ss;

		m_src._READ_BYTE(&ss);

		m_ccomp[i]->m_hsampling  = (ss >> 4) & 0x0f;
		m_ccomp[i]->m_vsampling  = (ss     ) & 0x0f;

		m_src._READ_BYTE(&m_ccomp[i]->m_q_selector);

		if(m_ccomp[i]->m_hsampling <= 0 || m_ccomp[i]->m_vsampling <= 0)
		{
			return JPEG_BAD_FRAME_SEGMENT;
		}

		TRC1("    id ",m_ccomp[i]->m_id);
		TRC1("      hsampling - ",m_ccomp[i]->m_hsampling);
		TRC1("      vsampling - ",m_ccomp[i]->m_vsampling);
		TRC1("      qselector - ",m_ccomp[i]->m_q_selector);
	}

	jerr = _set_sampling();
	if(JPEG_OK != jerr)
	{
		return jerr;
	}

	for(i = 0; i < m_jpeg_ncomp; i++)
	{
		m_ccomp[i]->m_h_factor = (m_jpeg_sampling == JS_444) ? 1 : (i == 0 || i == 3 ? 1 : 2);
		m_ccomp[i]->m_v_factor = (m_jpeg_sampling == JS_411) ? (i == 0 || i == 3 ? 1 : 2) : 1;
	}

	m_jpeg_mode = JPEG_PROGRESSIVE;

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseSOF2()


JERRCODE CJPEGDecoder::ParseSOF3(void)
{
	int i;
	int len;
	JERRCODE jerr;

	TRC0("-> SOF3");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);
	len -= 2;

	m_src._READ_BYTE(&m_jpeg_precision);

	if(m_jpeg_precision < 2 || m_jpeg_precision > 16)
	{
		return JPEG_BAD_FRAME_SEGMENT;
	}

	m_src._READ_WORD(&m_jpeg_height);
	m_src._READ_WORD(&m_jpeg_width);
	m_src._READ_BYTE(&m_jpeg_ncomp);

	TRC1("  height    - ",m_jpeg_height);
	TRC1("  width     - ",m_jpeg_width);
	TRC1("  nchannels - ",m_jpeg_ncomp);

	if(m_jpeg_ncomp != 1)
	{
		return JPEG_NOT_IMPLEMENTED;
	}

	len -= 6;

	if(len != m_jpeg_ncomp * 3)
	{
		return JPEG_BAD_SEGMENT_LENGTH;
	}

	for(i = 0; i < m_jpeg_ncomp; i++)
	{
		if(0 != m_ccomp[i])
		{
			delete m_ccomp[i];
			m_ccomp[i] = 0;
		}

		m_ccomp[i] = new CJPEGColorComponent;

		m_src._READ_BYTE(&m_ccomp[i]->m_id);

		int ss;

		m_src._READ_BYTE(&ss);

		m_ccomp[i]->m_hsampling  = (ss >> 4) & 0x0f;
		m_ccomp[i]->m_vsampling  = (ss     ) & 0x0f;

		m_src._READ_BYTE(&m_ccomp[i]->m_q_selector);

		if(m_ccomp[i]->m_hsampling <= 0 || m_ccomp[i]->m_vsampling <= 0)
		{
			return JPEG_BAD_FRAME_SEGMENT;
		}

		TRC1("    id ",m_ccomp[i]->m_id);
		TRC1("      hsampling - ",m_ccomp[i]->m_hsampling);
		TRC1("      vsampling - ",m_ccomp[i]->m_vsampling);
		TRC1("      qselector - ",m_ccomp[i]->m_q_selector);
	}

	jerr = _set_sampling();
	if(JPEG_OK != jerr)
	{
		return jerr;
	}

	for(i = 0; i < m_jpeg_ncomp; i++)
	{
		m_ccomp[i]->m_h_factor = (m_jpeg_sampling == JS_444) ? 1 : (i == 0 || i == 3 ? 1 : 2);
		m_ccomp[i]->m_v_factor = (m_jpeg_sampling == JS_411) ? (i == 0 || i == 3 ? 1 : 2) : 1;
	}

	m_jpeg_mode = JPEG_LOSSLESS;

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseSOF3()


JERRCODE CJPEGDecoder::ParseDRI(void)
{
	int len;

	TRC0("-> DRI");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);
	len -= 2;

	if(len != 2)
	{
		return JPEG_BAD_SEGMENT_LENGTH;
	}

	m_src._READ_WORD(&m_jpeg_restart_interval);

	TRC1("  restart interval - ",m_jpeg_restart_interval);

	m_restarts_to_go = m_jpeg_restart_interval;

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseDRI()


JERRCODE CJPEGDecoder::ParseRST(void)
{
	JERRCODE jerr;

	TRC0("-> RST");

	if(m_marker == 0xff)
	{
		m_src.currPos--;
		m_marker = JM_NONE;
	}

	if(m_marker == JM_NONE)
	{
		jerr = NextMarker(&m_marker);
		if(JPEG_OK != jerr)
		{
			LOG0("Error: NextMarker() failed");
			return JPEG_INTERNAL_ERROR;
		}
	}

	TRC1("restart interval ",m_next_restart_num);
	if(m_marker == ((int)JM_RST0 + m_next_restart_num))
	{
		m_marker = JM_NONE;
	}
	else
	{
		LOG1("  - got marker   - ",m_marker);
		LOG1("  - but expected - ",(int)JM_RST0 + m_next_restart_num);
		m_marker = JM_NONE;
		//    return JPEG_BAD_RESTART;
	}

	// Update next-restart state
	m_next_restart_num = (m_next_restart_num + 1) & 7;

	return JPEG_OK;
} // CJPEGDecoder::ParseRST()


JERRCODE CJPEGDecoder::ParseSOS(void)
{
	int i;
	int ci;
	int len;

	TRC0("-> SOS");

	if(m_src.currPos + 2 >= m_src.DataLen)
	{
		LOG0("Error: buffer too small");
		return JPEG_BUFF_TOO_SMALL;
	}

	m_src._READ_WORD(&len);

	// store position to return to in subsequent ReadData call
	m_sos_len = len;

	len -= 2;

	int ncomps;

	m_src._READ_BYTE(&ncomps);

	if(ncomps < 1 || ncomps > MAX_COMPS_PER_SCAN)
	{
		return JPEG_BAD_SCAN_SEGMENT;
	}

	if(JPEG_PROGRESSIVE != m_jpeg_mode && ncomps < m_jpeg_ncomp)
	{
		// does not support scan-interleaved images for now..
		return JPEG_NOT_IMPLEMENTED;
	}

	if(len != ((ncomps * 2) + 4))
	{
		return JPEG_BAD_SEGMENT_LENGTH;
	}

	TRC1("  ncomps - ",ncomps);

	for(i = 0; i < ncomps; i++)
	{
		int id;
		int huff_sel;

		m_src._READ_BYTE(&id);
		m_src._READ_BYTE(&huff_sel);

		TRC1("    id - ",id);
		TRC1("      dc_selector - ",(huff_sel >> 4) & 0x0f);
		TRC1("      ac_selector - ",(huff_sel     ) & 0x0f);

		for(ci = 0; ci < m_jpeg_ncomp; ci++)
		{
			if(id == m_ccomp[ci]->m_id)
			{
				m_curr_comp_no = ci;
				goto comp_id_match;
			}
		}

		return JPEG_BAD_COMPONENT_ID;

comp_id_match:

		m_ccomp[ci]->m_dc_selector = (huff_sel >> 4) & 0x0f;
		m_ccomp[ci]->m_ac_selector = (huff_sel     ) & 0x0f;
	}

	m_src._READ_BYTE(&m_ss);
	m_src._READ_BYTE(&m_se);

	int t;

	m_src._READ_BYTE(&t);

	m_ah = (t >> 4) & 0x0f;
	m_al = (t     ) & 0x0f;

	TRC1("  Ss - ",m_ss);
	TRC1("  Se - ",m_se);
	TRC1("  Ah - ",m_ah);
	TRC1("  Al - ",m_al);

	// detect JPEG color space
	if(m_jfif_app0_detected)
	{
		switch(m_jpeg_ncomp)
		{
			case 1:  m_jpeg_color = JC_GRAY;    break;
			case 3:  m_jpeg_color = JC_YCBCR;   break;
			default: m_jpeg_color = JC_UNKNOWN; break;
		}
	}

	if(m_adobe_app14_detected)
	{
		switch(m_adobe_app14_transform)
		{
			case 0:
				switch(m_jpeg_ncomp)
				{
					case 1:  m_jpeg_color = JC_GRAY;    break;
					case 3:  m_jpeg_color = JC_RGB;     break;
					case 4:  m_jpeg_color = JC_CMYK;    break;
					default: m_jpeg_color = JC_UNKNOWN; break;
				}
				break;

			case 1:  m_jpeg_color = JC_YCBCR;   break;
			case 2:  m_jpeg_color = JC_YCCK;    break;
			default: m_jpeg_color = JC_UNKNOWN; break;
		}
	}

	// try to guess what color space is used...
	if(!m_jfif_app0_detected && !m_adobe_app14_detected)
	{
		switch(m_jpeg_ncomp)
		{
			case 1:  m_jpeg_color = JC_GRAY;    break;
			case 3:  m_jpeg_color = JC_YCBCR;   break;
			default: m_jpeg_color = JC_UNKNOWN; break;
		}
	}

	m_restarts_to_go   = m_jpeg_restart_interval;
	m_next_restart_num = 0;

	m_marker = JM_NONE;

	return JPEG_OK;
} // CJPEGDecoder::ParseSOS()


JERRCODE CJPEGDecoder::ParseJPEGBitStream(JOPERATION op)
{
	JERRCODE jerr = JPEG_OK;

	m_marker = JM_NONE;

	for(;;)
	{
		if(JM_NONE == m_marker)
		{
			jerr = NextMarker(&m_marker);
			if(JPEG_OK != jerr)
			{
				return jerr;
			}
		}

		switch(m_marker)
		{
			case JM_SOI:
				jerr = ParseSOI();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_APP0:
				jerr = ParseAPP0();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_APP14:
				jerr = ParseAPP14();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_COM:
				jerr = ParseCOM();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_DQT:
				jerr = ParseDQT();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_SOF0:
			case JM_SOF1:
				jerr = ParseSOF0();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_SOF2:
				jerr = ParseSOF2();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_SOF3:
				jerr = ParseSOF3();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_SOF5:
			case JM_SOF6:
			case JM_SOF7:
			case JM_SOF9:
			case JM_SOFA:
			case JM_SOFB:
			case JM_SOFD:
			case JM_SOFE:
			case JM_SOFF:
				return JPEG_NOT_IMPLEMENTED;

			case JM_DHT:
				jerr = ParseDHT();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_DRI:
				jerr = ParseDRI();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_SOS:
				jerr = ParseSOS();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}

				if(JO_READ_HEADER == op)
				{
					m_src.currPos -= m_sos_len + 2;
					// stop here, when we are reading header
					return JPEG_OK;
				}

				if(JO_READ_DATA == op)
				{
					jerr = Init();
					if(JPEG_OK != jerr)
					{
						return jerr;
					}

					switch(m_jpeg_mode)
					{
						case JPEG_BASELINE:
							jerr = DecodeScanBaseline();
							break;

						case JPEG_PROGRESSIVE:
							{
								jerr = DecodeScanProgressive();

								m_ac_scans_completed = 0;
								for(int i = 0; i < m_jpeg_ncomp; i++)
								{
									m_ac_scans_completed += m_ccomp[i]->m_ac_scan_completed;
								}

								if(JPEG_OK != jerr ||
										(m_dc_scan_completed != 0 && m_ac_scans_completed == m_jpeg_ncomp))
								{
									jerr = PerformDCT();
									if(JPEG_OK != jerr)
									{
										return jerr;
									}

									jerr = UpSampling();
									if(JPEG_OK != jerr)
									{
										return jerr;
									}

									jerr = ColorConvert();
									if(JPEG_OK != jerr)
									{
										return jerr;
									}
								}

								break;
							}

						case JPEG_LOSSLESS:
							jerr = DecodeScanLossless();
							break;
					}

					if(JPEG_OK != jerr)
						return jerr;
				}

				break;

			case JM_RST0:
			case JM_RST1:
			case JM_RST2:
			case JM_RST3:
			case JM_RST4:
			case JM_RST5:
			case JM_RST6:
			case JM_RST7:
				jerr = ParseRST();
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
				break;

			case JM_EOI:
				jerr = ParseEOI();
				goto Exit;

			default:
				TRC1("-> Unknown marker ",m_marker);
				TRC0("..Skipping");
				jerr = SkipMarker();
				if(JPEG_OK != jerr)
					return jerr;

				break;
		}
	}

Exit:

	return jerr;
} // CJPEGDecoder::ParseJPEGBitStream()


JERRCODE CJPEGDecoder::Init(void)
{
	int i;
	int sz;
	int num_threads = 1;
	int ss_buf_size = 0;
	int cc_buf_size = 0;

	m_nblock    = 1;
	num_threads = get_num_threads();

	m_num_threads = num_threads;

	if(m_jpeg_sampling == JS_411)
	{
		// there is no threading support for 411 sampling yet
		num_threads = 1;
		set_num_threads(num_threads);
	}

	// not implemented yet
	if(m_jpeg_sampling == JS_OTHER)
	{
		return JPEG_NOT_IMPLEMENTED;
	}

	for(i = 0; i < m_jpeg_ncomp; i++)
	{
		switch(m_jpeg_mode)
		{
			case JPEG_BASELINE:
				{
					switch(m_jpeg_sampling)
					{
						case JS_444:
							{
								ss_buf_size = 0;
								m_nblock    = m_jpeg_ncomp;

								break;
							}

						case JS_422:
							{
								if(i == 0 || i == 3)
									ss_buf_size = 0;
								else
									ss_buf_size = m_numxMCU*((m_mcuWidth>>1)+2) * num_threads * m_mcuHeight;

								m_nblock = (m_jpeg_ncomp == 3) ? 4 : 6;

								break;
							}

						case JS_411:
							{
								if(i == 0 || i == 3)
									ss_buf_size = 0;
								else
									ss_buf_size = m_numxMCU*((m_mcuWidth>>1)+2) * num_threads * ((m_mcuHeight>>1)+2);

								m_nblock = (m_jpeg_ncomp == 3) ? 6 : 10;

								break;
							}
						case JS_OTHER:
							break;
					}

					cc_buf_size = (m_numxMCU*m_mcuWidth)*(num_threads*m_mcuHeight);

					if(NULL == m_block_buffer)
					{
						sz = DCTSIZE2 * m_nblock * m_numxMCU * num_threads * sizeof(Ipp16s);

						m_block_buffer = (Ipp16s*)ippMalloc(sz);
						if(NULL == m_block_buffer)
						{
							return JPEG_OUT_OF_MEMORY;
						}

						ippsZero_8u((Ipp8u*)m_block_buffer,sz);
					}

					break;
				} // JPEG_BASELINE

			case JPEG_PROGRESSIVE:
				{
					ss_buf_size = (m_numxMCU*m_mcuWidth+2) * (m_numyMCU*m_mcuHeight+2);
					cc_buf_size = (m_numxMCU*m_mcuWidth)   * (m_numyMCU*m_mcuHeight);

					if(NULL == m_coefbuf)
					{
						sz = m_numxMCU*m_numyMCU*MAX_BYTES_PER_MCU*sizeof(Ipp16s);

						m_coefbuf = (Ipp16s*)ippMalloc(sz);
						if(NULL == m_coefbuf)
						{
							return JPEG_OUT_OF_MEMORY;
						}

						ippsZero_8u((Ipp8u*)m_coefbuf,sz);
					}

					break;
				} // JPEG_PROGRESSIVE

			case JPEG_LOSSLESS:
				{
					ss_buf_size = m_numxMCU*m_mcuWidth*sizeof(Ipp16s);
					cc_buf_size = m_numxMCU*m_mcuWidth*sizeof(Ipp16s);

					if(NULL == m_block_buffer)
					{
						sz = m_numxMCU*sizeof(Ipp16s);

						m_block_buffer = (Ipp16s*)ippMalloc(sz);
						if(NULL == m_block_buffer)
						{
							return JPEG_OUT_OF_MEMORY;
						}

						ippsZero_8u((Ipp8u*)m_block_buffer,sz);
					}

					break;
				} // JPEG_LOSSLESS
		}

		if(NULL == m_ccomp[i]->m_ss_buffer)
		{
			if(ss_buf_size)
			{
				m_ccomp[i]->m_ss_buffer = (Ipp8u*)ippMalloc(ss_buf_size*2);
				if(NULL == m_ccomp[i]->m_ss_buffer)
				{
					return JPEG_OUT_OF_MEMORY;
				}
			}
			m_ccomp[i]->m_curr_row = (Ipp16s*)m_ccomp[i]->m_ss_buffer;
			m_ccomp[i]->m_prev_row = (Ipp16s*)m_ccomp[i]->m_ss_buffer + m_dst.width;
		}

		if(NULL == m_ccomp[i]->m_cc_buffer)
		{
			m_ccomp[i]->m_cc_buffer = (Ipp8u*)ippMalloc(cc_buf_size);
			if(NULL == m_ccomp[i]->m_cc_buffer)
			{
				return JPEG_OUT_OF_MEMORY;
			}
		}

		if(NULL == m_ccomp[i]->m_top_row)
		{
			if(m_jpeg_sampling == JS_411)
			{
				m_ccomp[i]->m_top_row = (Ipp8u*)ippMalloc((m_ccWidth>>1)+2);
				if(NULL == m_ccomp[i]->m_top_row)
				{
					return JPEG_OUT_OF_MEMORY;
				}
			}
		}
	} // m_jpeg_ncomp

	m_state.Create();

	return JPEG_OK;
} // CJPEGDecoder::Init()


JERRCODE CJPEGDecoder::ColorConvert(int nMCURow,int idThread)
{
	IppStatus status;
	int threadoffset = (m_numxMCU*m_mcuWidth)*(idThread*m_mcuHeight);//OMP

	if(nMCURow == m_numyMCU - 1)
	{
		m_ccHeight = m_mcuHeight - m_yPadding;
	}

	IppiSize roi = { m_dst.width, m_ccHeight };

	Ipp8u* dst = m_dst.p.Data8u + nMCURow*m_mcuHeight*m_dst.lineStep;

	if((m_jpeg_color == JC_UNKNOWN && m_dst.color == JC_UNKNOWN) ||
			(m_jpeg_color == m_dst.color))
	{
		switch(m_jpeg_ncomp)
		{
			case 1:
				{
					status = ippiCopy_8u_C1R(m_ccomp[0]->m_cc_buffer,m_ccWidth,dst,m_dst.lineStep,roi);

					if(ippStsNoErr != status)
					{
						LOG1("IPP Error: ippiCopy_8u_C1R() failed - ",status);
						return JPEG_INTERNAL_ERROR;
					}
				}
				break;

			case 3:
				{
					const Ipp8u* src[3];
					src[0] = m_ccomp[0]->m_cc_buffer + threadoffset;
					src[1] = m_ccomp[1]->m_cc_buffer + threadoffset;
					src[2] = m_ccomp[2]->m_cc_buffer + threadoffset;

					status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

					if(ippStsNoErr != status)
					{
						LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
						return JPEG_INTERNAL_ERROR;
					}
				}
				break;

			case 4:
				{
					const Ipp8u* src[4];
					src[0] = m_ccomp[0]->m_cc_buffer + threadoffset;
					src[1] = m_ccomp[1]->m_cc_buffer + threadoffset;
					src[2] = m_ccomp[2]->m_cc_buffer + threadoffset;
					src[3] = m_ccomp[3]->m_cc_buffer + threadoffset;

					status = ippiCopy_8u_P4C4R(src,m_ccWidth,dst,m_dst.lineStep,roi);

					if(ippStsNoErr != status)
					{
						LOG1("IPP Error: ippiCopy_8u_P4C4R() failed - ",status);
						return JPEG_INTERNAL_ERROR;
					}
				}
				break;

			default:
				return JPEG_NOT_IMPLEMENTED;
		}
	}

	// Gray to Gray
	if(m_jpeg_color == JC_GRAY && m_dst.color == JC_GRAY)
	{
		status = ippiCopy_8u_C1R(m_ccomp[0]->m_cc_buffer + threadoffset,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_C1R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// Gray to RGB
	if(m_jpeg_color == JC_GRAY && m_dst.color == JC_RGB)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer + threadoffset;
		src[1] = m_ccomp[0]->m_cc_buffer + threadoffset;
		src[2] = m_ccomp[0]->m_cc_buffer + threadoffset;

		status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// Gray to BGR
	if(m_jpeg_color == JC_GRAY && m_dst.color == JC_BGR)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer + threadoffset;
		src[1] = m_ccomp[0]->m_cc_buffer + threadoffset;
		src[2] = m_ccomp[0]->m_cc_buffer + threadoffset;

		status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// RGB to RGB
	if(m_jpeg_color == JC_RGB && m_dst.color == JC_RGB)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer + threadoffset;
		src[1] = m_ccomp[1]->m_cc_buffer + threadoffset;
		src[2] = m_ccomp[2]->m_cc_buffer + threadoffset;

		status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// RGB to BGR
	if(m_jpeg_color == JC_RGB && m_dst.color == JC_BGR)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[2]->m_cc_buffer + threadoffset;
		src[1] = m_ccomp[1]->m_cc_buffer + threadoffset;
		src[2] = m_ccomp[0]->m_cc_buffer + threadoffset;

		status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// YCbCr to RGB
	if(m_jpeg_color == JC_YCBCR && m_dst.color == JC_RGB)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer + threadoffset;
		src[1] = m_ccomp[1]->m_cc_buffer + threadoffset;
		src[2] = m_ccomp[2]->m_cc_buffer + threadoffset;

		status = ippiYCbCrToRGB_JPEG_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiYCbCrToRGB_JPEG_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// YCbCr to BGR
	if(m_jpeg_color == JC_YCBCR && m_dst.color == JC_BGR)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer + threadoffset;
		src[1] = m_ccomp[1]->m_cc_buffer + threadoffset;
		src[2] = m_ccomp[2]->m_cc_buffer + threadoffset;

		status = ippiYCbCrToBGR_JPEG_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiYCbCrToBGR_JPEG_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// CMYK to CMYK
	if(m_jpeg_color == JC_CMYK && m_dst.color == JC_CMYK)
	{
		const Ipp8u* src[4];
		src[0] = m_ccomp[0]->m_cc_buffer + threadoffset;
		src[1] = m_ccomp[1]->m_cc_buffer + threadoffset;
		src[2] = m_ccomp[2]->m_cc_buffer + threadoffset;
		src[3] = m_ccomp[3]->m_cc_buffer + threadoffset;

		status = ippiCopy_8u_P4C4R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P4C4R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// YCCK to CMYK
	if(m_jpeg_color == JC_YCCK && m_dst.color == JC_CMYK)
	{
		const Ipp8u* src[4];
		src[0] = m_ccomp[0]->m_cc_buffer + threadoffset;
		src[1] = m_ccomp[1]->m_cc_buffer + threadoffset;
		src[2] = m_ccomp[2]->m_cc_buffer + threadoffset;
		src[3] = m_ccomp[3]->m_cc_buffer + threadoffset;

		status = ippiYCCKToCMYK_JPEG_8u_P4C4R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiYCCKToCMYK_JPEG_8u_P4C4R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	return JPEG_OK;
} // CJPEGDecoder::ColorConvert()


JERRCODE CJPEGDecoder::ColorConvert(void)
{
	IppStatus status;

	IppiSize roi = { m_dst.width, m_dst.height };

	Ipp8u* dst = m_dst.p.Data8u;

	if(m_jpeg_color == JC_UNKNOWN && m_dst.color == JC_UNKNOWN)
	{
		switch(m_jpeg_ncomp)
		{
			case 1:
				{
					status = ippiCopy_8u_C1R(m_ccomp[0]->m_cc_buffer,m_ccWidth,dst,m_dst.lineStep,roi);

					if(ippStsNoErr != status)
					{
						LOG1("IPP Error: ippiCopy_8u_C1R() failed - ",status);
						return JPEG_INTERNAL_ERROR;
					}
				}
				break;

			case 3:
				{
					const Ipp8u* src[3];
					src[0] = m_ccomp[0]->m_cc_buffer;
					src[1] = m_ccomp[1]->m_cc_buffer;
					src[2] = m_ccomp[2]->m_cc_buffer;

					status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

					if(ippStsNoErr != status)
					{
						LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
						return JPEG_INTERNAL_ERROR;
					}
				}
				break;

			case 4:
				{
					const Ipp8u* src[4];
					src[0] = m_ccomp[0]->m_cc_buffer;
					src[1] = m_ccomp[1]->m_cc_buffer;
					src[2] = m_ccomp[2]->m_cc_buffer;
					src[3] = m_ccomp[3]->m_cc_buffer;

					status = ippiCopy_8u_P4C4R(src,m_ccWidth,dst,m_dst.lineStep,roi);

					if(ippStsNoErr != status)
					{
						LOG1("IPP Error: ippiCopy_8u_P4C4R() failed - ",status);
						return JPEG_INTERNAL_ERROR;
					}
				}
				break;

			default:
				return JPEG_NOT_IMPLEMENTED;
		}
	}

	// Gray to Gray
	if(m_jpeg_color == JC_GRAY && m_dst.color == JC_GRAY)
	{
		status = ippiCopy_8u_C1R(m_ccomp[0]->m_cc_buffer,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_C1R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// Gray to RGB
	if(m_jpeg_color == JC_GRAY && m_dst.color == JC_RGB)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer;
		src[1] = m_ccomp[0]->m_cc_buffer;
		src[2] = m_ccomp[0]->m_cc_buffer;

		status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// Gray to BGR
	if(m_jpeg_color == JC_GRAY && m_dst.color == JC_BGR)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer;
		src[1] = m_ccomp[0]->m_cc_buffer;
		src[2] = m_ccomp[0]->m_cc_buffer;

		status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// RGB to RGB
	if(m_jpeg_color == JC_RGB && m_dst.color == JC_RGB)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer;
		src[1] = m_ccomp[1]->m_cc_buffer;
		src[2] = m_ccomp[2]->m_cc_buffer;

		status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// RGB to BGR
	if(m_jpeg_color == JC_RGB && m_dst.color == JC_BGR)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[2]->m_cc_buffer;
		src[1] = m_ccomp[1]->m_cc_buffer;
		src[2] = m_ccomp[0]->m_cc_buffer;

		status = ippiCopy_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// YCbCr to RGB
	if(m_jpeg_color == JC_YCBCR && m_dst.color == JC_RGB)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer;
		src[1] = m_ccomp[1]->m_cc_buffer;
		src[2] = m_ccomp[2]->m_cc_buffer;

		status = ippiYCbCrToRGB_JPEG_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiYCbCrToRGB_JPEG_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// YCbCr to BGR
	if(m_jpeg_color == JC_YCBCR && m_dst.color == JC_BGR)
	{
		const Ipp8u* src[3];
		src[0] = m_ccomp[0]->m_cc_buffer;
		src[1] = m_ccomp[1]->m_cc_buffer;
		src[2] = m_ccomp[2]->m_cc_buffer;

		status = ippiYCbCrToBGR_JPEG_8u_P3C3R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiYCbCrToBGR_JPEG_8u_P3C3R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// CMYK to CMYK
	if(m_jpeg_color == JC_CMYK && m_dst.color == JC_CMYK)
	{
		const Ipp8u* src[4];
		src[0] = m_ccomp[0]->m_cc_buffer;
		src[1] = m_ccomp[1]->m_cc_buffer;
		src[2] = m_ccomp[2]->m_cc_buffer;
		src[3] = m_ccomp[3]->m_cc_buffer;

		status = ippiCopy_8u_P4C4R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiCopy_8u_P4C4R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	// YCCK to CMYK
	if(m_jpeg_color == JC_YCCK && m_dst.color == JC_CMYK)
	{
		const Ipp8u* src[4];
		src[0] = m_ccomp[0]->m_cc_buffer;
		src[1] = m_ccomp[1]->m_cc_buffer;
		src[2] = m_ccomp[2]->m_cc_buffer;
		src[3] = m_ccomp[3]->m_cc_buffer;

		status = ippiYCCKToCMYK_JPEG_8u_P4C4R(src,m_ccWidth,dst,m_dst.lineStep,roi);

		if(ippStsNoErr != status)
		{
			LOG1("IPP Error: ippiYCCKToCMYK_JPEG_8u_P4C4R() failed - ",status);
			return JPEG_INTERNAL_ERROR;
		}
	}

	return JPEG_OK;
} // CJPEGDecoder::ColorConvert()

#define STEP_SS ((m_ccWidth>>1)+2)

JERRCODE CJPEGDecoder::UpSampling(int nMCURow,int idThread)
{
	int i, k;
	int threadOffsetCC;//OMP
	int threadOffsetSS; // OMP
	IppStatus status;

	threadOffsetCC = (m_numxMCU*m_mcuWidth)*(idThread*m_mcuHeight);//OMP
	threadOffsetSS = 0; // OMP

	if(m_jpeg_sampling == JS_422)
		threadOffsetSS = m_numxMCU*((m_mcuWidth>>1)+2) * idThread * m_mcuHeight;//OMP
	else if(m_jpeg_sampling == JS_411)
		threadOffsetSS = m_numxMCU*((m_mcuWidth>>1)+2) * idThread * ((m_mcuHeight>>1)+2);//OMP

	for(k = 0; k < m_jpeg_ncomp; k++)
	{
		// sampling 444
		if(m_ccomp[k]->m_h_factor == 1 && m_ccomp[k]->m_v_factor == 1)
		{
			Ipp8u* src = m_ccomp[k]->m_ss_buffer;
			Ipp8u* dst = m_ccomp[k]->m_cc_buffer;
			if(src)
			{
				ippsCopy_8u(src,dst,m_ccWidth*m_mcuHeight);
			}
		}

		// sampling 422
		if(m_ccomp[k]->m_h_factor == 2 && m_ccomp[k]->m_v_factor == 1)
		{
			// pad most left and most right column
			Ipp8u* ss_buf = m_ccomp[k]->m_ss_buffer + threadOffsetSS;
			Ipp8u* cc_buf = m_ccomp[k]->m_cc_buffer + threadOffsetCC;
			Ipp8u* p1 = ss_buf;
			Ipp8u* p2 = ss_buf + (m_ccWidth>>1) + 1;
			int step  = STEP_SS;

			for(i = 0; i < m_ccHeight; i++)
			{
				p1[0] = p1[ 1];
				p2[0] = p2[-1];
				p1 += step;
				p2 += step;
			}

			IppiSize roiSrc = { m_ccWidth>>1, m_mcuHeight };
			IppiSize roiDst = { m_ccWidth,    m_mcuHeight };
			Ipp8u* src = ss_buf + 1;
			Ipp8u* dst = cc_buf;

			status = ippiSampleUpH2V1_JPEG_8u_C1R(src,STEP_SS,roiSrc,dst,m_ccWidth,roiDst);
			if(ippStsNoErr != status)
			{
				LOG0("Error: ippiSampleUpH2V1_JPEG_8u_C1R() failed!");
				return JPEG_INTERNAL_ERROR;
			}
		}

		// sampling 411
		if(m_ccomp[k]->m_h_factor == 2 && m_ccomp[k]->m_v_factor == 2)
		{
			// pad most left and most right columns
			Ipp8u* ss_buf = m_ccomp[k]->m_ss_buffer + threadOffsetSS;
			Ipp8u* cc_buf = m_ccomp[k]->m_cc_buffer + threadOffsetCC;
			Ipp8u* p1 = ss_buf;
			Ipp8u* p2 = ss_buf + (m_ccWidth>>1) + 1;
			int step  = STEP_SS;

			for(i = 0; i < (m_ccHeight>>1); i++)
			{
				p1[0] = p1[ 1];
				p2[0] = p2[-1];
				p1 += step;
				p2 += step;
			}
			IppiSize roiSrc = { m_ccWidth>>1, m_mcuHeight>>1 };
			IppiSize roiDst = { m_ccWidth,    m_mcuHeight    };
			Ipp8u* src = ss_buf + STEP_SS + 1;
			Ipp8u* dst = cc_buf;
			if(nMCURow == 0)
			{
				p1 = ss_buf + STEP_SS;
				p2 = ss_buf;
				ippsCopy_8u(p1,p2,STEP_SS);

				p1 = ss_buf + STEP_SS*(8+0);
				p2 = ss_buf + STEP_SS*(8+1);
				ippsCopy_8u(p1,p2,STEP_SS);

				p1 = m_ccomp[k]->m_top_row;
				ippsCopy_8u(p2,p1,STEP_SS);
			}
			else
			{
				p1 = m_ccomp[k]->m_top_row;
				p2 = ss_buf;
				ippsCopy_8u(p1,p2,STEP_SS);

				p2 = ss_buf + STEP_SS*(8+0);
				ippsCopy_8u(p2,p1,STEP_SS);

				p1 = ss_buf + STEP_SS*(8+0);
				p2 = ss_buf + STEP_SS*(8+1);
				ippsCopy_8u(p1,p2,STEP_SS);
			}

			status = ippiSampleUpH2V2_JPEG_8u_C1R(src,STEP_SS,roiSrc,dst,m_ccWidth,roiDst);
			if(ippStsNoErr != status)
			{
				LOG0("Error: ippiSampleUpH2V2_JPEG_8u_C1R() failed!");
				return JPEG_INTERNAL_ERROR;
			}
		}//411
	} // for m_jpeg_ncomp

	return JPEG_OK;
} // CJPEGDecoder::UpSampling()


JERRCODE CJPEGDecoder::UpSampling(void)
{
	int i, k;
	IppStatus status;

	for(k = 0; k < m_jpeg_ncomp; k++)
	{
		// sampling 444
		if(m_ccomp[k]->m_h_factor == 1 && m_ccomp[k]->m_v_factor == 1)
		{
			Ipp8u* src = m_ccomp[k]->m_ss_buffer;
			Ipp8u* dst = m_ccomp[k]->m_cc_buffer;
			IppiSize roi;
			roi.width  = m_dst.width;
			roi.height = m_dst.height;

			ippiCopy_8u_C1R(src,m_ccWidth,dst,m_ccWidth,roi);
		}

		// sampling 422
		if(m_ccomp[k]->m_h_factor == 2 && m_ccomp[k]->m_v_factor == 1)
		{
			// pad most left and most right columns
			Ipp8u* p1 = m_ccomp[k]->m_ss_buffer;
			Ipp8u* p2 = m_ccomp[k]->m_ss_buffer + (m_ccWidth>>1) + 1;
			int step  = m_ccWidth;

			for(i = 0; i < m_ccHeight; i++)
			{
				p1[0] = p1[ 1];
				p2[0] = p2[-1];
				p1 += step;
				p2 += step;
			}

			IppiSize roiSrc = { m_ccWidth>>1, m_ccHeight };
			IppiSize roiDst = { m_dst.width, m_dst.height };
			Ipp8u* src = m_ccomp[k]->m_ss_buffer+1;
			Ipp8u* dst = m_ccomp[k]->m_cc_buffer;

			status = ippiSampleUpH2V1_JPEG_8u_C1R(src,m_ccWidth,roiSrc,dst,m_ccWidth,roiDst);
			if(ippStsNoErr != status)
			{
				LOG0("Error: ippiSampleUpH2V1_JPEG_8u_C1R() failed!");
				return JPEG_INTERNAL_ERROR;
			}
		}

		// sampling 411
		if(m_ccomp[k]->m_h_factor == 2 && m_ccomp[k]->m_v_factor == 2)
		{
			// pad most left and most right columns
			Ipp8u* p1 = m_ccomp[k]->m_ss_buffer;
			Ipp8u* p2 = m_ccomp[k]->m_ss_buffer + (m_ccWidth>>1) + 1;
			int step  = m_ccWidth;

			for(i = 0; i < m_ccHeight>>1; i++)
			{
				p1[0] = p1[ 1];
				p2[0] = p2[-1];
				p1 += step;
				p2 += step;
			}

			// replicate top row
			p1 = m_ccomp[k]->m_ss_buffer + m_ccWidth;
			p2 = m_ccomp[k]->m_ss_buffer;
			ippsCopy_8u(p1,p2,m_ccWidth);

			// replicate bottom row
			p1 = m_ccomp[k]->m_ss_buffer + m_ccWidth*m_ccHeight-1;
			p2 = m_ccomp[k]->m_ss_buffer + m_ccWidth*m_ccHeight;
			ippsCopy_8u(p1,p2,m_ccWidth);

			IppiSize roiSrc = { m_ccWidth>>1, m_ccHeight>>1 };
			IppiSize roiDst = { m_dst.width, m_dst.height };
			Ipp8u* src = m_ccomp[k]->m_ss_buffer + m_ccWidth + 1;
			Ipp8u* dst = m_ccomp[k]->m_cc_buffer;

			status = ippiSampleUpH2V2_JPEG_8u_C1R(src,m_ccWidth,roiSrc,dst,m_ccWidth,roiDst);
			if(ippStsNoErr != status)
			{
				LOG0("Error: ippiSampleUpH2V2_JPEG_8u_C1R() failed!");
				return JPEG_INTERNAL_ERROR;
			}
		}
	} // for m_jpeg_ncomp

	return JPEG_OK;
} // CJPEGDecoder::UpSampling()


JERRCODE CJPEGDecoder::PerformDCT(void)
{
	int       i, j, n, k, l;
	int       size;
	int       dst_step;
	Ipp8u*    dst;
	Ipp16u*   qtbl;
	Ipp16s*   block;
	IppStatus status;

	dst_step = m_ccWidth;

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
						dst  = m_ccomp[n]->m_ss_buffer +
							i*8*m_ccomp[n]->m_vsampling*m_ccWidth +
							j*8*m_ccomp[n]->m_hsampling +
							k*8*m_ccWidth;

						if(m_ccomp[n]->m_v_factor == 2)
						{
							dst += m_ccWidth;
						}

						if(m_ccomp[n]->m_h_factor == 2)
						{
							dst++;
						}

						dst += l*8;

						status = ippiDCTQuantInv8x8LS_JPEG_16s8u_C1R(
								block,
								dst,
								dst_step,
								qtbl);

						if(ippStsNoErr != status)
						{
							LOG0("Error: ippiDCTQuantInv8x8LS_JPEG_16s8u_C1R() failed!");
							return JPEG_INTERNAL_ERROR;
						}

						block += DCTSIZE2;
					} // for m_hsampling
				} // for m_vsampling
			}
		}
	}

	return JPEG_OK;
} // CJPEGDecoder::PerformDCT()


JERRCODE CJPEGDecoder::ReadHeader(
		int*    width,
		int*    height,
		int*    nchannels,
		JCOLOR* color,
		JSS*    sampling,
		int*    precision)
{
	JERRCODE jerr;

	jerr = ParseJPEGBitStream(JO_READ_HEADER);

	if(JPEG_OK != jerr)
	{
		LOG0("Error: ParseJPEGBitStream() failed");
		return jerr;
	}

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

	m_numxMCU = (m_jpeg_width  + (m_mcuWidth  - 1)) / m_mcuWidth;
	m_numyMCU = (m_jpeg_height + (m_mcuHeight - 1)) / m_mcuHeight;

	m_xPadding = m_numxMCU * m_mcuWidth  - m_jpeg_width;
	m_yPadding = m_numyMCU * m_mcuHeight - m_jpeg_height;

	m_ccWidth  = m_mcuWidth * m_numxMCU;
	m_ccHeight = (JPEG_PROGRESSIVE == m_jpeg_mode) ? m_mcuHeight * m_numyMCU : m_mcuHeight;

	*width     = m_jpeg_width;
	*height    = m_jpeg_height;
	*nchannels = m_jpeg_ncomp;
	*precision = m_jpeg_precision;
	*color     = m_jpeg_color;
	*sampling  = m_jpeg_sampling;

	return JPEG_OK;
} // CJPEGDecoder::ReadHeader()


JERRCODE CJPEGDecoder::ReadData(void)
{
	return ParseJPEGBitStream(JO_READ_DATA);
} // CJPEGDecoder::ReadData()


JERRCODE CJPEGDecoder::DecodeHuffmanMCURowBL(Ipp16s* pMCUBuf)
{
	int       j, n, k, l;
	Ipp8u*    src;
	int       srcLen;
	JERRCODE  jerr;
	IppStatus status;

	src    = m_src.pData;
	srcLen = m_src.DataLen;

	for(j = 0; j < m_numxMCU; j++)
	{
		if(m_jpeg_restart_interval)
		{
			if(m_restarts_to_go == 0)
			{
				jerr = ProcessRestart();
				if(JPEG_OK != jerr)
				{
					LOG0("Error: ProcessRestart() failed!");
					return jerr;
				}
			}
		}

		for(n = 0; n < m_jpeg_ncomp; n++)
		{
			Ipp16s*                lastDC = &m_ccomp[n]->m_lastDC;
			IppiDecodeHuffmanSpec* dctbl  = m_dctbl[m_ccomp[n]->m_dc_selector];
			IppiDecodeHuffmanSpec* actbl  = m_actbl[m_ccomp[n]->m_ac_selector];

			for(k = 0; k < m_ccomp[n]->m_vsampling; k++)
			{
				for(l = 0; l < m_ccomp[n]->m_hsampling; l++)
				{
					status = ippiDecodeHuffman8x8_JPEG_1u16s_C1(
							src,
							srcLen,
							&m_src.currPos,
							pMCUBuf,
							lastDC,
							(int*)&m_marker,
							dctbl,
							actbl,
							m_state);

					if(ippStsNoErr > status)
					{
						LOG0("Error: ippiDecodeHuffman8x8_JPEG_1u16s_C1() failed!");
						return JPEG_INTERNAL_ERROR;
					}

					pMCUBuf += DCTSIZE2;
				} // for m_hsampling
			} // for m_vsampling
		} // for m_jpeg_ncomp

		m_restarts_to_go--;
	} // for m_numxMCU

	return JPEG_OK;
} // CJPEGDecoder::DecodeHuffmanMCURowBL()


JERRCODE CJPEGDecoder::DecodeHuffmanMCURowLS(Ipp16s* pMCUBuf)
{
	int       j, n, k, l;
	Ipp8u*    src;
	int       srcLen;
	JERRCODE  jerr;
	IppStatus status;

	src    = m_src.pData;
	srcLen = m_src.DataLen;

	for(j = 0; j < m_numxMCU; j++)
	{
		if(m_jpeg_restart_interval)
		{
			if(m_restarts_to_go == 0)
			{
				jerr = ProcessRestart();
				if(JPEG_OK != jerr)
				{
					LOG0("Error: ProcessRestart() failed!");
					return jerr;
				}
			}
		}

		for(n = 0; n < m_jpeg_ncomp; n++)
		{
			IppiDecodeHuffmanSpec* dctbl = m_dctbl[m_ccomp[n]->m_dc_selector];

			for(k = 0; k < m_ccomp[n]->m_vsampling; k++)
			{
				for(l = 0; l < m_ccomp[n]->m_hsampling; l++)
				{
					status = ippiDecodeHuffmanOne_JPEG_1u16s_C1(
							src,
							srcLen,
							&m_src.currPos,
							pMCUBuf,
							(int*)&m_marker,
							dctbl,
							m_state);

					if(ippStsNoErr > status)
					{
						LOG0("Error: ippiDecodeHuffmanOne_JPEG_1u16s_C1() failed!");
						return JPEG_INTERNAL_ERROR;
					}

					pMCUBuf++;
				} // for m_hsampling
			} // for m_vsampling
		} // for m_jpeg_ncomp

		m_restarts_to_go --;
	} // for m_numxMCU

	return JPEG_OK;
} // CJPEGDecoder::DecodeHuffmanMCURowLS()


JERRCODE CJPEGDecoder::DCT_QNT_SSCC_MCURowBL(Ipp16s* pMCUBuf,int idThread,int mcu_row)
{
	int       j, n, k, l;
	int       threadOffsetCC;
	int       threadOffsetSS;
	Ipp8u*    dst      = 0;
	int       dst_step = m_ccWidth;
	JERRCODE  jerr;
	IppStatus status;

	threadOffsetCC = (m_numxMCU*m_mcuWidth)*(idThread*m_mcuHeight);
	threadOffsetSS = 0;

	if(m_jpeg_sampling == JS_422)
		threadOffsetSS = m_numxMCU*((m_mcuWidth>>1)+2) * idThread * m_mcuHeight; // OMP
	else if(m_jpeg_sampling == JS_411)
		threadOffsetSS = m_numxMCU*((m_mcuWidth>>1)+2) * idThread * ((m_mcuHeight>>1)+2); // OMP

	for(j = 0; j < m_numxMCU; j++)
	{
		for(n = 0; n < m_jpeg_ncomp; n++)
		{
			Ipp8u* ss_buf = m_ccomp[n]->m_ss_buffer + threadOffsetSS;
			Ipp8u* cc_buf = m_ccomp[n]->m_cc_buffer + threadOffsetCC;

			Ipp16u* qtbl = m_qntbl[m_ccomp[n]->m_q_selector];

			int hsam  = m_ccomp[n]->m_hsampling;
			int jhsam = j*8*hsam;

			for(k = 0; k < m_ccomp[n]->m_vsampling; k++)
			{
				if(m_jpeg_sampling == JS_444 || n == 0 || n == 3)
				{
					dst_step = m_ccWidth;
					dst      = cc_buf + jhsam + k*8*dst_step;
				}
				else
				{
					dst_step = (m_ccWidth >> 1) + 2;
					dst      = ss_buf + jhsam + k*8*dst_step;
				}

				if(m_ccomp[n]->m_v_factor == 2)
				{
					dst += dst_step;
				}

				if(m_ccomp[n]->m_h_factor == 2)
				{
					dst++;
				}

				for(l = 0; l < hsam; l++)
				{
					dst += l*8;

					status = ippiDCTQuantInv8x8LS_JPEG_16s8u_C1R(
							pMCUBuf,
							dst,
							dst_step,
							qtbl);

					if(ippStsNoErr > status)
					{
						LOG0("Error: ippiDCTQuantInv8x8LS_JPEG_16s8u_C1R() failed!");
						return JPEG_INTERNAL_ERROR;
					}

					pMCUBuf += DCTSIZE2;
				} // for m_hsampling
			} // for m_vsampling
		} // for m_jpeg_ncomp
	} // for m_numxMCU

	if(m_jpeg_sampling != JS_444)
	{
		jerr = UpSampling(mcu_row,idThread);
		if(JPEG_OK != jerr)
		{
			return jerr;
		}
	}

	jerr = ColorConvert(mcu_row,idThread);
	if(JPEG_OK != jerr)
	{
		return jerr;
	}

	return JPEG_OK;
} // CJPEGDecoder::DCT_QNT_SSCC_MCURowBL()


JERRCODE CJPEGDecoder::ReconstructMCURowLS(Ipp16s* pMCUBuf,int idThread,int mcu_row)
{
	Ipp16s*   pCurrRow;
	Ipp16s*   pPrevRow;
	Ipp8u*    pDst;
	IppiSize  roi;
	IppStatus status;

	pCurrRow = m_ccomp[0]->m_curr_row;
	pPrevRow = m_ccomp[0]->m_prev_row;

	roi.width  = m_dst.width;
	roi.height = 1;

	pDst = (Ipp8u*)m_dst.p.Data8u + mcu_row*m_dst.width;

	if(mcu_row != 0)
	{
		status = ippiReconstructPredRow_JPEG_16s_C1(
				pMCUBuf,pPrevRow,pCurrRow,m_dst.width,m_ss);
	}
	else
	{
		status = ippiReconstructPredFirstRow_JPEG_16s_C1(
				pMCUBuf,pCurrRow,m_dst.width,m_jpeg_precision,m_al);
	}

	if(ippStsNoErr != status)
	{
		return JPEG_INTERNAL_ERROR;
	}

	if(m_al)
	{
		status = ippsLShiftC_16s_I(m_al,pCurrRow,m_dst.width);
		if(ippStsNoErr != status)
		{
			return JPEG_INTERNAL_ERROR;
		}
	}

	status = ippiConvert_16s8u_C1R(pCurrRow,m_dst.width*sizeof(Ipp16s),pDst,m_dst.width,roi);
	if(ippStsNoErr != status)
	{
		return JPEG_INTERNAL_ERROR;
	}

	m_ccomp[0]->m_curr_row = pPrevRow;
	m_ccomp[0]->m_prev_row = pCurrRow;

	return JPEG_OK;
} // CJPEGDecoder::ReconstructMCURowLS()


JERRCODE CJPEGDecoder::DecodeScanBaseline(void)
{
	int scount = 0;
	IppStatus status;

	status = ippiDecodeHuffmanStateInit_JPEG_8u(m_state);
	if(ippStsNoErr != status)
	{
		return JPEG_INTERNAL_ERROR;
	}

	m_marker = JM_NONE;

#ifdef _OPENMP
#pragma  omp parallel default(shared)
#endif

	{
		int     i;
		int     idThread = 0;
		Ipp16s* pMCUBuf;  // the pointer to Buffer for a current thread.

#ifdef _OPENMP
		idThread = omp_get_thread_num(); // the thread id of the calling thread.
#endif

		pMCUBuf = m_block_buffer + idThread * m_numxMCU * m_nblock * DCTSIZE2;

		i = 0;

		while(i < m_numyMCU)
		{
#ifdef _OPENMP
#pragma omp critical (JPEG_OMP)
#endif
			{
				i = scount;
				scount++;
				if(i < m_numyMCU)
				{
					DecodeHuffmanMCURowBL(pMCUBuf);
				}
			}

			if(i < m_numyMCU)
			{
				DCT_QNT_SSCC_MCURowBL(pMCUBuf, idThread, i);
			}

			i++;
		} // for m_numyMCU
	} // OMP

	if(m_jpeg_sampling == JS_411)
		set_num_threads(m_num_threads);

	return JPEG_OK;
} // CJPEGDecoder::DecodeScanBaseline()


JERRCODE CJPEGDecoder::DecodeScanProgressive(void)
{
	int       i, j, k, n, l, c;
	int       size;
	Ipp8u*    src;
	int       srcLen;
	JERRCODE  jerr;
	IppStatus status;

	m_scan_count++;

	status = ippiDecodeHuffmanStateInit_JPEG_8u(m_state);
	if(ippStsNoErr != status)
	{
		return JPEG_INTERNAL_ERROR;
	}

	m_marker = JM_NONE;

	src    = m_src.pData;
	srcLen = m_src.DataLen;

	for(size = 0, k = 0; k < m_jpeg_ncomp; k++)
	{
		size += (m_ccomp[k]->m_hsampling * m_ccomp[k]->m_vsampling);
	}

	Ipp16s* block;

	if(m_ss != 0 && m_se != 0)
	{
		// AC scan
		for(i = 0; i < m_numyMCU; i++)
		{
			for(k = 0; k < m_ccomp[m_curr_comp_no]->m_vsampling; k++)
			{
				if(i*m_ccomp[m_curr_comp_no]->m_vsampling*8 + k*8 >= m_jpeg_height)
					break;

				for(j = 0; j < m_numxMCU; j++)
				{
					block = m_coefbuf + (DCTSIZE2*size*(j+(i*m_numxMCU)));

					// skip any relevant components
					for(c = 0; c < m_ccomp[m_curr_comp_no]->m_comp_no; c++)
					{
						block += (DCTSIZE2*m_ccomp[c]->m_hsampling*
								m_ccomp[c]->m_vsampling);
					}

					// Skip over relevant 8x8 blocks from this component.
					block += (k * DCTSIZE2 * m_ccomp[m_curr_comp_no]->m_hsampling);

					for(l = 0; l < m_ccomp[m_curr_comp_no]->m_hsampling; l++)
					{
						// Ignore the last column(s) of the image.
						if(((j*m_ccomp[m_curr_comp_no]->m_hsampling*8) + (l*8)) >= m_jpeg_width)
							break;

						if(m_jpeg_restart_interval)
						{
							if(m_restarts_to_go == 0)
							{
								jerr = ProcessRestart();
								if(JPEG_OK != jerr)
								{
									LOG0("Error: ProcessRestart() failed!");
									return jerr;
								}
							}
						}

						IppiDecodeHuffmanSpec* actbl = m_actbl[m_ccomp[m_curr_comp_no]->m_ac_selector];

						if(m_ah == 0)
						{
							status = ippiDecodeHuffman8x8_ACFirst_JPEG_1u16s_C1(
									src,
									srcLen,
									&m_src.currPos,
									block,
									(int*)&m_marker,
									m_ss,
									m_se,
									m_al,
									actbl,
									m_state);

							if(ippStsNoErr > status)
							{
								LOG0("Error: ippiDecodeHuffman8x8_ACFirst_JPEG_1u16s_C1() failed!");
								return JPEG_INTERNAL_ERROR;
							}
						}
						else
						{
							status = ippiDecodeHuffman8x8_ACRefine_JPEG_1u16s_C1(
									src,
									srcLen,
									&m_src.currPos,
									block,
									(int*)&m_marker,
									m_ss,
									m_se,
									m_al,
									actbl,
									m_state);

							if(ippStsNoErr > status)
							{
								LOG0("Error: ippiDecodeHuffman8x8_ACRefine_JPEG_1u16s_C1() failed!");
								return JPEG_INTERNAL_ERROR;
							}
						}

						block += DCTSIZE2;

						m_restarts_to_go --;
					} // for m_hsampling
				} // for m_numxMCU
			} // for m_vsampling
		} // for m_numyMCU
		if(m_al == 0)
		{
			m_ccomp[m_curr_comp_no]->m_ac_scan_completed = 1;
		}
	}
	else
	{
		// DC scan
		for(i = 0; i < m_numyMCU; i++)
		{
			for(j = 0; j < m_numxMCU; j++)
			{
				if(m_jpeg_restart_interval)
				{
					if(m_restarts_to_go == 0)
					{
						jerr = ProcessRestart();
						if(JPEG_OK != jerr)
						{
							LOG0("Error: ProcessRestart() failed!");
							return jerr;
						}
					}
				}

				block = m_coefbuf + (DCTSIZE2*size*(j+(i*m_numxMCU)));

				if(m_ah == 0)
				{
					// first DC scan
					for(n = 0; n < m_jpeg_ncomp; n++)
					{
						Ipp16s* lastDC = &m_ccomp[n]->m_lastDC;
						IppiDecodeHuffmanSpec* dctbl = m_dctbl[m_ccomp[n]->m_dc_selector];

						for(k = 0; k < m_ccomp[n]->m_vsampling; k++)
						{
							for(l = 0; l < m_ccomp[n]->m_hsampling; l++)
							{
								status = ippiDecodeHuffman8x8_DCFirst_JPEG_1u16s_C1(
										src,
										srcLen,
										&m_src.currPos,
										block,
										lastDC,
										(int*)&m_marker,
										m_al,
										dctbl,
										m_state);

								if(ippStsNoErr > status)
								{
									LOG0("Error: ippiDecodeHuffman8x8_DCFirst_JPEG_1u16s_C1() failed!");
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
								status = ippiDecodeHuffman8x8_DCRefine_JPEG_1u16s_C1(
										src,
										srcLen,
										&m_src.currPos,
										block,
										(int*)&m_marker,
										m_al,
										m_state);

								if(ippStsNoErr > status)
								{
									LOG0("Error: ippiDecodeHuffman8x8_DCRefine_JPEG_1u16s_C1() failed!");
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

		if(m_al == 0)
		{
			m_dc_scan_completed = 1;
		}
	}

	return JPEG_OK;
} // CJPEGDecoder::DecodeScanProgressive()


JERRCODE CJPEGDecoder::DecodeScanLossless(void)
{
#ifdef __TIMING__
	Ipp64u   c0;
	Ipp64u   c1;
#endif

	JERRCODE  jerr;
	IppStatus status;

	status = ippiDecodeHuffmanStateInit_JPEG_8u(m_state);
	if(ippStsNoErr != status)
	{
		return JPEG_INTERNAL_ERROR;
	}

	m_marker = JM_NONE;

	int     i;
	int     idThread = 0;
	Ipp16s* pMCUBuf;

	pMCUBuf = m_block_buffer;

	i = 0;

	while(i < m_numyMCU)
	{
		{
			if(i < m_numyMCU)
			{
#ifdef __TIMING__
				c0 = ippCoreGetCpuClocks();
#endif
				jerr = DecodeHuffmanMCURowLS(pMCUBuf);
				if(JPEG_OK != jerr)
				{
					return jerr;
				}
#ifdef __TIMING__
				c1 = ippCoreGetCpuClocks();
				m_clk_huff += c1 - c0;
#endif
			}

			i++; // advance counter to the next mcu row
		}

		if((i-1) < m_numyMCU)
		{
#ifdef __TIMING__
			c0 = ippCoreGetCpuClocks();
#endif
			jerr = ReconstructMCURowLS(pMCUBuf, idThread, i-1);
			if(JPEG_OK != jerr)
			{
				return jerr;
			}
#ifdef __TIMING__
			c1 = ippCoreGetCpuClocks();
			m_clk_diff += c1 - c0;
#endif
		}
	} // for m_numyMCU

	return JPEG_OK;
} // CJPEGDecoder::DecodeScanLossless()

