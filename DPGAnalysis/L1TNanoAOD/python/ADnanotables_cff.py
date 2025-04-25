import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.nano_eras_cff import *
from PhysicsTools.NanoAOD.common_cff import *
from PhysicsTools.NanoAOD.l1trig_cff import *

cicadaInputTable = cms.EDProducer(
    'CICADAInputTableProducer',
    regionsSrc = cms.InputTag('caloLayer1Digis'),
    regionsName = cms.string('Regions'),
)

simCicadaInputTable = cms.EDProducer(
    'CICADAInputTableProducer',
    regionsSrc = cms.InputTag('simCaloStage2Layer1Digis'),
    regionsName = cms.string('SimRegions'),
)

cicadaInputTask = cms.Task(
    cicadaInputTable,
    simCicadaInputTable,
)
