/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_NXDN_H
#define INCLUDED_QRADIOLINK_DEMOD_NXDN_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief NXDN Demodulator block
 * \ingroup qradiolink
 *
 * NXDN (Next Generation Digital Narrowband) 4FSK demodulator.
 * Supports both NXDN48 (2400 baud) and NXDN96 (4800 baud) modes.
 */
class QRADIOLINK_API demod_nxdn : public gr::hier_block2
{
public:
    typedef std::shared_ptr<demod_nxdn> sptr;

    /*!
     * \brief Make an NXDN demodulator block
     *
     * \param symbol_rate Symbol rate in baud (2400 for NXDN48, 4800 for NXDN96)
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 1000000)
     */
    static sptr make(int symbol_rate = 2400, int sps = 125, int samp_rate = 1000000);

protected:
    demod_nxdn(const std::string& name,
              gr::io_signature::sptr input_signature,
              gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_NXDN_H */

