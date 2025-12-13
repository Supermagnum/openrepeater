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

#include <gnuradio/qradiolink/demod_nxdn.h>
#include "demod_nxdn_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>

namespace gr {
namespace qradiolink {

demod_nxdn::sptr demod_nxdn::make(int symbol_rate, int sps, int samp_rate)
{
    return gnuradio::get_initial_sptr(new demod_nxdn_impl(symbol_rate, sps, samp_rate));
}

demod_nxdn_impl::demod_nxdn_impl(int symbol_rate, int sps, int samp_rate)
    : demod_nxdn("demod_nxdn",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::makev(4, 4, {sizeof(gr_complex), sizeof(gr_complex), sizeof(unsigned char), sizeof(float)})),
      d_symbol_rate(symbol_rate),
      d_sps(sps),
      d_samp_rate(samp_rate),
      d_target_samp_rate(symbol_rate * 5)  // 5 samples per symbol at IF
{
    unsigned int samples_per_symbol = 5;
    
    // NXDN 4FSK constellation points
    std::vector<gr_complex> constellation_points;
    constellation_points.push_back(gr_complex(-1.5, 0));
    constellation_points.push_back(gr_complex(-0.5, 0));
    constellation_points.push_back(gr_complex(0.5, 0));
    constellation_points.push_back(gr_complex(1.5, 0));
    
    int ntaps = 25 * samples_per_symbol;

    std::vector<int> pre_diff;

    auto constellation_4fsk = gr::digital::constellation_rect::make(
        constellation_points, pre_diff, 2, 4, 1, 1.0, 1.0);

    // Filter width based on symbol rate
    // NXDN48: 6.25 kHz, NXDN96: 12.5 kHz
    d_filter_width = (d_symbol_rate == 2400) ? 5000.0f : 10000.0f;

    // Resampler from input sample rate to IF
    std::vector<float> taps = gr::filter::firdes::low_pass_2(
        3, d_samp_rate * 3, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(3, d_sps, taps);

    // Phase modulator for constellation correction
    d_phase_mod = gr::analog::phase_modulator_fc::make(M_PI / 2);
    
    // Root raised cosine matched filter
    std::vector<float> symbol_filter_taps = gr::filter::firdes::root_raised_cosine(
        1, d_target_samp_rate, d_target_samp_rate / samples_per_symbol, 0.2, ntaps);
    d_symbol_filter = gr::filter::fft_filter_fff::make(1, symbol_filter_taps);
    
    float calculated_symbol_rate = ((float)d_target_samp_rate / (float)samples_per_symbol);
    float sps_deviation = 0.06;
    
    // Symbol synchronizer
    d_symbol_sync = gr::digital::symbol_sync_ff::make(
        gr::digital::TED_MUELLER_AND_MULLER, samples_per_symbol, 
        2 * M_PI / 100.0f, 1.0, 0.2869, sps_deviation, 1, constellation_4fsk);
    
    // FM demodulator
    d_fm_demod = gr::analog::quadrature_demod_cf::make(
        d_target_samp_rate / (M_PI / 2 * calculated_symbol_rate));
    
    d_level_control = gr::blocks::multiply_const_ff::make(0.9);
    d_complex_to_float = gr::blocks::complex_to_float::make();
    d_complex_to_float_corr = gr::blocks::complex_to_float::make();
    d_float_to_complex_corr = gr::blocks::float_to_complex::make();
    d_interleave = gr::blocks::interleave::make(4);
    d_slicer = gr::digital::binary_slicer_fb::make();
    d_packer = gr::blocks::pack_k_bits_bb::make(2);
    d_unpacker = gr::blocks::unpack_k_bits_bb::make(2);
    
    // NXDN symbol mapping (reverse of modulator)
    std::vector<int> map;
    map.push_back(3);  // 00 -> symbol 3 (1.5)
    map.push_back(1);  // 01 -> symbol 1 (-0.5)
    map.push_back(2);  // 10 -> symbol 2 (0.5)
    map.push_back(0);  // 11 -> symbol 0 (-1.5)
    d_symbol_map = gr::digital::map_bb::make(map);

    // NXDN descrambler (15-bit LFSR, matches scrambler)
    d_descrambler = gr::digital::descrambler_bb::make(0x6001, 0x7FFF, 15);

    // NXDN FEC decoder (convolutional code rate 1/2, constraint length 7)
    std::vector<int> polys;
    polys.push_back(109);  // 0x6D
    polys.push_back(79);    // 0x4F

    gr::fec::code::cc_decoder::sptr decoder =
        gr::fec::code::cc_decoder::make(80, 7, 2, polys);
    d_fec_decoder = gr::fec::decoder::make(decoder, 1, 1);

    // Connect blocks
    // Input -> resampler -> FM demod -> matched filter -> symbol sync
    connect(self(), 0, d_resampler, 0);
    connect(d_resampler, 0, self(), 0);  // Output 0: filtered I/Q
    connect(d_resampler, 0, d_fm_demod, 0);
    connect(d_fm_demod, 0, d_symbol_filter, 0);
    connect(d_symbol_filter, 0, d_symbol_sync, 0);
    connect(d_symbol_filter, 0, self(), 3);  // Output 3: symbol error rate (float)

    // Symbol sync -> level control -> phase mod -> constellation correction
    connect(d_symbol_sync, 0, d_level_control, 0);
    connect(d_level_control, 0, d_phase_mod, 0);
    connect(d_phase_mod, 0, self(), 1);  // Output 1: corrected constellation
    connect(d_phase_mod, 0, d_complex_to_float, 0);
    
    // Constellation -> interleave -> slicer -> pack -> symbol map -> unpack
    connect(d_complex_to_float, 0, d_interleave, 0);
    connect(d_complex_to_float, 1, d_interleave, 1);
    connect(d_interleave, 0, d_slicer, 0);
    connect(d_slicer, 0, d_packer, 0);
    connect(d_packer, 0, d_symbol_map, 0);
    connect(d_symbol_map, 0, d_unpacker, 0);
    
    // Unpack -> FEC decode -> descramble -> output
    connect(d_unpacker, 0, d_fec_decoder, 0);
    connect(d_fec_decoder, 0, d_descrambler, 0);
    connect(d_descrambler, 0, self(), 2);  // Output 2: decoded bytes
}

demod_nxdn_impl::~demod_nxdn_impl() {}

} // namespace qradiolink
} // namespace gr

