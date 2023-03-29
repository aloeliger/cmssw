import FWCore.ParameterSet.Config as cms
from L1Trigger.Phase2L1GT.l1tGTSingleInOutLUT import ONE_OVER_ISO_LUT
from L1Trigger.Phase2L1GT.l1tGTScales import scale_parameter

l1tGTSingleObjectCond = cms.EDFilter(
    "L1GTSingleObjectCond",
    scales=scale_parameter,
    one_over_iso_lut=ONE_OVER_ISO_LUT.config(),
)
