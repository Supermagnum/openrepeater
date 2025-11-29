/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_M17_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_M17_IMPL_H

#include <gnuradio/qradiolink/demod_m17.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/analog/pwr_squelch_cc.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/digital/symbol_sync_ff.h>
#include <gnuradio/analog/phase_modulator_fc.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/interleave.h>
#include <gnuradio/digital/binary_slicer_fb.h>
#include <gnuradio/digital/map_bb.h>
#include <gnuradio/blocks/pack_k_bits_bb.h>
#include <gnuradio/blocks/unpack_k_bits_bb.h>

namespace gr {
namespace qradiolink {

class demod_m17_impl : public demod_m17
{
private:
    gr::analog::quadrature_demod_cf::sptr d_fm_demod;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::fft_filter_ccf::sptr d_filter;
    gr::digital::symbol_sync_ff::sptr d_symbol_sync;
    gr::filter::fft_filter_fff::sptr d_symbol_filter;
    gr::analog::phase_modulator_fc::sptr d_phase_mod;
    gr::blocks::complex_to_float::sptr d_complex_to_float;
    gr::blocks::interleave::sptr d_interleave;
    gr::digital::binary_slicer_fb::sptr d_slicer;
    gr::digital::map_bb::sptr d_symbol_map;
    gr::blocks::pack_k_bits_bb::sptr d_packer;
    gr::blocks::unpack_k_bits_bb::sptr d_unpacker;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;

public:
    demod_m17_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~demod_m17_impl();
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_M17_IMPL_H */

