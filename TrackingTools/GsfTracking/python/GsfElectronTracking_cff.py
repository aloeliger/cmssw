import FWCore.ParameterSet.Config as cms

from RecoParticleFlow.PFTracking.trackerDrivenElectronSeeds_cff import *
from RecoEgamma.EgammaElectronProducers.ecalDrivenElectronSeeds_cfi import *
from RecoParticleFlow.PFTracking.mergedElectronSeeds_cfi import *

electronSeedsTask = cms.Task(trackerDrivenElectronSeeds,ecalDrivenElectronSeeds,electronMergedSeeds) 
electronSeeds = cms.Sequence(electronSeedsTask)
_electronSeedsTaskFromMultiCl = electronSeedsTask.copy()
_electronSeedsTaskFromMultiCl.add(ecalDrivenElectronSeedsFromMultiCl,electronMergedSeedsFromMultiCl)
_electronSeedsFromMultiCl = cms.Sequence(_electronSeedsTaskFromMultiCl)

from Configuration.Eras.Modifier_phase2_hgcal_cff import phase2_hgcal
phase2_hgcal.toReplaceWith(
  electronSeeds, _electronSeedsFromMultiCl )


from TrackingTools.GsfTracking.CkfElectronCandidateMaker_cff import *
from TrackingTools.GsfTracking.GsfElectronGsfFit_cff import *
electronGsfTracking = cms.Sequence(electronSeeds*electronCkfTrackCandidates*electronGsfTracks)
_electronGsfTracking = electronGsfTracking.copy()
_electronGsfTracking += cms.Sequence(electronCkfTrackCandidatesFromMultiCl*electronGsfTracksFromMultiCl)

from Configuration.Eras.Modifier_phase2_hgcal_cff import phase2_hgcal
phase2_hgcal.toReplaceWith(
  electronGsfTracking, _electronGsfTracking
)

from SimTracker.TrackAssociation.trackTimeValueMapProducer_cfi import trackTimeValueMapProducer
gsfTrackTimeValueMapProducer = trackTimeValueMapProducer.clone(trackSrc = cms.InputTag('electronGsfTracks'))

electronGsfTrackingWithTiming = cms.Sequence(electronGsfTracking.copy()*gsfTrackTimeValueMapProducer)

from Configuration.Eras.Modifier_phase2_timing_cff import phase2_timing
phase2_timing.toReplaceWith(electronGsfTracking, electronGsfTrackingWithTiming)
