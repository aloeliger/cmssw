import FWCore.ParameterSet.Config as cms

l1CaloSummaryEmuTree = cms.EDAnalyzer(
    "L1CaloSummaryTreeProducer",
    scoreToken = cms.untracked.InputTag("simCaloStage2Layer1Summary", "CICADAScore"),
    regionToken = cms.untracked.InputTag("simCaloStage2Layer1Digis"),
)
