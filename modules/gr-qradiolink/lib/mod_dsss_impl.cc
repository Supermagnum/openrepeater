/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */


#include <climits>
#include <cstdint>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gnuradio/qradiolink/mod_dsss.h>
#include "mod_dsss_impl.h"
#include <gnuradio/io_signature.h>

// DSSS encoder - using original location
// TODO: Migrate DSSS blocks separately
#include "../../src/gr/dsss_encoder_bb_impl.h"

namespace gr {
namespace qradiolink {

mod_dsss::sptr mod_dsss::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(
        new mod_dsss_impl(sps, samp_rate, carrier_freq, filter_width));
}

mod_dsss_impl::mod_dsss_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : mod_dsss("mod_dsss",
               gr::io_signature::make(1, 1, sizeof(char)),
               gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    std::vector<gr_complex> constellation;
    constellation.push_back(-1);
    constellation.push_back(1);
    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);

    static const int barker_13[] = {1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1};
    std::vector<int> dsss_code(barker_13, barker_13 + sizeof(barker_13) / sizeof(barker_13[0]));

    d_samples_per_symbol = sps; // 25
    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    int if_samp_rate = 5200;

    d_packed_to_unpacked = gr::blocks::packed_to_unpacked_bb::make(1, gr::GR_MSB_FIRST);
    d_unpacked_to_packed = gr::blocks::unpacked_to_packed_bb::make(1, gr::GR_MSB_FIRST);
    d_scrambler = gr::digital::scrambler_bb::make(0x8A, 0x7F, 7);

    gr::fec::code::cc_encoder::sptr encoder =
        gr::fec::code::cc_encoder::make(80, 7, 2, polys);
    d_encode_ccsds = gr::fec::encoder::make(encoder, 1, 1);

    d_chunks_to_symbols = gr::digital::chunks_to_symbols_bc::make(constellation);

    d_resampler = gr::filter::rational_resampler_ccf::make(
        d_samples_per_symbol,
        1,
        gr::filter::firdes::root_raised_cosine(
            d_samples_per_symbol, d_samples_per_symbol, 1.0, 0.35, 11 * d_samples_per_symbol));

    // DSSS encoder - using original namespace
    gr::dsss::dsss_encoder_bb::sptr d_dsss_encoder = gr::dsss::dsss_encoder_bb::make(dsss_code);
    d_amplify = gr::blocks::multiply_const_cc::make(0.65, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);
    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass_2(
            1, if_samp_rate, d_filter_width, 1200, 60, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_resampler_if = gr::filter::rational_resampler_ccf::make(
        50, 13, gr::filter::firdes::low_pass(50.0, if_samp_rate * 50, d_filter_width, d_filter_width * 5));
    d_resampler_rf = gr::filter::rational_resampler_ccf::make(
        50, 1, gr::filter::firdes::low_pass(50, d_samp_rate, d_filter_width, d_filter_width * 5));

    connect(self(), 0, d_packed_to_unpacked, 0);
    connect(d_packed_to_unpacked, 0, d_scrambler, 0);
    connect(d_scrambler, 0, d_encode_ccsds, 0);
    connect(d_encode_ccsds, 0, d_unpacked_to_packed, 0);
    connect(d_unpacked_to_packed, 0, d_dsss_encoder, 0);
    connect(d_dsss_encoder, 0, d_chunks_to_symbols, 0);
    connect(d_chunks_to_symbols, 0, d_resampler, 0);
    connect(d_resampler, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler_if, 0);
    connect(d_resampler_if, 0, d_filter, 0);
    connect(d_filter, 0, d_resampler_rf, 0);
    connect(d_resampler_rf, 0, self(), 0);
}

mod_dsss_impl::~mod_dsss_impl() {}

void mod_dsss_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_dsss::set_bb_gain(float value)
{
    // This should never be called, as mod_dsss is only an interface
    // The actual implementation is in mod_dsss_impl
}

} // namespace qradiolink
} // namespace gr

