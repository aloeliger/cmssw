import FWCore.ParameterSet.Config as cms

from Validation.CTPPS.simu_config.year_2016_cff import *

# alignment
from CalibPPS.ESProducers.ctppsRPAlignmentCorrectionsDataESSourceXML_cfi import *
alignmentFile = "Validation/CTPPS/alignment/2016_preTS2.xml"
ctppsRPAlignmentCorrectionsDataESSourceXML.MisalignedFiles = [alignmentFile]
ctppsRPAlignmentCorrectionsDataESSourceXML.RealFiles = [alignmentFile]

# beam optics
from CalibPPS.ESProducers.ctppsOpticalFunctionsESSource_cfi import *

config_2016_preTS2 = cms.PSet(
  validityRange = cms.EventRange("0:min - 999999:max"),

  opticalFunctions = cms.VPSet(
    cms.PSet( xangle = cms.double(185), fileName = cms.FileInPath("CalibPPS/ESProducers/data/optical_functions/2016_preTS2/version2/185urad.root") )
  ),

  scoringPlanes = cms.VPSet(
      # z in cm
      cms.PSet( rpId = cms.uint32(0x76100000), dirName = cms.string("XRPH_C6L5_B2"), z = cms.double(-20382.6) ),  # RP 002, strip
      cms.PSet( rpId = cms.uint32(0x76180000), dirName = cms.string("XRPH_D6L5_B2"), z = cms.double(-21255.1) ),  # RP 003, strip
      cms.PSet( rpId = cms.uint32(0x77100000), dirName = cms.string("XRPH_C6R5_B1"), z = cms.double(+20382.6) ),  # RP 102, strip
      cms.PSet( rpId = cms.uint32(0x77180000), dirName = cms.string("XRPH_D6R5_B1"), z = cms.double(+21255.1) ),  # RP 103, strip
  )
)

ctppsOpticalFunctionsESSource.configuration.append(config_2016_preTS2)

from CalibPPS.ESProducers.ctppsInterpolatedOpticalFunctionsESSource_cfi import *
ctppsInterpolatedOpticalFunctionsESSource.lhcInfoLabel = ""

# aperture cuts
ctppsDirectProtonSimulation.useEmpiricalApertures = True
#ctppsDirectProtonSimulation.empiricalAperture45="([xi]-(0.111 + [xangle] * 0.000E+00))/(127.0 + [xangle] * -0.000)"
#ctppsDirectProtonSimulation.empiricalAperture56="([xi]-(0.138 + [xangle] * 0.000E+00))/(191.6 + [xangle] * -0.000)"

ctppsDirectProtonSimulation.empiricalAperture45="3.13445E-05+(([xi]<0.115758)*0.00975733+([xi]>=0.115758)*0.0228616)*([xi]-0.115758)"
ctppsDirectProtonSimulation.empiricalAperture56="2.0617E-05+(([xi]<0.14324)*0.00475349+([xi]>=0.14324)*0.00629514)*([xi]-0.14324)"


# defaults
def SetDefaults(process):
  UseCrossingAngle(185, process)
