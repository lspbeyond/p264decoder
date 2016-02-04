/*****************************************************************************
 * p264: h264 encoder/decoder testing program.
 *****************************************************************************
 * Copyright (C) 2003 Laurent Aimar
 * $Id: p264.c,v 1.1 2004/06/03 19:24:12 fenrir Exp $
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
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
//lsp???

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <math.h>

#include <signal.h>
#define _GNU_SOURCE
#include <getopt.h>

#ifdef _MSC_VER
#include <io.h>     /* _setmode() */
#include <fcntl.h>  /* _O_BINARY */
#endif

#include "core/core.h"
#include "p264.h"

#ifndef _MSC_VER
#include "config.h"
#endif


#define DATA_MAX 3000000
uint8_t data[DATA_MAX];

typedef void *hnd_t;

/* Ctrl-C handler */
static int     b_ctrl_c = 0;
static int     b_exit_on_ctrl_c = 0;
static void    SigIntHandler( int a )
{
    if( b_exit_on_ctrl_c )
        exit(0);
    b_ctrl_c = 1;
}


static void Help( p264_param_t *defaults );
static int  Decode( p264_param_t  *param,int argc, char **argv);
/****************************************************************************
 * main:
 ****************************************************************************/
int main( int argc, char **argv )
{
    p264_param_t param;

#ifdef _MSC_VER
    _setmode(_fileno(stdin), _O_BINARY);    /* thanks to Marcos Morais <morais at dee.ufcg.edu.br> */
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    p264_param_default( &param );

    /* Control-C handler */
    signal( SIGINT, SigIntHandler );

    //lsp decoder
    //command line example: p264 -d test.264 [rec.yuv] [ref.yuv]
    if (argc>=3 && 0==strcmp(argv[1],"-d"))
    {
      return Decode(&param, argc, argv);
    }
    else
    {
      Help(&param);
      return -1;
    }
}

static const char * const overscan_str[] = { "undef", "show", "crop", NULL };
static const char * const vidformat_str[] = { "component", "pal", "ntsc", "secam", "mac", "undef", NULL };
static const char * const fullrange_str[] = { "off", "on", NULL };
static const char * const colorprim_str[] = { "", "bt709", "undef", "", "bt470m", "bt470bg", "smpte170m", "smpte240m", "film", NULL };
static const char * const transfer_str[] = { "", "bt709", "undef", "", "bt470m", "bt470bg", "smpte170m", "smpte240m", "linear", "log100", "log316", NULL };
static const char * const colmatrix_str[] = { "GBR", "bt709", "undef", "", "fcc", "bt470bg", "smpte170m", "smpte240m", "YCgCo", NULL };

static char const *strtable_lookup( const char * const table[], int index )
{
    int i = 0; while( table[i] ) i++;
    return ( ( index >= 0 && index < i ) ? table[ index ] : "???" );
}

/*****************************************************************************
 * Help:
 *****************************************************************************/
static void Help( p264_param_t *defaults )
{
    fprintf( stderr,
             "p264 Decoder:\n"
             "\n"
             "      -d <test.264> [recon.yuv] [origin.yuv]"
             "\n"
             );
}

/*****************************************************************************
 * Decode:
 *****************************************************************************/
//write frame yuv
void write_frame(p264_picture_t *pic,FILE *fp_rec)
{
  int	i;
  uint8_t* p;

  if(NULL==fp_rec)
    return;

  //y
  p=pic->img.plane[0];
  for (i=0; i<pic->i_height; i++)
  {
    fwrite(p,1,pic->i_width,fp_rec);
    p+=pic->img.i_stride[0];
  }
  //u
  p=pic->img.plane[1];
  for (i=0; i< pic->i_height>>1; i++)
  {
    fwrite(p,1,pic->i_width>>1,fp_rec);
    p+=pic->img.i_stride[1];
  }
  //v
  p=pic->img.plane[2];
  for (i=0; i< pic->i_height>>1; i++)
  {
    fwrite(p,1,pic->i_width>>1,fp_rec);
    p+=pic->img.i_stride[2];
  }

}

enum
{
  PARSE_STATE_INIT=0,
  PARSE_STATE_A,
  PARSE_STATE_B
};
static int  Decode( p264_param_t  *param,int argc, char **argv)
{
  p264_t *h;
  FILE *fp_h264=NULL, *fp_rec=NULL, *fp_ref=NULL;
  int i_data_size, i_cur, i_nal_start; //buffer correlative
  int b_eof_h264=0;
  int i_read;
  int b_need_buffer=0;
  int i_parse_state, i_start_code_size; //nal extraction correlative
  int b_find_nal;
  int b_decode_finish;
  p264_nal_t nal;
  p264_picture_t *p_pic_recon; //the recon picture
  int64_t i_start, i_end; //decode speed compute
  int i_frames=0;

  //[1] init
  fprintf(stderr,"decoding start...\n");
  //malloc
  nal.p_payload = malloc( DATA_MAX );
  //open h264 stream file
  fp_h264=fopen(argv[2],"rb");
  if (NULL==fp_h264)
  {
    fprintf(stderr, "open h264 stream file: %s failed\n",argv[2]);
    return -1;
  }
  //open rec file
  if (argc>=4)
  {
    fp_rec=fopen(argv[3],"wb");
  }
  //open ref file
  if (argc==5)
  {
    fp_ref=fopen(argv[4],"rb");
  }


  if( ( h = p264_decoder_open( param ) ) == NULL )
  {
    fprintf( stderr, "p264_decoder_open failed\n" );
    return -1;
  }

  i_start = p264_mdate();

  //confirm the first start code
  i_read=fread(&data[0],1,DATA_MAX,fp_h264);
  if(i_read < 4)
  {
    fprintf(stderr,"the h264 stream file is too small, even can't include the first start code\n");
    return -1;
  }
  i_data_size=i_read;
  if(data[0]!=0x00 || data[1]!=0x00 || data[2]!=0x00 || data[3]!=0x01 )
  {
    fprintf(stderr,"confirm the first start code failed\n");
    return -1;
  }
  i_cur=4;
  i_nal_start=4;
  b_need_buffer=0;
  b_decode_finish=0;
  //[2] main loop
  while (!b_ctrl_c && !b_decode_finish)
  {
    //[2.1] fill buffer
    if (b_need_buffer)
    {
      //transfer the partly read nal data to the top of the buffer
      //note: here, i_data_size must equal with i_cur
      i_data_size=i_cur-i_nal_start; 
      memmove(&data[0],&data[i_nal_start],i_data_size); //overlap is possible
      i_nal_start=0;
      i_cur=i_data_size;

      //read more data
      i_read=fread(&data[i_data_size],1,DATA_MAX-i_data_size,fp_h264);
      if (i_read<=0)
      {
        b_eof_h264=1;
      }
      else
      {
        i_data_size+=i_read;
      }

      b_need_buffer=0;//!!
    }
    else
    {
      i_parse_state=PARSE_STATE_INIT;
    }

    //[2.2] extract a nal
    b_find_nal=0;
    while (i_cur<i_data_size && !b_find_nal)
    {
      switch(i_parse_state) 
      {
      case PARSE_STATE_INIT:
        if (0==data[i_cur])
          i_parse_state=PARSE_STATE_A;
        else
          i_parse_state=PARSE_STATE_INIT;
        break;
      case PARSE_STATE_A:
        if (0==data[i_cur])
        {
          i_start_code_size=2;
          i_parse_state=PARSE_STATE_B;
        }
        else
          i_parse_state=PARSE_STATE_INIT;
        break;
      case PARSE_STATE_B:
        if (0==data[i_cur])
        {
          i_start_code_size++;
          i_parse_state=PARSE_STATE_B;
        }
        else if (1==data[i_cur])
        {
          i_start_code_size++;
          b_find_nal=1;
        }
        else
        {
          i_parse_state=PARSE_STATE_INIT;
        }
        break;
      default: 
        //can't jump here
        break;
      }
      i_cur++;//!!
    }//end of while (i_cur<i_data_size && !b_find_nal)

    //when we haven't found a nal...
    if (!b_find_nal && !b_eof_h264)
    {
      b_need_buffer=1;
      continue;
    }

    //judge decode finish
    if (i_cur>=i_data_size)
    {
      if (b_eof_h264)
      {
        i_start_code_size=0;//for it is the last nal //lsp!!!!
        b_decode_finish=1;
      }
      else
      {
        b_need_buffer=1; //the buffer is extractly used over
      }
    }
   
    //[2.3] decode the nal (find nal type...)
    p264_nal_decode( &nal, &data[i_nal_start], i_cur-i_nal_start-i_start_code_size );

    //update the data index
    i_nal_start=i_cur;


    //lspdebug
    //printf("nal size:%d, type: %d\n",nal.i_payload,nal.i_type);

    //[2.4] decode the nal content
    p264_decoder_decode( h, &p_pic_recon, &nal );
    if(NULL!=p_pic_recon)
    {
      i_frames++;
      if(NULL!=fp_rec)
      {
        write_frame(p_pic_recon, fp_rec);
      }
    }

    //if(h->sh.i_frame_num==4) //lspdebug
    //{
    //  break;
    //}

  }//end of while (!b_ctrl_c)

  i_end = p264_mdate();

  if( i_frames > 0 )
  {
    double fps = (double)i_frames * (double)1000000 /
      (double)( i_end - i_start );

    fprintf(stderr, "decoded total %d frames \n", i_frames);
    fprintf(stderr, "decoding speed: %.2f fps\n", fps);
  }

  //[3] free and close
   p264_decoder_close( h );

   //close file
   fclose(fp_h264);
   if (NULL!=fp_rec)
   {
     fclose(fp_rec);
   }
   if (NULL!=fp_ref)
   {
     fclose(fp_ref);
   }

   free( nal.p_payload );

   return 0;

}

