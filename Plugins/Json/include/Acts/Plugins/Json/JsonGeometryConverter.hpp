// This file is part of the Acts project.
//
// Copyright (C) 2017-2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Material/ISurfaceMaterial.hpp"
#include "Acts/Material/IVolumeMaterial.hpp"
#include "Acts/Material/MaterialProperties.hpp"
#include "Acts/Plugins/Json/lib/json.hpp"
#include "Acts/Utilities/BinUtility.hpp"
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Utilities/Logger.hpp"

// Convenience shorthand
using json = nlohmann::json;

namespace Acts {

  /// @class JsonGeometryConverter
  ///
  /// @brief read the material from Json
  class JsonGeometryConverter
  {

  public:

    using SurfaceMaterialMap
        = std::map<GeometryID, std::shared_ptr<const ISurfaceMaterial>>;

    using VolumeMaterialMap
        = std::map<GeometryID, std::shared_ptr<const IVolumeMaterial>>;

    using DetectorMaterialMaps = std::pair<SurfaceMaterialMap, VolumeMaterialMap>;

    using geo_id_value = uint64_t;

    using SurfaceMaterialRep
        = std::map<geo_id_value, const ISurfaceMaterial*>;
    using VolumeMaterialRep
        = std::map<geo_id_value, const IVolumeMaterial*>;

    /// @brief Layer representation for Json writing
    struct LayerRep
    {
      // the layer id
      GeometryID layerID;

      SurfaceMaterialRep      sensitives;
      SurfaceMaterialRep      approaches;
      const ISurfaceMaterial* representing = nullptr;

      /// The LayerRep is actually worth it to write out
      operator bool() const
      {
        return (sensitives.size() != 0 or approaches.size() != 0
        or representing != nullptr);
      }
    };

    /// @brief Volume representation for Json writing
    struct VolumeRep
    {
      // The geometry id
      GeometryID volumeID;

      /// The namne
      std::string volumeName;

      std::map<geo_id_value, LayerRep> layers;
      SurfaceMaterialRep               boundaries;
      const IVolumeMaterial*           material = nullptr;

      /// The VolumeRep is actually worth it to write out
      operator bool() const
      {
        return (layers.size() != 0 or boundaries.size() != 0
        or material != nullptr);
      }
    };

    /// @brief Detector representation for Json writing
    struct DetectorRep
    {
      std::map<geo_id_value, VolumeRep> volumes;
    };

    /// @class Config
    /// Configuration of the Reader
    class Config
    {
    public:
      /// The geometry version
      std::string geoversion = "undefined";
      /// The detector tag
      std::string detkey = "detector";
      /// The volume identification string
      std::string volkey = "volumes";
      /// The name identification
      std::string namekey = "name";
      /// The boundary surface string
      std::string boukey = "boundaries";
      /// The layer identification string
      std::string laykey = "layers";
      /// The volume material string
      std::string matkey = "material";
      /// The approach identification string
      std::string appkey = "approach";
      /// The sensitive identification string
      std::string senkey = "sensitive";
      /// The representing idntification string
      std::string repkey = "representing";
      /// The bin keys
      std::string bin0key = "bin0";
      /// The bin1 key
      std::string bin1key = "bin1";
      /// The type key -> proto, else
      std::string typekey = "type";
      /// The data key
      std::string datakey = "data";
      /// The geoid key
      std::string geoidkey = "geoid";
      /// The default logger
      std::shared_ptr<const Logger> logger;
      /// The name of the writer
      std::string name = "";

      /// Steering to handle sensitive data
      bool processSensitives = true;
      /// Steering to handle approach data
      bool processApproaches = true;
      /// Steering to handle representing data
      bool processRepresenting = true;
      /// Steering to handle boundary data
      bool processBoundaries = true;
      /// Steering to handle volume data
      bool processVolumes = true;
      /// Write out data
      bool writeData = true;

      /// Constructor
      ///
      /// @param lname Name of the writer tool
      /// @param lvl The output logging level
      Config(const std::string&   lname = "JsonGeometryConverter",
             Logging::Level lvl   = Logging::INFO)
        : logger(getDefaultLogger(lname, lvl)), name(lname)
      {
      }
    };

    /// Constructor
    ///
    /// @param cfg configuration struct for the reader
    JsonGeometryConverter(const Config& cfg);

    /// Destructor
    ~JsonGeometryConverter() = default;

    /// Convert method
    ///
    /// @param surfaceMaterialMap The indexed material map collection
    std::pair<std::map<GeometryID, std::shared_ptr<const ISurfaceMaterial>>,
              std::map<GeometryID, std::shared_ptr<const IVolumeMaterial>>>
    jsonToMaterialMaps(const json& materialmaps);

    /// Convert method
    ///
    /// @param surfaceMaterialMap The indexed material map collection
    json
    materialMapsToJson(const DetectorMaterialMaps& maps);

    /// Write method
    ///
    /// @param tGeometry is the tracking geometry which contains the material
    json
    trackingGeometryToJson(const TrackingGeometry& tGeometry);

  private:
    /// Convert to internal representation method, recursive call
    ///
    /// @param tGeometry is the tracking geometry which contains the material
    void
    convertToRep(DetectorRep& detRep, const TrackingVolume& tVolume);

    /// Convert to internal representation method
    ///
    /// @param tGeometry is the tracking geometry which contains the material
    LayerRep
    convertToRep(const Layer& tLayer);

    /// Create the Surface Material from Json
    /// - factory method, ownership given
    /// @param material is the json part representing a material object
    const ISurfaceMaterial*
    jsonToSurfaceMaterial(const json& material);

    /// Create the Material Matrix from Json
    ///
    /// @param data is the json part representing a material data array
    MaterialPropertiesMatrix
    jsonToMaterialMatrix(const json& data);

    /// Create the BinUtility for from Json
    BinUtility
    jsonToBinUtility(const json& bin);

    /// Create Json from a detector represenation
    json
    detectorRepToJson(const DetectorRep& detRep);

    /// SurfaceMaterial to Json
    ///
    /// @param the SurfaceMaterial
    json
    surfaceMaterialToJson(const ISurfaceMaterial& sMaterial);

    /// The config class
    Config m_cfg;

    /// Private access to the logging instance
    const Logger&
    logger() const
    {
      return *m_cfg.logger;
    }
  };

}  // namespace Acts
