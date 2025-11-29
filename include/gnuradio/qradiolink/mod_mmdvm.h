/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_MMDVM_H
#define INCLUDED_QRADIOLINK_MOD_MMDVM_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief MMDVM Modulator block
 * \ingroup qradiolink
 *
 * Multi-Mode Digital Voice Modem modulator.
 */
class QRADIOLINK_API mod_mmdvm : public gr::hier_block2
{
public:
    typedef std::shared_ptr<mod_mmdvm> sptr;

    /*!
     * \brief Make an MMDVM modulator block
     *
     * \param sps Samples per symbol (default: 10)
     * \param samp_rate Sample rate (default: 250000)
     * \param carrier_freq Carrier frequency (default: 1700)
     * \param filter_width Filter width (default: 5000)
     */
    static sptr make(int sps = 10,
                     int samp_rate = 250000,
                     int carrier_freq = 1700,
                     int filter_width = 5000);

    virtual void set_bb_gain(float value);

protected:
    mod_mmdvm(const std::string& name,
              gr::io_signature::sptr input_signature,
              gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_MMDVM_H */

