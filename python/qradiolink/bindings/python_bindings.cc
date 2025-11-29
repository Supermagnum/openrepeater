/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define PY_ARRAY_UNIQUE_SYMBOL QRADIOLINK_ARRAY_API
#include <numpy/arrayobject.h>

namespace py = pybind11;

void bind_mod_2fsk(py::module&);
void bind_mod_4fsk(py::module&);
void bind_mod_am(py::module&);
void bind_mod_gmsk(py::module&);
void bind_mod_bpsk(py::module&);
void bind_mod_ssb(py::module&);
void bind_mod_qpsk(py::module&);
// void bind_mod_nbfm(py::module&);  // Commented out - mod_nbfm_impl.cc requires missing emphasis.h
// void bind_mod_dsss(py::module&);  // Commented out - mod_dsss_impl.cc requires missing dsss_encoder_bb_impl.h
void bind_mod_m17(py::module&);
void bind_mod_dmr(py::module&);
void bind_demod_2fsk(py::module&);
void bind_demod_am(py::module&);
void bind_demod_ssb(py::module&);
// void bind_demod_wbfm(py::module&);  // Commented out - demod_wbfm_impl.cc requires missing emphasis.h
// void bind_demod_nbfm(py::module&);  // Commented out - demod_nbfm_impl.cc requires missing emphasis.h
void bind_demod_bpsk(py::module&);
void bind_demod_qpsk(py::module&);
void bind_demod_gmsk(py::module&);
void bind_demod_4fsk(py::module&);
// void bind_demod_dsss(py::module&);  // Commented out - demod_dsss_impl.cc requires missing dsss_decoder_cc_impl.h
void bind_demod_m17(py::module&);
void bind_m17_deframer(py::module&);

// We need this hack because import_array() returns NULL
// for newer Python versions.
// This function is also necessary because it ensures access to the C API
// and removes a warning.
void* init_numpy()
{
    import_array();
    return NULL;
}

PYBIND11_MODULE(qradiolink_python, m)
{
    // Initialize the numpy C API
    // (otherwise we will see segmentation faults)
    init_numpy();

    // Ensure GNU Radio's Python module is loaded first
    // This registers hier_block2 and other base types that our bindings need
    py::module::import("gnuradio.gr");

    m.doc() = "QRadioLink GNU Radio blocks";

    // Bind blocks
    bind_mod_2fsk(m);
    bind_mod_4fsk(m);
    bind_mod_am(m);
    bind_mod_gmsk(m);
    bind_mod_bpsk(m);
    bind_mod_ssb(m);
    bind_mod_qpsk(m);
    // bind_mod_nbfm(m);  // Commented out - mod_nbfm_impl.cc requires missing emphasis.h
    // bind_mod_dsss(m);  // Commented out - mod_dsss_impl.cc requires missing dsss_encoder_bb_impl.h
    bind_mod_m17(m);
    bind_mod_dmr(m);
    bind_demod_2fsk(m);
    bind_demod_am(m);
    bind_demod_ssb(m);
    // bind_demod_wbfm(m);  // Commented out - demod_wbfm_impl.cc requires missing emphasis.h
    // bind_demod_nbfm(m);  // Commented out - demod_nbfm_impl.cc requires missing emphasis.h
    bind_demod_bpsk(m);
    bind_demod_qpsk(m);
    bind_demod_gmsk(m);
    bind_demod_4fsk(m);
    // bind_demod_dsss(m);  // Commented out - demod_dsss_impl.cc requires missing dsss_decoder_cc_impl.h
    bind_demod_m17(m);
    bind_m17_deframer(m);
}

