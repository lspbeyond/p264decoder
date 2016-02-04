/*****************************************************************************
* lists.c: h264 decoder library
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

//#include <stdlib.h>
//#include <stdio.h>
//#include <assert.h>

#include "core/core.h"
#include "lists.h"

//note: when array sequence is short and almost sorted, insert sort is the best choice
void insert_sort(p264_frame_t **list, int i_list_idx, p264_frame_t *ref)
{
  int j,k;

  //[1] find the position for current ref
  if (SHORT_TERM_REF==ref->i_ref_type)
  {
    for (j=i_list_idx; j>=0; j--)
    {
      if (ref->i_pic_num < list[j]->i_pic_num)
      {
        break;
      }
    }
  }
  else if (LONG_TERM_REF==ref->i_ref_type)
  {
    for (j=i_list_idx; j=0; j--)
    {
      if (ref->i_long_term_pic_num > list[j]->i_long_term_pic_num)
      {
        break;
      }
    }
  }

  //[2] adjust the back part of the list
  for (k=i_list_idx; k>j; k--)
  {
    list[k+1]=list[k];
  }

  //[3] insert ref into list
  list[j+1]=ref;

}

//lsptod: support b and field
int p264_lists_init(p264_t *h)
{
  int i_max_frame_num = 1<< h->sps->i_log2_max_frame_num;
  int i;
  int i_short_term_count;

  //i_slice case: no list needed
  if(h->sh.i_type == SLICE_TYPE_I || h->sh.i_type == SLICE_TYPE_SI)
  {
    h->i_ref0=0;
    h->i_ref1=0;
    return 0;
  }

  //[1] compute picture num
  i_short_term_count=0;
  for( i = 1; i < h->sps->i_num_ref_frames + 1; i++ )
  {
    p264_frame_t *ref=h->frames.reference[i];
    if(SHORT_TERM_REF==ref->i_ref_type)
    {
      int i_frame_num_wrap;
      if(ref->i_frame_num > h->sh.i_frame_num)
      {
        i_frame_num_wrap=ref->i_frame_num - i_max_frame_num;
      }
      else
      {
        i_frame_num_wrap=ref->i_frame_num; 
      }
      ref->i_pic_num=i_frame_num_wrap;
      i_short_term_count++;
    }
  }

  //[2]Slice P: insert and sort
  if(h->sh.i_type == SLICE_TYPE_P || h->sh.i_type == SLICE_TYPE_SP)
  {
    int i_list0_short_idx=-1;
    int i_list0_long_idx=-1;
    //<1> add short term ref to list0 and sort
    for( i = 1; i < h->sps->i_num_ref_frames + 1; i++ )
    {
      p264_frame_t *ref=h->frames.reference[i];
      if (SHORT_TERM_REF==ref->i_ref_type)
      {
        insert_sort(h->fref0, i_list0_short_idx, ref);
        i_list0_short_idx++;
      }
    }
    //<2> add long term ref to list0 and sort
    for( i = 1; i < h->sps->i_num_ref_frames + 1; i++ )
    {
      p264_frame_t *ref=h->frames.reference[i];
      if (LONG_TERM_REF==ref->i_ref_type)
      {
        insert_sort(&h->fref0[i_short_term_count], i_list0_long_idx, ref);
        i_list0_long_idx++;
      }
    }
    //<3> set list0 size
    h->i_ref0=i_list0_short_idx+i_list0_long_idx+2;
  }//end of if(h->sh->i_type == SLICE_TYPE_P || h->sh->i_type == SLICE_TYPE_SP)

  //[3]Slice B: //lsptodo


  //lspdebug
  //printf("list_size:%d\n",h->i_ref0);

  return 0;
}

//lsptodo
int p264_lists_reorder(p264_t *h)
{
  return 0;
}

//lsptodo: support adaptive mode and discontinue frame num case
int p264_lists_marking(p264_t *h)
{
  int i,j;
  p264_slice_header_t *sh=&h->sh;

  if (NAL_SLICE_IDR==h->i_nal_type)
  {
    if (sh->b_no_output_of_prior_pics) //clear all
    {
      for( i = 1; i < h->sps->i_num_ref_frames + 1; i++ )
      {
        h->frames.reference[i]->i_ref_type=NOT_USED_FOR_REF;
      }
    }

    if (sh->b_long_term_reference_flag) //allow long term 
    {
      h->fdec->i_ref_type=LONG_TERM_REF;
    }
    else
    {
      h->fdec->i_ref_type=SHORT_TERM_REF;
    }
    //change
    h->fdec=h->frames.reference[1];
    h->frames.reference[1]=h->frames.reference[0];
    h->frames.reference[0]=h->fdec;
    h->frames.i_used_size=2;
  }//end of if (NAL_SLICE_IDR==h->i_nal_type)
  else //NON IDR
  {
    if(h->sh.b_adaptive_ref_pic_marking_mode)//lsptodo
    {
      printf("not support adaptive ref marking yet");
      return -1;
    }
    else //FIFO, also called sliding window mode
    {
      //<0> !!!
      h->fdec->i_ref_type=SHORT_TERM_REF;
      //<1> find the min picnum ref
      if(h->frames.i_used_size < h->sps->i_num_ref_frames+1)
      {
        i=h->frames.i_used_size;
        h->frames.i_used_size++;
      }
      else
      {
        for( i = h->frames.i_used_size-1; i>=0; i-- )
        {
          if (SHORT_TERM_REF==h->frames.reference[i]->i_ref_type)
          { 
            h->frames.reference[i]->i_ref_type=NOT_USED_FOR_REF;
            break;
          }
        }
      }
      h->fdec=h->frames.reference[i];//!!!
      //<2> update the list
      for(j=i; j>0; j--)
      {
        h->frames.reference[j]=h->frames.reference[j-1];
      }
      h->frames.reference[0]=h->fdec;
    }//end of FIFO
  }//end of non IDR

  //lspdebug
  //printf("ref_count:%d--",h->sps->i_num_ref_frames);
  //for( i = 1; i < h->sps->i_num_ref_frames + 1; i++ )
  //{
  //  printf("%d,",h->frames.reference[i]->i_frame_num);
  //}
  //printf("\n");

  return 0;
}