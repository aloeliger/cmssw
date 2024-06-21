import FWCore.ParameterSet.Config as cms

l1CaloLayer1Tree = cms.EDAnalyzer(
    "L1CaloLayer1TreeProducer",
    regionToken = cms.untracked.InputTag("caloLayer1Digis"),
    towerToken = cms.untracked.InputTag("caloLayer1Digis"),
)

l1CaloLayer1EmuTree = cms.EDAnalyzer(
    "L1CaloLayer1TreeProducer",
    regionToken = cms.untracked.InputTag("simCaloStage2Layer1Digis"),
    towerToken = cms.untracked.InputTag("simCaloStage2Layer1Digis"),
)
