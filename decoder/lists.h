/*****************************************************************************
 * lists.h: h264 decoder library
 *****************************************************************************
 * Copyright (C) 2006 p264 project
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

#ifndef _LISTS_H_
#define _LISTS_H_

enum ref_type_e
{
  NOT_USED_FOR_REF=0,
  SHORT_TERM_REF,
  LONG_TERM_REF
};

int p264_lists_init(p264_t *h);
int p264_lists_reorder(p264_t *h);
int p264_lists_marking(p264_t *h);

#endif