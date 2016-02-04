/*****************************************************************************
 * macroblock.c: h264 decoder library
 *****************************************************************************
 * Copyright (C) 2003 Laurent Aimar
 * $Id: macroblock.c,v 1.1 2004/06/03 19:27:07 fenrir Exp $
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *
 * Maintained by Peter Lee (lspbeyond@126.com)
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
#include <string.h>
#include <stdint.h>

#include "core/core.h"
#include "core/vlc.h"
#include "vlc.h"
#include "dec_cavlc.h"
#include "macroblock.h"


static const uint8_t i16x16_eg_to_cbp[26][3] = 
{
  0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 3, 0, 0,
    0, 1, 0, 1, 1, 0, 2, 1, 0, 3, 1, 0, 0, 2, 0, 
    1, 2, 0, 2, 2, 0, 3, 2, 0, 0, 0, 15, 1, 0, 15, 
    2, 0, 15, 3, 0, 15, 0, 1, 15, 1, 1, 15, 2, 1, 15, 
    3, 1, 15, 0, 2, 15, 1, 2, 15, 2, 2, 15, 3, 2, 15
};

static const uint8_t golomb_to_intra4x4_cbp[48] =
{
  47, 31, 15, 0, 23, 27, 29, 30, 7, 11, 13, 14, 39, 43, 45,
    46, 16, 3, 5, 10, 12, 19, 21, 26, 28, 35, 37, 42, 44, 1,
    2, 4, 8, 17, 18, 20, 24, 6, 9, 22, 25, 32, 33, 34, 36, 40,
    38, 41
};

static const uint8_t golomb_to_inter_cbp[48] = 
{
  0, 16, 1, 2, 4, 8, 32, 3, 5, 10, 12, 15, 47, 7, 11, 13, 14,
    6, 9, 31, 35, 37, 42, 44, 33, 34, 36, 40, 39, 43, 45, 46, 17,
    18, 20, 24, 19, 21, 26, 28, 23, 27, 29, 30, 22, 25, 38, 41
};

enum //mb big type
{
  MB_I=0,
  MB_P,
  MB_B,
  MB_SI,
  MB_SP
};
int read_mb_type(p264_t *h, bs_t *s)
{
  int i_mb_big_type;
  int i_mb_type;

  i_mb_type=bs_read_ue( s );
  //if(h->sh.i_frame_num==1) //lspdebug
  //{
  //  printf("%d,",i_mb_type);
  //}

  //find the mb big type
  switch( h->sh.i_type )
  {
  case SLICE_TYPE_I:
    i_mb_big_type=MB_I;
    break;
  case SLICE_TYPE_P:
    if(i_mb_type<5)
    {
      i_mb_big_type=MB_P;
    }
    else
    {
      i_mb_big_type=MB_I;
      i_mb_type-=5;
    }
    break;
  case SLICE_TYPE_B:
    if(i_mb_type<23)
    {
      i_mb_big_type=MB_B;
    }
    else
    {
      i_mb_big_type=MB_I;
      i_mb_type-=23;
    }
    break;
  default:
    fprintf( stderr, "slice unsupported yet \n" ); //lsptodo
    return -1;
  }//end of switch( h->sh.i_type )

  //set the predict mode and cbp info
  if (MB_I == i_mb_big_type)
  {
    if (0==i_mb_type)
    {
      h->mb.i_type=I_4x4;
    }
    else if (i_mb_type<25)
    {
      h->mb.i_type=I_16x16;
      h->mb.i_intra16x16_pred_mode = (i_mb_type - 1)%4;
      h->mb.i_cbp_chroma = ( (i_mb_type-1) / 4 )%3;
      h->mb.i_cbp_luma   = (i_mb_type>12) ? 0x0f : 0x00;
    }
    else if (25==i_mb_type)
    {
      h->mb.i_type=I_PCM;
    }
    else
    {
      fprintf( stderr, "invalid mb type %d \n", i_mb_type);
      return -1;
    }
  }//end of if (MB_I == i_mb_big_type)
  else if (MB_P == i_mb_big_type)
  {
    if (0==i_mb_type) //是否完整了？？
    {
      h->mb.i_type=P_L0;
      h->mb.i_partition = D_16x16;
    }
    else if (1==i_mb_type)
    {
      h->mb.i_type=P_L0;
      h->mb.i_partition = D_16x8;
    }
    else if (2==i_mb_type)
    {
      h->mb.i_type=P_L0;
      h->mb.i_partition = D_8x16;
    }
    else if (3==i_mb_type || 4==i_mb_type)
    {
      h->mb.i_type=P_8x8; 
      h->mb.i_partition = D_8x8;
    }
    else
    {
      fprintf( stderr, "invalid mb type %d\n", i_mb_type);
      return -1;
    }
  }//end of else if (MB_P == i_mb_big_type)
  else if (MB_B == i_mb_big_type)
  {
    //lsptodo: 
  }//end of else if (MB_B == i_mb_big_type)

  return 0;

}

//lspfinish
void set_mb_partition_ref( p264_t *h, int i_list, int i_part_idx, int i_ref )
{
  int x,  y;
  int width,  height;
  int dx, dy;
  int block8_idx;
  int i_partition=h->mb.i_partition;

  width=p264_mb_partition_size_table[i_partition][0]*2;
  height=p264_mb_partition_size_table[i_partition][1]*2;

  block8_idx=p264_mb_partition_size_table[i_partition][2]*i_part_idx;
  x=p264_block8_to_block4_table[block8_idx][0];
  y=p264_block8_to_block4_table[block8_idx][1];

  for( dx = 0; dx < width; dx++ )
  {
    for( dy = 0; dy < height; dy++ )
    {
      int j=P264_SCAN8_0 + (y+dy)*8 + x+dx;
      h->mb.cache.ref[i_list][p264_scan8[j]]=i_ref;
    }
  }

}

//lspfinish
void set_mb_partition_mv( p264_t *h, int i_list, int i_part_idx, int mvd[2] )
{
  int x,  y;
  int width,  height;
  int dx, dy;
  int block8_idx;
  int i_partition=h->mb.i_partition;
  int mv[2];

  width=p264_mb_partition_size_table[i_partition][0]*2;
  height=p264_mb_partition_size_table[i_partition][1]*2;

  block8_idx=p264_mb_partition_size_table[i_partition][2]*i_part_idx;
  x=p264_block8_to_block4_table[block8_idx][0];
  y=p264_block8_to_block4_table[block8_idx][1];

  p264_mb_predict_mv( h, i_list, block8_idx*4, width, mv );
  mv[0]+=mvd[0];
  mv[1]+=mvd[1];

  for( dx = 0; dx < width; dx++ )
  {
    for( dy = 0; dy < height; dy++ )
    {
      int j=P264_SCAN8_0 + (y+dy)*8 + x+dx;
      h->mb.cache.mv[i_list][j][0]=mv[0];
      h->mb.cache.mv[i_list][j][1]=mv[1];
    }
  }

}

//lspfinish
void set_sub_mb_partition_mv( p264_t *h, int i_list, int i_part_idx, int i_sub_part_idx, int mvd[2] )
{
  int width,  height;
  int dx, dy;
  int i_sub_part=h->mb.i_sub_partition[i_part_idx];
  int mv[2];

  width=p264_mb_partition_size_table[i_sub_part][0];
  height=p264_mb_partition_size_table[i_sub_part][1];

  p264_mb_predict_mv( h, i_list, i_part_idx*4+i_sub_part_idx, width, mv );
  mv[0]+=mvd[0];
  mv[1]+=mvd[1];

  for( dx = 0; dx < width; dx++ )
  {
    for( dy = 0; dy < height; dy++ )
    {
      int j=i_part_idx*4 + dy*2 + dx;
      h->mb.cache.mv[i_list][p264_scan8[j]][0]=mv[0];
      h->mb.cache.mv[i_list][p264_scan8[j]][1]=mv[1];
    }
  }

}


int read_intra_mb_pred(p264_t *h, bs_t *s)
{
  int i;

  if( h->mb.i_type == I_4x4 )
  {
    for( i = 0; i < 16; i++ )
    {
      int b_coded;

      b_coded = bs_read1( s ); //true means it's the most probable mode

      if( b_coded )
      {
        h->mb.cache.intra4x4_pred_mode[p264_scan8[i]] = p264_mb_predict_intra4x4_mode( h, i );
      }
      else
      {
        int i_predicted_mode = p264_mb_predict_intra4x4_mode( h, i );
        int i_mode = bs_read( s, 3 );

        if( i_mode >= i_predicted_mode )
        {
          h->mb.cache.intra4x4_pred_mode[p264_scan8[i]] = i_mode + 1;
        }
        else
        {
          h->mb.cache.intra4x4_pred_mode[p264_scan8[i]] = i_mode;
        }
      }
    }
  }

  h->mb.i_chroma_pred_mode = bs_read_ue( s );

  return 0;
}

//lsptodo: support b
int read_inter_mb_pred(p264_t *h, bs_t *s)
{
  int i_part = p264_mb_partition_count_table[h->mb.i_partition];
  int i;
  int mvd[2];

  //ref
  if (h->sh.i_num_ref_idx_l0_active > 1)
  {
    int i_ref;
    for( i = 0; i < i_part; i++ )
    {
      i_ref = bs_read_te( s, h->sh.i_num_ref_idx_l0_active - 1 );

      set_mb_partition_ref(h,0,i,i_ref);
    }
  }
  else
  {
    for (i=0; i<16; i++)
    {
      h->mb.cache.ref[0][p264_scan8[i]]=0; 
    }
  }

  //mv
  for( i = 0; i < i_part; i++ )
  {
    mvd[0] = bs_read_se( s );
    mvd[1] = bs_read_se( s );

    set_mb_partition_mv(h,0,i,mvd);
  }

  return 0;
}

//lsptodo: support b
int read_inter_sub_mb_pred(p264_t *h, bs_t *s)
{
  int i;

  //read sub mb type
  for( i = 0; i < 4; i++ )
  {
    int i_sub_partition;

    i_sub_partition = bs_read_ue( s );
    switch( i_sub_partition )
    {
    case 0:
      h->mb.i_sub_partition[i] = D_L0_8x8;
      break;
    case 1:
      h->mb.i_sub_partition[i] = D_L0_8x4;
      break;
    case 2:
      h->mb.i_sub_partition[i] = D_L0_4x8;
      break;
    case 3:
      h->mb.i_sub_partition[i] = D_L0_4x4;
      break;
    default:
      fprintf( stderr, "invalid i_sub_partition\n" );
      return -1;
    }
  }

  //ref
  if (h->sh.i_num_ref_idx_l0_active > 1)//lsp?? bs_read_te can judge it already
  {
    int i_ref;
    for( i = 0; i < 4; i++ )
    {
      i_ref = bs_read_te( s, h->sh.i_num_ref_idx_l0_active - 1 );

      set_mb_partition_ref(h,0,i,i_ref);
    }
  }
  else
  {
    for (i=0; i<16; i++)
    {
      h->mb.cache.ref[0][p264_scan8[i]]=0; 
    }
  }

  //mv
  for( i = 0; i < 4; i++ )
  {
    int i_sub_part=p264_mb_partition_count_table[h->mb.i_sub_partition[i]];
    int j;
    int mvd[2];

    for (j=0; j<i_sub_part; j++)
    {
      mvd[0] = bs_read_se( s );
      mvd[1] = bs_read_se( s );

      set_sub_mb_partition_mv(h,0,i,j,mvd);
    }
  }

  return 0;
}

int read_residual(p264_t *h, bs_t *s)
{
  int i;
  
  //[1] luma
  if( h->mb.i_type == I_16x16 )
  {
    /* DC Luma */
    if( p264dec_read_residual_block_cavlc( h, s, BLOCK_INDEX_LUMA_DC , h->dct.luma16x16_dc, 16 ) < 0 )
    {
      return -1;
    }

    /* AC Luma */
    for( i = 0; i < 16; i++ )
    {
      if( h->mb.i_cbp_luma & ( 1 << ( i / 4 ) ) )
      {
        if( p264dec_read_residual_block_cavlc( h, s, i, h->dct.block[i].residual_ac, 15 ) < 0 )
        {
          return -1;
        }
      }
      else
      {
        h->mb.cache.non_zero_count[p264_scan8[i]]=0;
      }
    }
  }//end of if( h->mb.i_type == I_16x16 )
  else
  {
    for( i = 0; i < 16; i++ )
    {
      if( h->mb.i_cbp_luma & ( 1 << ( i / 4 ) ) )
      {
        if( p264dec_read_residual_block_cavlc( h, s, i, h->dct.block[i].luma4x4, 16 ) < 0 )
        {
          return -1;
        }
      }
      else
      {
        h->mb.cache.non_zero_count[p264_scan8[i]]=0;
      }
    }
  }

  //[2] chroma
  if( h->mb.i_cbp_chroma &0x03 )    /* Chroma DC residual present */
  {
    if( p264dec_read_residual_block_cavlc( h, s, BLOCK_INDEX_CHROMA_DC, h->dct.chroma_dc[0], 4 ) < 0 ||
      p264dec_read_residual_block_cavlc( h, s, BLOCK_INDEX_CHROMA_DC, h->dct.chroma_dc[1], 4 ) < 0 )
    {
      return -1;
    }
  }

  if( h->mb.i_cbp_chroma&0x02 ) /* Chroma AC residual present */
  {
    for( i = 0; i < 8; i++ )
    {
      if( p264dec_read_residual_block_cavlc( h, s, 16 + i, h->dct.block[16+i].residual_ac, 15 ) < 0 )
      {
        return -1;
      }
    }
  }
  else
  {
    for( i = 0; i < 8; i++ )
    {
      h->mb.cache.non_zero_count[p264_scan8[16+i]]=0;
    }
  }
  
  return 0;
}

int p264_macroblock_read_cavlc( p264_t *h, bs_t *s )
{
  int i;

  //[1] skip mb
  if( h->sh.i_type != SLICE_TYPE_I && h->sh.i_type != SLICE_TYPE_SI && h->i_skip_run==-1)
  {
    h->i_skip_run = bs_read_ue( s );//skip mb number
    if(h->i_skip_run>0)
    {
      return 0;//!!!
    }
  }

  //[2] mb type
  if( read_mb_type(h,s) < 0 )
  {
    fprintf(stderr,"read mb type failed!\n");
    return -1;
  }

  //[3] read pred mode, mv, ref (important!)
  if(I_PCM==h->mb.i_type)//lsptodo
  {
    fprintf(stderr,"unsupport i_pcm mb\n");
    return -1;    
  }
  else if(IS_INTRA( h->mb.i_type ))
  {
    if(read_intra_mb_pred(h,s)<0)
    {
      fprintf(stderr,"read intra mb pred mode failed\n");
      return -1;    
    }
  }
  else if( h->mb.i_type == P_8x8 || h->mb.i_type == B_8x8)
  {
    if(read_inter_sub_mb_pred(h,s)<0)
    {
      fprintf(stderr,"read inter sub mb pred mode failed\n");
      return -1;    
    }
  }
  else if( h->mb.i_type != B_DIRECT )
  {
    if(read_inter_mb_pred(h,s)<0)
    {
      fprintf(stderr,"read inter mb pred mode failed\n");
      return -1;    
    }
  }

  //[4] cbp
  if( h->mb.i_type != I_16x16 )
  {
    int i_cbp;

    i_cbp = bs_read_ue( s );
    if( i_cbp >= 48 )
    {
      fprintf( stderr, "invalid cbp\n" );
      return -1;
    }

    if( h->mb.i_type == I_4x4 )
    {
      i_cbp = golomb_to_intra4x4_cbp[i_cbp];
    }
    else
    {
      i_cbp = golomb_to_inter_cbp[i_cbp];
    }
    h->mb.i_cbp_luma   = i_cbp&0x0f;
    h->mb.i_cbp_chroma = i_cbp >> 4;
    //printf("%d,",i_cbp); //lspdebug
  }

  //[5] residual
  if( h->mb.i_cbp_luma > 0 || h->mb.i_cbp_chroma > 0 || h->mb.i_type == I_16x16 )
  { //need read residual
    h->mb.i_qp = bs_read_se( s ) + h->pps->i_pic_init_qp + h->sh.i_qp_delta;

    if(read_residual(h,s)<0)
    {
      fprintf(stderr, "read residual data failed\n");
      return -1;
    }
  }
  else //it's an all zero coeff mb
  {
    h->mb.i_qp = h->pps->i_pic_init_qp + h->sh.i_qp_delta;
    for( i = 0; i < 16; i++ )
    {
      h->mb.cache.non_zero_count[p264_scan8[i]]=0;
    }
    for( i = 0; i < 8; i++ )
    {
      h->mb.cache.non_zero_count[p264_scan8[16+i]]=0;
    }
  }

  //fprintf( stderr, "mb read type=%d\n", h->mb.i_type );

  return 0;
}

int p264_macroblock_read_cabac( p264_t *h, bs_t *s )
{
  return -1;
}

/****************************************************************************
* UnScan functions
****************************************************************************/
static const int scan_zigzag_x[16]={0, 1, 0, 0, 1, 2, 3, 2, 1, 0, 1, 2, 3, 3, 2, 3};
static const int scan_zigzag_y[16]={0, 0, 1, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 2, 3, 3};

static inline void unscan_zigzag_4x4full( int16_t dct[4][4], int level[16] )
{
  int i;

  for( i = 0; i < 16; i++ )
  {
    dct[scan_zigzag_y[i]][scan_zigzag_x[i]] = level[i];
  }
}
static inline void unscan_zigzag_4x4( int16_t dct[4][4], int level[15] )
{
  int i;

  for( i = 1; i < 16; i++ )
  {
    dct[scan_zigzag_y[i]][scan_zigzag_x[i]] = level[i - 1];
  }
}

static inline void unscan_zigzag_2x2_dc( int16_t dct[2][2], int level[4] )
{
  dct[0][0] = level[0];
  dct[0][1] = level[1];
  dct[1][0] = level[2];
  dct[1][1] = level[3];
}

//////////////////////////////////////////////////////////////////////////
//valid intra mode function
//////////////////////////////////////////////////////////////////////////
int valid_intra16x16_mode(p264_t *h, uint8_t *src, int i_stride, int i_mode)
{
  int b_left=h->mb.i_neighbour & MB_LEFT;
  int b_top=h->mb.i_neighbour & MB_TOP;
  //int b_top_right=h->mb.i_neighbour & MB_TOPRIGHT;
  int b_top_left=h->mb.i_neighbour & MB_TOPLEFT;

  //[1] further detail about DC_MODE
  if(I_PRED_16x16_DC==i_mode)
  {
    if(b_top_left)
    {
      return I_PRED_16x16_DC;
    }
    else if(b_left)
    {
      return I_PRED_16x16_DC_LEFT;
    }
    else if(b_top)
    {
      return I_PRED_16x16_DC_TOP;
    }
    else
    {
      return I_PRED_16x16_DC_128;
    }
  }

  //[2] check  invalid case

  return i_mode;

}

int valid_intra4x4_mode(p264_t *h, uint8_t *src, int i_stride, int i, int i_mode)
{
  int b_left=h->mb.i_neighbour4[i] & MB_LEFT;
  int b_top=h->mb.i_neighbour4[i] & MB_TOP;
  int b_top_right=h->mb.i_neighbour4[i] & MB_TOPRIGHT;
  int b_top_left=h->mb.i_neighbour4[i] & MB_TOPLEFT;

  //[1] further detail about DC_MODE
  if(I_PRED_4x4_DC==i_mode)
  {
    if(b_left && b_top)
    {
      return I_PRED_4x4_DC;
    }
    else if(b_left)
    {
      return I_PRED_4x4_DC_LEFT;
    }
    else if(b_top)
    {
      return I_PRED_4x4_DC_TOP;
    }
    else
    {
      return I_PRED_4x4_DC_128;
    }
  }

  //[2] constant the invalid neighbour pixel
  if(!b_left)
  {
    src[-1]=src[-1+i_stride]=src[-1+2*i_stride]=src[-1+3*i_stride]=128;
  }
  if(!b_top)
  {
    src[-i_stride]=src[1 - i_stride]=src[2 - i_stride]=src[3 - i_stride]=128;
  }
  if(!b_top_right)
  {
    src[4-i_stride]=src[5 - i_stride]=src[6 - i_stride]=src[7 - i_stride]=src[3 - i_stride];
  }
  if(!b_top_left)
  {
    src[-1-i_stride]=128;
  }

  //[3] check the mode whether is valid //lsp??
  
  return i_mode;

}

int valid_intra8x8c_mode(p264_t *h, int i_mode)
{
  int b_left=h->mb.i_neighbour & MB_LEFT;
  int b_top=h->mb.i_neighbour & MB_TOP;
  //int b_top_right=h->mb.i_neighbour & MB_TOPRIGHT;
  int b_top_left=h->mb.i_neighbour & MB_TOPLEFT;

  //[1] further detail about DC_MODE
  if(I_PRED_CHROMA_DC==i_mode)
  {
    if(b_top_left)
    {
      return I_PRED_CHROMA_DC;
    }
    else if(b_left)
    {
      return I_PRED_CHROMA_DC_LEFT;
    }
    else if(b_top)
    {
      return I_PRED_CHROMA_DC_TOP;
    }
    else
    {
      return I_PRED_CHROMA_DC_128;
    }
  }

  //[2] check  invalid case

  return i_mode;

}

int p264_macroblock_decode( p264_t *h )
{
  const int i_stride = h->mb.pic.i_stride[0];
  uint8_t  *p_dst = h->mb.pic.p_fdec[0];
  int i_qscale;
  int ch;
  int i;

  if( !IS_INTRA(h->mb.i_type ) )
  {
    /* Motion compensation */
    p264_mb_mc( h);
  }

  /* luma */
  i_qscale = h->mb.i_qp;
  if( h->mb.i_type == I_16x16 )
  {
    int16_t dct4x4[16+1][4][4];
    int i_mode=h->mb.i_intra16x16_pred_mode;

    i_mode=valid_intra16x16_mode(h,p_dst,i_stride,i_mode);
    
    /* do the right prediction */
    h->predict_16x16[i_mode]( p_dst, i_stride );

    /* get dc coeffs */
    unscan_zigzag_4x4full( dct4x4[0], h->dct.luma16x16_dc );
    h->dctf.idct4x4dc( dct4x4[0]);
    p264_mb_dequant_4x4_dc( dct4x4[0], h->dequant4_mf[CQM_4IY], i_qscale );

    /* decode the 16x16 macroblock */
    for( i = 0; i < 16; i++ )
    {
      unscan_zigzag_4x4( dct4x4[1+i], h->dct.block[i].residual_ac );
      h->quantf.dequant_4x4( dct4x4[1+i], h->dequant4_mf[CQM_4IY],  i_qscale );

      /* copy dc coeff */
      dct4x4[1+i][0][0] = dct4x4[0][block_idx_y[i]][block_idx_x[i]];
    }

    /* put pixels to fdec */
    h->dctf.add16x16_idct( p_dst, i_stride, &dct4x4[1] );
  }
  else if( h->mb.i_type == I_4x4 )
  {
    for( i = 0; i < 16; i++ )
    {
      uint8_t *p_dst_by;
      int     i_mode;
      //int     b_emu;
      int16_t dct4x4[4][4];
      
      /* Do the right prediction */
      p_dst_by = p_dst + 4 * block_idx_x[i] + 4 * block_idx_y[i] * i_stride;
      //i_mode   = p264_mb_pred_mode4x4_valid( mb, i, h->mb.block[i].i_intra4x4_pred_mode, &b_emu );
      i_mode=h->mb.cache.intra4x4_pred_mode[p264_scan8[i]];
      i_mode=valid_intra4x4_mode(h,p_dst_by,i_stride,i,i_mode);
      //if( b_emu ) //lsp?? what's that?
      //{
      //  fprintf( stderr, "mmmh b_emu\n" );
      //  memset( &p_dst_by[4], p_dst_by[3], 4 );
      //}
      h->predict_4x4[i_mode]( p_dst_by, i_stride );

      if(h->mb.cache.non_zero_count[p264_scan8[i]]>0)
      {
        /* decode one 4x4 block */
        unscan_zigzag_4x4full( dct4x4, h->dct.block[i].luma4x4 );

        h->quantf.dequant_4x4( dct4x4, h->dequant4_mf[CQM_4IY], i_qscale );

        /* output samples to fdec */
        h->dctf.add4x4_idct( p_dst_by, i_stride, dct4x4 );
      }
    }
  }
  else /* Inter mb */
  {
    for( i = 0; i < 16; i++ )
    {
      uint8_t *p_dst_by;
      int16_t dct4x4[4][4];

      if( h->mb.cache.non_zero_count[p264_scan8[i]]>0 )
      {
        unscan_zigzag_4x4full( dct4x4, h->dct.block[i].luma4x4 );
        h->quantf.dequant_4x4( dct4x4, h->dequant4_mf[CQM_4PY], i_qscale );

        p_dst_by = p_dst + 4 * block_idx_x[i] + 4 * block_idx_y[i] * i_stride;
        /* output samples to fdec */
        h->dctf.add4x4_idct( p_dst_by, i_stride, dct4x4 ); //lspmust???
      }
    }
  }

  /* chroma */
  i_qscale = i_chroma_qp_table[p264_clip3( i_qscale + h->pps->i_chroma_qp_index_offset, 0, 51 )];
  if( IS_INTRA( h->mb.i_type ) )
  {
    int i_mode=valid_intra8x8c_mode(h,h->mb.i_chroma_pred_mode);
    /* do the right prediction */
    h->predict_8x8c[i_mode]( h->mb.pic.p_fdec[1], h->mb.pic.i_stride[1] );
    h->predict_8x8c[i_mode]( h->mb.pic.p_fdec[2], h->mb.pic.i_stride[2] );
  }

  if( h->mb.i_cbp_chroma != 0 )
  {
    for( ch = 0; ch < 2; ch++ )
    {
      int16_t dct2x2[2][2];
      int16_t dct4x4[4][4][4];

      /* get dc chroma */
      unscan_zigzag_2x2_dc( dct2x2, h->dct.chroma_dc[ch] );
      h->dctf.idct2x2dc( dct2x2);
      //p264_mb_dequant_2x2_dc( dct2x2,h->dequant4_mf[CQM_4IC + b_inter], i_qscale );
      p264_mb_dequant_2x2_dc( dct2x2,h->dequant4_mf[CQM_4IC], i_qscale );

      for( i = 0; i < 4; i++ )
      {
        unscan_zigzag_4x4( dct4x4[i], h->dct.block[16+i+ch*4].residual_ac );
        //lsp?? 
        //h->quantf.dequant_4x4( dct4x4[i], h->dequant4_mf[CQM_4IC + b_inter], i_qscale );
        h->quantf.dequant_4x4( dct4x4[i], h->dequant4_mf[CQM_4IC], i_qscale );
      }

      /* calculate dct coeffs */
      for( i = 0; i < 4; i++ )
      {
        /* copy dc coeff */
        dct4x4[i][0][0] = dct2x2[block_idx_y[i]][block_idx_x[i]];
      }
      h->dctf.add8x8_idct( h->mb.pic.p_fdec[1+ch],  h->mb.pic.i_stride[1+ch], dct4x4 );
    }
  }

  return 0;
}

void p264_macroblock_decode_skip( p264_t *h )
{
  int i;
  int x, y;
  int mv[2];

  /* decode it as a 16x16 with no luma/chroma */
  h->mb.i_type = P_L0;
  h->mb.i_partition = D_16x16;
  h->mb.i_cbp_luma = 0;
  h->mb.i_cbp_chroma = 0;
  for( i = 0; i < 16 + 8; i++ )
  {
    h->mb.cache.non_zero_count[p264_scan8[i]] = 0;
  }

  /* set ref0 */
  for( i = 0; i < 16; i++ )
  {
    h->mb.cache.ref[0][p264_scan8[i]] = 0; 
  }

  /* get mv */
  p264_mb_predict_mv_pskip( h, mv );

  //set mv for cache save
  for( y = 0; y < 4; y++ )
  {
    for( x = 0; x < 4; x++ )
    {
      h->mb.cache.mv[0][P264_SCAN8_0+x+8*y][0]=mv[0];
      h->mb.cache.mv[0][P264_SCAN8_0+x+8*y][1]=mv[1];
    }
  }

  /* Motion compensation */
  p264_mb_mc( h );

  h->mb.i_type = P_SKIP; //!!
}