import FWCore.ParameterSet.Config as cms

algorithms = cms.VPSet()

l1tGTAlgoBlockProducer = cms.EDProducer(
    "L1GTAlgoBlockProducer",
    algorithms = algorithms
)
