// This file is part of the Acts project.
//
// Copyright (C) 2021-2022 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Acts/Plugins/Python/Utilities.hpp"
#include "ActsExamples/Alignment/AlignmentAlgorithm.hpp"

#include <bitset>
#include <memory>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace ActsExamples;
using namespace Acts;

namespace Acts::Python {

void addAlignment(Context& ctx) {
  // auto mex = ctx.get("examples");
  auto [m, mex] = ctx.get("main", "examples");
  {
    using Config = ActsExamples::AlignmentAlgorithm::Config;

    auto alg =
        py::class_<ActsExamples::AlignmentAlgorithm, ActsExamples::IAlgorithm,
                   std::shared_ptr<ActsExamples::AlignmentAlgorithm>>(
            mex, "AlignmentAlgorithm")
            .def(py::init<const Config&, Acts::Logging::Level>(),
                 py::arg("config"), py::arg("level"))
            .def_property_readonly("config",
                                   &ActsExamples::AlignmentAlgorithm::config);

    auto c = py::class_<Config>(alg, "Config")
                 .def(py::init<>())
                 .def(
                     "setIterationStateFromInt",
                     [](Config& cfg, const std::map<int, int>& values) {
                       for (const auto& [k, v] : values) {
                         cfg.iterationState[k] = std::bitset<6>(v);
                       }
                     },
                     py::arg("values"))
                 .def_readwrite("alignedTransformUpdater",
                                &Config::alignedTransformUpdater);
    //.def_property("alignedTransformUpdater",
    //		[](Config& cfg) -> ActsAlignment::AlignedTransformUpdater& {
    //		return cfg.alignedTransformUpdater;
    //		},
    //		[](Config& cfg, ActsAlignment::AlignedTransformUpdater updater)
    //{ 		cfg.alignedTransformUpdater = std::move(updater);
    //		});

    ACTS_PYTHON_STRUCT_BEGIN(c, Config);
    ACTS_PYTHON_MEMBER(inputMeasurements);
    // ACTS_PYTHON_MEMBER(inputSourceLinks);
    ACTS_PYTHON_MEMBER(inputProtoTracks);
    ACTS_PYTHON_MEMBER(inputInitialTrackParameters);
    ACTS_PYTHON_MEMBER(outputAlignmentParameters);
    ACTS_PYTHON_MEMBER(align);
    // ACTS_PYTHON_MEMBER(alignedTransformUpdater);
    ACTS_PYTHON_MEMBER(alignedDetElements);
    ACTS_PYTHON_MEMBER(iterationState);
    ACTS_PYTHON_MEMBER(chi2ONdfCutOff);
    ACTS_PYTHON_MEMBER(deltaChi2ONdfCutOff);
    ACTS_PYTHON_MEMBER(maxNumIterations);
    ACTS_PYTHON_MEMBER(maxNumTracks);
    ACTS_PYTHON_MEMBER(m_groups);
    ACTS_PYTHON_MEMBER(trackingGeometry);
    ACTS_PYTHON_MEMBER(magneticField);
    // ACTS_PYTHON_MEMBER(alignContext);
    ACTS_PYTHON_STRUCT_END();
  }
  // ACTS_PYTHON_DECLARE_ALGORITHM(ActsExamples::AlignmentAlgorithm, mex,
  // "AlignmentAlgorithm",inputMeasurements,inputProtoTracks,inputInitialTrackParameters,outputAlignmentParameters,align,alignedTransformUpdater,alignedDetElements,iterationState,chi2ONdfCutOff,deltaChi2ONdfCutOff,maxNumIterations,maxNumTracks,m_groups);
}

}  // namespace Acts::Python
