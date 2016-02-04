/*****************************************************************************
 * p264: h264 encoder
 *****************************************************************************
 * Copyright (C) 2005 p264 project
 *
 * Author: Tuukka Toivonen <tuukkat@ee.oulu.fi>
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

#ifndef _VISUALIZE_H
#define _VISUALIZE_H 1

#include "core/core.h"

void p264_visualize_init( p264_t *h );
void p264_visualize_mb( p264_t *h );
void p264_visualize_show( p264_t *h );
void p264_visualize_close( p264_t *h );

#endif
