/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_NXDN_H
#define INCLUDED_QRADIOLINK_MOD_NXDN_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief NXDN Modulator
 * \ingroup qradiolink
 *
 * This block implements an NXDN (4FSK) modulator for Next Generation Digital Narrowband.
 * Supports both NXDN48 (2400 baud) and NXDN96 (4800 baud) modes.
 */
class QRADIOLINK_API mod_nxdn : public hier_block2
{
public:
    typedef std::shared_ptr<mod_nxdn> sptr;

    /*!
     * \brief Make an NXDN modulator block
     *
     * \param symbol_rate Symbol rate in baud (2400 for NXDN48, 4800 for NXDN96)
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 1000000)
     * \param carrier_freq Carrier frequency in Hz (default: 1700)
     * \param filter_width Filter width in Hz (default: 6000 for NXDN48, 12000 for NXDN96)
     */
    static sptr make(int symbol_rate = 2400,
                     int sps = 125,
                     int samp_rate = 1000000,
                     int carrier_freq = 1700,
                     int filter_width = 6000);

    /*!
     * \brief Set baseband gain
     * \param value Gain value
     */
    virtual void set_bb_gain(float value);

protected:
    mod_nxdn(const std::string& name,
            gr::io_signature::sptr input_signature,
            gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_NXDN_H */

