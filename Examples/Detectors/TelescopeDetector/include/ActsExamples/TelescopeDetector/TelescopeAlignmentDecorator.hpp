// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsExamples/Framework/AlgorithmContext.hpp"
#include "ActsExamples/Framework/IContextDecorator.hpp"
#include "ActsExamples/Framework/ProcessCode.hpp"
#include "ActsExamples/Framework/RandomNumbers.hpp"
#include "ActsExamples/TelescopeDetector/BuildTelescopeDetector.hpp"
#include "ActsExamples/TelescopeDetector/TelescopeDetectorElement.hpp"
#include <Acts/Geometry/TrackingGeometry.hpp>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace ActsExamples {
struct AlgorithmContext;

namespace Telescope {

class TelescopeAlignmentDecorator : public IContextDecorator {
 public:
  using DetectorStore = std::vector<
      std::shared_ptr<ActsExamples::Telescope::TelescopeDetectorElement>>;
  struct Config {
    // whether use the nominal geometry
    bool nominal = true;
    // path of Json file which is used to store the misalignment matrix of each
    // detector element
    // @todo use `JsonMisalignmentConfig`
    std::string misAlignedGeoJsonPath = "telescope-misalignment-matrix.json";
    // tracking geometry
    DetectorStore detectorStore = {};
    std::vector<double> positions{{0, 30, 60, 120, 150, 180}};
    std::vector<double> stereos{{0, 0, 0, 0, 0, 0}};
    std::array<double, 2> offsets{{0, 0}};
    std::array<double, 2> bounds{{25, 100}};
    double thickness{};
    ActsExamples::Telescope::TelescopeSurfaceType surfaceType;
    Acts::BinningValue binValue;
  };

  TelescopeAlignmentDecorator(const Config& cfg,
                              std::unique_ptr<const Acts::Logger> logger =
                                  Acts::getDefaultLogger("AlignmentDecorator",
                                                         Acts::Logging::INFO));
  ~TelescopeAlignmentDecorator() override = default;
  ProcessCode decorate(AlgorithmContext& context) override;
  const std::string& name() const override { return m_name; }

 private:
  Config m_cfg;                                  ///< the configuration class
  std::unique_ptr<const Acts::Logger> m_logger;  ///!< the logging instance
  const Acts::Logger& logger() const { return *m_logger; }
  std::string m_name = "Aligned Detector";
  std::unordered_map<std::string, Acts::Transform3>
      m_misalignmentAtConstruction;
  std::unordered_map<std::string, Acts::Transform3> m_nominalStore;
  std::unordered_map<std::string, Acts::Transform3> m_mistransform;
  void initializeMisFromJson(const std::string& misAlignedGeoJsonFile);
};

inline void TelescopeAlignmentDecorator::initializeMisFromJson(
    const std::string& misJson) {
  std::ifstream file(misJson);
  if (!file.is_open())
    throw std::runtime_error("Unable to open misalignment json file");
  nlohmann::json jsonData;
  file >> jsonData;
  for (auto& [key, value] : jsonData.items()) {
    if (value.is_array() && value.size() == 6) {
      double x = value[0].get<double>();
      double y = value[1].get<double>();
      double z = value[2].get<double>();
      double alpha = value[3].get<double>() / 180 * M_PI;
      double beta = value[4].get<double>() / 180 * M_PI;
      double gamma = value[5].get<double>() / 180 * M_PI;
      Acts::Transform3 translation =
          Eigen::Affine3d(Eigen::Translation3d(x, y, z));
      Acts::Transform3 delta_rotationx =
          Eigen::Affine3d(Eigen::AngleAxisd(alpha, Eigen::Vector3d::UnitX()));
      Acts::Transform3 delta_rotationy =
          Eigen::Affine3d(Eigen::AngleAxisd(beta, Eigen::Vector3d::UnitY()));
      Acts::Transform3 delta_rotationz =
          Eigen::Affine3d(Eigen::AngleAxisd(gamma, Eigen::Vector3d::UnitZ()));
      m_misalignmentAtConstruction[key] =
          translation * delta_rotationx * delta_rotationy * delta_rotationz;
    }
  }
}
}  // namespace Telescope
}  // namespace ActsExamples
