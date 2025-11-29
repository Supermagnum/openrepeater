/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_BPSK_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_BPSK_IMPL_H

#include <gnuradio/qradiolink/mod_bpsk.h>
#include <gnuradio/blocks/packed_to_unpacked.h>
#include <gnuradio/digital/chunks_to_symbols.h>
#include <gnuradio/digital/scrambler_bb.h>
#include <gnuradio/fec/cc_encoder.h>
#include <gnuradio/fec/encoder.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/blocks/multiply_const.h>

namespace gr {
namespace qradiolink {

class mod_bpsk_impl : public mod_bpsk
{
private:
    gr::blocks::packed_to_unpacked_bb::sptr d_packed_to_unpacked;
    gr::digital::chunks_to_symbols_bc::sptr d_chunks_to_symbols;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::fec::encoder::sptr d_encode_ccsds;
    gr::digital::scrambler_bb::sptr d_scrambler;
    gr::filter::rational_resampler_ccf::sptr d_resampler;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;

public:
    mod_bpsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~mod_bpsk_impl();

    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_BPSK_IMPL_H */

