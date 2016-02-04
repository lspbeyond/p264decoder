/*****************************************************************************
* dec_cavlc.c: h264 decoder library
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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "core/core.h"
#include "dec_cavlc.h"

typedef struct  
{
    uint8_t len;
    uint8_t trailing_ones;
    uint8_t total_coeff;
} vlc_coeff_token_t;
#define VLC(a, b, c) {a, b, c}
#define VLC2(a, b, c) VLC(a, b, c), VLC(a, b, c)
#define VLC4(a, b, c) VLC2(a, b, c), VLC2(a, b, c)

/* ++ cavlc tables ++ */
static const vlc_coeff_token_t coeff4_0[] = 
{
    VLC(6, 0, 2),   /* 0001 00 */
    VLC(6, 3, 3),   /* 0001 01 */
    VLC(6, 1, 2),   /* 0001 10 */
    VLC(6, 0, 1),   /* 0001 11 */
};

static const vlc_coeff_token_t coeff4_1[] = 
{
    VLC2(7, 3, 4),   /* 0000 000(0) */
    VLC(8, 2, 4),   /* 0000 0010 */
    VLC(8, 1, 4),   /* 0000 0011 */
    VLC2(7, 2, 3),   /* 0000 010(0) */
    VLC2(7, 1, 3),   /* 0000 011(0) */
    VLC4(6, 0, 4),   /* 0000 10(00) */
    VLC4(6, 0, 3),   /* 0000 11(00) */
};

static const vlc_coeff_token_t coeff3_0[] =
{
    VLC(6, 0, 1),    /* 0000 00 */ 
    VLC(6, 1, 1),    /* 0000 01 */ 
    VLC(-1, -1, -1), /* 0000 10 */ 
    VLC(6, 0, 0),    /* 0000 11 */
    VLC(6, 0, 2),    /* 0001 00 */
    VLC(6, 1, 2),    /* 0001 01 */
    VLC(6, 2, 2),    /* 0001 10 */
    VLC(-1, -1, -1), /* 0001 11 */
    VLC(6, 0, 3),    /* 0010 00 */
    VLC(6, 1, 3),    /* 0010 01 */
    VLC(6, 2, 3),    /* 0010 10 */
    VLC(6, 3, 3),    /* 0010 11 */
    VLC(6, 0, 4),    /* 0011 00 */
    VLC(6, 1, 4),    /* 0011 01 */
    VLC(6, 2, 4),    /* 0011 10 */
    VLC(6, 3, 4),    /* 0011 11 */
    VLC(6, 0, 5),    /* 0100 00 */
    VLC(6, 1, 5),    /* 0100 01 */
    VLC(6, 2, 5),    /* 0100 10 */
    VLC(6, 3, 5),    /* 0100 11 */
    VLC(6, 0, 6),    /* 0101 00 */
    VLC(6, 1, 6),    /* 0101 01 */
    VLC(6, 2, 6),    /* 0101 10 */
    VLC(6, 3, 6),    /* 0101 11 */
    VLC(6, 0, 7),    /* 0110 00 */
    VLC(6, 1, 7),    /* 0110 01 */
    VLC(6, 2, 7),    /* 0110 10 */
    VLC(6, 3, 7),    /* 0110 11 */
    VLC(6, 0, 8),
    VLC(6, 1, 8),
    VLC(6, 2, 8),
    VLC(6, 3, 8),
    VLC(6, 0, 9),
    VLC(6, 1, 9),
    VLC(6, 2, 9),
    VLC(6, 3, 9),
    VLC(6, 0, 10),
    VLC(6, 1, 10),
    VLC(6, 2, 10),
    VLC(6, 3, 10),
    VLC(6, 0, 11),
    VLC(6, 1, 11),
    VLC(6, 2, 11),
    VLC(6, 3, 11),
    VLC(6, 0, 12),
    VLC(6, 1, 12),
    VLC(6, 2, 12),
    VLC(6, 3, 12),
    VLC(6, 0, 13),
    VLC(6, 1, 13),
    VLC(6, 2, 13),
    VLC(6, 3, 13),
    VLC(6, 0, 14),
    VLC(6, 1, 14),
    VLC(6, 2, 14),
    VLC(6, 3, 14),
    VLC(6, 0, 15),
    VLC(6, 1, 15),
    VLC(6, 2, 15),
    VLC(6, 3, 15),
    VLC(6, 0, 16),
    VLC(6, 1, 16),
    VLC(6, 2, 16),
    VLC(6, 3, 16)
};

static const vlc_coeff_token_t coeff2_0[] = 
{
    VLC(4, 3, 7),   /* 1000 */
    VLC(4, 3, 6),   /* 1001 */
    VLC(4, 3, 5),   /* 1010 */
    VLC(4, 3, 4),   /* 1011 */
    VLC(4, 3, 3),   /* 1100 */
    VLC(4, 2, 2),   /* 1101 */
    VLC(4, 1, 1),   /* 1110 */
    VLC(4, 0, 0),   /* 1111 */
};

static const vlc_coeff_token_t coeff2_1[] = 
{
    VLC(5, 1, 5),   /* 0100 0 */
    VLC(5, 2, 5),
    VLC(5, 1, 4),
    VLC(5, 2, 4),
    VLC(5, 1, 3),
    VLC(5, 3, 8),
    VLC(5, 2, 3),
    VLC(5, 1, 2),
};

static const vlc_coeff_token_t coeff2_2[] = 
{
    VLC(6, 0, 3),   /* 0010 00 */
    VLC(6, 2, 7),
    VLC(6, 1, 7),
    VLC(6, 0, 2),
    VLC(6, 3, 9),
    VLC(6, 2, 6),
    VLC(6, 1, 6),
    VLC(6, 0, 1),
};

static const vlc_coeff_token_t coeff2_3[] = 
{
    VLC(7, 0, 7),   /* 0001 000 */
    VLC(7, 0, 6),
    VLC(7, 2, 9),
    VLC(7, 0, 5),
    VLC(7, 3, 10),
    VLC(7, 2, 8),
    VLC(7, 1, 8),
    VLC(7, 0, 4),
};

static const vlc_coeff_token_t coeff2_4[] = 
{
    VLC(8, 3, 12),   /* 0000 1000 */
    VLC(8, 2, 11),
    VLC(8, 1, 10),
    VLC(8, 0, 9),
    VLC(8, 3, 11),
    VLC(8, 2, 10),
    VLC(8, 1, 9),
    VLC(8, 0, 8),
};

static const vlc_coeff_token_t coeff2_5[] = 
{
    VLC(9, 0, 12),   /* 0000 0100 0 */
    VLC(9, 2, 13),
    VLC(9, 1, 12),
    VLC(9, 0, 11),
    VLC(9, 3, 13),
    VLC(9, 2, 12),
    VLC(9, 1, 11),
    VLC(9, 0, 10),
};

static const vlc_coeff_token_t coeff2_6[] = 
{
    VLC(-1, -1, -1),   /* 0000 0000 00 */
    VLC(10, 0, 16),    /* 0000 0000 01 */
    VLC(10, 3, 16),    /* 0000 0000 10 */
    VLC(10, 2, 16),    /* 0000 0000 11 */
    VLC(10, 1, 16),    /* 0000 0001 00 */
    VLC(10, 0, 15),    /* 0000 0001 01 */
    VLC(10, 3, 15),    /* 0000 0001 10 */
    VLC(10, 2, 15),    /* 0000 0001 11 */
    VLC(10, 1, 15),    /* 0000 0010 00 */
    VLC(10, 0, 14),
    VLC(10, 3, 14),
    VLC(10, 2, 14),
    VLC(10, 1, 14),
    VLC(10, 0, 13),
    VLC2(9, 1, 13),    /* 0000 0011 1(0) */
};

static const vlc_coeff_token_t coeff1_0[] = 
{
    VLC(4, 3, 4),  /* 0100 */
    VLC(4, 3, 3),  /* 0101 */
    VLC2(3, 2, 2), /* 011(0) */
    VLC4(2, 1, 1), /* 10 */
    VLC4(2, 0, 0), /* 11 */
};

static const vlc_coeff_token_t coeff1_1[] = 
{
    VLC(6, 3, 7),   /* 0001 00 */
    VLC(6, 2, 4),   /* 0001 01 */
    VLC(6, 1, 4),   /* 0001 10 */
    VLC(6, 0, 2),   /* 0001 11 */
    VLC(6, 3, 6),   /* 0010 00 */
    VLC(6, 2, 3),   /* 0010 01 */
    VLC(6, 1, 3),   /* 0010 10 */
    VLC(6, 0, 1),   /* 0010 11*/
    VLC2(5, 3, 5),   /* 0011 0(0)*/
    VLC2(5, 1, 2),   /* 0011 1(0)*/
};

static const vlc_coeff_token_t coeff1_2[] = 
{
    VLC(9, 3, 9),   /* 0000 0010 0 */
    VLC(9, 2, 7),   /* 0000 0010 1 */
    VLC(9, 1, 7),   /* 0000 0011 0 */
    VLC(9, 0, 6),   /* 0000 0011 1 */

    VLC2(8, 0, 5),   /* 0000 0100 */
    VLC2(8, 2, 6),   /* 0000 0101 */
    VLC2(8, 1, 6),   /* 0000 0110 */
    VLC2(8, 0, 4),   /* 0000 0111 */

    VLC4(7, 3, 8),    /* 0000 100 */
    VLC4(7, 2, 5),    /* 0000 101 */
    VLC4(7, 1, 5),    /* 0000 110 */
    VLC4(7, 0, 3),    /* 0000 111 */
};

static const vlc_coeff_token_t coeff1_3[] = 
{
    VLC(11, 3, 11),   /* 0000 0001 000 */
    VLC(11, 2, 9),    /* 0000 0001 001 */
    VLC(11, 1, 9),    /* 0000 0001 010 */
    VLC(11, 0, 8),    /* 0000 0001 011 */
    VLC(11, 3, 10),   /* 0000 0001 100 */
    VLC(11, 2, 8),    /* 0000 0001 101 */
    VLC(11, 1, 8),    /* 0000 0001 110 */
    VLC(11, 0, 7),    /* 0000 0001 111 */
};

static const vlc_coeff_token_t coeff1_4[] = 
{
    VLC(12, 0, 11),   /* 0000 0000 1000 */
    VLC(12, 2, 11),   /* 0000 0000 1001 */
    VLC(12, 1, 11),   /* 0000 0000 1010 */
    VLC(12, 0, 10),   /* 0000 0000 1011 */
    VLC(12, 3, 12),   /* 0000 0000 1100 */
    VLC(12, 2, 10),   /* 0000 0000 1101 */
    VLC(12, 1, 10),   /* 0000 0000 1110 */
    VLC(12, 0, 9),    /* 0000 0000 1111 */
};

static const vlc_coeff_token_t coeff1_5[] = 
{
    VLC(13, 3, 14),   /* 0000 0000 0100 0 */
    VLC(13, 2, 13),   /* 0000 0000 0100 1 */
    VLC(13, 1, 13),   /* 0000 0000 0101 0 */
    VLC(13, 0, 13),   /* 0000 0000 0101 1 */
    VLC(13, 3, 13),   /* 0000 0000 0110 0 */
    VLC(13, 2, 12),   /* 0000 0000 0110 1 */
    VLC(13, 1, 12),   /* 0000 0000 0111 0 */
    VLC(13, 0, 12),   /* 0000 0000 0111 1 */
};

static const vlc_coeff_token_t coeff1_6[] = 
{
    VLC2(-1, -1, -1),  /* 0000 0000 0000 00 */
    VLC2(13, 3, 15),   /* 0000 0000 0000 1(0) */
    VLC(14, 3, 16),   /* 0000 0000 0001 00 */
    VLC(14, 2, 16),   /* 0000 0000 0001 01 */
    VLC(14, 1, 16),   /* 0000 0000 0001 10 */
    VLC(14, 0, 16),   /* 0000 0000 0001 11 */

    VLC(14, 1, 15),   /* 0000 0000 0010 00 */
    VLC(14, 0, 15),   /* 0000 0000 0010 01 */
    VLC(14, 2, 15),   /* 0000 0000 0010 10 */
    VLC(14, 1, 14),   /* 0000 0000 0010 11 */
    VLC2(13, 2, 14),   /* 0000 0000 0011 0(0) */
    VLC2(13, 0, 14),   /* 0000 0000 0011 1(0) */
};

static const vlc_coeff_token_t coeff0_0[] = 
{
    VLC2(-1, -1, -1), /* 0000 0000 0000 000(0) */
    VLC2(15, 1, 13),  /* 0000 0000 0000 001(0) */
    VLC(16, 0, 16),   /* 0000 0000 0000 0100 */
    VLC(16, 2, 16),   
    VLC(16, 1, 16),
    VLC(16, 0, 15),
    VLC(16, 3, 16),
    VLC(16, 2, 15),
    VLC(16, 1, 15),
    VLC(16, 0, 14),
    VLC(16, 3, 15),
    VLC(16, 2, 14),
    VLC(16, 1, 14),
    VLC(16, 0, 13),   /* 0000 0000 0000 1111 */
    VLC2(15, 3, 14),  /* 0000 0000 0001 000(0) */
    VLC2(15, 2, 13),
    VLC2(15, 1, 12),
    VLC2(15, 0, 12),
    VLC2(15, 3, 13),
    VLC2(15, 2, 12),
    VLC2(15, 1, 11),
    VLC2(15, 0, 11),  /* 0000 0000 0001 111(0) */
    VLC4(14, 3, 12),  /* 0000 0000 0010 00(00) */
    VLC4(14, 2, 11),
    VLC4(14, 1, 10),
    VLC4(14, 0, 10),
    VLC4(14, 3, 11),
    VLC4(14, 2, 10),
    VLC4(14, 1, 9),
    VLC4(14, 0, 9),  /* 0000 0000 0011 11(00) */
};

static const vlc_coeff_token_t coeff0_1[] = 
{
    VLC(13, 0, 8),   /* 0000 0000 0100 0 */
    VLC(13, 2, 9),
    VLC(13, 1, 8),
    VLC(13, 0, 7),
    VLC(13, 3, 10),
    VLC(13, 2, 8),
    VLC(13, 1, 7),
    VLC(13, 0, 6),  /* 0000 0000 0111 1 */
};

static const vlc_coeff_token_t coeff0_2[] = 
{
    VLC(11, 3, 9),   /* 0000 0000 100 */
    VLC(11, 2, 7),
    VLC(11, 1, 6),
    VLC(11, 0, 5),   /* 0000 0000 111 */
    VLC2(10, 3, 8),  /* 0000 0001 00(0) */
    VLC2(10, 2, 6),
    VLC2(10, 1, 5),
    VLC2(10, 0, 4),  /* 0000 0001 11(0) */
    VLC4(9, 3, 7),  /* 0000 0010 0(0) */
    VLC4(9, 2, 5),
    VLC4(9, 1, 4),
    VLC4(9, 0, 3),  /* 0000 0011 1(0) */
};

static const vlc_coeff_token_t coeff0_3[] = 
{
    VLC(8, 3, 6),   /* 0000 0100 */
    VLC(8, 2, 4),
    VLC(8, 1, 3),
    VLC(8, 0, 2),
    VLC2(7, 3, 5),  /* 0000 100 */
    VLC2(7, 2, 3),
    VLC4(6, 3, 4),  /* 0000 11 */
};

static const vlc_coeff_token_t coeff0_4[] = 
{
    VLC(6, 1, 2),    /* 0001 00 */
    VLC(6, 0, 1),    /* 0001 01 */
    VLC2(5, 3, 3)    /* 0001 1 */
};

static const vlc_coeff_token_t coeff0_5[] = 
{
    VLC(-1, -1, -1),   /* 000 */
    VLC(3, 2, 2),      /* 001 */
    VLC2(2, 1, 1),     /* 01 */
    VLC4(1, 0, 0)      /* 1 */
};

static const uint8_t prefix_table0[] = 
{
    -1,
    3,
    2, 2,
    1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0
};

static const uint8_t prefix_table1[] = 
{
    -1,
    7,
    6, 6,
    5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4
};

static const uint8_t prefix_table2[] =
{
    -1,
    11,
    10, 10,
    9, 9, 9, 9,
    8, 8, 8, 8, 8, 8, 8, 8
};

static const uint8_t prefix_table3[] = 
{
    -1,
    15,
    14, 14,
    13, 13, 13, 13,
    12, 12, 12, 12, 12, 12, 12, 12
};

#undef VLC
#undef VLC2
#undef VLC4
#define VLC(a, b) {a, b}
#define VLC2(a, b) VLC(a, b), VLC(a, b)
#define VLC4(a, b) VLC2(a, b), VLC2(a, b)
#define VLC8(a, b) VLC4(a, b), VLC4(a, b)

typedef struct  
{
    uint8_t num;
    uint8_t len;
} zero_count_t;

static const zero_count_t total_zero_table1_0[] = 
{
    VLC(-1, -1),
    VLC(15, 9), /* 0000 0000 1 */
    VLC(14, 9),
    VLC(13, 9), /* 0000 0001 1 */
    VLC2(12, 8),/* 0000 0010 */
    VLC2(11, 8),/* 0000 0011 */
    VLC4(10, 7),/* 0000 010 */
    VLC4(9, 7), /* 0000 011 */
    VLC8(8, 6), /* 0000 10 */
    VLC8(7, 6), /* 0000 11 */
};

static const zero_count_t total_zero_table1_1[] = 
{
    VLC2(-1, -1),
    VLC(6, 5), /* 0001 0 */
    VLC(5, 5), /* 0001 1 */
    VLC2(4, 4),/* 0010 */
    VLC2(3, 4),/* 0011 */
    VLC4(2, 3),/* 010 */
    VLC4(1, 3),/* 011 */
    VLC8(0, 1), /*1 */
    VLC8(0, 1), /*1 */
};

static const zero_count_t total_zero_table2_0[] = 
{
    VLC(14, 6), /* 0000 00 */
    VLC(13, 6),
    VLC(12, 6),
    VLC(11, 6),
    VLC2(10, 5),/* 0001 0 */
    VLC2(9, 5),
};

static const zero_count_t total_zero_table2_1[] = 
{
    VLC2(-1, -1),
    VLC(8, 4), /* 0010 */
    VLC(7, 4), /* 0011 */
    VLC(6, 4),
    VLC(5, 4),
    VLC2(4, 3),/* 011 */
    VLC2(3, 3),/* 100 */
    VLC2(2, 3), /*101 */
    VLC2(1, 3), /*110 */
    VLC2(0, 3), /*111 */
};

static const zero_count_t total_zero_table3_0[] = 
{
    VLC(13, 6), /* 0000 00 */
    VLC(11, 6),
    VLC2(12, 5),/* 0000 1 */
    VLC2(10, 5),/* 0001 0 */
    VLC2(9, 5), /* 0001 1 */
};

static const zero_count_t total_zero_table3_1[] = 
{
    VLC2(-1, -1),
    VLC(8, 4), /* 0010 */
    VLC(5, 4), /* 0011 */
    VLC(4, 4),
    VLC(0, 4),
    VLC2(7, 3),/* 011 */
    VLC2(6, 3),/* 100 */
    VLC2(3, 3), /*101 */
    VLC2(2, 3), /*110 */
    VLC2(1, 3), /*111 */
};

static const zero_count_t total_zero_table6_0[] = 
{
    VLC(10, 6), /* 0000 00 */
    VLC(0, 6),
    VLC2(1, 5),/* 0000 1 */
    VLC4(8, 4),/* 0000 1 */
};

static const zero_count_t total_zero_table6_1[] = 
{
    VLC(-1, -1),
    VLC(9, 3), /* 001 */
    VLC(7, 3), /* 010 */
    VLC(6, 3),
    VLC(5, 3),
    VLC(4, 3),
    VLC(3, 3),
    VLC(2, 3)
};

static const zero_count_t total_zero_table7_0[] = 
{
    VLC(9, 6), /* 0000 00 */
    VLC(0, 6),
    VLC2(1, 5),/* 0000 1 */
    VLC4(7, 4),/* 0001 */
};

static const zero_count_t total_zero_table7_1[] = 
{
    VLC(-1, -1),
    VLC(8, 3), /* 001 */
    VLC(6, 3), /* 010 */
    VLC(4, 3),
    VLC(3, 3),
    VLC(2, 3),
    VLC2(5, 2)
};

static const zero_count_t total_zero_table8_0[] = 
{
    VLC(8, 6), /* 0000 00 */
    VLC(0, 6),
    VLC2(2, 5),/* 0000 1 */
    VLC4(1, 4),/* 0001 */
};

static const zero_count_t total_zero_table8_1[] = 
{
    VLC(-1, -1),
    VLC(7, 3), /* 001 */
    VLC(6, 3), /* 010 */
    VLC(3, 3),
    VLC2(5, 2),
    VLC2(4, 2)
};

static const zero_count_t total_zero_table9_0[] = 
{
    VLC(1, 6), /* 0000 00 */
    VLC(0, 6),
    VLC2(7, 5),/* 0000 1 */
    VLC4(2, 4),/* 0001 */
};

static const zero_count_t total_zero_table9_1[] = 
{
    VLC(-1, -1),
    VLC(5, 3), /* 001 */
    VLC2(6, 2), /* 01 */
    VLC2(4, 2),
    VLC2(3, 2),
};

static const zero_count_t total_zero_table4_0[] = 
{
    VLC(12, 5), /* 0000 0 */
    VLC(11, 5),
    VLC(10, 5), /* 0000 1 */
    VLC(0, 5),  /* 0001 1 */
    VLC2(9, 4), /* 0010 */
    VLC2(7, 4),
    VLC2(3, 4),
    VLC2(2, 4), /* 0101 */
    VLC4(8, 3), /* 011 */
};

static const zero_count_t total_zero_table4_1[] = 
{
    VLC(6, 3),   /* 100 */
    VLC(5, 3),   /* 101 */
    VLC(4, 3),   /* 110 */
    VLC(1, 3)    /* 111 */
};

static const zero_count_t total_zero_table5_0[] = 
{
    VLC(11, 5),  /* 0000 0 */
    VLC(9, 5),
    VLC2(10, 4), /* 0000 1 */
    VLC2(8, 4),  /* 0010 */
    VLC2(2, 4),
    VLC2(1, 4),
    VLC2(0, 4),
    VLC4(7, 3)
};

static const zero_count_t total_zero_table5_1[] = 
{
    VLC(6, 3), /* 100 */
    VLC(5, 3),
    VLC(4, 3),
    VLC(3, 3)
};

static const zero_count_t total_zero_table10_0[] = 
{
    VLC(1, 5), /* 0000 0 */
    VLC(0, 5),
    VLC2(6, 4), /* 0000 1 */
};

static const zero_count_t total_zero_table10_1[] = 
{
    VLC(-1, -1),
    VLC(2, 3), /* 001 */
    VLC2(5, 2), /* 01 */
    VLC2(4, 2),
    VLC2(3, 2),
};

static const zero_count_t total_zero_table11_0[] = 
{
    VLC(0, 4), /* 0000 */
    VLC(1, 4),
    VLC2(2, 3), /* 010 */
    VLC2(3, 3),
    VLC2(5, 3),
    VLC8(4, 1)
};

static const zero_count_t total_zero_table12_0[] = 
{
    VLC(0, 4), /* 0000 */
    VLC(1, 4),
    VLC2(4, 3), /* 010 */
    VLC4(2, 2),
    VLC8(3, 1)
};

static const zero_count_t total_zero_table13_0[] = 
{
    VLC(0, 3), /* 000 */
    VLC(1, 3),
    VLC2(3, 2), /* 01 */
    VLC4(2, 1),
};

static const zero_count_t total_zero_table14_0[] = 
{
    VLC(0, 2), 
    VLC(1, 2),
    VLC2(2, 1),
};

static const zero_count_t total_zero_table_chroma[3][8] = 
{
    {
        VLC(3, 3), 
        VLC(2, 3),
        VLC2(1, 2),
        VLC4(0, 1)
    },
    {
        VLC2(2, 2),
        VLC2(1, 2),
        VLC4(0, 1)
    },
    {
        VLC4(1, 1),
        VLC4(0, 1)
    }
};

static const zero_count_t run_before_table_0[7][8] = 
{
    {
        VLC4(1, 1),
        VLC4(0, 1)
    },
    {
        VLC2(2, 2),
        VLC2(1, 2),
        VLC4(0, 1)
    },
    {
        VLC2(3, 2),
        VLC2(2, 2),
        VLC2(1, 2),
        VLC2(0, 2)
    },
    {
        VLC(4, 3),
        VLC(3, 3),
        VLC2(2, 2),
        VLC2(1, 2),
        VLC2(0, 2)
    },
    {
        VLC(5, 3),
        VLC(4, 3),
        VLC(3, 3),
        VLC(2, 3),
        VLC2(1, 2),
        VLC2(0, 2),
    },
    {
        VLC(1, 3),
        VLC(2, 3),
        VLC(4, 3),
        VLC(3, 3),
        VLC(6, 3),
        VLC(5, 3),
        VLC2(0, 2)
    },
    {
        VLC(-1, -1),
        VLC(6, 3),
        VLC(5, 3),
        VLC(4, 3),
        VLC(3, 3),
        VLC(2, 3),
        VLC(1, 3),
        VLC(0, 3)
    }
};

static const uint8_t run_before_table_1[] =
{
    -1,
    10,
    9, 9,
    8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7
};

static const uint8_t run_before_table_2[] =
{
    -1,
    14,
    13, 13,
    12, 12, 12, 12,
    11, 11, 11, 11, 11, 11, 11, 11
};
/* -- cavlc tables -- */


/* nC == -1 */
void
read_coff_token_t4(bs_t *s, uint8_t* trailing_ones, uint8_t* total_coff)
{
    int32_t code;

    code = bs_show(s, 8);
    if (code >= 16)
    {
        if (code >= 128)
        {
            /* 1 */
            *trailing_ones = 1;
            *total_coff = 1;
            bs_skip(s, 1);
        }
        else if (code >= 64)
        {
            /* 01 */
            *trailing_ones = 0;
            *total_coff = 0;
            bs_skip(s, 2);
        }
        else if (code >= 32)
        {
            /* 001 */
            *trailing_ones = 2;
            *total_coff = 2;
            bs_skip(s, 3);
        }
        else
        {
            code = (code >> 2) - 4;

            *trailing_ones = coeff4_0[code].trailing_ones;
            *total_coff = coeff4_0[code].total_coeff;
            bs_skip(s, 6);
        }
    }
    else
    {
        *trailing_ones = coeff4_1[code].trailing_ones;
        *total_coff = coeff4_1[code].total_coeff;
        bs_skip(s, coeff4_1[code].len);
    }
}

/* nC >= 8 */
void
read_coff_token_t3(bs_t *s, uint8_t* trailing_ones, uint8_t* total_coff)
{
    int32_t code;

    code = bs_read(s, 6);

    *trailing_ones = coeff3_0[code].trailing_ones;
    *total_coff = coeff3_0[code].total_coeff;
}

/* 8 > nC >= 4 */
void
read_coff_token_t2(bs_t *s, uint8_t* trailing_ones, uint8_t* total_coff)
{
    int32_t code;
    const vlc_coeff_token_t* table;

    code = bs_show(s, 10);
    if (code >= 512)
    {
        table = coeff2_0;
        code = (code >> 6) - 8;
    }
    else if (code >= 256)
    {
        table = coeff2_1;
        code = (code >> 5) - 8;
    }
    else if (code >= 128)
    {
        table = coeff2_2;
        code = (code >> 4) - 8;
    }
    else if (code >= 64)
    {
        table = coeff2_3;
        code = (code >> 3) - 8;
    }
    else if (code >= 32)
    {
        table = coeff2_4;
        code = (code >> 2) - 8;
    }
    else if (code >= 16)
    {
        table = coeff2_5;
        code = (code >> 1) - 8;
    }
    else
    {
        table = coeff2_6;
    }

    *trailing_ones = table[code].trailing_ones;
    *total_coff = table[code].total_coeff;
    bs_skip(s, table[code].len);
}

/* 4 > nC >= 2 */
void
read_coff_token_t1(bs_t *s, uint8_t* trailing_ones, uint8_t* total_coff)
{
    int32_t code;
    const vlc_coeff_token_t* table;

    code = bs_show(s, 14);
    if (code >= 4096)
    {
        table = coeff1_0;
        code = (code >> 10) - 4;
    }
    else if (code >= 1024)
    {
        table = coeff1_1;
        code = (code >> 8) - 4;
    }
    else if (code >= 128)
    {
        table = coeff1_2;
        code = (code >> 5) - 4;
    }
    else if (code >= 64)
    {
        table = coeff1_3;
        code = (code >> 3) - 8;
    }
    else if (code >= 32)
    {
        table = coeff1_4;
        code = (code >> 2) - 8;
    }
    else if (code >= 16)
    {
        table = coeff1_5;
        code = (code >> 1) - 8;
    }
    else
    {
        table = coeff1_6;
    }

    *trailing_ones = table[code].trailing_ones;
    *total_coff = table[code].total_coeff;
    bs_skip(s, table[code].len);
}

/* 2 > nC >= 0 */
void
read_coff_token_t0(bs_t *s, uint8_t* trailing_ones, uint8_t* total_coff)
{
    int32_t code;
    const vlc_coeff_token_t* table;

    code = bs_show(s, 16);
    if (code >= 8192)
    {
        table = coeff0_5;
        code >>= 13;
    }
    else if (code >= 4096)
    {
        table = coeff0_4;
        code = (code >> 10) - 4;
    }
    else if (code >= 1024)
    {
        table = coeff0_3;
        code = (code >> 8) - 4;
    }
    else if (code >= 128)
    {
        table = coeff0_2;
        code = (code >> 5) - 4;
    }
    else if (code >= 64)
    {
        table = coeff0_1;
        code = (code >> 3) - 8;
    }
    else
    {
        table = coeff0_0;
    }

    *trailing_ones = table[code].trailing_ones;
    *total_coff = table[code].total_coeff;
    bs_skip(s, table[code].len);
}

uint8_t
read_level_prefix(bs_t *s)
{
    uint8_t prefix;
    int32_t code;

    code = bs_show(s, 16);
    if (code >= 4096)
    {
        prefix = prefix_table0[code >> 12];
    }
    else if (code >= 256)
    {
        prefix = prefix_table1[code >> 8];
    }
    else if (code >= 16)
    {
        prefix = prefix_table2[code >> 4];
    }
    else
    {
        prefix = prefix_table3[code];
    }

    bs_skip(s, prefix + 1);

    return prefix;
}

uint8_t
read_total_zero1(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 9);
    if (code >= 32)
    {
        code >>= 4;
        total_zero = total_zero_table1_1[code].num;
        bs_skip(s, total_zero_table1_1[code].len);
    }
    else
    {
        total_zero = total_zero_table1_0[code].num;
        bs_skip(s, total_zero_table1_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero2(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 6);
    if (code >= 8)
    {
        code >>= 2;
        total_zero = total_zero_table2_1[code].num;
        bs_skip(s, total_zero_table2_1[code].len);
    }
    else
    {
        total_zero = total_zero_table2_0[code].num;
        bs_skip(s, total_zero_table2_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero3(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 6);
    if (code >= 8)
    {
        code >>= 2;
        total_zero = total_zero_table3_1[code].num;
        bs_skip(s, total_zero_table3_1[code].len);
    }
    else
    {
        total_zero = total_zero_table3_0[code].num;
        bs_skip(s, total_zero_table3_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero6(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 6);
    if (code >= 8)
    {
        code >>= 3;
        total_zero = total_zero_table6_1[code].num;
        bs_skip(s, total_zero_table6_1[code].len);
    }
    else
    {
        total_zero = total_zero_table6_0[code].num;
        bs_skip(s, total_zero_table6_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero7(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 6);
    if (code >= 8)
    {
        code >>= 3;
        total_zero = total_zero_table7_1[code].num;
        bs_skip(s, total_zero_table7_1[code].len);
    }
    else
    {
        total_zero = total_zero_table7_0[code].num;
        bs_skip(s, total_zero_table7_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero8(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 6);
    if (code >= 8)
    {
        code >>= 3;
        total_zero = total_zero_table8_1[code].num;
        bs_skip(s, total_zero_table8_1[code].len);
    }
    else
    {
        total_zero = total_zero_table8_0[code].num;
        bs_skip(s, total_zero_table8_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero9(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 6);
    if (code >= 8)
    {
        code >>= 3;
        total_zero = total_zero_table9_1[code].num;
        bs_skip(s, total_zero_table9_1[code].len);
    }
    else
    {
        total_zero = total_zero_table9_0[code].num;
        bs_skip(s, total_zero_table9_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero4(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 5);
    if (code >= 16)
    {
        code = (code >> 2) - 4;
        total_zero = total_zero_table4_1[code].num;
        bs_skip(s, total_zero_table4_1[code].len);
    }
    else
    {
        total_zero = total_zero_table4_0[code].num;
        bs_skip(s, total_zero_table4_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero5(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 5);
    if (code >= 16)
    {
        code = (code >> 2) - 4;
        total_zero = total_zero_table5_1[code].num;
        bs_skip(s, total_zero_table5_1[code].len);
    }
    else
    {
        total_zero = total_zero_table5_0[code].num;
        bs_skip(s, total_zero_table5_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero10(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 5);
    if (code >= 4)
    {
        code >>= 2;
        total_zero = total_zero_table10_1[code].num;
        bs_skip(s, total_zero_table10_1[code].len);
    }
    else
    {
        total_zero = total_zero_table10_0[code].num;
        bs_skip(s, total_zero_table10_0[code].len);
    }

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero11(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 4);
    total_zero = total_zero_table11_0[code].num;
    bs_skip(s, total_zero_table11_0[code].len);

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_total_zero12(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 4);
    total_zero = total_zero_table12_0[code].num;
    bs_skip(s, total_zero_table12_0[code].len);

    return total_zero;
}

uint8_t
read_total_zero13(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 3);
    total_zero = total_zero_table13_0[code].num;
    bs_skip(s, total_zero_table13_0[code].len);

    return total_zero;
}

uint8_t
read_total_zero14(bs_t *s)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 2);
    total_zero = total_zero_table14_0[code].num;
    bs_skip(s, total_zero_table14_0[code].len);

    return total_zero;
}

uint8_t
read_total_zero15(bs_t *s)
{
    return bs_read1(s);
}

uint8_t
read_total_zero_chroma(bs_t *s, uint8_t total_coeff)
{
    uint8_t total_zero;
    int32_t code;

    code = bs_show(s, 3);
    total_zero = total_zero_table_chroma[total_coeff - 1][code].num;
    bs_skip(s, total_zero_table_chroma[total_coeff - 1][code].len);

    assert(total_zero != 255);

    return total_zero;
}

uint8_t
read_run_before(bs_t *s, uint8_t zero_left)
{
    int32_t code;
    uint8_t run_before;

    assert(zero_left != 255);

    code = bs_show(s, 3);
    if (zero_left <= 6)
    {
        run_before = run_before_table_0[zero_left - 1][code].num;
        bs_skip(s, run_before_table_0[zero_left - 1][code].len);
    }
    else
    {
        bs_skip(s, 3);
        if (code > 0)
        {
            run_before = run_before_table_0[6][code].num;
        }
        else
        {
            code = bs_show(s, 4);
            if (code > 0)
            {
                run_before = run_before_table_1[code];
                bs_skip(s, run_before - 6);
            }
            else
            {
                bs_skip(s, 4);
                code = bs_show(s, 4);
                run_before = run_before_table_2[code];
                bs_skip(s, run_before - 10);
            }
        }
    }

    assert(run_before >= 0 && run_before <= 14);

    return run_before;
}

int p264dec_read_residual_block_cavlc( p264_t *h, bs_t *s, int idx, int *l, int i_count )
{
    uint8_t trailing_ones, total_coeff;
    int32_t i, j;
    int32_t zero_left = 0;
    int16_t level[16]; //order: is from high frequency to low frequency
    uint8_t run[16];

    if(idx == BLOCK_INDEX_CHROMA_DC)
    {
      read_coff_token_t4(s, &trailing_ones, &total_coeff);
    }
    else
    {
      //[1] NC-> total coeff and trailing ones
      /* P264_mb_predict_non_zero_code return 0 <-> (16+16+1)>>1 = 16 */
      int32_t nC = 0;
      typedef void (*read_coff_token_t)(bs_t *s, uint8_t* trailing_ones, uint8_t* total_coff);
      static const read_coff_token_t read_coeff[17] = 
      {
          read_coff_token_t0, read_coff_token_t0,
          read_coff_token_t1, read_coff_token_t1,
          read_coff_token_t2, read_coff_token_t2,
          read_coff_token_t2, read_coff_token_t2,
          read_coff_token_t3, read_coff_token_t3,
          read_coff_token_t3, read_coff_token_t3,
          read_coff_token_t3, read_coff_token_t3,
          read_coff_token_t3, read_coff_token_t3,
          read_coff_token_t3
      };

      if(idx == BLOCK_INDEX_LUMA_DC)
      {
          // predict nC = (nA + nB) / 2;
          nC = p264_mb_predict_non_zero_code( h, 0 );

          read_coeff[nC](s, &trailing_ones, &total_coeff);
      }
      else
      {
          // predict nC = (nA + nB) / 2;
          nC = p264_mb_predict_non_zero_code( h, idx );

          read_coeff[nC](s, &trailing_ones, &total_coeff);

          assert(total_coeff != 255);
          assert(trailing_ones != 255);

          h->mb.cache.non_zero_count[p264_scan8[idx]]=total_coeff; //lsp
      }
    }//end of if(idx == BLOCK_INDEX_CHROMA_DC) else

    if (total_coeff > 0)
    {
        int suffix_length = 0;
        int level_code;

        if (total_coeff > 10 && trailing_ones < 3)
            suffix_length = 1;

        //[3] read the trailing ones sign, then set the according level
        for(i = 0 ; i < trailing_ones ; i ++)
        {
            level[i] = 1 - 2 * bs_read( s, 1 );
        }

        //[4] read the other non zero coeff
        for( ; i < total_coeff ; i ++)
        {
            int level_suffixsize;
            int level_suffix;
            int level_prefix = read_level_prefix(s);

            level_suffixsize = suffix_length;
            if (suffix_length == 0 && level_prefix == 14)
                level_suffixsize = 4;
            else if (level_prefix == 15)
                level_suffixsize = 12;
            if (level_suffixsize > 0)
                level_suffix = bs_read(s, level_suffixsize);
            else
                level_suffix = 0;
            level_code = (level_prefix << suffix_length) + level_suffix;
            if (level_prefix == 15 && suffix_length == 0)
            {
                level_code += 15;
            }
            if (i == trailing_ones && trailing_ones < 3)
            {
                level_code += 2;
            }
            if (level_code % 2 == 0)
            {
                level[i] = (level_code + 2) >> 1;
            }
            else
            {
                level[i] = (-level_code - 1) >> 1;
            }

            if (suffix_length == 0)
                suffix_length = 1;

            if (abs(level[i]) > (3 << (suffix_length - 1)) &&
                suffix_length < 6)
            {
                suffix_length ++;
            }
        }//end of for( ; i < total_coeff ; i ++)

        //[5] total_zero
        if (total_coeff < i_count)
        {
            typedef uint8_t (*read_total_zero_t)(bs_t *s);
            static read_total_zero_t total_zero_f[] =
            {
                read_total_zero1, read_total_zero2, read_total_zero3, read_total_zero4,
                read_total_zero5, read_total_zero6, read_total_zero7, read_total_zero8,
                read_total_zero9, read_total_zero10, read_total_zero11, read_total_zero12,
                read_total_zero13, read_total_zero14, read_total_zero15
            };

            if(idx != BLOCK_INDEX_CHROMA_DC)
                zero_left = total_zero_f[total_coeff - 1](s);
            else
                zero_left = read_total_zero_chroma(s, total_coeff);
        }

        //[6] run before
        for(i = 0 ; i < total_coeff - 1 ; i ++)
        {
            if (zero_left > 0)
            {
                run[i] = read_run_before(s, zero_left);
            }
            else
            {
                run[i] = 0;
            }
            zero_left -= run[i];
        }
        run[total_coeff - 1] = zero_left;

        //[annex] save into l[]
        j = -1;
        for(i = total_coeff - 1 ; i >= 0 ; i --)
        {
            j +=run[i] + 1;
            l[j] = level[i];
        }
    }//end of if (total_coeff > 0)

    return 0;
}