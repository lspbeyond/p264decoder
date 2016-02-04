/*****************************************************************************
 * p264.h: h264 encoder library
 *****************************************************************************
 * Copyright (C) 2003 Laurent Aimar
 * $Id: p264.h,v 1.1 2004/06/03 19:24:12 fenrir Exp $
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

#ifndef _P264_H
#define _P264_H 1

#if !defined(_STDINT_H) && !defined(_STDINT_H_) && \
    !defined(_INTTYPES_H) && !defined(_INTTYPES_H_)
# ifdef _MSC_VER
#  pragma message("You must include stdint.h or inttypes.h before p264.h")
# else
#  warning You must include stdint.h or inttypes.h before p264.h
# endif
#endif

#include <stdarg.h>

#define P264_BUILD 42

/* p264_t:
 *      opaque handler for decoder and encoder */
typedef struct p264_t p264_t;

/****************************************************************************
 * Initialisation structure and function.
 ****************************************************************************/
/* CPU flags
 */
#define P264_CPU_MMX        0x000001    /* mmx */
#define P264_CPU_MMXEXT     0x000002    /* mmx-ext*/
#define P264_CPU_SSE        0x000004    /* sse */
#define P264_CPU_SSE2       0x000008    /* sse 2 */
#define P264_CPU_3DNOW      0x000010    /* 3dnow! */
#define P264_CPU_3DNOWEXT   0x000020    /* 3dnow! ext */
#define P264_CPU_ALTIVEC    0x000040    /* altivec */

/* Analyse flags
 */
#define P264_ANALYSE_I4x4       0x0001  /* Analyse i4x4 */
#define P264_ANALYSE_I8x8       0x0002  /* Analyse i8x8 (requires 8x8 transform) */
#define P264_ANALYSE_PSUB16x16  0x0010  /* Analyse p16x8, p8x16 and p8x8 */
#define P264_ANALYSE_PSUB8x8    0x0020  /* Analyse p8x4, p4x8, p4x4 */
#define P264_ANALYSE_BSUB16x16  0x0100  /* Analyse b16x8, b8x16 and b8x8 */
#define P264_DIRECT_PRED_NONE        0
#define P264_DIRECT_PRED_SPATIAL     1
#define P264_DIRECT_PRED_TEMPORAL    2
#define P264_ME_DIA                  0
#define P264_ME_HEX                  1
#define P264_ME_UMH                  2
#define P264_ME_ESA                  3
#define P264_CQM_FLAT                0
#define P264_CQM_JVT                 1
#define P264_CQM_CUSTOM              2

static const char * const p264_direct_pred_names[] = { "none", "spatial", "temporal", 0 };
static const char * const p264_motion_est_names[] = { "dia", "hex", "umh", "esa", 0 };

/* Colorspace type
 */
#define P264_CSP_MASK           0x00ff  /* */
#define P264_CSP_NONE           0x0000  /* Invalid mode     */
#define P264_CSP_I420           0x0001  /* yuv 4:2:0 planar */
#define P264_CSP_I422           0x0002  /* yuv 4:2:2 planar */
#define P264_CSP_I444           0x0003  /* yuv 4:4:4 planar */
#define P264_CSP_YV12           0x0004  /* yuv 4:2:0 planar */
#define P264_CSP_YUYV           0x0005  /* yuv 4:2:2 packed */
#define P264_CSP_RGB            0x0006  /* rgb 24bits       */
#define P264_CSP_BGR            0x0007  /* bgr 24bits       */
#define P264_CSP_BGRA           0x0008  /* bgr 32bits       */
#define P264_CSP_VFLIP          0x1000  /* */

/* Slice type
 */
#define P264_TYPE_AUTO          0x0000  /* Let p264 choose the right type */
#define P264_TYPE_IDR           0x0001
#define P264_TYPE_I             0x0002
#define P264_TYPE_P             0x0003
#define P264_TYPE_BREF          0x0004  /* Non-disposable B-frame */
#define P264_TYPE_B             0x0005
#define IS_P264_TYPE_I(x) ((x)==P264_TYPE_I || (x)==P264_TYPE_IDR)
#define IS_P264_TYPE_B(x) ((x)==P264_TYPE_B || (x)==P264_TYPE_BREF)

/* Log level
 */
#define P264_LOG_NONE          (-1)
#define P264_LOG_ERROR          0
#define P264_LOG_WARNING        1
#define P264_LOG_INFO           2
#define P264_LOG_DEBUG          3

typedef struct
{
    int i_start, i_end;
    int b_force_qp;
    int i_qp;
    float f_bitrate_factor;
} p264_zone_t;

typedef struct
{
    /* CPU flags */
    unsigned int cpu;
    int         i_threads;  /* divide each frame into multiple slices, encode in parallel */

    /* Video Properties */
    int         i_width;
    int         i_height;
    int         i_csp;  /* CSP of encoded bitstream, only i420 supported */
    int         i_level_idc; 
    int         i_frame_total; /* number of frames to encode if known, else 0 */

    struct
    {
        /* they will be reduced to be 0 < x <= 65535 and prime */
        int         i_sar_height;
        int         i_sar_width;

        int         i_overscan;    /* 0=undef, 1=no overscan, 2=overscan */
        
        /* see h264 annex E for the values of the following */
        int         i_vidformat;
        int         b_fullrange;
        int         i_colorprim;
        int         i_transfer;
        int         i_colmatrix;
        int         i_chroma_loc;    /* both top & bottom */
    } vui;

    int         i_fps_num;
    int         i_fps_den;

    /* Bitstream parameters */
    int         i_frame_reference;  /* Maximum number of reference frames */
    int         i_keyint_max;       /* Force an IDR keyframe at this interval */
    int         i_keyint_min;       /* Scenecuts closer together than this are coded as I, not IDR. */
    int         i_scenecut_threshold; /* how aggressively to insert extra I frames */
    int         i_bframe;   /* how many b-frame between 2 references pictures */
    int         b_bframe_adaptive;
    int         i_bframe_bias;
    int         b_bframe_pyramid;   /* Keep some B-frames as references */

    int         b_deblocking_filter;
    int         i_deblocking_filter_alphac0;    /* [-6, 6] -6 light filter, 6 strong */
    int         i_deblocking_filter_beta;       /* [-6, 6]  idem */

    int         b_cabac;
    int         i_cabac_init_idc;

    int         i_cqm_preset;
    char        *psz_cqm_file;      /* JM format */
    uint8_t     cqm_4iy[16];        /* used only if i_cqm_preset == P264_CQM_CUSTOM */
    uint8_t     cqm_4ic[16];
    uint8_t     cqm_4py[16];
    uint8_t     cqm_4pc[16];
    uint8_t     cqm_8iy[64];
    uint8_t     cqm_8py[64];

    /* Log */
    void        (*pf_log)( void *, int i_level, const char *psz, va_list );
    void        *p_log_private;
    int         i_log_level;
    int         b_visualize;

    /* Encoder analyser parameters */
    struct
    {
        unsigned int intra;     /* intra partitions */
        unsigned int inter;     /* inter partitions */

        int          b_transform_8x8;
        int          b_weighted_bipred; /* implicit weighting for B-frames */
        int          i_direct_mv_pred; /* spatial vs temporal mv prediction */
        int          i_chroma_qp_offset;

        int          i_me_method; /* motion estimation algorithm to use (P264_ME_*) */
        int          i_me_range; /* integer pixel motion estimation search range (from predicted mv) */
        int          i_mv_range; /* maximum length of a mv (in pixels) */
        int          i_subpel_refine; /* subpixel motion estimation quality */
        int          b_chroma_me; /* chroma ME for subpel and mode decision in P-frames */
        int          b_bframe_rdo; /* RD based mode decision for B-frames */
        int          b_mixed_references; /* allow each mb partition in P-frames to have it's own reference number */
        int          i_trellis;  /* trellis RD quantization */
        int          b_fast_pskip; /* early SKIP detection on P-frames */

        int          b_psnr;    /* Do we compute PSNR stats (save a few % of cpu) */
    } analyse;

    /* Rate control parameters */
    struct
    {
        int         i_qp_constant;  /* 0-51 */
        int         i_qp_min;       /* min allowed QP value */
        int         i_qp_max;       /* max allowed QP value */
        int         i_qp_step;      /* max QP step between frames */

        int         b_cbr;          /* use bitrate instead of CQP */
        int         i_bitrate;
        int         i_rf_constant;  /* 1pass VBR, nominal QP */
        float       f_rate_tolerance;
        int         i_vbv_max_bitrate;
        int         i_vbv_buffer_size;
        float       f_vbv_buffer_init;
        float       f_ip_factor;
        float       f_pb_factor;

        /* 2pass */
        int         b_stat_write;   /* Enable stat writing in psz_stat_out */
        char        *psz_stat_out;
        int         b_stat_read;    /* Read stat from psz_stat_in and use it */
        char        *psz_stat_in;

        /* 2pass params (same as ffmpeg ones) */
        char        *psz_rc_eq;     /* 2 pass rate control equation */
        float       f_qcompress;    /* 0.0 => cbr, 1.0 => constant qp */
        float       f_qblur;        /* temporally blur quants */
        float       f_complexity_blur; /* temporally blur complexity */
        p264_zone_t *zones;         /* ratecontrol overrides */
        int         i_zones;        /* sumber of zone_t's */
        char        *psz_zones;     /* alternate method of specifying zones */
    } rc;

    int b_aud;                  /* generate access unit delimiters */
    int b_repeat_headers;       /* put SPS/PPS before each keyframe */
} p264_param_t;

typedef struct {
    int level_idc;
    int mbps;        // max macroblock processing rate (macroblocks/sec)
    int frame_size;  // max frame size (macroblocks)
    int dpb;         // max decoded picture buffer (bytes)
    int bitrate;     // max bitrate (kbit/sec)
    int cpb;         // max vbv buffer (kbit)
    int mv_range;    // max vertical mv component range (pixels)
    int mvs_per_2mb; // max mvs per 2 consecutive mbs.
    int slice_rate;  // ??
    int bipred8x8;   // limit bipred to >=8x8
    int direct8x8;   // limit b_direct to >=8x8
    int frame_only;  // forbid interlacing
} p264_level_t;

/* all of the levels defined in the standard, terminated by .level_idc=0 */
extern const p264_level_t p264_levels[];

/* p264_param_default:
 *      fill p264_param_t with default values and do CPU detection */
void    p264_param_default( p264_param_t * );

/****************************************************************************
 * Picture structures and functions.
 ****************************************************************************/
typedef struct
{
    int     i_csp; //color s plane

    int     i_plane;
    int     i_stride[4];
    uint8_t *plane[4];
} p264_image_t;

typedef struct
{
    /* In: force picture type (if not auto) XXX: ignored for now
     * Out: type of the picture encoded */
    int     i_type;
    /* In: force quantizer for > 0 */
    int     i_qpplus1;
    /* In: user pts, Out: pts of encoded picture (user)*/
    int64_t i_pts;

    //lsp out: for decoding
    int i_width;
    int i_height;

    /* In: raw data */
    p264_image_t img;
} p264_picture_t;

/* p264_picture_alloc:
 *  alloc data for a picture. You must call p264_picture_clean on it. */
void p264_picture_alloc( p264_picture_t *pic, int i_csp, int i_width, int i_height );

/* p264_picture_clean:
 *  free associated resource for a p264_picture_t allocated with
 *  p264_picture_alloc ONLY */
void p264_picture_clean( p264_picture_t *pic );

/****************************************************************************
 * NAL structure and functions:
 ****************************************************************************/
/* nal */
enum nal_unit_type_e
{
    NAL_UNKNOWN = 0,
    NAL_SLICE   = 1,
    NAL_SLICE_DPA   = 2,
    NAL_SLICE_DPB   = 3,
    NAL_SLICE_DPC   = 4,
    NAL_SLICE_IDR   = 5,    /* ref_idc != 0 */
    NAL_SEI         = 6,    /* ref_idc == 0 */
    NAL_SPS         = 7,
    NAL_PPS         = 8,
    NAL_AUD         = 9,
    /* ref_idc == 0 for 6,9,10,11,12 */
};
enum nal_priority_e
{
    NAL_PRIORITY_DISPOSABLE = 0,
    NAL_PRIORITY_LOW        = 1,
    NAL_PRIORITY_HIGH       = 2,
    NAL_PRIORITY_HIGHEST    = 3,
};

typedef struct
{
    int i_ref_idc;  /* nal_priority_e */
    int i_type;     /* nal_unit_type_e */

    /* This data are raw payload */
    int     i_payload;
    uint8_t *p_payload;
} p264_nal_t;

/* p264_nal_encode:
 *      encode a nal into a buffer, setting the size.
 *      if b_annexeb then a long synch work is added
 *      XXX: it currently doesn't check for overflow */
int p264_nal_encode( void *, int *, int b_annexeb, p264_nal_t *nal );

/* p264_nal_decode:
 *      decode a buffer nal into a p264_nal_t */
int p264_nal_decode( p264_nal_t *nal, void *, int );

/****************************************************************************
 * Encoder functions:
 ****************************************************************************/

/* p264_encoder_open:
 *      create a new encoder handler, all parameters from p264_param_t are copied */
p264_t *p264_encoder_open   ( p264_param_t * );
/* p264_encoder_reconfig:
 *      change encoder options while encoding,
 *      analysis-related parameters from p264_param_t are copied */
int     p264_encoder_reconfig( p264_t *, p264_param_t * );
/* p264_encoder_headers:
 *      return the SPS and PPS that will be used for the whole stream */
int     p264_encoder_headers( p264_t *, p264_nal_t **, int * );
/* p264_encoder_encode:
 *      encode one picture */
int     p264_encoder_encode ( p264_t *, p264_nal_t **, int *, p264_picture_t *, p264_picture_t * );
/* p264_encoder_close:
 *      close an encoder handler */
void    p264_encoder_close  ( p264_t * );

/* XXX: decoder isn't working so no need to export it */
/****************************************************************************
* Decoder functions:
****************************************************************************/
//lsp
p264_t *p264_decoder_open   ( p264_param_t *param );
void    p264_decoder_close  ( p264_t *h );
int     p264_decoder_decode( p264_t *h,
                            p264_picture_t **pp_pic, p264_nal_t *nal );

/****************************************************************************
 * Private stuff for internal usage:
 ****************************************************************************/
#ifdef __P264__
#   ifdef _MSC_VER
#       define inline __inline
#       define DECLARE_ALIGNED( type, var, n ) __declspec(align(n)) type var
#		define strncasecmp(s1, s2, n) strnicmp(s1, s2, n)
#   else
#       define DECLARE_ALIGNED( type, var, n ) type var __attribute__((aligned(n)))
#   endif
#endif

#endif
