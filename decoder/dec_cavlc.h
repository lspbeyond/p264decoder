/*****************************************************************************
 * dec_cavlc.h: h264 decoder library
 *****************************************************************************
 * Copyright (C) 2005 p264 project
 * $Id: $
 *
 * Authors: Peter Lee (lspbeyond@126.com)
 * 2005-2006 at icas of Ningbo university, China
 * p264 Decoder site: http://p264decoder.zj.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#ifndef _DEC_CAVLC_H_
#define _DEC_CAVLC_H_

#define BLOCK_INDEX_CHROMA_DC   (-1)
#define BLOCK_INDEX_LUMA_DC     (-2)

int p264dec_read_residual_block_cavlc( p264_t *h, bs_t *s, int idx, int *l, int i_count );

#endif