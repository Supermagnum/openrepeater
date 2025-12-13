/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_DSSS_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_DSSS_IMPL_H

#include <gnuradio/qradiolink/mod_dsss.h>
#include <gnuradio/blocks/packed_to_unpacked.h>
#include <gnuradio/blocks/unpacked_to_packed.h>
#include <gnuradio/digital/chunks_to_symbols.h>
#include <gnuradio/digital/scrambler_bb.h>
#include <gnuradio/fec/cc_encoder.h>
#include <gnuradio/fec/encoder.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/dsss/dsss_encoder_bb.h>

namespace gr {
namespace qradiolink {

class mod_dsss_impl : public mod_dsss
{
private:
    gr::blocks::packed_to_unpacked_bb::sptr d_packed_to_unpacked;
    gr::blocks::unpacked_to_packed_bb::sptr d_unpacked_to_packed;
    gr::digital::chunks_to_symbols_bc::sptr d_chunks_to_symbols;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::fec::encoder::sptr d_encode_ccsds;
    gr::digital::scrambler_bb::sptr d_scrambler;
    gr::dsss::dsss_encoder_bb::sptr d_dsss_encoder;
    gr::filter::fft_filter_ccf::sptr d_filter;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::rational_resampler_ccf::sptr d_resampler_if;
    gr::filter::rational_resampler_ccf::sptr d_resampler_rf;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;

public:
    mod_dsss_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~mod_dsss_impl();

    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_DSSS_IMPL_H */

