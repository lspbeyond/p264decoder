/*****************************************************************************
 * cpu.c: h264 encoder library
 *****************************************************************************
 * Copyright (C) 2003 Laurent Aimar
 * $Id: cpu.c,v 1.1 2004/06/03 19:27:06 fenrir Exp $
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

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "p264.h"
#include "cpu.h"

#if defined(ARCH_X86) || defined(ARCH_X86_64)
extern int  p264_cpu_cpuid_test( void );
extern uint32_t  p264_cpu_cpuid( uint32_t op, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx );
extern void p264_emms( void );

uint32_t p264_cpu_detect( void )
{
    uint32_t cpu = 0;

    uint32_t eax, ebx, ecx, edx;
    int      b_amd;


    if( !p264_cpu_cpuid_test() )
    {
        /* No cpuid */
        return 0;
    }

    p264_cpu_cpuid( 0, &eax, &ebx, &ecx, &edx);
    if( eax == 0 )
    {
        return 0;
    }
    b_amd   = (ebx == 0x68747541) && (ecx == 0x444d4163) && (edx == 0x69746e65);

    p264_cpu_cpuid( 1, &eax, &ebx, &ecx, &edx );
    if( (edx&0x00800000) == 0 )
    {
        /* No MMX */
        return 0;
    }
    cpu = P264_CPU_MMX;
    if( (edx&0x02000000) )
    {
        /* SSE - identical to AMD MMX extensions */
        cpu |= P264_CPU_MMXEXT|P264_CPU_SSE;
    }
    if( (edx&0x04000000) )
    {
        /* Is it OK ? */
        cpu |= P264_CPU_SSE2;
    }

    p264_cpu_cpuid( 0x80000000, &eax, &ebx, &ecx, &edx );
    if( eax < 0x80000001 )
    {
        /* no extended capabilities */
        return cpu;
    }

    p264_cpu_cpuid( 0x80000001, &eax, &ebx, &ecx, &edx );
    if( edx&0x80000000 )
    {
        cpu |= P264_CPU_3DNOW;
    }
    if( b_amd && (edx&0x00400000) )
    {
        /* AMD MMX extensions */
        cpu |= P264_CPU_MMXEXT;
    }

    return cpu;
}

void     p264_cpu_restore( uint32_t cpu )
{
    if( cpu&(P264_CPU_MMX|P264_CPU_MMXEXT|P264_CPU_3DNOW|P264_CPU_3DNOWEXT) )
    {
        p264_emms();
    }
}

#elif defined( ARCH_PPC )

#ifdef SYS_MACOSX
#include <sys/sysctl.h>
uint32_t p264_cpu_detect( void )
{
    /* Thank you VLC */
    uint32_t cpu = 0;
    int      selectors[2] = { CTL_HW, HW_VECTORUNIT };
    int      has_altivec = 0;
    size_t   length = sizeof( has_altivec );
    int      error = sysctl( selectors, 2, &has_altivec, &length, NULL, 0 );

    if( error == 0 && has_altivec != 0 )
    {
        cpu |= P264_CPU_ALTIVEC;
    }

    return cpu;
}

#elif defined( SYS_LINUX )
uint32_t p264_cpu_detect( void )
{
    /* FIXME (Linux PPC) */
    return P264_CPU_ALTIVEC;
}
#endif

void     p264_cpu_restore( uint32_t cpu )
{
}

#else

uint32_t p264_cpu_detect( void )
{
    return 0;
}

void     p264_cpu_restore( uint32_t cpu )
{
}

#endif
