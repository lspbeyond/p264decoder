/*****************************************************************************
 * p264: h264 decoder
 *****************************************************************************
 * Copyright (C) 2003 Laurent Aimar
 * $Id: decoder.c,v 1.1 2004/06/03 19:27:07 fenrir Exp $
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
#include "core/cpu.h"
#include "core/vlc.h"

#include "macroblock.h"
#include "set.h"
#include "vlc.h"
#include "lists.h"


static void p264_slice_idr( p264_t *h )
{
  int i;

  h->i_poc_msb = 0;
  h->i_poc_lsb = 0;
  h->i_frame_offset = 0;
  h->i_frame_num = 0;

  if( h->sps ) //needed???
  {
    for( i = 0; i < h->sps->i_num_ref_frames + 1; i++ )
    {
      h->frames.reference[i]->i_ref_type=NOT_USED_FOR_REF;
      h->frames.reference[i]->i_poc = -1;
    }

    h->fdec = h->frames.reference[0];
    h->i_ref0 = 0;
    h->i_ref1 = 0;
  }
}

/* The slice reading is split in two part: 
*      - part 1 is before ref_pic_list_reordering( )
*      - part 2 is after  dec_ref_pic_marking( )
*/
static int p264_slice_header_part1_read( bs_t *s,
                                        p264_slice_header_t *sh, p264_sps_t sps_array[32], p264_pps_t pps_array[256], int b_idr )
{
  sh->i_first_mb = bs_read_ue( s );
  sh->i_type = bs_read_ue( s );
  if( sh->i_type >= 5 )
  {
    sh->i_type -= 5; //0-4 equal with 5-9
  }
  sh->i_pps_id = bs_read_ue( s );
  if( bs_eof( s ) || sh->i_pps_id >= 256 || pps_array[sh->i_pps_id].i_id == -1 )
  {
    fprintf( stderr, "invalid pps_id %d in slice header\n", sh->i_pps_id);
    return -1;
  }

  sh->pps = &pps_array[sh->i_pps_id];
  sh->sps = &sps_array[sh->pps->i_sps_id];    /* valid if pps valid */

  sh->i_frame_num = bs_read( s, sh->sps->i_log2_max_frame_num );
  if( !sh->sps->b_frame_mbs_only )
  {
    sh->b_field_pic = bs_read1( s );
    if( sh->b_field_pic )
    {
      sh->b_bottom_field = bs_read1( s );
    }
  }

  if( b_idr )
  {
    sh->i_idr_pic_id = bs_read_ue( s );
  }
  else
  {
    sh->i_idr_pic_id = 0;
  }

  if( sh->sps->i_poc_type == 0 )
  {
    sh->i_poc_lsb = bs_read( s, sh->sps->i_log2_max_poc_lsb );
    if( sh->pps->b_pic_order && !sh->b_field_pic )
    {
      sh->i_delta_poc_bottom = bs_read_se( s );
    }
  }
  else if( sh->sps->i_poc_type == 1 && !sh->sps->b_delta_pic_order_always_zero )
  {
    sh->i_delta_poc[0] = bs_read_se( s );
    if( sh->pps->b_pic_order && !sh->b_field_pic )
    {
      sh->i_delta_poc[1] = bs_read_se( s );
    }
  }

  if( sh->pps->b_redundant_pic_cnt )
  {
    sh->i_redundant_pic_cnt = bs_read_ue( s );
  }

  if( sh->i_type == SLICE_TYPE_B )
  {
    sh->b_direct_spatial_mv_pred = bs_read1( s );
  }

  if( sh->i_type == SLICE_TYPE_P || sh->i_type == SLICE_TYPE_SP || sh->i_type == SLICE_TYPE_B )
  {
    sh->b_num_ref_idx_override = bs_read1( s );

    sh->i_num_ref_idx_l0_active = sh->pps->i_num_ref_idx_l0_active; /* default */
    sh->i_num_ref_idx_l1_active = sh->pps->i_num_ref_idx_l1_active; /* default */

    if( sh->b_num_ref_idx_override )
    {
      sh->i_num_ref_idx_l0_active = bs_read_ue( s ) + 1;
      if( sh->i_type == SLICE_TYPE_B )
      {
        sh->i_num_ref_idx_l1_active = bs_read_ue( s ) + 1;
      }
    }
  }

  return bs_eof( s ) ? -1 : 0;
}

static int p264_slice_header_part2_read( bs_t *s, p264_slice_header_t *sh )
{
  if( sh->pps->b_cabac && sh->i_type != SLICE_TYPE_I && sh->i_type != SLICE_TYPE_SI )
  {
    sh->i_cabac_init_idc = bs_read_ue( s );
  }
  sh->i_qp_delta = bs_read_se( s );

  if( sh->i_type == SLICE_TYPE_SI || sh->i_type == SLICE_TYPE_SP )
  {
    if( sh->i_type == SLICE_TYPE_SP )
    {
      sh->b_sp_for_swidth = bs_read1( s );
    }
    sh->i_qs_delta = bs_read_se( s );
  }

  if( sh->pps->b_deblocking_filter_control )
  {
    sh->i_disable_deblocking_filter_idc = bs_read_ue( s );
    if( sh->i_disable_deblocking_filter_idc != 1 )
    {
      sh->i_alpha_c0_offset = bs_read_se( s );
      sh->i_beta_offset = bs_read_se( s );
    }
  }
  else
  {
    sh->i_alpha_c0_offset = 0;
    sh->i_beta_offset = 0;
  }

  if( sh->pps->i_num_slice_groups > 1 && sh->pps->i_slice_group_map_type >= 3 && sh->pps->i_slice_group_map_type <= 5 )
  {
    /* FIXME */
    return -1;
  }
  return 0;
}

//lspfinish
static int p264_slice_header_ref_pic_reordering( bs_t *s, p264_slice_header_t *sh )
{
  int b_flag;

  if (sh->i_type!=SLICE_TYPE_I && sh->i_type!=SLICE_TYPE_SI)
  {
    b_flag=sh->b_ref_pic_list_reordering_l0=bs_read1(s);
    if (b_flag)
    {
      int i=0;
      int idc;
      while (1)
      {
        idc=sh->ref_pic_list_order[0][i].idc=bs_read_ue(s);
        if (3==idc)
        {
          break;
        }
        else if (idc>3)
        {
          fprintf(stderr,"wrong reordering of pic nums idc\n");
          return -1;
        }
        else
        {
          sh->ref_pic_list_order[0][i].arg=bs_read_ue(s);
        }
        i++; //!!
      }//end of while (1)
    }//end of if (b_flag)
  }

  if (sh->i_type==SLICE_TYPE_B)
  {
    b_flag=sh->b_ref_pic_list_reordering_l1=bs_read1(s);
    if (b_flag)
    {
      int i=0;
      int idc;
      while (1)
      {
        idc=sh->ref_pic_list_order[1][i].idc=bs_read_ue(s);
        if (3==idc)
        {
          break;
        }
        else if (idc>3)
        {
          fprintf(stderr,"wrong reordering of pic nums idc\n");
          return -1;
        }
        else
        {
          sh->ref_pic_list_order[1][i].arg=bs_read_ue(s);
        }
        i++; //!!
      }//end of while (1)
    }
  }

  return 0;
}

static int p264_slice_header_pred_weight_table( p264_t *h, bs_t *s )
{
  return -1;
}

//lspfinish
static int p264_slice_header_dec_ref_pic_marking( bs_t *s, p264_slice_header_t *sh, int i_nal_type  )
{
  if( i_nal_type == NAL_SLICE_IDR )
  {
    sh->b_no_output_of_prior_pics = bs_read1( s );
    sh->b_long_term_reference_flag = bs_read1( s );
  }
  else
  {
    sh->b_adaptive_ref_pic_marking_mode = bs_read1( s );
    if( sh->b_adaptive_ref_pic_marking_mode )
    {
      int i=0;
      int cmd;
      while(1) 
      {
        cmd=sh->ref_pic_marking[i].i_control_operation=bs_read_ue(s);
        if (cmd==0)
        {
          break;
        }
        else if (cmd>6)
        {
          fprintf(stderr,"wrong memory mangement control operation\n");
          return -1;
        }
        else if (cmd!=5)
        {
          sh->ref_pic_marking[i].arg=bs_read_ue(s);
        }
        i++; //!!
      }
    }//end of if( sh->b_adaptive_ref_pic_marking_mode )
  }

  return 0;
}

//when param set is NULL or changed
void p264_decoder_context_init(p264_t *h)
{
    int i;

    h->sps = h->sh.sps;
    h->pps = h->sh.pps;

    p264_cqm_init( h ); //lsp?? not good

    h->pic_dec->i_width = h->param.i_width = 16 * h->sps->i_mb_width;
    h->pic_dec->i_height = h->param.i_height= 16 * h->sps->i_mb_height;
	  //p264_picture_alloc( h->pic_dec, P264_CSP_I420, h->param.i_width, h->param.i_height );

    h->mb.i_mb_count = h->sps->i_mb_width * h->sps->i_mb_height;

    fprintf( stderr, "p264: %dx%d\n", h->pic_dec->i_width, h->pic_dec->i_height );

    for( i = 0; i < h->sps->i_num_ref_frames + 1; i++ ) //the additional one for current decode frame
    {
      h->frames.reference[i] = p264_frame_new( h );
      h->frames.reference[i]->i_ref_type=NOT_USED_FOR_REF;
      h->frames.reference[i]->i_poc = -1;
    }
    h->fdec = h->frames.reference[0];
    h->fenc=h->fdec; //be compatible with mb_cache_load function, no actual meaning
    h->i_ref0 = 0;
    h->i_ref1 = 0;

    h->i_poc_msb = 0;
    h->i_poc_lsb = 0;
    h->i_frame_offset = 0;
    h->i_frame_num = 0;

    p264_macroblock_cache_init(h);
    h->mb.mv[0] = h->fdec->mv[0]; //lsp??? is it necessary?
    h->mb.mv[1] = h->fdec->mv[1];
    h->mb.ref[0] = h->fdec->ref[0];
    h->mb.ref[1] = h->fdec->ref[1];
    h->mb.type = h->fdec->mb_type;
}

//before param set changed or at the end of the decoder(p264_decoder_close)
void p264_decoder_context_clean(p264_t *h)
{
	int i;

	p264_macroblock_cache_end(h);

  if( h->pic_dec->i_width != 0 && h->pic_dec->i_height != 0 )
  {
    for( i = 0; i < h->sps->i_num_ref_frames + 1; i++ )
    {
      p264_frame_delete( h->frames.reference[i]);
    }
    //p264_picture_clean(h->pic_dec);
  }
	
  h->sps = NULL;
  h->pps = NULL;
}

/****************************************************************************
* Decode a slice header and setup h for mb decoding.
****************************************************************************/
static int p264_slice_header_decode( p264_t *h, bs_t *s, p264_nal_t *nal )
{
  //[1] /* read the first part of the slice */
  if( p264_slice_header_part1_read( s, &h->sh,
    h->sps_array, h->pps_array,
    nal->i_type == NAL_SLICE_IDR ? 1 : 0 ) < 0 )
  {
    fprintf( stderr, "p264_slice_header_part1_read failed\n" );
    return -1;
  }

  //[2] initialize h according to the change of sps and pps
  do //it's a smart way
  {
    //init the decode environment
    if( h->sps == NULL || h->pps == NULL )
    {
      p264_decoder_context_init(h);
      break;
    }

    //free the old context, because picture size may be changed between two sps
    if( h->sps != h->sh.sps || h->pps != h->sh.pps ) //sh means slice header
    {
      p264_decoder_context_clean(h);
    }
    else
    {
      break;//!!
    }
  } while(1);

  //[3] ref list reordering, pred weight table, marking
  ///* calculate poc for current frame */
  //if( h->sps->i_poc_type == 0 )
  //{
  //  int i_max_poc_lsb = 1 << h->sps->i_log2_max_poc_lsb;

  //  if( h->sh.i_poc_lsb < h->i_poc_lsb && h->i_poc_lsb - h->sh.i_poc_lsb >= i_max_poc_lsb/2 )
  //  {
  //    h->i_poc_msb += i_max_poc_lsb;
  //  }
  //  else if( h->sh.i_poc_lsb > h->i_poc_lsb  && h->sh.i_poc_lsb - h->i_poc_lsb > i_max_poc_lsb/2 )
  //  {
  //    h->i_poc_msb -= i_max_poc_lsb;
  //  }
  //  h->i_poc_lsb = h->sh.i_poc_lsb;

  //  h->i_poc = h->i_poc_msb + h->sh.i_poc_lsb;
  //}
  //else if( h->sps->i_poc_type == 1 )
  //{
  //  /* FIXME */
  //  return -1;
  //}
  //else
  //{
  //  if( nal->i_type == NAL_SLICE_IDR )
  //  {
  //    h->i_frame_offset = 0;
  //    h->i_poc = 0;
  //  }
  //  else
  //  {
  //    if( h->sh.i_frame_num < h->i_frame_num )
  //    {
  //      h->i_frame_offset += 1 << h->sps->i_log2_max_frame_num;
  //    }
  //    if( nal->i_ref_idc > 0 )
  //    {
  //      h->i_poc = 2 * ( h->i_frame_offset + h->sh.i_frame_num );
  //    }
  //    else
  //    {
  //      h->i_poc = 2 * ( h->i_frame_offset + h->sh.i_frame_num ) - 1;
  //    }
  //  }
  //  h->i_frame_num = h->sh.i_frame_num;
  //}

  //fprintf( stderr, "p264: pic type=%s poc:%d\n",
  //  h->sh.i_type == SLICE_TYPE_I ? "I" : (h->sh.i_type == SLICE_TYPE_P ? "P" : "B?" ),
  //  h->i_poc );

  //if( h->sh.i_type != SLICE_TYPE_I && h->sh.i_type != SLICE_TYPE_P )
  //{
  //  fprintf( stderr, "only SLICE I/P supported\n" );
  //  return -1;
  //}

  //[4] read the ref pic reordering (only read, do after read all the slice header)
  if( p264_slice_header_ref_pic_reordering( s, &h->sh ) < 0 )
  {
    return -1;
  }

  //[5]
  //if( ( (h->sh.i_type == SLICE_TYPE_P || h->sh.i_type == SLICE_TYPE_SP) && h->sh.pps->b_weighted_pred  ) ||
  //  ( h->sh.i_type == SLICE_TYPE_B && h->sh.pps->b_weighted_bipred ) )
  //{
  //  if( p264_slice_header_pred_weight_table( h, s ) < 0 )
  //  {
  //    return -1;
  //  }
  //}

  //[6] read dec_ref_pic_marking (only read, do after decode a picture)
  if( nal->i_ref_idc != 0 )
  {
    p264_slice_header_dec_ref_pic_marking( s, &h->sh, nal->i_type );
  }

  //[7] 
  if( p264_slice_header_part2_read( s, &h->sh ) < 0 )
  {
    fprintf( stderr, "p264_slice_header_part2_read failed\n" );
    return -1;
  }

  return 0;
}

void p264_macroblock_init(p264_t *h)
{
  memset(&h->dct,0,sizeof(struct dct_t));


  //lsp??? temp way, set a big number, no limit of mv
  h->mb.mv_min[0]=-9999;
  h->mb.mv_max[0]=9999;
  h->mb.mv_min[1]=-9999;
  h->mb.mv_max[1]=9999;
}

static int p264_slice_data_decode( p264_t *h, bs_t *s )
{
  int mb_xy = h->sh.i_first_mb;
  int i_mb_x, i_mb_y;
  int i_ret = 0;

  if( h->pps->b_cabac )//lsptodo
  {
    /* TODO: alignement and cabac init */
  }

  h->b_pic_output=0;
  h->i_skip_run=-1;
  /* FIXME field decoding */
  for( ;; )
  {
    //lsptodo: support the case a frme have multi-slice
    if( mb_xy >= h->sps->i_mb_width * h->sps->i_mb_height )//lsp?? temp way
    {
      h->b_pic_output=1; 
      break;
    }

    //if(h->sh.i_frame_num==1 && mb_xy==95)  //lspdebug
    //{
    //  h->b_pic_output=1;
    //}

    //[0]
    p264_macroblock_init(h);


    //[1] load mb cache
    i_mb_y = mb_xy / h->sps->i_mb_width;
    i_mb_x = mb_xy % h->sps->i_mb_width;
    //!note: h->fenc point to f->dec, just want the function work, 
    //in fact, decoder don't need f->fenc
    //h->param.analyse is used in cache_load and it's no meaning to decoder too.
    p264_macroblock_cache_load(h, i_mb_x, i_mb_y);

    //[2] mb read
    if(h->i_skip_run<1) //-1: no skip, 0: skip_run decrease to 0 currently
    {
      if( h->pps->b_cabac )
      {
        if( h->sh.i_type != SLICE_TYPE_I && h->sh.i_type != SLICE_TYPE_SI )
        {
          /* TODO */
        }
        i_ret = p264_macroblock_read_cabac( h, s );
      }
      else//CAVLC
      {
        i_ret = p264_macroblock_read_cavlc( h, s );
      }//end of if( h->pps->b_cabac ) else

      if( i_ret < 0 )
      {
        fprintf( stderr, "p264_macroblock_read failed [%d,%d]\n", h->mb.i_mb_x, h->mb.i_mb_y );
        break;
      }
    }

    //[3] mb decode
    if(h->i_skip_run>0)
    {
      p264_macroblock_decode_skip(h);
      h->i_skip_run--; //!!
    }
    else
    {
      if( p264_macroblock_decode( h ) < 0 )
      {
        fprintf( stderr, "p264_macroblock_decode failed\n" );
        /* try to do some error correction ;) */
      }
      h->i_skip_run=-1; //not very good way!!
    }

    //[4] save the current mb decode info to mb table
    p264_macroblock_cache_save(h); //lsp

    mb_xy++; //lsptodo: support slice map group
    //if(h->sh.i_frame_num==1 && mb_xy>100)  //lspdebug
    //{
    //  h->b_pic_output=1;
    //  break;
    //}
  }//end of for( ;; )

  return i_ret;
}

/****************************************************************************
* p264_slice_decode: decode one slice
****************************************************************************/
static int p264_slice_decode( p264_t *h, bs_t *s, p264_nal_t *nal )
{
  int i;
  int i_ret;

  //[1] slice header
  if( ( i_ret = p264_slice_header_decode( h, s, nal ) ) < 0 )
  {
    fprintf( stderr, "p264: p264_slice_header_decode failed\n" );
    return -1;
  }

  //lspdebug
  //if (h->sh.i_frame_num==1)
  //{
  //  printf("ok");
  //}

  // slice level init
  h->fdec->i_frame_num = h->sh.i_frame_num; //lsp?? may be it's better written in header_read

  //[2] lists 
  p264_lists_init(h);
  p264_lists_reorder(h);

  //[3] slice data decode. important!!
  if( h->sh.i_redundant_pic_cnt == 0 && i_ret == 0 )
  {
    if( ( i_ret = p264_slice_data_decode( h, s ) ) < 0 )
    {
      fprintf( stderr, "p264: p264_slice_data_decode failed\n" );
      return -1;
    }
  }

  //[4] 
  //note: if decode a whole picture
  if(h->b_pic_output)
  {
    /* apply deblocking filter to the current decoded picture */
    //note: apply to h->fdec->plane[0,1,2]
    if( !h->pps->b_deblocking_filter_control || h->sh.i_disable_deblocking_filter_idc != 1 )
    {
      p264_frame_deblocking_filter( h, h->sh.i_type );
    }
    /* expand border for frame reference TODO avoid it when using b-frame */
    p264_frame_expand_border( h->fdec );

    /* create filtered images */ //half pel interpolation!!
    p264_frame_filter( h->param.cpu, h->fdec );
    /* expand border of filtered images */
    p264_frame_expand_border_filtered( h->fdec );
	
    // output recon picture
    h->pic_dec->img.i_plane = h->fdec->i_plane;
    for( i = 0; i < h->fdec->i_plane; i++ )
    {
      h->pic_dec->img.i_stride[i] = h->fdec->i_stride[i];
      h->pic_dec->img.plane[i]    = h->fdec->plane[i];
    }
   
    //lists marking
    p264_lists_marking(h); //lsp
  }

  return 0;
}

/****************************************************************************
 *
 ******************************* p264 libs **********************************
 *
 ****************************************************************************/

/****************************************************************************
 * p264_decoder_open:
 ****************************************************************************/
p264_t *p264_decoder_open   ( p264_param_t *param )
{
    p264_t *h = p264_malloc( sizeof( p264_t ) );
    int i;

    memset( h, 0, sizeof( p264_t ) );

    memcpy( &h->param, param, sizeof( p264_param_t ) );

    /* no SPS and PPS active yet */
    h->sps = NULL;
    h->pps = NULL;

    for( i = 0; i < 32; i++ )
    {
      h->sps_array[i].i_id = -1;  /* invalidate it */
    }
    for( i = 0; i < 256; i++ )
    {
      h->pps_array[i].i_id = -1;  /* invalidate it */
    }

    //note: since there isn't enough info, we can't use p264_picture_alloc()
    h->pic_dec = p264_malloc( sizeof( p264_picture_t ) );
    memset( h->pic_dec, 0, sizeof( p264_picture_t ) );

    /* init predict_XxX */
    p264_predict_16x16_init( h->param.cpu, h->predict_16x16 );
    p264_predict_8x8c_init( h->param.cpu, h->predict_8x8c );
    p264_predict_8x8_init( h->param.cpu, h->predict_8x8 );
    p264_predict_4x4_init( h->param.cpu, h->predict_4x4 );

    p264_pixel_init( h->param.cpu, &h->pixf );
    p264_dct_init( h->param.cpu, &h->dctf );
    p264_mc_init( h->param.cpu, &h->mc );
    p264_quant_init( h, h->param.cpu, &h->quantf );
    p264_deblock_init( h->param.cpu, &h->loopf );

    /* create the vlc table (we could remove it from p264_t but it will need
     * to introduce a p264_init() for global librarie) */
    for( i = 0; i < 5; i++ )
    {
        /* max 2 step */
        h->p264_coeff_token_lookup[i] = p264_vlc_table_lookup_new( p264_coeff_token[i], 17*4, 4 );
    }
    /* max 2 step */
    h->p264_level_prefix_lookup = p264_vlc_table_lookup_new( p264_level_prefix, 16, 8 );

    for( i = 0; i < 15; i++ )
    {
        /* max 1 step */
        h->p264_total_zeros_lookup[i] = p264_vlc_table_lookup_new( p264_total_zeros[i], 16, 9 );
    }
    for( i = 0;i < 3; i++ )
    {
        /* max 1 step */
        h->p264_total_zeros_dc_lookup[i] = p264_vlc_table_lookup_new( p264_total_zeros_dc[i], 4, 3 );
    }
    for( i = 0;i < 7; i++ )
    {
        /* max 2 step */
        h->p264_run_before_lookup[i] = p264_vlc_table_lookup_new( p264_run_before[i], 15, 6 );
    }

    return h;
}

/****************************************************************************
 * p264_decoder_decode: decode one nal unit
 ****************************************************************************/
int     p264_decoder_decode( p264_t *h,
                            p264_picture_t **pp_pic, p264_nal_t *nal )
{
  int i_ret = 0;
  bs_t bs;

  /* no picture */
  *pp_pic = NULL;
  h->b_pic_output=0;

  /* init bitstream reader */
  bs_init( &bs, nal->p_payload, nal->i_payload );

  h->i_nal_type=nal->i_type; //!!!

  switch( nal->i_type )
  {
  case NAL_SPS:
    if( ( i_ret = p264_sps_read( &bs, h->sps_array ) ) < 0 )
    {
      fprintf( stderr, "p264: p264_sps_read failed\n" );
    }
    break;

  case NAL_PPS:
    if( ( i_ret = p264_pps_read( &bs, h->pps_array ) ) < 0 )
    {
      fprintf( stderr, "p264: p264_pps_read failed\n" );
    }
    break;

  case NAL_SLICE_IDR:
    //fprintf( stderr, "p264: NAL_SLICE_IDR\n" );
    p264_slice_idr( h ); //note: then do below
  case NAL_SLICE:
    if( ( i_ret = p264_slice_decode( h, &bs, nal ) ) < 0 )
    {
      fprintf( stderr, "p264: p264_slice_decode failed\n" );
    }
    else if(h->b_pic_output)
    {
      *pp_pic = h->pic_dec;
    }
    break;

  case NAL_SLICE_DPA:
  case NAL_SLICE_DPB:
  case NAL_SLICE_DPC:
    fprintf( stderr, "partitioned stream unsupported\n" );
    i_ret = -1;
    break;

  case NAL_SEI:
  default:
    break;
  }

  /* restore CPU state (before using float again) */
  p264_cpu_restore( h->param.cpu );

  return i_ret;
}

/****************************************************************************
 * p264_decoder_close:
 ****************************************************************************/
//note: free the space init both by p264_slice_header_decode and p264_decoder_open
void    p264_decoder_close  ( p264_t *h )
{
    int i;

	//[0] output decode result info
	//

	//[1] free the space init by p264_slice_header_decode
	p264_decoder_context_clean(h);

	//[2] free the space init by p264_decoder_open
  /* free vlc table */
  for( i = 0; i < 5; i++ )
  {
      p264_vlc_table_lookup_delete( h->p264_coeff_token_lookup[i] );
  }
  p264_vlc_table_lookup_delete( h->p264_level_prefix_lookup );

  for( i = 0; i < 15; i++ )
  {
      p264_vlc_table_lookup_delete( h->p264_total_zeros_lookup[i] );
  }
  for( i = 0;i < 3; i++ )
  {
      p264_vlc_table_lookup_delete( h->p264_total_zeros_dc_lookup[i] );
  }
  for( i = 0;i < 7; i++ )
  {
      p264_vlc_table_lookup_delete( h->p264_run_before_lookup[i] );
  }

	p264_free(h->pic_dec);
  p264_free( h );
}

