/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_DPMR_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_DPMR_IMPL_H

#include <gnuradio/qradiolink/demod_dpmr.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/digital/symbol_sync_ff.h>
#include <gnuradio/digital/constellation.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/analog/phase_modulator_fc.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/interleave.h>
#include <gnuradio/digital/binary_slicer_fb.h>
#include <gnuradio/blocks/pack_k_bits_bb.h>
#include <gnuradio/blocks/unpack_k_bits_bb.h>
#include <gnuradio/digital/map_bb.h>
#include <gnuradio/digital/descrambler_bb.h>
#include <gnuradio/fec/cc_decoder.h>
#include <gnuradio/fec/decoder.h>
#include <gnuradio/filter/firdes.h>
#include <vector>

namespace gr {
namespace qradiolink {

class demod_dpmr_impl : public demod_dpmr
{
private:
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::analog::quadrature_demod_cf::sptr d_fm_demod;
    gr::filter::fft_filter_fff::sptr d_symbol_filter;
    gr::digital::symbol_sync_ff::sptr d_symbol_sync;
    gr::analog::phase_modulator_fc::sptr d_phase_mod;
    gr::blocks::multiply_const_ff::sptr d_level_control;
    gr::blocks::complex_to_float::sptr d_complex_to_float;
    gr::blocks::complex_to_float::sptr d_complex_to_float_corr;
    gr::blocks::float_to_complex::sptr d_float_to_complex_corr;
    gr::blocks::interleave::sptr d_interleave;
    gr::digital::binary_slicer_fb::sptr d_slicer;
    gr::blocks::pack_k_bits_bb::sptr d_packer;
    gr::blocks::unpack_k_bits_bb::sptr d_unpacker;
    gr::digital::map_bb::sptr d_symbol_map;
    gr::digital::descrambler_bb::sptr d_descrambler;
    gr::fec::decoder::sptr d_fec_decoder;

    int d_sps;
    int d_samp_rate;
    int d_target_samp_rate;
    float d_filter_width;
    int d_symbol_rate;

public:
    demod_dpmr_impl(int sps, int samp_rate);
    ~demod_dpmr_impl();
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_DPMR_IMPL_H */

