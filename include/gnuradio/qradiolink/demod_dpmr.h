/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_DPMR_H
#define INCLUDED_QRADIOLINK_DEMOD_DPMR_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief dPMR Demodulator block
 * \ingroup qradiolink
 *
 * dPMR (Digital Private Mobile Radio) 4FSK demodulator.
 * Standard: ETSI TS 102 658
 */
class QRADIOLINK_API demod_dpmr : public gr::hier_block2
{
public:
    typedef std::shared_ptr<demod_dpmr> sptr;

    /*!
     * \brief Make a dPMR demodulator block
     *
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 1000000)
     */
    static sptr make(int sps = 125, int samp_rate = 1000000);

protected:
    demod_dpmr(const std::string& name,
              gr::io_signature::sptr input_signature,
              gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_DPMR_H */

