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
#include <gnuradio/qradiolink/mod_dpmr.h>
#include "mod_dpmr_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace gr {
namespace qradiolink {

mod_dpmr::sptr mod_dpmr::make(int sps,
                               int samp_rate,
                               int carrier_freq,
                               int filter_width)
{
    return gnuradio::get_initial_sptr(
        new mod_dpmr_impl(sps, samp_rate, carrier_freq, filter_width));
}

mod_dpmr_impl::mod_dpmr_impl(int sps,
                              int samp_rate,
                              int carrier_freq,
                              int filter_width)
    : mod_dpmr("mod_dpmr",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_symbol_rate = 2400;  // dPMR fixed at 2400 baud
    d_samp_rate = samp_rate;
    d_sps = sps;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;

    // dPMR uses 4FSK with constellation points at -1.5, -0.5, 0.5, 1.5
    std::vector<float> constellation;
    constellation.push_back(-1.5);
    constellation.push_back(-0.5);
    constellation.push_back(0.5);
    constellation.push_back(1.5);

    // dPMR symbol mapping (from ETSI TS 102 658)
    std::vector<int> map;
    map.push_back(0);  // 00 -> -1.5
    map.push_back(1);  // 01 -> -0.5
    map.push_back(2);  // 10 -> 0.5
    map.push_back(3);  // 11 -> 1.5

    // Calculate IF sample rate and samples per symbol
    d_samples_per_symbol = 5;
    d_if_samp_rate = d_symbol_rate * d_samples_per_symbol;

    // dPMR scrambler: Different polynomial from NXDN
    // From ETSI TS 102 658 specification
    // Using polynomial x^15 + x^14 + x^13 + x^11 + 1
    // This is different from NXDN's x^15 + x^14 + 1
    d_scrambler = gr::digital::scrambler_bb::make(0x6801, 0x7FFF, 15);

    // dPMR FEC: Convolutional code rate 1/2, constraint length 7
    // Polynomials from ETSI TS 102 658
    std::vector<int> polys;
    polys.push_back(109);  // 0x6D
    polys.push_back(79);   // 0x4F

    gr::fec::code::cc_encoder::sptr encoder =
        gr::fec::code::cc_encoder::make(80, 7, 2, polys);
    d_fec_encoder = gr::fec::encoder::make(encoder, 1, 1);

    // Build signal processing chain
    d_packed_to_unpacked = gr::blocks::packed_to_unpacked_bb::make(1, gr::GR_MSB_FIRST);
    d_packer = gr::blocks::pack_k_bits_bb::make(2);
    d_map = gr::digital::map_bb::make(map);

    d_chunks_to_symbols = gr::digital::chunks_to_symbols_bf::make(constellation);

    // Root raised cosine filter for pulse shaping
    // Rolloff factor 0.2 (typical for dPMR)
    int nfilts = 25 * d_samples_per_symbol;
    if ((nfilts % 2) == 0)
        nfilts += 1;

    std::vector<float> first_resampler_taps = gr::filter::firdes::root_raised_cosine(
        d_samples_per_symbol, d_if_samp_rate, d_symbol_rate, 0.2, nfilts);
    d_first_resampler = gr::filter::rational_resampler_fff::make(
        d_samples_per_symbol, 1, first_resampler_taps);

    // Frequency modulator
    // dPMR deviation: ±600 Hz for 2400 baud
    float deviation = 600.0f;
    float sensitivity = (2.0f * M_PI * deviation) / d_if_samp_rate;
    d_fm_modulator = gr::analog::frequency_modulator_fc::make(sensitivity);

    // IF filter
    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass(
            1, d_if_samp_rate, d_filter_width, d_filter_width,
            gr::fft::window::WIN_BLACKMAN_HARRIS));

    // Resampler to final sample rate
    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate * 3, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(d_sps, 3, interp_taps);

    d_amplify = gr::blocks::multiply_const_cc::make(0.9, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);

    // Connect blocks
    // Input bytes -> unpack -> scrambler -> FEC -> pack to 2-bit symbols -> map -> symbols
    connect(self(), 0, d_packed_to_unpacked, 0);
    connect(d_packed_to_unpacked, 0, d_scrambler, 0);
    connect(d_scrambler, 0, d_fec_encoder, 0);
    connect(d_fec_encoder, 0, d_packer, 0);
    connect(d_packer, 0, d_map, 0);
    connect(d_map, 0, d_chunks_to_symbols, 0);

    // Symbols -> pulse shaping -> FM modulator -> IF filter -> resample -> output
    connect(d_chunks_to_symbols, 0, d_first_resampler, 0);
    connect(d_first_resampler, 0, d_fm_modulator, 0);
    connect(d_fm_modulator, 0, d_filter, 0);
    connect(d_filter, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler, 0);
    connect(d_resampler, 0, self(), 0);
}

mod_dpmr_impl::~mod_dpmr_impl() {}

void mod_dpmr_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_dpmr::set_bb_gain(float value)
{
    // This should never be called, as mod_dpmr is only an interface
    // The actual implementation is in mod_dpmr_impl
}

} // namespace qradiolink
} // namespace gr

