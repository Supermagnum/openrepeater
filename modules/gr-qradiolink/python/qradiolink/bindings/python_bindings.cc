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
void bind_mod_nbfm(py::module&);
void bind_mod_dsss(py::module&);
void bind_mod_m17(py::module&);
void bind_mod_dmr(py::module&);
void bind_mod_dpmr(py::module&);
void bind_mod_nxdn(py::module&);
void bind_mod_freedv(py::module&);
void bind_mod_mmdvm(py::module&);
void bind_pocsag_encoder(py::module&);
void bind_dstar_encoder(py::module&);
void bind_ysf_encoder(py::module&);
void bind_p25_encoder(py::module&);
void bind_demod_2fsk(py::module&);
void bind_demod_am(py::module&);
void bind_demod_ssb(py::module&);
void bind_demod_wbfm(py::module&);
void bind_demod_nbfm(py::module&);
void bind_demod_bpsk(py::module&);
void bind_demod_qpsk(py::module&);
void bind_demod_gmsk(py::module&);
void bind_demod_4fsk(py::module&);
void bind_demod_dsss(py::module&);
void bind_demod_m17(py::module&);
void bind_demod_dmr(py::module&);
void bind_demod_dpmr(py::module&);
void bind_demod_nxdn(py::module&);
void bind_demod_freedv(py::module&);
void bind_demod_mmdvm_multi(py::module&);
void bind_demod_mmdvm_multi2(py::module&);
void bind_pocsag_decoder(py::module&);
void bind_dstar_decoder(py::module&);
void bind_ysf_decoder(py::module&);
void bind_p25_decoder(py::module&);
void bind_rssi_tag_block(py::module&);
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

    // Ensure GNU Radio's Python modules are loaded first
    // This registers hier_block2 and other base types that our bindings need
    py::module::import("gnuradio.gr");
    // Import vocoder module to register freedv_api types before they're used
    py::module::import("gnuradio.vocoder");

    m.doc() = "QRadioLink GNU Radio blocks";

    // Bind blocks
    // Note: Each bind_* function registers a type with pybind11.
    // If the module is loaded multiple times, this will cause "already registered" errors.
    // This should not happen in normal usage, but can occur if both build and installed
    // versions are in the Python path.
    bind_mod_2fsk(m);
    bind_mod_4fsk(m);
    bind_mod_am(m);
    bind_mod_gmsk(m);
    bind_mod_bpsk(m);
    bind_mod_ssb(m);
    bind_mod_qpsk(m);
    bind_mod_nbfm(m);
    bind_mod_dsss(m);
    bind_mod_m17(m);
    bind_mod_dmr(m);
    bind_mod_dpmr(m);
    bind_mod_nxdn(m);
    bind_mod_freedv(m);
    bind_mod_mmdvm(m);
    bind_pocsag_encoder(m);
    bind_dstar_encoder(m);
    bind_ysf_encoder(m);
    bind_p25_encoder(m);
    bind_demod_2fsk(m);
    bind_demod_am(m);
    bind_demod_ssb(m);
    bind_demod_wbfm(m);
    bind_demod_nbfm(m);
    bind_demod_bpsk(m);
    bind_demod_qpsk(m);
    bind_demod_gmsk(m);
    bind_demod_4fsk(m);
    bind_demod_dsss(m);
    bind_demod_m17(m);
    bind_demod_dmr(m);
    bind_demod_dpmr(m);
    bind_demod_nxdn(m);
    bind_demod_freedv(m);
    bind_demod_mmdvm_multi(m);
    bind_demod_mmdvm_multi2(m);
    bind_pocsag_decoder(m);
    bind_dstar_decoder(m);
    bind_ysf_decoder(m);
    bind_p25_decoder(m);
    bind_rssi_tag_block(m);
    bind_m17_deframer(m);
}

