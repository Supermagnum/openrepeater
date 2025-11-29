/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_MMDVM_MULTI2_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_MMDVM_MULTI2_IMPL_H

#include <gnuradio/qradiolink/mod_mmdvm_multi2.h>
#include <gnuradio/analog/frequency_modulator_fc.h>
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/pfb_synthesizer_ccf.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/qradiolink/mmdvm_source.h>
#include <gnuradio/qradiolink/zero_idle_bursts.h>
#include <vector>

// Forward declaration - BurstTimer is application-level
class BurstTimer;

#ifndef MAX_MMDVM_CHANNELS
#define MAX_MMDVM_CHANNELS 7
#endif

namespace gr {
namespace qradiolink {

class mod_mmdvm_multi2_impl : public mod_mmdvm_multi2
{
private:
    gr::analog::frequency_modulator_fc::sptr d_fm_modulator[MAX_MMDVM_CHANNELS];
    gr::filter::rational_resampler_ccf::sptr d_resampler[MAX_MMDVM_CHANNELS];
    gr::blocks::multiply_const_cc::sptr d_amplify[MAX_MMDVM_CHANNELS];
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::blocks::multiply_const_ff::sptr d_audio_amplify[MAX_MMDVM_CHANNELS];
    gr::filter::fft_filter_ccf::sptr d_filter[MAX_MMDVM_CHANNELS];
    gr::blocks::short_to_float::sptr d_short_to_float[MAX_MMDVM_CHANNELS];
    gr::filter::pfb_synthesizer_ccf::sptr d_synthesizer;
    gr::qradiolink::mmdvm_source::sptr d_mmdvm_source;
    gr::qradiolink::zero_idle_bursts::sptr d_zero_idle[MAX_MMDVM_CHANNELS];
    gr::blocks::multiply_const_cc::sptr d_divide_level;
    gr::blocks::null_source::sptr d_null_source[10];

    int d_samp_rate;
    int d_sps;
    int d_carrier_freq;
    int d_filter_width;
    int d_num_channels;
    bool d_use_tdma;

public:
    mod_mmdvm_multi2_impl(BurstTimer* burst_timer,
                         int num_channels,
                         int channel_separation,
                         bool use_tdma,
                         int sps,
                         int samp_rate,
                         int carrier_freq,
                         int filter_width);
    ~mod_mmdvm_multi2_impl();

    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_MMDVM_MULTI2_IMPL_H */

