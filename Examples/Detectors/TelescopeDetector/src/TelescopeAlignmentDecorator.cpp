// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ActsExamples/TelescopeDetector/TelescopeAlignmentDecorator.hpp"

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "ActsExamples/Framework/AlgorithmContext.hpp"
#include "ActsExamples/Framework/RandomNumbers.hpp"
#include "ActsExamples/TelescopeDetector/TelescopeDetectorElement.hpp"
#include "ActsExamples/TelescopeDetector/TelescopeGeometryContext.hpp"
#include <Acts/Geometry/TrackingGeometry.hpp>

#include <ostream>
#include <thread>
#include <utility>

ActsExamples::Telescope::TelescopeAlignmentDecorator::
    TelescopeAlignmentDecorator(const Config& cfg,
                                std::unique_ptr<const Acts::Logger> logger)
    : m_cfg(cfg), m_logger(std::move(logger)) {}

ActsExamples::ProcessCode
ActsExamples::Telescope::TelescopeAlignmentDecorator::decorate(
    AlgorithmContext& context) {
  ActsExamples::Telescope::TelescopeGeometryContext telescopeGeoCtx =
      ActsExamples::Telescope::TelescopeGeometryContext(m_cfg.nominal);
  if (!m_cfg.nominal) {
    initializeMisFromJson(m_cfg.misAlignedGeoJsonPath);

    std::unordered_map<std::string, Acts::Transform3> aStore;
    Acts::GeometryContext nominalCtx = {};
    for (auto& ldet : m_cfg.detectorStore) {
      unsigned long long id = ldet->identifier();
      aStore[std::to_string(id)] = ldet->nominalTransform(nominalCtx);
    }
    m_nominalStore = std::move(aStore);

    for (const auto& entry : m_misalignmentAtConstruction) {
      const std::string& identifier = entry.first;
      const Acts::Transform3& misalignmentTransform = entry.second;
      auto nominalIt = m_nominalStore.find(identifier);
      if (nominalIt != m_nominalStore.end()) {
        const Acts::Transform3& nominalTransform = nominalIt->second;
        Eigen::Matrix3d R1 = nominalTransform.rotation();
        Eigen::Vector3d T1 = nominalTransform.translation();
        Eigen::Matrix3d R2 = misalignmentTransform.rotation();
        Eigen::Vector3d T2 = misalignmentTransform.translation();
        Eigen::Matrix3d R3 = R1 * R2;
        Eigen::Vector3d T3 = T1 + T2;
        const Acts::Transform3& TF =
            Eigen::Affine3d(Eigen::Translation3d(T3)) * Eigen::Affine3d(R3);
        m_mistransform[identifier] = TF;
      }
    }
    telescopeGeoCtx.setMisalignmentStore(m_mistransform);
  }
  context.geoContext = telescopeGeoCtx;
  return ProcessCode::SUCCESS;
}
