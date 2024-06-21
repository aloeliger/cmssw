import FWCore.ParameterSet.Config as cms

simCaloStage2Digis = cms.EDProducer(
    "L1TStage2Layer2Producer",
    towerToken = cms.InputTag("simCaloStage2Layer1Digis"),
    #towerToken = cms.InputTag("caloStage2Digis","CaloTower"),
    firmware = cms.int32(1),
    useStaticConfig = cms.bool(False)
)
