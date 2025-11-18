/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_STRETCHER_CC_H
#define INCLUDED_QRADIOLINK_STRETCHER_CC_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief CESSB Stretcher for complex signals
 * \ingroup qradiolink
 *
 * This block implements envelope stretching for complex signals,
 * used in CESSB (Clipped-Envelope Single Sideband) processing.
 */
class QRADIOLINK_API stretcher_cc : virtual public gr::block
{
public:
    typedef std::shared_ptr<stretcher_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of qradiolink::stretcher_cc.
     */
    static sptr make();
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_STRETCHER_CC_H */

