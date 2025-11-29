/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_MMDVM_MULTI_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_MMDVM_MULTI_IMPL_H

#include <gnuradio/qradiolink/demod_mmdvm_multi.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/analog/pwr_squelch_ff.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/rotator_cc.h>
#include <gnuradio/qradiolink/mmdvm_sink.h>
#include <gnuradio/qradiolink/rssi_tag_block.h>
#include <vector>

// Forward declaration - BurstTimer is application-level
class BurstTimer;

#ifndef MAX_MMDVM_CHANNELS
#define MAX_MMDVM_CHANNELS 7
#endif

namespace gr {
namespace qradiolink {

class demod_mmdvm_multi_impl : public demod_mmdvm_multi
{
private:
    gr::blocks::float_to_short::sptr d_float_to_short[MAX_MMDVM_CHANNELS];
    gr::analog::quadrature_demod_cf::sptr d_fm_demod[MAX_MMDVM_CHANNELS];
    gr::blocks::multiply_const_ff::sptr d_level_control[MAX_MMDVM_CHANNELS];
    gr::filter::rational_resampler_ccf::sptr d_first_resampler;
    gr::filter::rational_resampler_ccf::sptr d_resampler[MAX_MMDVM_CHANNELS];
    gr::filter::fft_filter_ccf::sptr d_filter[MAX_MMDVM_CHANNELS];
    gr::qradiolink::mmdvm_sink::sptr d_mmdvm_sink;
    gr::qradiolink::rssi_tag_block::sptr d_rssi_tag_block[MAX_MMDVM_CHANNELS];
    gr::blocks::rotator_cc::sptr d_rotator[MAX_MMDVM_CHANNELS];

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_num_channels;
    bool d_use_tdma;

public:
    demod_mmdvm_multi_impl(BurstTimer* burst_timer,
                          int num_channels,
                          int channel_separation,
                          bool use_tdma,
                          int sps,
                          int samp_rate,
                          int carrier_freq,
                          int filter_width);
    ~demod_mmdvm_multi_impl();

    void set_filter_width(int filter_width) override;
    void calibrate_rssi(float level) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_MMDVM_MULTI_IMPL_H */

