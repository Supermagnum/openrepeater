/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_DMR_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_DMR_IMPL_H

#include <gnuradio/qradiolink/demod_dmr.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/digital/symbol_sync_ff.h>
#include <gnuradio/analog/phase_modulator_fc.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/interleave.h>
#include <gnuradio/digital/binary_slicer_fb.h>
#include <gnuradio/digital/map_bb.h>
#include <gnuradio/blocks/pack_k_bits_bb.h>
#include <gnuradio/blocks/unpack_k_bits_bb.h>
#include <gnuradio/digital/constellation.h>
#include <vector>

namespace gr {
namespace qradiolink {

class demod_dmr_impl : public demod_dmr
{
private:
    gr::analog::quadrature_demod_cf::sptr d_fm_demod;
    gr::blocks::multiply_const_ff::sptr d_level_control;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::blocks::float_to_complex::sptr d_float_to_complex_corr;
    gr::blocks::complex_to_float::sptr d_complex_to_float_corr;
    gr::filter::fft_filter_fff::sptr d_symbol_filter;
    gr::digital::symbol_sync_ff::sptr d_symbol_sync;
    gr::blocks::complex_to_float::sptr d_complex_to_float;
    gr::blocks::interleave::sptr d_interleave;
    gr::analog::phase_modulator_fc::sptr d_phase_mod;
    gr::digital::binary_slicer_fb::sptr d_slicer;
    gr::digital::map_bb::sptr d_symbol_map;
    gr::blocks::pack_k_bits_bb::sptr d_packer;
    gr::blocks::unpack_k_bits_bb::sptr d_unpacker;
    int d_sps;
    int d_samp_rate;
    int d_filter_width;
    float d_target_samp_rate;

public:
    demod_dmr_impl(int sps, int samp_rate);
    ~demod_dmr_impl();
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_DMR_IMPL_H */

