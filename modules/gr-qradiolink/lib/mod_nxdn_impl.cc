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
#include <gnuradio/qradiolink/mod_nxdn.h>
#include "mod_nxdn_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace gr {
namespace qradiolink {

mod_nxdn::sptr mod_nxdn::make(int symbol_rate,
                               int sps,
                               int samp_rate,
                               int carrier_freq,
                               int filter_width)
{
    return gnuradio::get_initial_sptr(
        new mod_nxdn_impl(symbol_rate, sps, samp_rate, carrier_freq, filter_width));
}

mod_nxdn_impl::mod_nxdn_impl(int symbol_rate,
                              int sps,
                              int samp_rate,
                              int carrier_freq,
                              int filter_width)
    : mod_nxdn("mod_nxdn",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_symbol_rate = symbol_rate;
    d_samp_rate = samp_rate;
    d_sps = sps;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;

    // NXDN uses 4FSK with constellation points at -1.5, -0.5, 0.5, 1.5
    std::vector<float> constellation;
    constellation.push_back(-1.5);
    constellation.push_back(-0.5);
    constellation.push_back(0.5);
    constellation.push_back(1.5);

    // NXDN symbol mapping (from MMDVM reference)
    // Bits map to symbols: 00=-1.5, 01=-0.5, 10=0.5, 11=1.5
    std::vector<int> map;
    map.push_back(0);  // 00 -> -1.5
    map.push_back(1);  // 01 -> -0.5
    map.push_back(2);  // 10 -> 0.5
    map.push_back(3);  // 11 -> 1.5

    // Calculate IF sample rate and samples per symbol
    // NXDN typically uses 5 samples per symbol at IF
    d_samples_per_symbol = 5;
    d_if_samp_rate = d_symbol_rate * d_samples_per_symbol;

    // NXDN scrambler: 15-bit LFSR with polynomial x^15 + x^14 + 1
    // Initial state: 0x7FFF (all ones)
    // Using GNU Radio scrambler with mask 0x7FFF (15 bits), polynomial 0x6001
    // Note: GNU Radio scrambler uses different polynomial format
    // For 15-bit LFSR: polynomial = 0x6001 (x^15 + x^14 + 1)
    d_scrambler = gr::digital::scrambler_bb::make(0x6001, 0x7FFF, 15);

    // NXDN FEC: Convolutional code rate 1/2, constraint length 7
    // Polynomials from NXDN spec (similar to DMR)
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
    // Rolloff factor 0.2 (typical for NXDN)
    int nfilts = 25 * d_samples_per_symbol;
    if ((nfilts % 2) == 0)
        nfilts += 1;

    std::vector<float> first_resampler_taps = gr::filter::firdes::root_raised_cosine(
        d_samples_per_symbol, d_if_samp_rate, d_symbol_rate, 0.2, nfilts);
    d_first_resampler = gr::filter::rational_resampler_fff::make(
        d_samples_per_symbol, 1, first_resampler_taps);

    // Frequency modulator
    // NXDN deviation: ±600 Hz for 2400 baud, ±1200 Hz for 4800 baud
    float deviation = (d_symbol_rate == 2400) ? 600.0f : 1200.0f;
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

mod_nxdn_impl::~mod_nxdn_impl() {}

void mod_nxdn_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_nxdn::set_bb_gain(float value)
{
    // This should never be called, as mod_nxdn is only an interface
    // The actual implementation is in mod_nxdn_impl
}

} // namespace qradiolink
} // namespace gr

