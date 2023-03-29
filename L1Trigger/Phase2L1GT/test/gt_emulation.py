import FWCore.ParameterSet.Config as cms

from Configuration.Eras.Era_Phase2C17I13M9_cff import Phase2C17I13M9

process = cms.Process('L1TEmulation', Phase2C17I13M9)


############################################################
# import standard configurations
############################################################

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.Geometry.GeometryExtended2026D49Reco_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag

process.GlobalTag = GlobalTag(process.GlobalTag, '125X_mcRun4_realistic_v2', '')


process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(0)  # default: 0

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

############################################################
# Source
############################################################

process.source = cms.Source("PoolSource",
                            fileNames=cms.untracked.vstring(
                                '/store/mc/Phase2Fall22DRMiniAOD/MinBias_TuneCP5_14TeV-pythia8/GEN-SIM-DIGI-RAW-MINIAOD/PU200_125X_mcRun4_realistic_v2-v1/30007/017df0e0-4fae-4f31-aae6-2c4915423b0c.root',
                            ),
                            inputCommands = cms.untracked.vstring("keep *","drop l1tTkPrimaryVertexs_L1TkPrimaryVertex_*_*")
)

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(20))

############################################################
# Raw to Digi
############################################################

process.load('Configuration.StandardSequences.RawToDigi_cff')

process.pRawToDigi = cms.Path(process.RawToDigi)

############################################################
# Upstream Emulators
############################################################

process.load('SimCalorimetry.HcalTrigPrimProducers.hcaltpdigi_cff') # needed to read HCal TPs

process.load('L1Trigger.TrackTrigger.TrackTrigger_cff')
process.load("L1Trigger.TrackFindingTracklet.L1HybridEmulationTracks_cff") 
process.load("L1Trigger.TrackerDTC.ProducerES_cff") 
process.load("L1Trigger.TrackerDTC.ProducerED_cff")

process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.StandardSequences.SimL1Emulator_cff')

process.UpstreamEmulators = cms.Task(
    process.TrackerDTCProducer,
    process.TTClustersFromPhase2TrackerDigis,
    process.TTStubsFromPhase2TrackerDigis,
    process.offlineBeamSpot,
    process.l1tTTTracksFromTrackletEmulation,
    process.l1tTTTracksFromExtendedTrackletEmulation,
    process.SimL1EmulatorTask
)
 
process.pUpstreamEmulators = cms.Path(process.UpstreamEmulators)

############################################################
# L1 Global Trigger Emulation
############################################################

# Conditions
from L1Trigger.Phase2L1GT.l1tGTSingleObjectCond_cfi import l1tGTSingleObjectCond
from L1Trigger.Phase2L1GT.l1tGTDoubleObjectCond_cfi import l1tGTDoubleObjectCond
from L1Trigger.Phase2L1GT.l1tGTTripleObjectCond_cfi import l1tGTTripleObjectCond
from L1Trigger.Phase2L1GT.l1tGTQuadObjectCond_cfi import l1tGTQuadObjectCond

from L1Trigger.Phase2L1GT.l1tGTAlgoBlockProducer_cff import algorithms

# Some dummy seeds to test tracker interface
process.DoubleJetCondition = l1tGTDoubleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GTTPromptJets"),
        minPt = cms.double(12)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GTTDisplacedJets"),
        minPt = cms.double(10)
    )
)
process.pDoubleJetCondition = cms.Path(process.DoubleJetCondition)
algorithms.append(cms.PSet(expression = cms.string("pDoubleJetCondition")))

process.TripleJetCondition = l1tGTTripleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GTTPromptJets"),
        minPt = cms.double(50)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GTTPromptJets"),
        minPt = cms.double(40)
    ),
    collection3 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GTTPromptJets"),
        minPt = cms.double(25)
    )
)
process.pTripleJetCondition = cms.Path(process.TripleJetCondition)
algorithms.append(cms.PSet(expression = cms.string("pTripleJetCondition")))

process.tkElePuppiJet = l1tGTDoubleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "CL2Electrons"),
        minPt = cms.double(28),
        minEta = cms.double(-2.1),
        maxEta = cms.double(2.1),
        maxIso = cms.double(0.2)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "CL2Jets"),
        minPt = cms.double(40),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    minDR = cms.double(0.3),
    maxDz = cms.double(1)
)
process.pTkElePuppiJet = cms.Path(process.tkElePuppiJet)
algorithms += [cms.PSet(expression = cms.string("pTkElePuppiJet"))]

# Some seeds from https://twiki.cern.ch/twiki/pub/CMS/PhaseIIL1TriggerMenuTools/L1Menu_emulators12_3_x_060522.pdf

process.SingleTkMuon = l1tGTSingleObjectCond.clone(
    tag =  cms.InputTag("l1tGTProducer", "GMTTkMuons"),
    minPt = cms.double(22),
    minEta = cms.double(-2.4),
    maxEta = cms.double(2.4)
)
process.pSingleTkMuon = cms.Path(process.SingleTkMuon)
algorithms.append(cms.PSet(expression = cms.string("pSingleTkMuon")))

process.DoubleTkMuon = l1tGTDoubleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(15),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(7),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    maxDz = cms.double(1),
)
process.pDoubleTkMuon = cms.Path(process.DoubleTkMuon)
algorithms.append(cms.PSet(expression = cms.string("pDoubleTkMuon")))

process.TripleTkMuon = l1tGTTripleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(5),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(3),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    collection3 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(3),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    delta12 = cms.PSet(
        maxDz = cms.double(1)
    ),
    delta13 = cms.PSet(
        maxDz = cms.double(1)
    ),
    delta23 = cms.PSet(
        maxDz = cms.double(1)
    )
)
process.pTripleTkMuon = cms.Path(process.TripleTkMuon)
algorithms.append(cms.PSet(expression = cms.string("pTripleTkMuon")))

# TODO Some other missing seeds

process.HadSinglePuppiJet = l1tGTSingleObjectCond.clone(
    tag =  cms.InputTag("l1tGTProducer", "CL2Jets"),
    minPt = cms.double(230),
    minEta = cms.double(-2.4),
    maxEta = cms.double(2.4)
)
process.pHadSinglePuppiJet = cms.Path(process.HadSinglePuppiJet)
algorithms.append(cms.PSet(expression = cms.string("pHadSinglePuppiJet")))


process.HadDoublePuppiJet = l1tGTDoubleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "CL2Jets"),
        minPt = cms.double(112),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "CL2Jets"),
        minPt = cms.double(112),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    maxDEta = cms.double(1.6),
)
process.pHadDoublePuppiJet = cms.Path(process.HadDoublePuppiJet)
algorithms.append(cms.PSet(expression = cms.string("pHadDoublePuppiJet")))

# TODO some other seeds

# ET miss seed

process.PuppiMET = l1tGTSingleObjectCond.clone(
    tag = cms.InputTag("l1tGTProducer", "CL2Jets"),
    minPt = cms.double(200)
)
process.pPuppiMET = cms.Path(process.PuppiMET)
algorithms.append(cms.PSet(expression = cms.string("pPuppiMET")))

process.VBFDoublePuppiJet = l1tGTDoubleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "CL2Jets"),
        minPt = cms.double(160),
        minEta = cms.double(-5),
        maxEta = cms.double(5)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "CL2Jets"),
        minPt = cms.double(35),
        minEta = cms.double(-5),
        maxEta = cms.double(5)
    ),
    minInvMass = cms.double(620),
)
process.pVBFDoublePuppiJet = cms.Path(process.VBFDoublePuppiJet)
algorithms.append(cms.PSet(expression = cms.string("pVBFDoublePuppiJet")))


# B-physics seeds from https://twiki.cern.ch/twiki/pub/CMS/PhaseIIL1TriggerMenuTools/L1Menu_emulators12_3_x_060522.pdf
process.doubleTkMuon1 = l1tGTDoubleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(2),
        minEta = cms.double(-1.5),
        maxEta = cms.double(1.5)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(2),
        minEta = cms.double(-1.5),
        maxEta = cms.double(1.5)
    ),
    maxDR = cms.double(1.4),
    maxDz = cms.double(1),
    os = cms.bool(True)
)
process.pDoubleTkMuon1 = cms.Path(process.doubleTkMuon1)
algorithms.append(cms.PSet(expression = cms.string("pDoubleTkMuon1")))

process.doubleTkMuon2 = l1tGTDoubleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(4),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(4),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    maxDR = cms.double(1.2),
    maxDz = cms.double(1),
    os = cms.bool(True)
)
process.pDoubleTkMuon2 = cms.Path(process.doubleTkMuon2)
algorithms.append(cms.PSet(expression = cms.string("pDoubleTkMuon2")))

process.doubleTkMuon3 = l1tGTDoubleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(4.5),
        minEta = cms.double(-2.0),
        maxEta = cms.double(2.0)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(4.5),
        minEta = cms.double(-2.0),
        maxEta = cms.double(2.0)
    ),
    minInvMass = cms.double(7),
    maxInvMass = cms.double(18),
    maxDz = cms.double(1),
    os = cms.bool(True)
)
process.pDoubleTkMuon3 = cms.Path(process.doubleTkMuon3)
algorithms.append(cms.PSet(expression = cms.string("pDoubleTkMuon3")))

process.tripleTkMuon1 = l1tGTTripleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(5),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(3),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    collection3 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(2),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    delta12 = cms.PSet(
        os = cms.bool(True),
        minInvMass = cms.double(0),
        maxInvMass = cms.double(9),
        maxDz = cms.double(1)
    ),
    delta13 = cms.PSet(
        maxDz = cms.double(1)
    ),
    delta23 = cms.PSet(
        maxDz = cms.double(1)
    )
)
process.pTripleTkMuon1 = cms.Path(process.tripleTkMuon1)
algorithms.append(cms.PSet(expression = cms.string("pTripleTkMuon1")))


process.tripleTkMuon2 = l1tGTTripleObjectCond.clone(
    collection1 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(5),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    collection2 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(3),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    collection3 = cms.PSet(
        tag = cms.InputTag("l1tGTProducer", "GMTTkMuons"),
        minPt = cms.double(2),
        minEta = cms.double(-2.4),
        maxEta = cms.double(2.4)
    ),
    delta12 = cms.PSet(
        os = cms.bool(True),
        maxDz = cms.double(1)
    ),
    delta13 = cms.PSet(
        minInvMass = cms.double(5),
        maxInvMass = cms.double(17),
        maxDz = cms.double(1)
    ),
    delta23 = cms.PSet(
        maxDz = cms.double(1)
    )
)
process.pTripleTkMuon2 = cms.Path(process.tripleTkMuon2)
algorithms.append(cms.PSet(expression = cms.string("pTripleTkMuon2")))

############################################################
# Analyzable output
############################################################

process.out = cms.OutputModule("PoolOutputModule",
outputCommands = cms.untracked.vstring('drop *',
        'keep *_l1tGTProducer_*_L1TEmulation',
        'keep l1tP2GTCandidatesl1tP2GTCandidatel1tP2GTCandidatesl1tP2GTCandidateedmrefhelperFindUsingAdvanceedmRefs_*_*_L1TEmulation',
        'keep *_l1tGTAlgoBlockProducer_*_L1TEmulation',
        'keep *_TriggerResults_*_L1TEmulation'
    ),
    fileName=cms.untracked.string("l1t_emulation.root")
)

process.pOut = cms.EndPath(process.out)
