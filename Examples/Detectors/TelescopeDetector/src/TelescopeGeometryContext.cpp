// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ActsExamples/TelescopeDetector/TelescopeGeometryContext.hpp"

#include "ActsExamples/TelescopeDetector/TelescopeDetectorElement.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

using namespace Acts;

const Acts::Transform3&
ActsExamples::Telescope::TelescopeGeometryContext::contextualTransform(
    const TelescopeDetectorElement& dElement) const {
  if (!this->isNominal() && !this->isAligned()) {
    auto it = m_misalignmentStore.find(std::to_string(dElement.identifier()));
    if (it != m_misalignmentStore.end())
      return it->second;
    else
      return dElement.nominalTransform(TelescopeGeometryContext());
  } else if (!this->isNominal() && this->isAligned()) {
    auto it = m_alignmentStore.find(std::to_string(dElement.identifier()));
    if (it != m_alignmentStore.end())
      return it->second;
    else
      return dElement.nominalTransform(TelescopeGeometryContext());
  } else
    return dElement.nominalTransform(TelescopeGeometryContext());
}

void ActsExamples::Telescope::TelescopeGeometryContext::setMisalignmentStore(
    std::unordered_map<std::string, Transform3> alignmentStore) {
  m_misalignmentStore = alignmentStore;
}
