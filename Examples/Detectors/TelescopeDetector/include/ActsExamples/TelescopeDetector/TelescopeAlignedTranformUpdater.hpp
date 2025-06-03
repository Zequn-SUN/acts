// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "ActsAlignment/Kernel/Alignment.hpp"
#include "ActsExamples/TelescopeDetector/TelescopeDetectorElement.hpp"
#include "ActsExamples/TelescopeDetector/TelescopeGeometryContext.hpp"

namespace ActsExamples::Telescope {

inline ActsAlignment::AlignedTransformUpdater alignedTransformUpdater =
    ActsAlignment::AlignedTransformUpdater([](Acts::DetectorElementBase*
                                                  detElement,
                                              const Acts::GeometryContext& gctx,
                                              const Acts::Transform3&
                                                  aTransform) -> bool {
      auto* alignedDetElement =
          dynamic_cast<ActsExamples::Telescope::TelescopeDetectorElement*>(
              detElement);
      const ActsExamples::Telescope::TelescopeGeometryContext* telescopeGtx =
          gctx.maybeGet<ActsExamples::Telescope::TelescopeGeometryContext>();
      if (alignedDetElement && telescopeGtx != nullptr) {
        telescopeGtx->setAlignmentStore(
            alignedDetElement->identifier(),
            std::make_unique<Acts::Transform3>(aTransform));
        return true;
      }
      return false;
    });
}
