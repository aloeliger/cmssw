import FWCore.ParameterSet.Config as cms
from HeterogeneousCore.CUDACore.SwitchProducerCUDA import SwitchProducerCUDA
from Configuration.ProcessModifiers.gpu_cff import gpu


from RecoLocalCalo.EcalRecProducers.ecalUncalibRecHitPhase2_cfi import ecalUncalibRecHitPhase2 as _ecalUncalibRecHitPhase2
ecalUncalibRecHitPhase2 = SwitchProducerCUDA(
  cpu = _ecalUncalibRecHitPhase2.clone()
)

# cpu weights
ecalUncalibRecHitPhase2Task = cms.Task(ecalUncalibRecHitPhase2)

# conditions used on gpu


from RecoLocalCalo.EcalRecProducers.ecalPh2DigiToGPUProducer_cfi import ecalPh2DigiToGPUProducer as _ecalPh2DigiToGPUProducer
ecalPh2DigiToGPUProducer = _ecalPh2DigiToGPUProducer.clone()

# gpu weights
from RecoLocalCalo.EcalRecProducers.ecalUncalibRecHitPhase2GPU_cfi import ecalUncalibRecHitPhase2GPU as _ecalUncalibRecHitPhase2GPU
ecalUncalibRecHitPhase2GPU = _ecalUncalibRecHitPhase2GPU.clone(
  digisLabelEB = cms.InputTag('ecalPh2DigiToGPUProducer', 'ebDigis')
)

# copy the uncalibrated rechits from GPU to CPU
from RecoLocalCalo.EcalRecProducers.ecalCPUUncalibRecHitProducer_cfi import ecalCPUUncalibRecHitProducer as _ecalCPUUncalibRecHitProducer
ecalMultiFitUncalibRecHitSoAnew = _ecalCPUUncalibRecHitProducer.clone(
  recHitsInLabelEB = cms.InputTag('ecalUncalibRecHitPhase2GPU', 'EcalUncalibRecHitsEB'),
  produceEE = cms.bool(False)
)


# convert the uncalibrated rechits from SoA to legacy format
from RecoLocalCalo.EcalRecProducers.ecalUncalibRecHitConvertGPU2CPUFormat_cfi import ecalUncalibRecHitConvertGPU2CPUFormat as _ecalUncalibRecHitConvertGPU2CPUFormat
gpu.toModify(ecalUncalibRecHitPhase2,
  cuda = _ecalUncalibRecHitConvertGPU2CPUFormat.clone(
  recHitsLabelGPUEB = cms.InputTag('ecalMultiFitUncalibRecHitSoAnew', 'EcalUncalibRecHitsEB'),
  produceEE = cms.bool(False)
    )
)

gpu.toReplaceWith(ecalUncalibRecHitPhase2Task, cms.Task(
  # convert phase2 digis to GPU SoA
  ecalPh2DigiToGPUProducer, 
  # ECAL weights running on GPU
  ecalUncalibRecHitPhase2GPU,
  # copy the uncalibrated rechits from GPU to CPU
  ecalMultiFitUncalibRecHitSoAnew,
  # ECAL multifit running on CPU, or convert the uncalibrated rechits from SoA to legacy format
  ecalUncalibRecHitPhase2,
))
