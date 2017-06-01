/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2014, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2014, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux
 * Copyright (c) 2003-2014, Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2007, Callum Lerwick <seg@haxxed.com>
 * Copyright (c) 2012, Carl Hetherington
 * Copyright (c) 2017, IntoPIX SA <support@intopix.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "opj_includes.h"
#include "t1_luts.h"

/** @defgroup T1 T1 - Implementation of the tier-1 coding */
/*@{*/

#define T1_FLAGS(x, y) (t1->flags[x + 1 + ((y / 4) + 1) * (t1->w+2)])

/** @name Local static functions */
/*@{*/

static INLINE OPJ_BYTE opj_t1_getctxno_zc(opj_mqc_t *mqc, OPJ_UINT32 f);
static INLINE OPJ_UINT32 opj_t1_getctxno_mag(OPJ_UINT32 f);
static OPJ_INT16 opj_t1_getnmsedec_sig(OPJ_UINT32 x, OPJ_UINT32 bitpos);
static OPJ_INT16 opj_t1_getnmsedec_ref(OPJ_UINT32 x, OPJ_UINT32 bitpos);
static INLINE void opj_t1_update_flags(opj_flag_t *flagsp, OPJ_UINT32 ci,
                                       OPJ_UINT32 s, OPJ_UINT32 stride);


/**
Decode significant pass
*/

static INLINE void opj_t1_dec_sigpass_step_raw(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_INT32 vsc,
    OPJ_UINT32 row);
static INLINE void opj_t1_dec_sigpass_step_mqc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_UINT32 row,
    OPJ_UINT32 flags_stride);
static INLINE void opj_t1_dec_sigpass_step_mqc_vsc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_INT32 vsc,
    OPJ_UINT32 row);


/**
Encode significant pass
*/
static void opj_t1_enc_sigpass(opj_t1_t *t1,
                               OPJ_INT32 bpno,
                               OPJ_INT32 *nmsedec,
                               OPJ_BYTE type,
                               OPJ_UINT32 cblksty);

/**
Decode significant pass
*/
static void opj_t1_dec_sigpass_raw(
    opj_t1_t *t1,
    OPJ_INT32 bpno,
    OPJ_INT32 cblksty);
static void opj_t1_dec_sigpass_mqc_vsc(
    opj_t1_t *t1,
    OPJ_INT32 bpno);



/**
Encode refinement pass
*/
static void opj_t1_enc_refpass(opj_t1_t *t1,
                               OPJ_INT32 bpno,
                               OPJ_INT32 *nmsedec,
                               OPJ_BYTE type,
                               OPJ_UINT32 cblksty);

/**
Decode refinement pass
*/
static void opj_t1_dec_refpass_raw(
    opj_t1_t *t1,
    OPJ_INT32 bpno);
static void opj_t1_dec_refpass_mqc_vsc(
    opj_t1_t *t1,
    OPJ_INT32 bpno);


/**
Decode refinement pass
*/

static INLINE void  opj_t1_dec_refpass_step_raw(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 poshalf,
    OPJ_UINT32 row);
static INLINE void opj_t1_dec_refpass_step_mqc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 poshalf,
    OPJ_UINT32 row);
static INLINE void opj_t1_dec_refpass_step_mqc_vsc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 poshalf,
    OPJ_INT32 vsc,
    OPJ_UINT32 row);


/**
Decode clean-up pass
*/
static void opj_t1_dec_clnpass_step_partial(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_UINT32 row);
static void opj_t1_dec_clnpass_step(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_UINT32 row);
static void opj_t1_dec_clnpass_step_vsc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_INT32 partial,
    OPJ_INT32 vsc,
    OPJ_UINT32 row);
/**
Encode clean-up pass
*/
static void opj_t1_enc_clnpass(
    opj_t1_t *t1,
    OPJ_INT32 bpno,
    OPJ_INT32 *nmsedec,
    OPJ_UINT32 cblksty);

static OPJ_FLOAT64 opj_t1_getwmsedec(
    OPJ_INT32 nmsedec,
    OPJ_UINT32 compno,
    OPJ_UINT32 level,
    OPJ_UINT32 orient,
    OPJ_INT32 bpno,
    OPJ_UINT32 qmfbid,
    OPJ_FLOAT64 stepsize,
    OPJ_UINT32 numcomps,
    const OPJ_FLOAT64 * mct_norms,
    OPJ_UINT32 mct_numcomps);

static void opj_t1_encode_cblk(opj_t1_t *t1,
                               opj_tcd_cblk_enc_t* cblk,
                               OPJ_UINT32 orient,
                               OPJ_UINT32 compno,
                               OPJ_UINT32 level,
                               OPJ_UINT32 qmfbid,
                               OPJ_FLOAT64 stepsize,
                               OPJ_UINT32 cblksty,
                               OPJ_UINT32 numcomps,
                               opj_tcd_tile_t * tile,
                               const OPJ_FLOAT64 * mct_norms,
                               OPJ_UINT32 mct_numcomps);

/**
Decode 1 code-block
@param t1 T1 handle
@param cblk Code-block coding parameters
@param orient
@param roishift Region of interest shifting value
@param cblksty Code-block style
*/
static OPJ_BOOL opj_t1_decode_cblk(opj_t1_t *t1,
                                   opj_tcd_cblk_dec_t* cblk,
                                   OPJ_UINT32 orient,
                                   OPJ_UINT32 roishift,
                                   OPJ_UINT32 cblksty);

static OPJ_BOOL opj_t1_allocate_buffers(opj_t1_t *t1,
                                        OPJ_UINT32 w,
                                        OPJ_UINT32 h);

/*@}*/

/*@}*/

/* ----------------------------------------------------------------------- */

static INLINE OPJ_BYTE opj_t1_getctxno_zc(opj_mqc_t *mqc, OPJ_UINT32 f)
{
    return mqc->lut_ctxno_zc_orient[(f & T1_SIGMA_NEIGHBOURS)];
}

static INLINE OPJ_UINT32 opj_t1_getctxtno_sc_or_spb_index(OPJ_UINT32 fX,
        OPJ_UINT32 pfX,
        OPJ_UINT32 nfX,
        OPJ_UINT32 ci)
{
    /*
      0 pfX T1_CHI_THIS           T1_LUT_SGN_W
      1 tfX T1_SIGMA_1            T1_LUT_SIG_N
      2 nfX T1_CHI_THIS           T1_LUT_SGN_E
      3 tfX T1_SIGMA_3            T1_LUT_SIG_W
      4  fX T1_CHI_(THIS - 1)     T1_LUT_SGN_N
      5 tfX T1_SIGMA_5            T1_LUT_SIG_E
      6  fX T1_CHI_(THIS + 1)     T1_LUT_SGN_S
      7 tfX T1_SIGMA_7            T1_LUT_SIG_S
    */

    OPJ_UINT32 lu = (fX >> (ci * 3U)) & (T1_SIGMA_1 | T1_SIGMA_3 | T1_SIGMA_5 |
                                         T1_SIGMA_7);

    lu |= (pfX >> (T1_CHI_THIS_I      + (ci * 3U))) & (1U << 0);
    lu |= (nfX >> (T1_CHI_THIS_I - 2U + (ci * 3U))) & (1U << 2);
    if (ci == 0U) {
        lu |= (fX >> (T1_CHI_0_I - 4U)) & (1U << 4);
    } else {
        lu |= (fX >> (T1_CHI_1_I - 4U + ((ci - 1U) * 3U))) & (1U << 4);
    }
    lu |= (fX >> (T1_CHI_2_I - 6U + (ci * 3U))) & (1U << 6);
    return lu;
}

static INLINE OPJ_BYTE opj_t1_getctxno_sc(OPJ_UINT32 lu)
{
    return lut_ctxno_sc[lu];
}

static INLINE OPJ_UINT32 opj_t1_getctxno_mag(OPJ_UINT32 f)
{
    OPJ_UINT32 tmp = (f & T1_SIGMA_NEIGHBOURS) ? T1_CTXNO_MAG + 1 : T1_CTXNO_MAG;
    OPJ_UINT32 tmp2 = (f & T1_MU_0) ? T1_CTXNO_MAG + 2 : tmp;
    return tmp2;
}

static INLINE OPJ_BYTE opj_t1_getspb(OPJ_UINT32 lu)
{
    return lut_spb[lu];
}

static OPJ_INT16 opj_t1_getnmsedec_sig(OPJ_UINT32 x, OPJ_UINT32 bitpos)
{
    if (bitpos > 0) {
        return lut_nmsedec_sig[(x >> (bitpos)) & ((1 << T1_NMSEDEC_BITS) - 1)];
    }

    return lut_nmsedec_sig0[x & ((1 << T1_NMSEDEC_BITS) - 1)];
}

static OPJ_INT16 opj_t1_getnmsedec_ref(OPJ_UINT32 x, OPJ_UINT32 bitpos)
{
    if (bitpos > 0) {
        return lut_nmsedec_ref[(x >> (bitpos)) & ((1 << T1_NMSEDEC_BITS) - 1)];
    }

    return lut_nmsedec_ref0[x & ((1 << T1_NMSEDEC_BITS) - 1)];
}


static INLINE void opj_t1_update_flags(opj_flag_t *flagsp, OPJ_UINT32 ci,
                                       OPJ_UINT32 s, OPJ_UINT32 stride)
{
    /* east */
    flagsp[-1] |= T1_SIGMA_5 << (3U * ci);

    /* mark target as significant */
    *flagsp |= ((s << T1_CHI_1_I) | T1_SIGMA_4) << (3U * ci);

    /* west */
    flagsp[1] |= T1_SIGMA_3 << (3U * ci);

    /* north-west, north, north-east */
    if (ci == 0U) {
        opj_flag_t* north = flagsp - stride;
        *north |= (s << T1_CHI_5_I) | T1_SIGMA_16;
        north[-1] |= T1_SIGMA_17;
        north[1] |= T1_SIGMA_15;
    }

    /* south-west, south, south-east */
    if (ci == 3U) {
        opj_flag_t* south = flagsp + stride;
        *south |= (s << T1_CHI_0_I) | T1_SIGMA_1;
        south[-1] |= T1_SIGMA_2;
        south[1] |= T1_SIGMA_0;
    }
}

/**
Encode significant pass
*/
static INLINE void opj_t1_enc_sigpass_step(opj_t1_t *t1,
        opj_flag_t *flagsp,
        OPJ_INT32 *datap,
        OPJ_INT32 bpno,
        OPJ_INT32 one,
        OPJ_INT32 *nmsedec,
        OPJ_BYTE type,
        OPJ_UINT32 ci,
        OPJ_UINT32 vsc
                                          )
{
    OPJ_UINT32 v;

    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */

    OPJ_UINT32 vsc_mask = vsc ? ~((T1_SIGMA_SW | T1_SIGMA_S | T1_SIGMA_SE |
                                   T1_CHI_S) << (ci * 3U)) : ~0U;
    OPJ_UINT32 const flags = *flagsp & vsc_mask;

    if ((flags & ((T1_SIGMA_THIS | T1_PI_THIS) << (ci * 3U))) == 0U &&
            (flags & (T1_SIGMA_NEIGHBOURS << (ci * 3U))) != 0U) {
        OPJ_UINT32 ctxt1 = opj_t1_getctxno_zc(mqc, flags >> (ci * 3U));
        v = opj_int_abs(*datap) & one ? 1 : 0;
#ifdef DEBUG_ENC_SIG
        fprintf(stderr, "   ctxt1=%d\n", ctxt1);
#endif
        opj_mqc_setcurctx(mqc, ctxt1);
        if (type == T1_TYPE_RAW) {  /* BYPASS/LAZY MODE */
            opj_mqc_bypass_enc(mqc, v);
        } else {
            opj_mqc_encode(mqc, v);
        }
        if (v) {
            /* Note: using flags instead of *flagsp & vsc_mask result */
            /* in slow down. Probably because of register pressure */
            OPJ_UINT32 lu = opj_t1_getctxtno_sc_or_spb_index(
                                *flagsp & vsc_mask,
                                flagsp[-1] & vsc_mask, flagsp[1] & vsc_mask,
                                ci);
            OPJ_UINT32 ctxt2 = opj_t1_getctxno_sc(lu);
            v = *datap < 0 ? 1 : 0;
            *nmsedec += opj_t1_getnmsedec_sig((OPJ_UINT32)opj_int_abs(*datap),
                                              (OPJ_UINT32)bpno);
#ifdef DEBUG_ENC_SIG
            fprintf(stderr, "   ctxt2=%d\n", ctxt2);
#endif
            opj_mqc_setcurctx(mqc, ctxt2);
            if (type == T1_TYPE_RAW) {  /* BYPASS/LAZY MODE */
                opj_mqc_bypass_enc(mqc, v);
            } else {
                OPJ_UINT32 spb = opj_t1_getspb(lu);
#ifdef DEBUG_ENC_SIG
                fprintf(stderr, "   spb=%d\n", spb);
#endif
                opj_mqc_encode(mqc, v ^ spb);
            }
            opj_t1_update_flags(flagsp, ci, v, t1->w + 2);
        }
        *flagsp |= T1_PI_THIS << (ci * 3U);
    }
}

static INLINE void opj_t1_dec_sigpass_step_raw(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_INT32 vsc,
    OPJ_UINT32 ci)
{
    OPJ_UINT32 v;
    opj_raw_t *raw = &(t1->raw);       /* RAW component */

    OPJ_UINT32 vsc_mask = vsc ? ~((T1_SIGMA_SW | T1_SIGMA_S | T1_SIGMA_SE |
                                   T1_CHI_S) << (ci * 3U)) : ~0U;
    OPJ_UINT32 const flags = *flagsp & vsc_mask;

    if ((flags & ((T1_SIGMA_THIS | T1_PI_THIS) << (ci * 3U))) == 0U &&
            (flags & (T1_SIGMA_NEIGHBOURS << (ci * 3U))) != 0U) {
        if (opj_raw_decode(raw)) {
            v = opj_raw_decode(raw);
            *datap = v ? -oneplushalf : oneplushalf;
            opj_t1_update_flags(flagsp, ci, v, t1->w + 2);
        }
        *flagsp |= T1_PI_THIS << (ci * 3U);
    }
}

static INLINE void opj_t1_dec_sigpass_step_mqc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_UINT32 ci,
    OPJ_UINT32 flags_stride)
{
    OPJ_UINT32 v;

    opj_mqc_t *mqc = &(t1->mqc);       /* MQC component */
    OPJ_UINT32 const flags = *flagsp;

    if ((flags & ((T1_SIGMA_THIS | T1_PI_THIS) << (ci * 3U))) == 0U &&
            (flags & (T1_SIGMA_NEIGHBOURS << (ci * 3U))) != 0U) {
        OPJ_UINT32 lu = opj_t1_getctxtno_sc_or_spb_index(
                            *flagsp,
                            flagsp[-1], flagsp[1],
                            ci);
        OPJ_UINT32 ctxt1 = opj_t1_getctxno_zc(mqc, flags >> (ci * 3U));
        opj_mqc_setcurctx(mqc, ctxt1);
        if (opj_mqc_decode(mqc)) {
            OPJ_UINT32 ctxt2 = opj_t1_getctxno_sc(lu);
            OPJ_UINT32 spb = opj_t1_getspb(lu);
            opj_mqc_setcurctx(mqc, ctxt2);
            v = opj_mqc_decode(mqc) ^ spb;
            *datap = v ? -oneplushalf : oneplushalf;
            opj_t1_update_flags(flagsp, ci, v, flags_stride);
        }
        *flagsp |= T1_PI_THIS << (ci * 3U);
    }
}

static INLINE void opj_t1_dec_sigpass_step_mqc_vsc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_INT32 vsc,
    OPJ_UINT32 ci)
{
    OPJ_UINT32 v;

    opj_mqc_t *mqc = &(t1->mqc);       /* MQC component */

    OPJ_UINT32 vsc_mask = vsc ? ~((T1_SIGMA_SW | T1_SIGMA_S | T1_SIGMA_SE |
                                   T1_CHI_S) << (ci * 3U)) : ~0U;
    OPJ_UINT32 const flags = *flagsp & vsc_mask;

    if ((flags & ((T1_SIGMA_THIS | T1_PI_THIS) << (ci * 3U))) == 0U &&
            (flags & (T1_SIGMA_NEIGHBOURS << (ci * 3U))) != 0U) {
        OPJ_UINT32 lu = opj_t1_getctxtno_sc_or_spb_index(
                            *flagsp & vsc_mask,
                            flagsp[-1] & vsc_mask, flagsp[1] & vsc_mask,
                            ci);
        OPJ_UINT32 ctxt1 = opj_t1_getctxno_zc(mqc, flags >> (ci * 3U));
        opj_mqc_setcurctx(mqc, ctxt1);
        if (opj_mqc_decode(mqc)) {
            OPJ_UINT32 ctxt2 = opj_t1_getctxno_sc(lu);
            OPJ_UINT32 spb = opj_t1_getspb(lu);
            opj_mqc_setcurctx(mqc, ctxt2);
            v = opj_mqc_decode(mqc) ^ spb;
            *datap = v ? -oneplushalf : oneplushalf;
            opj_t1_update_flags(flagsp, ci, v, t1->w + 2);
        }
        *flagsp |= T1_PI_THIS << (ci * 3U);
    }
}


static void opj_t1_enc_sigpass(opj_t1_t *t1,
                               OPJ_INT32 bpno,
                               OPJ_INT32 *nmsedec,
                               OPJ_BYTE type,
                               OPJ_UINT32 cblksty
                              )
{
    OPJ_UINT32 i, k;
    OPJ_INT32 const one = 1 << (bpno + T1_NMSEDEC_FRACBITS);
    opj_flag_t* f = &T1_FLAGS(0, 0);
    OPJ_UINT32 const extra = 2;

    *nmsedec = 0;
#ifdef DEBUG_ENC_SIG
    fprintf(stderr, "enc_sigpass: bpno=%d\n", bpno);
#endif
    for (k = 0; k < (t1->h & ~3U); k += 4) {
#ifdef DEBUG_ENC_SIG
        fprintf(stderr, " k=%d\n", k);
#endif
        for (i = 0; i < t1->w; ++i) {
#ifdef DEBUG_ENC_SIG
            fprintf(stderr, " i=%d\n", i);
#endif
            if (*f == 0U) {
                /* Nothing to do for any of the 4 data points */
                f++;
                continue;
            }
            opj_t1_enc_sigpass_step(
                t1,
                f,
                &t1->data[((k + 0) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                type,
                0, 0);
            opj_t1_enc_sigpass_step(
                t1,
                f,
                &t1->data[((k + 1) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                type,
                1, 0);
            opj_t1_enc_sigpass_step(
                t1,
                f,
                &t1->data[((k + 2) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                type,
                2, 0);
            opj_t1_enc_sigpass_step(
                t1,
                f,
                &t1->data[((k + 3) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                type,
                3, cblksty & J2K_CCP_CBLKSTY_VSC);
            ++f;
        }
        f += extra;
    }

    if (k < t1->h) {
        OPJ_UINT32 j;
#ifdef DEBUG_ENC_SIG
        fprintf(stderr, " k=%d\n", k);
#endif
        for (i = 0; i < t1->w; ++i) {
#ifdef DEBUG_ENC_SIG
            fprintf(stderr, " i=%d\n", i);
#endif
            if (*f == 0U) {
                /* Nothing to do for any of the 4 data points */
                f++;
                continue;
            }
            for (j = k; j < t1->h; ++j) {
                opj_t1_enc_sigpass_step(
                    t1,
                    f,
                    &t1->data[(j * t1->data_stride) + i],
                    bpno,
                    one,
                    nmsedec,
                    type,
                    j - k,
                    (j == t1->h - 1 && (cblksty & J2K_CCP_CBLKSTY_VSC) != 0));
            }
            ++f;
        }
    }
}

static void opj_t1_dec_sigpass_raw(
    opj_t1_t *t1,
    OPJ_INT32 bpno,
    OPJ_INT32 cblksty)
{
    OPJ_INT32 one, half, oneplushalf;
    OPJ_UINT32 i, j, k;
    opj_flag_t *flagsp = &T1_FLAGS(0, 0);
    one = 1 << bpno;
    half = one >> 1;
    oneplushalf = one | half;
    if ((cblksty & J2K_CCP_CBLKSTY_VSC)) {
        OPJ_INT32 *data1 = t1->data;
        for (k = 0; k < t1->h; k += 4) {
            for (i = 0; i < t1->w; ++i) {
                OPJ_INT32* data2 = data1 + i;
                for (j = k; j < k + 4 && j < t1->h; ++j) {
                    OPJ_INT32 vsc = (j == k + 3 || j == t1->h - 1) ? 1 : 0;
                    opj_t1_dec_sigpass_step_raw(
                        t1,
                        flagsp,
                        data2,
                        oneplushalf,
                        vsc,
                        j - k);
                    data2 += t1->w;
                }
                flagsp ++;
            }
            data1 += t1->w << 2;
            flagsp += 2;
        }
    } else {
        OPJ_INT32 *data1 = t1->data;
        for (k = 0; k < (t1->h & ~3U); k += 4) {
            for (i = 0; i < t1->w; ++i) {
                OPJ_INT32* data2 = data1 + i;
                opj_t1_dec_sigpass_step_raw(
                    t1,
                    flagsp,
                    data2,
                    oneplushalf,
                    0, /* vsc */
                    0U);
                data2 += t1->w;
                opj_t1_dec_sigpass_step_raw(
                    t1,
                    flagsp,
                    data2,
                    oneplushalf,
                    0, /* vsc */
                    1U);
                data2 += t1->w;
                opj_t1_dec_sigpass_step_raw(
                    t1,
                    flagsp,
                    data2,
                    oneplushalf,
                    0, /* vsc */
                    2U);
                data2 += t1->w;
                opj_t1_dec_sigpass_step_raw(
                    t1,
                    flagsp,
                    data2,
                    oneplushalf,
                    0, /* vsc */
                    3U);
                data2 += t1->w;
                flagsp ++;
            }
            data1 += t1->w << 2;
            flagsp += 2;
        }
        if (k < t1->h) {
            for (i = 0; i < t1->w; ++i) {
                OPJ_INT32* data2 = data1 + i;
                for (j = 0; j < t1->h - k; ++j) {
                    opj_t1_dec_sigpass_step_raw(
                        t1,
                        flagsp,
                        data2,
                        oneplushalf,
                        0, /* vsc */
                        j);
                    data2 += t1->w;
                }
                flagsp ++;
            }
        }
    }
}

#define opj_t1_dec_sigpass_mqc_internal(t1, bpno, w, h, flags_stride) \
{ \
        OPJ_INT32 one, half, oneplushalf; \
        OPJ_UINT32 i, j, k; \
        OPJ_INT32 *data1 = t1->data; \
        opj_flag_t *flagsp = &t1->flags[flags_stride + 1]; \
        one = 1 << bpno; \
        half = one >> 1; \
        oneplushalf = one | half; \
        for (k = 0; k < (h & ~3u); k += 4) { \
                for (i = 0; i < w; ++i) { \
                        OPJ_INT32 *data2 = data1 + i; \
                        if( *flagsp != 0 ) { \
                            opj_t1_dec_sigpass_step_mqc(t1, flagsp, data2, oneplushalf, 0U, flags_stride); \
                            data2 += w; \
                            opj_t1_dec_sigpass_step_mqc(t1, flagsp, data2, oneplushalf, 1U, flags_stride); \
                            data2 += w; \
                            opj_t1_dec_sigpass_step_mqc(t1, flagsp, data2, oneplushalf, 2U, flags_stride); \
                            data2 += w; \
                            opj_t1_dec_sigpass_step_mqc(t1, flagsp, data2, oneplushalf, 3U, flags_stride); \
                            data2 += w; \
                        } \
                        flagsp ++; \
                } \
                data1 += w << 2; \
                flagsp += 2; \
        } \
        for (i = 0; i < w; ++i) { \
                OPJ_INT32 *data2 = data1 + i; \
                for (j = k; j < h; ++j) { \
                        opj_t1_dec_sigpass_step_mqc(t1, flagsp, data2, oneplushalf, j - k, flags_stride); \
                        data2 += w; \
                } \
                flagsp ++; \
        } \
}

static void opj_t1_dec_sigpass_mqc_64x64(
    opj_t1_t *t1,
    OPJ_INT32 bpno)
{
    opj_t1_dec_sigpass_mqc_internal(t1, bpno, 64, 64, 66);
}

static void opj_t1_dec_sigpass_mqc_generic(
    opj_t1_t *t1,
    OPJ_INT32 bpno)
{
    opj_t1_dec_sigpass_mqc_internal(t1, bpno, t1->w, t1->h, t1->w + 2);
}

static void opj_t1_dec_sigpass_mqc_vsc(
    opj_t1_t *t1,
    OPJ_INT32 bpno)
{
    OPJ_INT32 one, half, oneplushalf, vsc;
    OPJ_UINT32 i, j, k;
    OPJ_INT32 *data1 = t1->data;
    opj_flag_t *flagsp = &T1_FLAGS(0, 0);
    one = 1 << bpno;
    half = one >> 1;
    oneplushalf = one | half;
    for (k = 0; k < (t1->h & ~3U); k += 4U) {
        for (i = 0; i < t1->w; ++i) {
            OPJ_INT32 *data2 = data1 + i;
            opj_t1_dec_sigpass_step_mqc_vsc(t1, flagsp, data2, oneplushalf, 0, 0U);
            data2 += t1->w;
            opj_t1_dec_sigpass_step_mqc_vsc(t1, flagsp, data2, oneplushalf, 0, 1U);
            data2 += t1->w;
            opj_t1_dec_sigpass_step_mqc_vsc(t1, flagsp, data2, oneplushalf, 0, 2U);
            data2 += t1->w;
            opj_t1_dec_sigpass_step_mqc_vsc(t1, flagsp, data2, oneplushalf, 1, 3U);
            data2 += t1->w;
            flagsp ++;
        }
        data1 += t1->w << 2;
        flagsp += 2;
    }
    for (i = 0; i < t1->w; ++i) {
        OPJ_INT32 *data2 = data1 + i;
        for (j = k; j < t1->h; ++j) {
            vsc = (j == t1->h - 1) ? 1 : 0;
            opj_t1_dec_sigpass_step_mqc_vsc(
                t1,
                flagsp,
                data2,
                oneplushalf,
                vsc,
                j - k);
            data2 += t1->w;
        }
        flagsp ++;
    }
}


/**
Encode refinement pass step
*/
static INLINE void opj_t1_enc_refpass_step(opj_t1_t *t1,
        opj_flag_t *flagsp,
        OPJ_INT32 *datap,
        OPJ_INT32 bpno,
        OPJ_INT32 one,
        OPJ_INT32 *nmsedec,
        OPJ_BYTE type,
        OPJ_UINT32 ci,
        OPJ_UINT32 vsc)
{
    OPJ_UINT32 v;

    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */

    OPJ_UINT32 const shift_flags =
        vsc ?
        ((*flagsp >> (ci * 3U)) & ~(T1_SIGMA_SW | T1_SIGMA_S | T1_SIGMA_SE | T1_CHI_S))
        :
        (*flagsp >> (ci * 3U));

    if ((shift_flags & (T1_SIGMA_THIS | T1_PI_THIS)) == T1_SIGMA_THIS) {
        OPJ_UINT32 ctxt = opj_t1_getctxno_mag(shift_flags);
        *nmsedec += opj_t1_getnmsedec_ref((OPJ_UINT32)opj_int_abs(*datap),
                                          (OPJ_UINT32)bpno);
        v = opj_int_abs(*datap) & one ? 1 : 0;
#ifdef DEBUG_ENC_REF
        fprintf(stderr, "  ctxt=%d\n", ctxt);
#endif
        opj_mqc_setcurctx(mqc, ctxt);
        if (type == T1_TYPE_RAW) {  /* BYPASS/LAZY MODE */
            opj_mqc_bypass_enc(mqc, v);
        } else {
            opj_mqc_encode(mqc, v);
        }
        *flagsp |= T1_MU_THIS << (ci * 3U);
    }
}


static INLINE void opj_t1_dec_refpass_step_raw(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 poshalf,
    OPJ_UINT32 ci)
{
    OPJ_UINT32 v;

    opj_raw_t *raw = &(t1->raw);       /* RAW component */

    if ((*flagsp & ((T1_SIGMA_THIS | T1_PI_THIS) << (ci * 3U))) ==
            (T1_SIGMA_THIS << (ci * 3U))) {
        v = opj_raw_decode(raw);
        *datap += (v ^ (*datap < 0)) ? poshalf : -poshalf;
        *flagsp |= T1_MU_THIS << (ci * 3U);
    }
}

static INLINE void opj_t1_dec_refpass_step_mqc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 poshalf,
    OPJ_UINT32 ci)
{
    OPJ_UINT32 v;

    opj_mqc_t *mqc = &(t1->mqc);       /* MQC component */

    if ((*flagsp & ((T1_SIGMA_THIS | T1_PI_THIS) << (ci * 3U))) ==
            (T1_SIGMA_THIS << (ci * 3U))) {
        OPJ_UINT32 ctxt = opj_t1_getctxno_mag(*flagsp >> (ci * 3U));
        opj_mqc_setcurctx(mqc, ctxt);
        v = opj_mqc_decode(mqc);
        *datap += (v ^ (*datap < 0)) ? poshalf : -poshalf;
        *flagsp |= T1_MU_THIS << (ci * 3U);
    }
}

static INLINE void opj_t1_dec_refpass_step_mqc_vsc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 poshalf,
    OPJ_INT32 vsc,
    OPJ_UINT32 ci)
{
    OPJ_UINT32 v;

    opj_mqc_t *mqc = &(t1->mqc);       /* MQC component */

    OPJ_UINT32 const shift_flags =
        vsc ?
        ((*flagsp >> (ci * 3U)) & ~(T1_SIGMA_SW | T1_SIGMA_S | T1_SIGMA_SE | T1_CHI_S))
        :
        (*flagsp >> (ci * 3U));


    if ((shift_flags & (T1_SIGMA_THIS | T1_PI_THIS)) == T1_SIGMA_THIS) {
        OPJ_UINT32 ctxt = opj_t1_getctxno_mag(shift_flags);
        opj_mqc_setcurctx(mqc, ctxt);
        v = opj_mqc_decode(mqc);
        *datap += (v ^ (*datap < 0)) ? poshalf : -poshalf;
        *flagsp |= T1_MU_THIS << (ci * 3U);
    }
}

static void opj_t1_enc_refpass(
    opj_t1_t *t1,
    OPJ_INT32 bpno,
    OPJ_INT32 *nmsedec,
    OPJ_BYTE type,
    OPJ_UINT32 cblksty)
{
    OPJ_UINT32 i, k;
    const OPJ_INT32 one = 1 << (bpno + T1_NMSEDEC_FRACBITS);
    opj_flag_t* f = &T1_FLAGS(0, 0);
    const OPJ_UINT32 extra = 2U;

    *nmsedec = 0;
#ifdef DEBUG_ENC_REF
    fprintf(stderr, "enc_refpass: bpno=%d\n", bpno);
#endif
    for (k = 0; k < (t1->h & ~3U); k += 4) {
#ifdef DEBUG_ENC_REF
        fprintf(stderr, " k=%d\n", k);
#endif
        for (i = 0; i < t1->w; ++i) {
#ifdef DEBUG_ENC_REF
            fprintf(stderr, " i=%d\n", i);
#endif
            if ((*f & (T1_SIGMA_4 | T1_SIGMA_7 | T1_SIGMA_10 | T1_SIGMA_13)) == 0) {
                /* none significant */
                f++;
                continue;
            }
            if ((*f & (T1_PI_0 | T1_PI_1 | T1_PI_2 | T1_PI_3)) ==
                    (T1_PI_0 | T1_PI_1 | T1_PI_2 | T1_PI_3)) {
                /* all processed by sigpass */
                f++;
                continue;
            }

            opj_t1_enc_refpass_step(
                t1,
                f,
                &t1->data[((k + 0) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                type,
                0, 0);
            opj_t1_enc_refpass_step(
                t1,
                f,
                &t1->data[((k + 1) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                type,
                1, 0);
            opj_t1_enc_refpass_step(
                t1,
                f,
                &t1->data[((k + 2) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                type,
                2, 0);
            opj_t1_enc_refpass_step(
                t1,
                f,
                &t1->data[((k + 3) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                type,
                3, cblksty & J2K_CCP_CBLKSTY_VSC);
            ++f;
        }
        f += extra;
    }

    if (k < t1->h) {
        OPJ_UINT32 j;
#ifdef DEBUG_ENC_REF
        fprintf(stderr, " k=%d\n", k);
#endif
        for (i = 0; i < t1->w; ++i) {
#ifdef DEBUG_ENC_REF
            fprintf(stderr, " i=%d\n", i);
#endif
            if ((*f & (T1_SIGMA_4 | T1_SIGMA_7 | T1_SIGMA_10 | T1_SIGMA_13)) == 0) {
                /* none significant */
                f++;
                continue;
            }
            for (j = k; j < t1->h; ++j) {
                opj_t1_enc_refpass_step(
                    t1,
                    f,
                    &t1->data[(j * t1->data_stride) + i],
                    bpno,
                    one,
                    nmsedec,
                    type,
                    j - k,
                    (j == t1->h - 1 && (cblksty & J2K_CCP_CBLKSTY_VSC) != 0));
            }
            ++f;
        }
    }
}


static void opj_t1_dec_refpass_raw(
    opj_t1_t *t1,
    OPJ_INT32 bpno)
{
    OPJ_INT32 one, poshalf;
    OPJ_UINT32 i, j, k;
    OPJ_INT32 *data1 = t1->data;
    opj_flag_t *flagsp = &T1_FLAGS(0, 0);
    one = 1 << bpno;
    poshalf = one >> 1;
    for (k = 0; k < (t1->h & ~3U); k += 4) {
        for (i = 0; i < t1->w; ++i) {
            OPJ_INT32 *data2 = data1 + i;
            opj_t1_dec_refpass_step_raw(
                t1,
                flagsp,
                data2,
                poshalf,
                0U);
            data2 += t1->w;
            opj_t1_dec_refpass_step_raw(
                t1,
                flagsp,
                data2,
                poshalf,
                1U);
            data2 += t1->w;
            opj_t1_dec_refpass_step_raw(
                t1,
                flagsp,
                data2,
                poshalf,
                2U);
            data2 += t1->w;
            opj_t1_dec_refpass_step_raw(
                t1,
                flagsp,
                data2,
                poshalf,
                3U);
            data2 += t1->w;
            flagsp ++;
        }
        data1 += t1->w << 2;
        flagsp += 2;
    }
    if (k < t1->h) {
        for (i = 0; i < t1->w; ++i) {
            OPJ_INT32 *data2 = data1 + i;
            for (j = k; j < t1->h; ++j) {
                opj_t1_dec_refpass_step_raw(
                    t1,
                    flagsp,
                    data2,
                    poshalf,
                    j - k);
                data2 += t1->w;
            }
            flagsp ++;
        }
    }
}

#define opj_t1_dec_refpass_mqc_internal(t1, bpno, w, h, flags_stride) \
{ \
        OPJ_INT32 one, poshalf; \
        OPJ_UINT32 i, j, k; \
        OPJ_INT32 *data1 = t1->data; \
        opj_flag_t *flagsp = &t1->flags[flags_stride + 1]; \
        one = 1 << bpno; \
        poshalf = one >> 1; \
        for (k = 0; k < (h & ~3u); k += 4) { \
                for (i = 0; i < w; ++i) { \
                        OPJ_INT32 *data2 = data1 + i; \
                        if( *flagsp != 0 ) { \
                            opj_t1_dec_refpass_step_mqc(t1, flagsp, data2, poshalf, 0U); \
                            data2 += w; \
                            opj_t1_dec_refpass_step_mqc(t1, flagsp, data2, poshalf, 1U); \
                            data2 += w; \
                            opj_t1_dec_refpass_step_mqc(t1, flagsp, data2, poshalf, 2U); \
                            data2 += w; \
                            opj_t1_dec_refpass_step_mqc(t1, flagsp, data2, poshalf, 3U); \
                            data2 += w; \
                        } \
                        flagsp ++; \
                } \
                data1 += w << 2; \
                flagsp += 2; \
        } \
        for (i = 0; i < w; ++i) { \
                OPJ_INT32 *data2 = data1 + i; \
                for (j = k; j < h; ++j) { \
                        opj_t1_dec_refpass_step_mqc(t1, flagsp, data2, poshalf, j - k); \
                        data2 += w; \
                } \
                flagsp ++; \
        } \
}

static void opj_t1_dec_refpass_mqc_64x64(
    opj_t1_t *t1,
    OPJ_INT32 bpno)
{
    opj_t1_dec_refpass_mqc_internal(t1, bpno, 64, 64, 66);
}

static void opj_t1_dec_refpass_mqc_generic(
    opj_t1_t *t1,
    OPJ_INT32 bpno)
{
    opj_t1_dec_refpass_mqc_internal(t1, bpno, t1->w, t1->h, t1->w + 2U);
}

static void opj_t1_dec_refpass_mqc_vsc(
    opj_t1_t *t1,
    OPJ_INT32 bpno)
{
    OPJ_INT32 one, poshalf;
    OPJ_UINT32 i, j, k;
    OPJ_INT32 vsc;
    OPJ_INT32 *data1 = t1->data;
    opj_flag_t *flagsp = &T1_FLAGS(0, 0);
    one = 1 << bpno;
    poshalf = one >> 1;
    for (k = 0; k < (t1->h & ~(OPJ_UINT32)3U); k += 4U) {
        for (i = 0; i < t1->w; ++i) {
            OPJ_INT32 *data2 = data1 + i;
            opj_t1_dec_refpass_step_mqc_vsc(t1, flagsp, data2, poshalf,
                                            0, 0U);
            data2 += t1->w;
            opj_t1_dec_refpass_step_mqc_vsc(t1, flagsp, data2, poshalf,
                                            0, 1U);
            data2 += t1->w;
            opj_t1_dec_refpass_step_mqc_vsc(t1, flagsp, data2, poshalf,
                                            0, 2U);
            data2 += t1->w;
            opj_t1_dec_refpass_step_mqc_vsc(t1, flagsp, data2, poshalf,
                                            1, 3U);
            data2 += t1->w;
            flagsp ++;
        }
        data1 += t1->w << 2;
        flagsp += 2U;
    }
    for (i = 0; i < t1->w; ++i) {
        OPJ_INT32 *data2 = data1 + i;
        for (j = k; j < t1->h; ++j) {
            vsc = (j == t1->h - 1) ? 1 : 0;
            opj_t1_dec_refpass_step_mqc_vsc(
                t1,
                flagsp,
                data2,
                poshalf,
                vsc,
                j - k);
            data2 += t1->w;
        }
        flagsp ++;
    }
}

/**
Encode clean-up pass step
*/
static void opj_t1_enc_clnpass_step(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 bpno,
    OPJ_INT32 one,
    OPJ_INT32 *nmsedec,
    OPJ_UINT32 agg,
    OPJ_UINT32 runlen,
    OPJ_UINT32 lim,
    OPJ_UINT32 cblksty)
{
    OPJ_UINT32 v;
    OPJ_UINT32 ci;
    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */

    const OPJ_UINT32 check = (T1_SIGMA_4 | T1_SIGMA_7 | T1_SIGMA_10 | T1_SIGMA_13 |
                              T1_PI_0 | T1_PI_1 | T1_PI_2 | T1_PI_3);

    if ((*flagsp & check) == check) {
        if (runlen == 0) {
            *flagsp &= ~(T1_PI_0 | T1_PI_1 | T1_PI_2 | T1_PI_3);
        } else if (runlen == 1) {
            *flagsp &= ~(T1_PI_1 | T1_PI_2 | T1_PI_3);
        } else if (runlen == 2) {
            *flagsp &= ~(T1_PI_2 | T1_PI_3);
        } else if (runlen == 3) {
            *flagsp &= ~(T1_PI_3);
        }
        return;
    }

    for (ci = runlen; ci < lim; ++ci) {
        OPJ_UINT32 vsc;
        OPJ_UINT32 vsc_mask;
        opj_flag_t flags;

        vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (ci == lim - 1)) ? 1 : 0;
        vsc_mask = vsc ? ~((T1_SIGMA_SW | T1_SIGMA_S | T1_SIGMA_SE | T1_CHI_S) <<
                           (ci * 3U)) : ~0U;
        flags = *flagsp & vsc_mask;

        if ((agg != 0) && (ci == runlen)) {
            goto LABEL_PARTIAL;
        }

        if (!(flags & ((T1_SIGMA_THIS | T1_PI_THIS) << (ci * 3U)))) {
            OPJ_UINT32 ctxt1 = opj_t1_getctxno_zc(mqc, flags >> (ci * 3U));
#ifdef DEBUG_ENC_CLN
            printf("   ctxt1=%d\n", ctxt1);
#endif
            opj_mqc_setcurctx(mqc, ctxt1);
            v = opj_int_abs(*datap) & one ? 1 : 0;
            opj_mqc_encode(mqc, v);
            if (v) {
                OPJ_UINT32 ctxt2, spb;
                OPJ_UINT32 lu;
LABEL_PARTIAL:
                /* Note: using flags instead of *flagsp & vsc_mask result */
                /* in slow down. Probably because of register pressure */
                lu = opj_t1_getctxtno_sc_or_spb_index(
                         *flagsp & vsc_mask,
                         flagsp[-1] & vsc_mask, flagsp[1] & vsc_mask,
                         ci);
                *nmsedec += opj_t1_getnmsedec_sig((OPJ_UINT32)opj_int_abs(*datap),
                                                  (OPJ_UINT32)bpno);
                ctxt2 = opj_t1_getctxno_sc(lu);
#ifdef DEBUG_ENC_CLN
                printf("   ctxt2=%d\n", ctxt2);
#endif
                opj_mqc_setcurctx(mqc, ctxt2);

                v = *datap < 0 ? 1 : 0;
                spb = opj_t1_getspb(lu);
#ifdef DEBUG_ENC_CLN
                printf("   spb=%d\n", spb);
#endif
                opj_mqc_encode(mqc, v ^ spb);
                opj_t1_update_flags(flagsp, ci, v, t1->w + 2U);
            }
        }
        *flagsp &= ~(T1_PI_THIS << (3U * ci));
        datap += t1->data_stride;
    }
}


static void opj_t1_dec_clnpass_step_partial(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_UINT32 ci)
{
    OPJ_UINT32 v;
    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */

    OPJ_UINT32 lu = opj_t1_getctxtno_sc_or_spb_index(
                        *flagsp, flagsp[-1], flagsp[1],
                        ci);
    opj_mqc_setcurctx(mqc, opj_t1_getctxno_sc(lu));
    v = opj_mqc_decode(mqc) ^ opj_t1_getspb(lu);
    *datap = v ? -oneplushalf : oneplushalf;
    opj_t1_update_flags(flagsp, ci, v, t1->w + 2U);
    /* *flagsp &= ~(T1_PI_THIS << (3U * ci)); */
}

static void opj_t1_dec_clnpass_step(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_UINT32 ci)
{
    OPJ_UINT32 v;

    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */
    if (!(*flagsp & ((T1_SIGMA_THIS | T1_PI_THIS) << (ci * 3U)))) {
        OPJ_UINT32 ctxt1 = opj_t1_getctxno_zc(mqc, *flagsp >> (ci * 3U));
        opj_mqc_setcurctx(mqc, ctxt1);
        if (opj_mqc_decode(mqc)) {
            OPJ_UINT32 lu = opj_t1_getctxtno_sc_or_spb_index(
                                *flagsp, flagsp[-1], flagsp[1],
                                ci);
            opj_mqc_setcurctx(mqc, opj_t1_getctxno_sc(lu));
            v = opj_mqc_decode(mqc) ^ opj_t1_getspb(lu);
            *datap = v ? -oneplushalf : oneplushalf;
            opj_t1_update_flags(flagsp, ci, v, t1->w + 2U);
        }
    }
    /* *flagsp &= ~(T1_PI_THIS << (3U * ci)); */
}

static void opj_t1_dec_clnpass_step_only_if_flag_not_sig_visit(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_UINT32 ci,
    OPJ_UINT32 flags_stride)
{
    OPJ_UINT32 v;

    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */

    OPJ_UINT32 ctxt1 = opj_t1_getctxno_zc(mqc, *flagsp >> (ci * 3U));
    opj_mqc_setcurctx(mqc, ctxt1);
    if (opj_mqc_decode(mqc)) {
        OPJ_UINT32 lu = opj_t1_getctxtno_sc_or_spb_index(
                            *flagsp, flagsp[-1], flagsp[1],
                            ci);
        opj_mqc_setcurctx(mqc, opj_t1_getctxno_sc(lu));
        v = opj_mqc_decode(mqc) ^ opj_t1_getspb(lu);
        *datap = v ? -oneplushalf : oneplushalf;
        opj_t1_update_flags(flagsp, ci, v, flags_stride);
    }
}

static void opj_t1_dec_clnpass_step_vsc(
    opj_t1_t *t1,
    opj_flag_t *flagsp,
    OPJ_INT32 *datap,
    OPJ_INT32 oneplushalf,
    OPJ_INT32 partial,
    OPJ_INT32 vsc,
    OPJ_UINT32 ci)
{
    OPJ_UINT32 v;

    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */

    OPJ_UINT32 vsc_mask = vsc ? ~((T1_SIGMA_SW | T1_SIGMA_S | T1_SIGMA_SE |
                                   T1_CHI_S) <<
                                  (ci * 3U)) : ~0U;
    OPJ_UINT32 flags = *flagsp & vsc_mask;
    if (partial) {
        goto LABEL_PARTIAL;
    }
    if (!(flags & ((T1_SIGMA_THIS | T1_PI_THIS) << (ci * 3U)))) {
        OPJ_UINT32 ctxt1 = opj_t1_getctxno_zc(mqc, flags >> (ci * 3U));
        opj_mqc_setcurctx(mqc, ctxt1);
        if (opj_mqc_decode(mqc)) {
            OPJ_UINT32 lu;
LABEL_PARTIAL:
            lu = opj_t1_getctxtno_sc_or_spb_index(
                     *flagsp & vsc_mask, flagsp[-1] & vsc_mask,
                     flagsp[1] & vsc_mask,
                     ci);
            opj_mqc_setcurctx(mqc, opj_t1_getctxno_sc(lu));
            v = opj_mqc_decode(mqc) ^ opj_t1_getspb(lu);
            *datap = v ? -oneplushalf : oneplushalf;
            opj_t1_update_flags(flagsp, ci, v, t1->w + 2U);
        }
    }
    /* *flagsp &= ~(T1_PI_THIS << (3U * ci)); */
}

static void opj_t1_enc_clnpass(
    opj_t1_t *t1,
    OPJ_INT32 bpno,
    OPJ_INT32 *nmsedec,
    OPJ_UINT32 cblksty)
{
    OPJ_UINT32 i, k;
    const OPJ_INT32 one = 1 << (bpno + T1_NMSEDEC_FRACBITS);
    OPJ_UINT32 agg, runlen;

    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */

    const OPJ_UINT32 agg_mask = (cblksty & J2K_CCP_CBLKSTY_VSC) ?
                                ~(T1_SIGMA_15 | T1_SIGMA_16 | T1_SIGMA_17 | T1_CHI_5) :
                                ~0U;

    *nmsedec = 0;
#ifdef DEBUG_ENC_CLN
    printf("enc_clnpass: bpno=%d\n", bpno);
#endif
    for (k = 0; k < (t1->h & ~3U); k += 4) {
#ifdef DEBUG_ENC_CLN
        printf(" k=%d\n", k);
#endif
        for (i = 0; i < t1->w; ++i) {
#ifdef DEBUG_ENC_CLN
            printf("  i=%d\n", i);
#endif
            agg = !(T1_FLAGS(i, k) & agg_mask);
#ifdef DEBUG_ENC_CLN
            printf("   agg=%d\n", agg);
#endif
            if (agg) {
                for (runlen = 0; runlen < 4; ++runlen) {
                    if (opj_int_abs(t1->data[((k + runlen)*t1->data_stride) + i]) & one) {
                        break;
                    }
                }
                opj_mqc_setcurctx(mqc, T1_CTXNO_AGG);
                opj_mqc_encode(mqc, runlen != 4);
                if (runlen == 4) {
                    continue;
                }
                opj_mqc_setcurctx(mqc, T1_CTXNO_UNI);
                opj_mqc_encode(mqc, runlen >> 1);
                opj_mqc_encode(mqc, runlen & 1);
            } else {
                runlen = 0;
            }
            opj_t1_enc_clnpass_step(
                t1,
                &T1_FLAGS(i, k),
                &t1->data[((k + runlen) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                agg,
                runlen,
                4U,
                cblksty);
        }
    }
    if (k < t1->h) {
        agg = 0;
        runlen = 0;
#ifdef DEBUG_ENC_CLN
        printf(" k=%d\n", k);
#endif
        for (i = 0; i < t1->w; ++i) {
#ifdef DEBUG_ENC_CLN
            printf("  i=%d\n", i);
            printf("   agg=%d\n", agg);
#endif
            opj_t1_enc_clnpass_step(
                t1,
                &T1_FLAGS(i, k),
                &t1->data[((k + runlen) * t1->data_stride) + i],
                bpno,
                one,
                nmsedec,
                agg,
                runlen,
                t1->h - k,
                cblksty);
        }
    }
}

#define opj_t1_dec_clnpass_internal(t1, bpno, cblksty, w, h, flags_stride) \
{ \
    OPJ_INT32 one, half, oneplushalf, agg, vsc; \
    OPJ_UINT32 runlen; \
    OPJ_UINT32 i, j, k; \
    OPJ_INT32 segsym = cblksty & J2K_CCP_CBLKSTY_SEGSYM; \
     \
    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */ \
     \
    one = 1 << bpno; \
    half = one >> 1; \
    oneplushalf = one | half; \
    if (cblksty & J2K_CCP_CBLKSTY_VSC) { \
    opj_flag_t *flagsp1 = &t1->flags[flags_stride + 1]; \
    const OPJ_UINT32 agg_mask =  ~(T1_SIGMA_15 | T1_SIGMA_16 | T1_SIGMA_17 | T1_CHI_5); \
    for (k = 0; k < h; k += 4) { \
        for (i = 0; i < w; ++i) { \
            opj_flag_t *flagsp2 = flagsp1 + i; \
            if (k + 3 < h) { \
                    agg = !(*flagsp2 & agg_mask); \
                } else { \
                agg = 0; \
            } \
            if (agg) { \
                opj_mqc_setcurctx(mqc, T1_CTXNO_AGG); \
                if (!opj_mqc_decode(mqc)) { \
                    continue; \
                } \
                opj_mqc_setcurctx(mqc, T1_CTXNO_UNI); \
                runlen = opj_mqc_decode(mqc); \
                runlen = (runlen << 1) | opj_mqc_decode(mqc); \
            } else { \
                runlen = 0; \
            } \
            for (j = k + runlen; j < k + 4 && j < h; ++j) { \
                    vsc = (j == k + 3 || j == h - 1) ? 1 : 0; \
                    opj_t1_dec_clnpass_step_vsc( \
                        t1, \
                        flagsp2, \
                        &t1->data[(j * w) + i], \
                        oneplushalf, \
                        agg && (j == k + runlen), \
                        vsc, j - k); \
            } \
            *flagsp2 &= ~(T1_PI_0 | T1_PI_1 | T1_PI_2 | T1_PI_3); \
        } \
        flagsp1 += flags_stride; \
    } \
    } else { \
        OPJ_INT32 *data1 = t1->data; \
        opj_flag_t *flagsp1 = &t1->flags[flags_stride + 1]; \
        for (k = 0; k < (h & ~3u); k += 4) { \
            for (i = 0; i < w; ++i) { \
                OPJ_INT32 *data2 = data1 + i; \
                opj_flag_t *flagsp2 = flagsp1 + i; \
                if (*flagsp2 == 0) { \
                    opj_mqc_setcurctx(mqc, T1_CTXNO_AGG); \
                    if (!opj_mqc_decode(mqc)) { \
                        continue; \
                    } \
                    opj_mqc_setcurctx(mqc, T1_CTXNO_UNI); \
                    runlen = opj_mqc_decode(mqc); \
                    runlen = (runlen << 1) | opj_mqc_decode(mqc); \
                    data2 += runlen * w; \
                    for (j = runlen; j < 4; ++j) { \
                        if (j == runlen) { \
                            opj_t1_dec_clnpass_step_partial(t1, flagsp2, data2, oneplushalf, j); \
                        } else { \
                            opj_t1_dec_clnpass_step(t1, flagsp2, data2, oneplushalf, j); \
                        } \
                        data2 += w; \
                    } \
                } else { \
                    if (!(*flagsp2 & ((T1_SIGMA_THIS | T1_PI_THIS) << (0 * 3U)))) {\
                        opj_t1_dec_clnpass_step_only_if_flag_not_sig_visit(t1, flagsp2, data2, oneplushalf, 0U, flags_stride); \
                    } \
                    data2 += w; \
                    if (!(*flagsp2 & ((T1_SIGMA_THIS | T1_PI_THIS) << (1 * 3U)))) {\
                        opj_t1_dec_clnpass_step_only_if_flag_not_sig_visit(t1, flagsp2, data2, oneplushalf, 1U, flags_stride); \
                    } \
                    data2 += w; \
                    if (!(*flagsp2 & ((T1_SIGMA_THIS | T1_PI_THIS) << (2 * 3U)))) {\
                        opj_t1_dec_clnpass_step_only_if_flag_not_sig_visit(t1, flagsp2, data2, oneplushalf, 2U, flags_stride); \
                    } \
                    data2 += w; \
                    if (!(*flagsp2 & ((T1_SIGMA_THIS | T1_PI_THIS) << (3 * 3U)))) {\
                        opj_t1_dec_clnpass_step_only_if_flag_not_sig_visit(t1, flagsp2, data2, oneplushalf, 3U, flags_stride); \
                    } \
                    data2 += w; \
                } \
                *flagsp2 &= ~(T1_PI_0 | T1_PI_1 | T1_PI_2 | T1_PI_3); \
            } \
            data1 += w << 2; \
            flagsp1 += flags_stride; \
        } \
        for (i = 0; i < w; ++i) { \
            OPJ_INT32 *data2 = data1 + i; \
            opj_flag_t *flagsp2 = flagsp1 + i; \
            for (j = k; j < h; ++j) { \
                opj_t1_dec_clnpass_step(t1, flagsp2, data2, oneplushalf, j - k); \
                data2 += w; \
            } \
            *flagsp2 &= ~(T1_PI_0 | T1_PI_1 | T1_PI_2 | T1_PI_3); \
        } \
    } \
 \
    if (segsym) { \
        OPJ_UINT32 v = 0; \
        opj_mqc_setcurctx(mqc, T1_CTXNO_UNI); \
        v = opj_mqc_decode(mqc); \
        v = (v << 1) | opj_mqc_decode(mqc); \
        v = (v << 1) | opj_mqc_decode(mqc); \
        v = (v << 1) | opj_mqc_decode(mqc); \
        /* \
        if (v!=0xa) { \
            opj_event_msg(t1->cinfo, EVT_WARNING, "Bad segmentation symbol %x\n", v); \
        } \
        */ \
    } \
}

static void opj_t1_dec_clnpass_64x64(
    opj_t1_t *t1,
    OPJ_INT32 bpno,
    OPJ_INT32 cblksty)
{
    opj_t1_dec_clnpass_internal(t1, bpno, cblksty, 64, 64, 66);
}

static void opj_t1_dec_clnpass_generic(
    opj_t1_t *t1,
    OPJ_INT32 bpno,
    OPJ_INT32 cblksty)
{
    opj_t1_dec_clnpass_internal(t1, bpno, cblksty, t1->w, t1->h,
                                t1->w + 2U);
}


/** mod fixed_quality */
static OPJ_FLOAT64 opj_t1_getwmsedec(
    OPJ_INT32 nmsedec,
    OPJ_UINT32 compno,
    OPJ_UINT32 level,
    OPJ_UINT32 orient,
    OPJ_INT32 bpno,
    OPJ_UINT32 qmfbid,
    OPJ_FLOAT64 stepsize,
    OPJ_UINT32 numcomps,
    const OPJ_FLOAT64 * mct_norms,
    OPJ_UINT32 mct_numcomps)
{
    OPJ_FLOAT64 w1 = 1, w2, wmsedec;
    OPJ_ARG_NOT_USED(numcomps);

    if (mct_norms && (compno < mct_numcomps)) {
        w1 = mct_norms[compno];
    }

    if (qmfbid == 1) {
        w2 = opj_dwt_getnorm(level, orient);
    } else {    /* if (qmfbid == 0) */
        w2 = opj_dwt_getnorm_real(level, orient);
    }

    wmsedec = w1 * w2 * stepsize * (1 << bpno);
    wmsedec *= wmsedec * nmsedec / 8192.0;

    return wmsedec;
}

static OPJ_BOOL opj_t1_allocate_buffers(
    opj_t1_t *t1,
    OPJ_UINT32 w,
    OPJ_UINT32 h)
{
    size_t flagssize;
    OPJ_UINT32 flags_stride;

    /* encoder uses tile buffer, so no need to allocate */
    if (!t1->encoder) {
        size_t datasize;

#if (SIZE_MAX / 0xFFFFFFFFU) < 0xFFFFFFFFU /* UINT32_MAX */
        /* Overflow check */
        if ((w > 0U) && ((size_t)h > (SIZE_MAX / (size_t)w))) {
            /* FIXME event manager error callback */
            return OPJ_FALSE;
        }
#endif
        datasize = (size_t)w * h;

        /* Overflow check */
        if (datasize > (SIZE_MAX / sizeof(OPJ_INT32))) {
            /* FIXME event manager error callback */
            return OPJ_FALSE;
        }

        if (datasize > (size_t)t1->datasize) {
            opj_aligned_free(t1->data);
            t1->data = (OPJ_INT32*) opj_aligned_malloc(datasize * sizeof(OPJ_INT32));
            if (!t1->data) {
                /* FIXME event manager error callback */
                return OPJ_FALSE;
            }
#if SIZE_MAX > 0xFFFFFFFFU /* UINT32_MAX */
            /* TODO remove this if t1->datasize type changes to size_t */
            /* Overflow check */
            if (datasize > (size_t)0xFFFFFFFFU /* UINT32_MAX */) {
                /* FIXME event manager error callback */
                return OPJ_FALSE;
            }
#endif
            t1->datasize = (OPJ_UINT32)datasize;
        }
        /* memset first arg is declared to never be null by gcc */
        if (t1->data != NULL) {
            memset(t1->data, 0, datasize * sizeof(OPJ_INT32));
        }
    }

    /* Overflow check */
    if (w > (0xFFFFFFFFU /* UINT32_MAX */ - 2U)) {
        /* FIXME event manager error callback */
        return OPJ_FALSE;
    }
    flags_stride = w + 2U; /* can't be 0U */

#if (SIZE_MAX - 3U) < 0xFFFFFFFFU /* UINT32_MAX */
    /* Overflow check */
    if (h > (0xFFFFFFFFU /* UINT32_MAX */ - 3U)) {
        /* FIXME event manager error callback */
        return OPJ_FALSE;
    }
#endif
    flagssize = (h + 3U) / 4U + 2U;

    /* Overflow check */
    if (flagssize > (SIZE_MAX / (size_t)flags_stride)) {
        /* FIXME event manager error callback */
        return OPJ_FALSE;
    }
    flagssize *= (size_t)flags_stride;
    {
        /* BIG FAT XXX */
        opj_flag_t* p;
        OPJ_UINT32 x;
        OPJ_UINT32 flags_height = (h + 3U) / 4U;

        if (flagssize > (size_t)t1->flagssize) {
            /* Overflow check */
            if (flagssize > (SIZE_MAX / sizeof(opj_flag_t))) {
                /* FIXME event manager error callback */
                return OPJ_FALSE;
            }
            opj_aligned_free(t1->flags);
            t1->flags = (opj_flag_t*) opj_aligned_malloc(flagssize * sizeof(
                            opj_flag_t));
            if (!t1->flags) {
                /* FIXME event manager error callback */
                return OPJ_FALSE;
            }
#if SIZE_MAX > 0xFFFFFFFFU /* UINT32_MAX */
            /* TODO remove this if t1->flagssize type changes to size_t */
            /* Overflow check */
            if (flagssize > (size_t)0xFFFFFFFFU /* UINT32_MAX */) {
                /* FIXME event manager error callback */
                return OPJ_FALSE;
            }
#endif
        }
        t1->flagssize = (OPJ_UINT32)flagssize;

        memset(t1->flags, 0, flagssize * sizeof(opj_flag_t));

        p = &t1->flags[0];
        for (x = 0; x < flags_stride; ++x) {
            /* magic value to hopefully stop any passes being interested in this entry */
            *p++ = (T1_PI_0 | T1_PI_1 | T1_PI_2 | T1_PI_3);
        }

        p = &t1->flags[((flags_height + 1) * flags_stride)];
        for (x = 0; x < flags_stride; ++x) {
            /* magic value to hopefully stop any passes being interested in this entry */
            *p++ = (T1_PI_0 | T1_PI_1 | T1_PI_2 | T1_PI_3);
        }

        if (h % 4) {
            OPJ_UINT32 v = 0;
            p = &t1->flags[((flags_height) * flags_stride)];
            if (h % 4 == 1) {
                v |= T1_PI_1 | T1_PI_2 | T1_PI_3;
            } else if (h % 4 == 2) {
                v |= T1_PI_2 | T1_PI_3;
            } else if (h % 4 == 3) {
                v |= T1_PI_3;
            }
            for (x = 0; x < flags_stride; ++x) {
                *p++ = v;
            }
        }
    }

    t1->w = w;
    t1->h = h;

    return OPJ_TRUE;
}

/* ----------------------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
/**
 * Creates a new Tier 1 handle
 * and initializes the look-up tables of the Tier-1 coder/decoder
 * @return a new T1 handle if successful, returns NULL otherwise
*/
opj_t1_t* opj_t1_create(OPJ_BOOL isEncoder)
{
    opj_t1_t *l_t1 = 00;

    l_t1 = (opj_t1_t*) opj_calloc(1, sizeof(opj_t1_t));
    if (!l_t1) {
        return 00;
    }

    l_t1->encoder = isEncoder;

    return l_t1;
}


/**
 * Destroys a previously created T1 handle
 *
 * @param p_t1 Tier 1 handle to destroy
*/
void opj_t1_destroy(opj_t1_t *p_t1)
{
    if (! p_t1) {
        return;
    }

    /* encoder uses tile buffer, so no need to free */
    if (!p_t1->encoder && p_t1->data) {
        opj_aligned_free(p_t1->data);
        p_t1->data = 00;
    }

    if (p_t1->flags) {
        opj_aligned_free(p_t1->flags);
        p_t1->flags = 00;
    }

    opj_free(p_t1);
}

typedef struct {
    OPJ_UINT32 resno;
    opj_tcd_cblk_dec_t* cblk;
    opj_tcd_band_t* band;
    opj_tcd_tilecomp_t* tilec;
    opj_tccp_t* tccp;
    volatile OPJ_BOOL* pret;
} opj_t1_cblk_decode_processing_job_t;

static void opj_t1_destroy_wrapper(void* t1)
{
    opj_t1_destroy((opj_t1_t*) t1);
}

static void opj_t1_clbl_decode_processor(void* user_data, opj_tls_t* tls)
{
    opj_tcd_cblk_dec_t* cblk;
    opj_tcd_band_t* band;
    opj_tcd_tilecomp_t* tilec;
    opj_tccp_t* tccp;
    OPJ_INT32* OPJ_RESTRICT datap;
    OPJ_UINT32 cblk_w, cblk_h;
    OPJ_INT32 x, y;
    OPJ_UINT32 i, j;
    opj_t1_cblk_decode_processing_job_t* job;
    opj_t1_t* t1;
    OPJ_UINT32 resno;
    OPJ_UINT32 tile_w;

    job = (opj_t1_cblk_decode_processing_job_t*) user_data;
    resno = job->resno;
    cblk = job->cblk;
    band = job->band;
    tilec = job->tilec;
    tccp = job->tccp;
    tile_w = (OPJ_UINT32)(tilec->x1 - tilec->x0);

    if (!*(job->pret)) {
        opj_free(job);
        return;
    }

    t1 = (opj_t1_t*) opj_tls_get(tls, OPJ_TLS_KEY_T1);
    if (t1 == NULL) {
        t1 = opj_t1_create(OPJ_FALSE);
        opj_tls_set(tls, OPJ_TLS_KEY_T1, t1, opj_t1_destroy_wrapper);
    }

    if (OPJ_FALSE == opj_t1_decode_cblk(
                t1,
                cblk,
                band->bandno,
                (OPJ_UINT32)tccp->roishift,
                tccp->cblksty)) {
        *(job->pret) = OPJ_FALSE;
        opj_free(job);
        return;
    }

    x = cblk->x0 - band->x0;
    y = cblk->y0 - band->y0;
    if (band->bandno & 1) {
        opj_tcd_resolution_t* pres = &tilec->resolutions[resno - 1];
        x += pres->x1 - pres->x0;
    }
    if (band->bandno & 2) {
        opj_tcd_resolution_t* pres = &tilec->resolutions[resno - 1];
        y += pres->y1 - pres->y0;
    }

    datap = t1->data;
    cblk_w = t1->w;
    cblk_h = t1->h;

    if (tccp->roishift) {
        OPJ_INT32 thresh = 1 << tccp->roishift;
        for (j = 0; j < cblk_h; ++j) {
            for (i = 0; i < cblk_w; ++i) {
                OPJ_INT32 val = datap[(j * cblk_w) + i];
                OPJ_INT32 mag = abs(val);
                if (mag >= thresh) {
                    mag >>= tccp->roishift;
                    datap[(j * cblk_w) + i] = val < 0 ? -mag : mag;
                }
            }
        }
    }
    if (tccp->qmfbid == 1) {
        OPJ_INT32* OPJ_RESTRICT tiledp = &tilec->data[(OPJ_UINT32)y * tile_w +
                                                       (OPJ_UINT32)x];
        for (j = 0; j < cblk_h; ++j) {
            i = 0;
            for (; i < (cblk_w & ~(OPJ_UINT32)3U); i += 4U) {
                OPJ_INT32 tmp0 = datap[(j * cblk_w) + i + 0U];
                OPJ_INT32 tmp1 = datap[(j * cblk_w) + i + 1U];
                OPJ_INT32 tmp2 = datap[(j * cblk_w) + i + 2U];
                OPJ_INT32 tmp3 = datap[(j * cblk_w) + i + 3U];
                ((OPJ_INT32*)tiledp)[(j * tile_w) + i + 0U] = tmp0 / 2;
                ((OPJ_INT32*)tiledp)[(j * tile_w) + i + 1U] = tmp1 / 2;
                ((OPJ_INT32*)tiledp)[(j * tile_w) + i + 2U] = tmp2 / 2;
                ((OPJ_INT32*)tiledp)[(j * tile_w) + i + 3U] = tmp3 / 2;
            }
            for (; i < cblk_w; ++i) {
                OPJ_INT32 tmp = datap[(j * cblk_w) + i];
                ((OPJ_INT32*)tiledp)[(j * tile_w) + i] = tmp / 2;
            }
        }
    } else {        /* if (tccp->qmfbid == 0) */
        OPJ_FLOAT32* OPJ_RESTRICT tiledp = (OPJ_FLOAT32*) &tilec->data[(OPJ_UINT32)y *
                                                         tile_w + (OPJ_UINT32)x];
        for (j = 0; j < cblk_h; ++j) {
            OPJ_FLOAT32* OPJ_RESTRICT tiledp2 = tiledp;
            for (i = 0; i < cblk_w; ++i) {
                OPJ_FLOAT32 tmp = (OPJ_FLOAT32) * datap * band->stepsize;
                *tiledp2 = tmp;
                datap++;
                tiledp2++;
            }
            tiledp += tile_w;
        }
    }

    opj_free(job);
}


void opj_t1_decode_cblks(opj_thread_pool_t* tp,
                         volatile OPJ_BOOL* pret,
                         opj_tcd_tilecomp_t* tilec,
                         opj_tccp_t* tccp
                        )
{
    OPJ_UINT32 resno, bandno, precno, cblkno;

    for (resno = 0; resno < tilec->minimum_num_resolutions; ++resno) {
        opj_tcd_resolution_t* res = &tilec->resolutions[resno];

        for (bandno = 0; bandno < res->numbands; ++bandno) {
            opj_tcd_band_t* OPJ_RESTRICT band = &res->bands[bandno];

            for (precno = 0; precno < res->pw * res->ph; ++precno) {
                opj_tcd_precinct_t* precinct = &band->precincts[precno];

                for (cblkno = 0; cblkno < precinct->cw * precinct->ch; ++cblkno) {
                    opj_tcd_cblk_dec_t* cblk = &precinct->cblks.dec[cblkno];
                    opj_t1_cblk_decode_processing_job_t* job;

                    job = (opj_t1_cblk_decode_processing_job_t*) opj_calloc(1,
                            sizeof(opj_t1_cblk_decode_processing_job_t));
                    if (!job) {
                        *pret = OPJ_FALSE;
                        return;
                    }
                    job->resno = resno;
                    job->cblk = cblk;
                    job->band = band;
                    job->tilec = tilec;
                    job->tccp = tccp;
                    job->pret = pret;
                    opj_thread_pool_submit_job(tp, opj_t1_clbl_decode_processor, job);
                    if (!(*pret)) {
                        return;
                    }
                } /* cblkno */
            } /* precno */
        } /* bandno */
    } /* resno */

    return;
}


static OPJ_BOOL opj_t1_decode_cblk(opj_t1_t *t1,
                                   opj_tcd_cblk_dec_t* cblk,
                                   OPJ_UINT32 orient,
                                   OPJ_UINT32 roishift,
                                   OPJ_UINT32 cblksty)
{
    opj_raw_t *raw = &(t1->raw);   /* RAW component */
    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */

    OPJ_INT32 bpno_plus_one;
    OPJ_UINT32 passtype;
    OPJ_UINT32 segno, passno;
    OPJ_BYTE type = T1_TYPE_MQ; /* BYPASS mode */

    mqc->lut_ctxno_zc_orient = lut_ctxno_zc + (orient << 9);

    if (!opj_t1_allocate_buffers(
                t1,
                (OPJ_UINT32)(cblk->x1 - cblk->x0),
                (OPJ_UINT32)(cblk->y1 - cblk->y0))) {
        return OPJ_FALSE;
    }

    bpno_plus_one = (OPJ_INT32)(roishift + cblk->numbps);
    passtype = 2;

    opj_mqc_resetstates(mqc);
    opj_mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);
    opj_mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
    opj_mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);

    for (segno = 0; segno < cblk->real_num_segs; ++segno) {
        opj_tcd_seg_t *seg = &cblk->segs[segno];

        /* BYPASS mode */
        type = ((bpno_plus_one <= ((OPJ_INT32)(cblk->numbps)) - 4) && (passtype < 2) &&
                (cblksty & J2K_CCP_CBLKSTY_LAZY)) ? T1_TYPE_RAW : T1_TYPE_MQ;
        /* FIXME: slviewer gets here with a null pointer. Why? Partially downloaded and/or corrupt textures? */
        if (seg->data == 00) {
            continue;
        }
        if (type == T1_TYPE_RAW) {
            opj_raw_init_dec(raw, (*seg->data) + seg->dataindex, seg->len);
        } else {
            if (OPJ_FALSE == opj_mqc_init_dec(mqc, (*seg->data) + seg->dataindex,
                                              seg->len)) {
                return OPJ_FALSE;
            }
        }

        if (t1->w == 64 && t1->h == 64) {
            for (passno = 0; (passno < seg->real_num_passes) &&
                    (bpno_plus_one >= 1); ++passno) {
                switch (passtype) {
                case 0:
                    if (type == T1_TYPE_RAW) {
                        opj_t1_dec_sigpass_raw(t1, bpno_plus_one, (OPJ_INT32)cblksty);
                    } else {
                        if (cblksty & J2K_CCP_CBLKSTY_VSC) {
                            opj_t1_dec_sigpass_mqc_vsc(t1, bpno_plus_one);
                        } else {
                            opj_t1_dec_sigpass_mqc_64x64(t1, bpno_plus_one);
                        }
                    }
                    break;
                case 1:
                    if (type == T1_TYPE_RAW) {
                        opj_t1_dec_refpass_raw(t1, bpno_plus_one);
                    } else {
                        if (cblksty & J2K_CCP_CBLKSTY_VSC) {
                            opj_t1_dec_refpass_mqc_vsc(t1, bpno_plus_one);
                        } else {
                            opj_t1_dec_refpass_mqc_64x64(t1, bpno_plus_one);
                        }
                    }
                    break;
                case 2:
                    opj_t1_dec_clnpass_64x64(t1, bpno_plus_one, (OPJ_INT32)cblksty);
                    break;
                }

                if ((cblksty & J2K_CCP_CBLKSTY_RESET) && type == T1_TYPE_MQ) {
                    opj_mqc_resetstates(mqc);
                    opj_mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);
                    opj_mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
                    opj_mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);
                }
                if (++passtype == 3) {
                    passtype = 0;
                    bpno_plus_one--;
                }
            }
        } else {
            for (passno = 0; (passno < seg->real_num_passes) &&
                    (bpno_plus_one >= 1); ++passno) {
                switch (passtype) {
                case 0:
                    if (type == T1_TYPE_RAW) {
                        opj_t1_dec_sigpass_raw(t1, bpno_plus_one, (OPJ_INT32)cblksty);
                    } else {
                        if (cblksty & J2K_CCP_CBLKSTY_VSC) {
                            opj_t1_dec_sigpass_mqc_vsc(t1, bpno_plus_one);
                        } else {
                            opj_t1_dec_sigpass_mqc_generic(t1, bpno_plus_one);
                        }
                    }
                    break;
                case 1:
                    if (type == T1_TYPE_RAW) {
                        opj_t1_dec_refpass_raw(t1, bpno_plus_one);
                    } else {
                        if (cblksty & J2K_CCP_CBLKSTY_VSC) {
                            opj_t1_dec_refpass_mqc_vsc(t1, bpno_plus_one);
                        } else {
                            opj_t1_dec_refpass_mqc_generic(t1, bpno_plus_one);
                        }
                    }
                    break;
                case 2:
                    opj_t1_dec_clnpass_generic(t1, bpno_plus_one, (OPJ_INT32)cblksty);
                    break;
                }

                if ((cblksty & J2K_CCP_CBLKSTY_RESET) && type == T1_TYPE_MQ) {
                    opj_mqc_resetstates(mqc);
                    opj_mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);
                    opj_mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
                    opj_mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);
                }
                if (++passtype == 3) {
                    passtype = 0;
                    bpno_plus_one--;
                }
            }
        }
    }
    return OPJ_TRUE;
}




OPJ_BOOL opj_t1_encode_cblks(opj_t1_t *t1,
                             opj_tcd_tile_t *tile,
                             opj_tcp_t *tcp,
                             const OPJ_FLOAT64 * mct_norms,
                             OPJ_UINT32 mct_numcomps
                            )
{
    OPJ_UINT32 compno, resno, bandno, precno, cblkno;

    tile->distotile = 0;        /* fixed_quality */

    for (compno = 0; compno < tile->numcomps; ++compno) {
        opj_tcd_tilecomp_t* tilec = &tile->comps[compno];
        opj_tccp_t* tccp = &tcp->tccps[compno];
        OPJ_UINT32 tile_w = (OPJ_UINT32)(tilec->x1 - tilec->x0);

        for (resno = 0; resno < tilec->numresolutions; ++resno) {
            opj_tcd_resolution_t *res = &tilec->resolutions[resno];

            for (bandno = 0; bandno < res->numbands; ++bandno) {
                opj_tcd_band_t* OPJ_RESTRICT band = &res->bands[bandno];
                OPJ_INT32 bandconst = 8192 * 8192 / ((OPJ_INT32) floor(band->stepsize * 8192));

                for (precno = 0; precno < res->pw * res->ph; ++precno) {
                    opj_tcd_precinct_t *prc = &band->precincts[precno];

                    for (cblkno = 0; cblkno < prc->cw * prc->ch; ++cblkno) {
                        opj_tcd_cblk_enc_t* cblk = &prc->cblks.enc[cblkno];
                        OPJ_INT32* OPJ_RESTRICT tiledp;
                        OPJ_UINT32 cblk_w;
                        OPJ_UINT32 cblk_h;
                        OPJ_UINT32 i, j, tileIndex = 0, tileLineAdvance;

                        OPJ_INT32 x = cblk->x0 - band->x0;
                        OPJ_INT32 y = cblk->y0 - band->y0;
                        if (band->bandno & 1) {
                            opj_tcd_resolution_t *pres = &tilec->resolutions[resno - 1];
                            x += pres->x1 - pres->x0;
                        }
                        if (band->bandno & 2) {
                            opj_tcd_resolution_t *pres = &tilec->resolutions[resno - 1];
                            y += pres->y1 - pres->y0;
                        }

                        if (!opj_t1_allocate_buffers(
                                    t1,
                                    (OPJ_UINT32)(cblk->x1 - cblk->x0),
                                    (OPJ_UINT32)(cblk->y1 - cblk->y0))) {
                            return OPJ_FALSE;
                        }

                        cblk_w = t1->w;
                        cblk_h = t1->h;
                        tileLineAdvance = tile_w - cblk_w;

                        tiledp = &tilec->data[(OPJ_UINT32)y * tile_w + (OPJ_UINT32)x];
                        t1->data = tiledp;
                        t1->data_stride = tile_w;
                        if (tccp->qmfbid == 1) {
                            for (j = 0; j < cblk_h; ++j) {
                                for (i = 0; i < cblk_w; ++i) {
                                    tiledp[tileIndex] *= (1 << T1_NMSEDEC_FRACBITS);
                                    tileIndex++;
                                }
                                tileIndex += tileLineAdvance;
                            }
                        } else {        /* if (tccp->qmfbid == 0) */
                            for (j = 0; j < cblk_h; ++j) {
                                for (i = 0; i < cblk_w; ++i) {
                                    OPJ_INT32 tmp = tiledp[tileIndex];
                                    tiledp[tileIndex] =
                                        opj_int_fix_mul_t1(
                                            tmp,
                                            bandconst);
                                    tileIndex++;
                                }
                                tileIndex += tileLineAdvance;
                            }
                        }

                        opj_t1_encode_cblk(
                            t1,
                            cblk,
                            band->bandno,
                            compno,
                            tilec->numresolutions - 1 - resno,
                            tccp->qmfbid,
                            band->stepsize,
                            tccp->cblksty,
                            tile->numcomps,
                            tile,
                            mct_norms,
                            mct_numcomps);

                    } /* cblkno */
                } /* precno */
            } /* bandno */
        } /* resno  */
    } /* compno  */
    return OPJ_TRUE;
}

/** mod fixed_quality */
static void opj_t1_encode_cblk(opj_t1_t *t1,
                               opj_tcd_cblk_enc_t* cblk,
                               OPJ_UINT32 orient,
                               OPJ_UINT32 compno,
                               OPJ_UINT32 level,
                               OPJ_UINT32 qmfbid,
                               OPJ_FLOAT64 stepsize,
                               OPJ_UINT32 cblksty,
                               OPJ_UINT32 numcomps,
                               opj_tcd_tile_t * tile,
                               const OPJ_FLOAT64 * mct_norms,
                               OPJ_UINT32 mct_numcomps)
{
    OPJ_FLOAT64 cumwmsedec = 0.0;

    opj_mqc_t *mqc = &(t1->mqc);   /* MQC component */

    OPJ_UINT32 passno;
    OPJ_INT32 bpno;
    OPJ_UINT32 passtype;
    OPJ_INT32 nmsedec = 0;
    OPJ_INT32 max;
    OPJ_UINT32 i, j;
    OPJ_BYTE type = T1_TYPE_MQ;
    OPJ_FLOAT64 tempwmsedec;

    mqc->lut_ctxno_zc_orient = lut_ctxno_zc + (orient << 9);

    max = 0;
    for (i = 0; i < t1->w; ++i) {
        for (j = 0; j < t1->h; ++j) {
            OPJ_INT32 tmp = abs(t1->data[i + j * t1->data_stride]);
            max = opj_int_max(max, tmp);
        }
    }

    cblk->numbps = max ? (OPJ_UINT32)((opj_int_floorlog2(max) + 1) -
                                      T1_NMSEDEC_FRACBITS) : 0;

    bpno = (OPJ_INT32)(cblk->numbps - 1);
    passtype = 2;

    opj_mqc_resetstates(mqc);
    opj_mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);
    opj_mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
    opj_mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);
    opj_mqc_init_enc(mqc, cblk->data);

    for (passno = 0; bpno >= 0; ++passno) {
        opj_tcd_pass_t *pass = &cblk->passes[passno];
        OPJ_UINT32 correction = 3;
        type = ((bpno < ((OPJ_INT32)(cblk->numbps) - 4)) && (passtype < 2) &&
                (cblksty & J2K_CCP_CBLKSTY_LAZY)) ? T1_TYPE_RAW : T1_TYPE_MQ;

        switch (passtype) {
        case 0:
            opj_t1_enc_sigpass(t1, bpno, &nmsedec, type, cblksty);
            break;
        case 1:
            opj_t1_enc_refpass(t1, bpno, &nmsedec, type, cblksty);
            break;
        case 2:
            opj_t1_enc_clnpass(t1, bpno, &nmsedec, cblksty);
            /* code switch SEGMARK (i.e. SEGSYM) */
            if (cblksty & J2K_CCP_CBLKSTY_SEGSYM) {
                opj_mqc_segmark_enc(mqc);
            }
            break;
        }

        /* fixed_quality */
        tempwmsedec = opj_t1_getwmsedec(nmsedec, compno, level, orient, bpno, qmfbid,
                                        stepsize, numcomps, mct_norms, mct_numcomps) ;
        cumwmsedec += tempwmsedec;
        tile->distotile += tempwmsedec;

        /* Code switch "RESTART" (i.e. TERMALL) */
        if ((cblksty & J2K_CCP_CBLKSTY_TERMALL) && !((passtype == 2) &&
                (bpno - 1 < 0))) {
            if (type == T1_TYPE_RAW) {
                opj_mqc_flush(mqc);
                correction = 1;
                /* correction = mqc_bypass_flush_enc(); */
            } else {            /* correction = mqc_restart_enc(); */
                opj_mqc_flush(mqc);
                correction = 1;
            }
            pass->term = 1;
        } else {
            if (((bpno < ((OPJ_INT32)(cblk->numbps) - 4) && (passtype > 0))
                    || ((bpno == ((OPJ_INT32)cblk->numbps - 4)) && (passtype == 2))) &&
                    (cblksty & J2K_CCP_CBLKSTY_LAZY)) {
                if (type == T1_TYPE_RAW) {
                    opj_mqc_flush(mqc);
                    correction = 1;
                    /* correction = mqc_bypass_flush_enc(); */
                } else {        /* correction = mqc_restart_enc(); */
                    opj_mqc_flush(mqc);
                    correction = 1;
                }
                pass->term = 1;
            } else {
                pass->term = 0;
            }
        }

        if (++passtype == 3) {
            passtype = 0;
            bpno--;
        }

        if (pass->term && bpno > 0) {
            type = ((bpno < ((OPJ_INT32)(cblk->numbps) - 4)) && (passtype < 2) &&
                    (cblksty & J2K_CCP_CBLKSTY_LAZY)) ? T1_TYPE_RAW : T1_TYPE_MQ;
            if (type == T1_TYPE_RAW) {
                opj_mqc_bypass_init_enc(mqc);
            } else {
                opj_mqc_restart_init_enc(mqc);
            }
        }

        pass->distortiondec = cumwmsedec;
        pass->rate = opj_mqc_numbytes(mqc) + correction;    /* FIXME */

        /* Code-switch "RESET" */
        if (cblksty & J2K_CCP_CBLKSTY_RESET) {
            opj_mqc_reset_enc(mqc);
        }
    }

    /* Code switch "ERTERM" (i.e. PTERM) */
    if (cblksty & J2K_CCP_CBLKSTY_PTERM) {
        opj_mqc_erterm_enc(mqc);
    } else /* Default coding */ if (!(cblksty & J2K_CCP_CBLKSTY_LAZY)) {
        opj_mqc_flush(mqc);
    }

    cblk->totalpasses = passno;

    for (passno = 0; passno < cblk->totalpasses; passno++) {
        opj_tcd_pass_t *pass = &cblk->passes[passno];
        if (pass->rate > opj_mqc_numbytes(mqc)) {
            pass->rate = opj_mqc_numbytes(mqc);
        }
        /*Preventing generation of FF as last data byte of a pass*/
        if ((pass->rate > 1) && (cblk->data[pass->rate - 1] == 0xFF)) {
            pass->rate--;
        }
        pass->len = pass->rate - (passno == 0 ? 0 : cblk->passes[passno - 1].rate);
    }
}
