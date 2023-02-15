from __future__ import print_function
from __future__ import absolute_import
from builtins import range
import FWCore.ParameterSet.Config as cms

# Parameters for runType
import FWCore.ParameterSet.VarParsing as VarParsing
import sys
import fnmatch
from .dqmPythonTypes import *

# part of the runTheMatrix magic
from Configuration.Applications.ConfigBuilder import filesFromDASQuery

options = VarParsing.VarParsing("analysis")

options.register(
    "runkey",
    "pp_run",
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.string,
    "Run Keys of CMS"
)

# Parameter for frontierKey
options.register('runUniqueKey',
    'InValid',
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.string,
    "Unique run key from RCMS for Frontier")

options.register('runNumber',
                 362720,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Run number. This run number has to be present in the dataset configured with the dataset option.")

options.register('maxLumi',
                 2000,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Only lumisections up to maxLumi are processed.")

options.register('minLumi',
                 1,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Only lumisections starting from minLumi are processed.")

options.register('lumiPattern',
                 '*0',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Only lumisections with numbers matching lumiPattern are processed.")

options.register('dataset',
                 'auto',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Dataset name like '/ExpressPhysicsPA/PARun2016D-Express-v1/FEVT', or 'auto' to guess it with a DAS query. A dataset_cfi.py that defines 'readFiles' and 'secFiles' (like a DAS Python snippet) will override this, to avoid DAS queries.")

options.register('noDB',
                 True, # default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "Don't upload the BeamSpot conditions to the DB")

options.parseArguments()

source = cms.Source ("PoolSource",fileNames = cms.untracked.vstring("/store/data/Run2022G/EphemeralHLTPhysics0/RAW/v1/000/362/720/00000/36f350d4-8e8a-4e38-b399-77ad9bf351dc.root"))
#source = cms.Source ("PoolSource",fileNames = readFiles, secondaryFileNames = secFiles, lumisToProcess = lumirange)
maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1000)
    #input = cms.untracked.int32(-1)
)

# Fix to allow scram to compile
#if len(sys.argv) > 1:
#  options.parseArguments()

runType = RunType()
if not options.runkey.strip():
    options.runkey = "pp_run"

runType.setRunType(options.runkey.strip())
