// This file is part of the ACTS project.
//
// Copyright (C) 2016-2024 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/GeometryContext.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

using namespace Acts;

namespace ActsExamples::Telescope {

class TelescopeDetectorElement;

/// @class GeometryContext
///
/// @brief Telescope specific geometry context for alignment handling
///
/// Extends the base GeometryContext to provide Telescope-specific alignment
/// capabilities. The context can be active or inactive, controlling whether
/// alignment corrections should be applied.
///
/// @note This context is specifically designed to work with TelescopeDetectorElement
/// and provides contextual transformations for alignment purposes.
///
class TelescopeGeometryContext : public GeometryContext {
 public:
  using AlignmentStore = std::unordered_map<std::string, Acts::Transform3>;

  /// Default constructor
  TelescopeGeometryContext() = default;

  /// Constructor
  explicit TelescopeGeometryContext(bool isGeometryNominal)
      : m_nominal(isGeometryNominal) {}

  /// The transform of this detector element within the given context
  ///
  /// @param dElement The detector element
  ///
  /// @return The transform of the detector element
  const Acts::Transform3& contextualTransform(
      const TelescopeDetectorElement& dElement) const;

  void setMisalignmentStore(
      std::unordered_map<std::string, Acts::Transform3> alignmentStore);

  /// @brief  Return the active status of the context
  /// @return boolean that indicates if the context is active
  bool isNominal() const { return m_nominal; }
  bool isAligned() const { return m_aligned; }
  void setAligned(bool flag) const { m_aligned = flag; }

  void setAlignmentStore(
      unsigned long long id,
      std::unique_ptr<Acts::Transform3> alignedTransform) const {
    m_alignmentStore[std::to_string(id)] = *alignedTransform;
    this->setAligned(1);
  }

  AlignmentStore getMisalignmentStore() const { return m_misalignmentStore; }

 private:
  mutable unsigned int iov = 0;
  mutable int m_iteration = 0;
  bool m_nominal = true;
  AlignmentStore m_misalignmentStore = {};
  mutable bool m_aligned = false;
  mutable AlignmentStore m_alignmentStore = {};
};

}  // namespace ActsExamples::Telescope
