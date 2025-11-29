/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_DSSS_DSSS_ENCODER_BB_H
#define INCLUDED_DSSS_DSSS_ENCODER_BB_H

#include <gnuradio/sync_block.h>
#include <gnuradio/dsss/api.h>

namespace gr {
namespace dsss {

/*!
 * \brief DSSS Encoder - spreads each input bit using a spreading code
 * \ingroup dsss
 *
 * This block takes unpacked bit input (one bit per byte, 0 or 1) and spreads
 * each bit using a spreading code (e.g., Barker-13). Each input bit produces
 * multiple output chips according to the spreading code length.
 */
class DSSS_API dsss_encoder_bb : public sync_block
{
public:
    typedef std::shared_ptr<dsss_encoder_bb> sptr;

    /*!
     * \brief Make a DSSS encoder block
     *
     * \param spreading_code Vector of spreading code chips (0s and 1s)
     */
    static sptr make(const std::vector<int>& spreading_code);

protected:
    dsss_encoder_bb(const std::string& name,
                    gr::io_signature::sptr input_signature,
                    gr::io_signature::sptr output_signature,
                    const std::vector<int>& spreading_code)
        : sync_block(name, input_signature, output_signature)
    {
        // Base class constructor - implementation in derived class
    }
};

} // namespace dsss
} // namespace gr

#endif /* INCLUDED_DSSS_DSSS_ENCODER_BB_H */

