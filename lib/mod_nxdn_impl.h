/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_NXDN_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_NXDN_IMPL_H

#include <gnuradio/qradiolink/mod_nxdn.h>
#include <gnuradio/analog/frequency_modulator_fc.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/blocks/packed_to_unpacked.h>
#include <gnuradio/digital/chunks_to_symbols.h>
#include <gnuradio/digital/map_bb.h>
#include <gnuradio/blocks/pack_k_bits_bb.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/digital/scrambler_bb.h>
#include <gnuradio/fec/cc_encoder.h>
#include <gnuradio/fec/encoder.h>
#include <vector>

namespace gr {
namespace qradiolink {

class mod_nxdn_impl : public mod_nxdn
{
private:
    gr::analog::frequency_modulator_fc::sptr d_fm_modulator;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::rational_resampler_fff::sptr d_first_resampler;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::filter::fft_filter_ccf::sptr d_filter;
    gr::blocks::packed_to_unpacked_bb::sptr d_packed_to_unpacked;
    gr::digital::chunks_to_symbols_bf::sptr d_chunks_to_symbols;
    gr::blocks::pack_k_bits_bb::sptr d_packer;
    gr::digital::map_bb::sptr d_map;
    gr::digital::scrambler_bb::sptr d_scrambler;
    gr::fec::encoder::sptr d_fec_encoder;

    int d_symbol_rate;
    int d_samp_rate;
    int d_sps;
    int d_samples_per_symbol;
    int d_carrier_freq;
    int d_filter_width;
    float d_if_samp_rate;

public:
    mod_nxdn_impl(int symbol_rate, int sps, int samp_rate, int carrier_freq, int filter_width);
    ~mod_nxdn_impl();

    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_NXDN_IMPL_H */

