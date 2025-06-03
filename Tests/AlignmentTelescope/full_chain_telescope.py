#!/usr/bin/env python3

import os
import argparse
from pathlib import Path
import math

import acts
import acts.examples

from typing import Optional

u = acts.UnitConstants

def update_geometry_flags(typeGeo):
    if typeGeo == 2:
        return False, True, False
    elif typeGeo == 3:
        return False, True, True
    else:
        return True, False, False
    
def runSimulation(
    detector: acts.Detector,
    trackingGeometry: acts.TrackingGeometry,
    field: acts.MagneticFieldProvider,
    outputDir: Path,
    inputParticlePath: Optional[Path] = None,
    inputHitsPath: Optional[Path] = None,
    decorators=[],
    directNavigation=False,
    reverseFilteringMomThreshold=0 * u.GeV,
    s: acts.examples.Sequencer = None,
):
    from acts.examples.simulation import (
        addParticleGun,
        MomentumConfig,
        EtaConfig,
        PhiConfig,
        ParticleConfig,
        ParticleSelectorConfig,
        addPythia8,
        addFatras,
        addGeant4,
        ParticleSelectorConfig,
        addDigitization,
        addParticleSelection,
    )
    from acts.examples.reconstruction import (
        addSeeding,
        SeedingAlgorithm,
        addKalmanTracks,
    )

    s = s or acts.examples.Sequencer(events=1, numThreads=1, logLevel=acts.logging.INFO)

    rnd = acts.examples.RandomNumbers(seed=626)
    outputDir = Path.cwd() / "telescope_simulation"
    if not outputDir.exists():
        outputDir.mkdir()
    postfix="fatras"
    logger = acts.logging.getLogger("Truth tracking example")

    vtxGen = acts.examples.GaussianVertexGenerator(stddev=acts.Vector4(0, 2, 2, 0), mean=acts.Vector4(0, 0, 0, 0))

    addParticleGun(
        s,
        MomentumConfig(
            10 * u.GeV,
            10 * u.GeV,
            transverse=True,
        ),
        EtaConfig(-0.1, 0.1),
        PhiConfig(-2.0 * u.degree, 2.0 * u.degree),
        ParticleConfig(1, acts.PdgParticle.eMuon, True),
        multiplicity=10000,
        vtxGen=vtxGen,
        rnd=rnd,
        outputDirRoot=outputDir / postfix,
        outputDirCsv=outputDir / postfix,
    )

    addFatras(
        s,
        trackingGeometry,
        field,
        rnd=rnd,
        outputDirRoot=outputDir / postfix,
        outputDirCsv=outputDir / postfix,
    )

    addDigitization(
        s,
        trackingGeometry,
        field,
        digiConfigFile="default-smearing-config-telescope.json",
        outputDirRoot=outputDir / postfix,
        outputDirCsv=outputDir / postfix,
        rnd=rnd,
    )

    return s

def runTruthTrackingKalman(
    detector: acts.Detector,
    trackingGeometry: acts.TrackingGeometry,
    field: acts.MagneticFieldProvider,
    outputDir: Path,
    inputParticlePath: Optional[Path] = None,
    inputHitsPath: Optional[Path] = None,
    decorators=[],
    directNavigation=False,
    reverseFilteringMomThreshold=0 * u.GeV,
    s: acts.examples.Sequencer = None,
    typeGeo=1,
):
    from acts.examples.simulation import (
        addParticleGun,
        MomentumConfig,
        EtaConfig,
        PhiConfig,
        ParticleConfig,
        ParticleSelectorConfig,
        addPythia8,
        addFatras,
        addGeant4,
        ParticleSelectorConfig,
        addDigitization,
        addParticleSelection,
    )
    from acts.examples.reconstruction import (
        addSeeding,
        SeedingAlgorithm,
        addKalmanTracks,
    )

    s = s or acts.examples.Sequencer(events=1, numThreads=1, logLevel=acts.logging.INFO)

    nominal=True,
    misaligned=False,
    aligned=False,
    nominal, misaligned, aligned = update_geometry_flags(typeGeo)

    rnd = acts.examples.RandomNumbers(seed=626)
    outputDir = Path.cwd() / "telescope_simulation"
    if not outputDir.exists():
        outputDir.mkdir()
    postfix="fatras"
    logger = acts.logging.getLogger("Truth tracking example")

    s.addReader(
        acts.examples.CsvSimHitReader(
            level=acts.logging.INFO,
            outputSimHits="simhits",
            inputDir=outputDir / postfix,
            inputStem="hits",
        )
    )

    s.addReader(
        acts.examples.CsvMeasurementReader(
            inputDir=outputDir / postfix,
            level=acts.logging.INFO,
            outputMeasurements="measurements",
            outputMeasurementSimHitsMap="measurement_simhits_map",
            outputMeasurementParticlesMap="measurement_particles_map",
            inputSimHits="simhits",
        )
    )

    s.addReader(
        acts.examples.CsvParticleReader(
            inputDir=outputDir / postfix,
            level=acts.logging.INFO,
            inputStem="particles_simulated",
            outputParticles="particles",
        )
    )

    if not nominal:
        for d in decorators:
            s.addContextDecorator(d)

    addParticleSelection(
        s,
        config=ParticleSelectorConfig(
            rho=(0.0, 24 * u.mm),
            absZ=(0.0, 1.0 * u.m),
            eta=(-0.88, 0.88),
            pt=(150 * u.MeV, None),
            removeNeutral=True,
        ),
        inputParticles="particles",
        outputParticles="particles_selected",
    )

    addSeeding(
        s,
        trackingGeometry,
        field,
        rnd=rnd,
        inputParticles="particles_input",
        seedingAlgorithm=SeedingAlgorithm.TruthSmeared,
        initialVarInflation=[1.] * 6,
        particleHypothesis=acts.ParticleHypothesis.muon,
    )

    if not aligned:
        addKalmanTracks(
            s,
            trackingGeometry,
            field,
            directNavigation,
            reverseFilteringMomThreshold,
        )

        s.addAlgorithm(
            acts.examples.TrackSelectorAlgorithm(
                level=acts.logging.INFO,
                inputTracks="tracks",
                outputTracks="selected-tracks",
                selectorConfig=acts.TrackSelector.Config(
                    minMeasurements=5,
                ),
            )
        )
        s.addWhiteboardAlias("tracks", "selected-tracks")

        if nominal:
            s.addWriter(
                acts.examples.RootTrackStatesWriter(
                    level=acts.logging.INFO,
                    inputTracks="tracks",
                    inputParticles="particles_selected",
                    inputTrackParticleMatching="track_particle_matching",
                    inputSimHits="simhits",
                    inputMeasurementSimHitsMap="measurement_simhits_map",
                    filePath=str(outputDir / "trackstates_kf.root"),
                )
            )

            s.addWriter(
                acts.examples.RootTrackSummaryWriter(
                    level=acts.logging.INFO,
                    inputTracks="tracks",
                    inputParticles="particles_selected",
                    inputTrackParticleMatching="track_particle_matching",
                    filePath=str(outputDir / "tracksummary_kf.root"),
                )
            )

            s.addWriter(
                acts.examples.TrackFitterPerformanceWriter(
                    level=acts.logging.INFO,
                    inputTracks="tracks",
                    inputParticles="particles_selected",
                    inputTrackParticleMatching="track_particle_matching",
                    filePath=str(outputDir / "performance_kf.root"),
                )
            )

        else:
            s.addWriter(
                acts.examples.RootTrackStatesWriter(
                    level=acts.logging.INFO,
                    inputTracks="tracks",
                    inputParticles="particles_selected",
                    inputTrackParticleMatching="track_particle_matching",
                    inputSimHits="simhits",
                    inputMeasurementSimHitsMap="measurement_simhits_map",
                    filePath=str(outputDir / "trackstates_kf_misaligned.root"),
                )
            )

            s.addWriter(
                acts.examples.RootTrackSummaryWriter(
                    level=acts.logging.INFO,
                    inputTracks="tracks",
                    inputParticles="particles_selected",
                    inputTrackParticleMatching="track_particle_matching",
                    filePath=str(outputDir / "tracksummary_kf_misaligned.root"),
                )
            )

            s.addWriter(
                acts.examples.TrackFitterPerformanceWriter(
                    level=acts.logging.INFO,
                    inputTracks="tracks",
                    inputParticles="particles_selected",
                    inputTrackParticleMatching="track_particle_matching",
                    filePath=str(outputDir / "performance_kf_misaligned.root"),
                )
            )

    if aligned:
        alignCfg = acts.examples.AlignmentAlgorithm.Config()
        alignCfg.inputMeasurements = "measurements"
        alignCfg.inputProtoTracks = "truth_particle_tracks"
        alignCfg.inputInitialTrackParameters = "estimatedparameters"
        alignCfg.outputAlignmentParameters = "tracks_parameters_aligned"
        alignCfg.trackingGeometry = trackingGeometry;
        alignCfg.magneticField = field;
        alignCfg.maxNumTracks = 10000;
        alignCfg.maxNumIterations = 100;

        Center1    = 1 << 1
        Center2    = 1 << 2
        Rotation2  = 1 << 5
        value = Center1 | Center2 | Rotation2
        alignCfg.setIterationStateFromInt({i: value for i in range(alignCfg.maxNumIterations)})

        tmp = list(alignCfg.alignedDetElements)
        for element in detector.detectorStore:
            if element.identifier() == 3 or element.identifier() == 4 or element.identifier() == 5:
                tmp.append(element)
        alignCfg.alignedDetElements = tmp

        #alignCfg.alignedTransformUpdater = acts.examples.TelescopeDetector.alignedTransformUpdater

        alignmentAlgorithm = acts.examples.AlignmentAlgorithm(
            config=alignCfg,
            level=acts.logging.VERBOSE,#INFO,
        )
        s.addAlgorithm(alignmentAlgorithm)

        addKalmanTracks(
            s,
            trackingGeometry,
            field,
            directNavigation,
            reverseFilteringMomThreshold,
        )

        s.addAlgorithm(
            acts.examples.TrackSelectorAlgorithm(
                level=acts.logging.INFO,
                inputTracks="tracks",
                outputTracks="selected-tracks",
                selectorConfig=acts.TrackSelector.Config(
                    minMeasurements=5,
                ),
            )
        )
        s.addWhiteboardAlias("tracks", "selected-tracks")

        s.addWriter(
            acts.examples.RootTrackStatesWriter(
                level=acts.logging.INFO,
                inputTracks="tracks",
                inputParticles="particles_selected",
                inputTrackParticleMatching="track_particle_matching",
                inputSimHits="simhits",
                inputMeasurementSimHitsMap="measurement_simhits_map",
                filePath=str(outputDir / "trackstates_kf_aligned.root"),
            )
        )

        s.addWriter(
            acts.examples.RootTrackSummaryWriter(
                level=acts.logging.INFO,
                inputTracks="tracks",
                inputParticles="particles_selected",
                inputTrackParticleMatching="track_particle_matching",
                filePath=str(outputDir / "tracksummary_kf_aligned.root"),
            )
        )

        s.addWriter(
            acts.examples.TrackFitterPerformanceWriter(
                level=acts.logging.INFO,
                inputTracks="tracks",
                inputParticles="particles_selected",
                inputTrackParticleMatching="track_particle_matching",
                filePath=str(outputDir / "performance_kf_aligned.root"),
            )
        )

    return s

u = acts.UnitConstants

if "__main__" == __name__:

    detector, trackingGeometry, decorators = acts.examples.TelescopeDetector.create(
        bounds=[200, 200],
        positions=[30, 60, 90, 120, 150, 180, 210, 240, 270],
        stereos=[0, 0, 0, 0, 0, 0, 0, 0, 0],
        binValue=0,
    )
    field = acts.ConstantBField(acts.Vector3(0, 0, 0))#2 * u.T))

    runSimulation(
        detector=detector,
        trackingGeometry=trackingGeometry,
        field=field,
        outputDir=Path.cwd(),
        decorators=decorators,
    ).run()

    runTruthTrackingKalman(
        detector=detector,
        trackingGeometry=trackingGeometry,
        field=field,
        outputDir=Path.cwd(),
        decorators=decorators,
        typeGeo=1,
    ).run()

    runTruthTrackingKalman(
        detector=detector,
        trackingGeometry=trackingGeometry,
        field=field,
        outputDir=Path.cwd(),
        decorators=decorators,
        typeGeo=2,
    ).run()

    runTruthTrackingKalman(
        detector=detector,
        trackingGeometry=trackingGeometry,
        field=field,
        outputDir=Path.cwd(),
        decorators=decorators,
        typeGeo=3,
    ).run()
