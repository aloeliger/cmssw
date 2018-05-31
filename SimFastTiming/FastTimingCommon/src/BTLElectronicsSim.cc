#include "SimFastTiming/FastTimingCommon/interface/BTLElectronicsSim.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "CLHEP/Random/RandPoissonQ.h"
#include "CLHEP/Random/RandGaussQ.h"


using namespace mtd;

BTLElectronicsSim::BTLElectronicsSim(const edm::ParameterSet& pset) :
  debug_( pset.getUntrackedParameter<bool>("debug",false) ),
  ScintillatorDecayTime_(pset.getParameter<double>("ScintillatorDecayTime")),
  ChannelTimeOffset_(pset.getParameter<double>("ChannelTimeOffset")),
  smearChannelTimeOffset_(pset.getParameter<double>("smearChannelTimeOffset")),
  EnergyThreshold_(pset.getParameter<double>("EnergyThreshold")),
  TimeThreshold1_(pset.getParameter<double>("TimeThreshold1")),
  TimeThreshold2_(pset.getParameter<double>("TimeThreshold2")),
  ReferencePulseNpe_(pset.getParameter<double>("ReferencePulseNpe")),
  Npe_to_pC_( pset.getParameter<double>("Npe_to_pC") ),
  Npe_to_V_( pset.getParameter<double>("Npe_to_V") ),
  adcNbits_( pset.getParameter<uint32_t>("adcNbits") ),
  tdcNbits_( pset.getParameter<uint32_t>("tdcNbits") ),
  adcSaturation_MIP_( pset.getParameter<double>("adcSaturation_MIP") ),
  adcLSB_MIP_( adcSaturation_MIP_/std::pow(2.,adcNbits_) ),
  adcThreshold_MIP_( pset.getParameter<double>("adcThreshold_MIP") ),
  toaLSB_ns_( pset.getParameter<double>("toaLSB_ns")) {    
}


void BTLElectronicsSim::run(const mtd::MTDSimHitDataAccumulator& input,
			    BTLDigiCollection& output,
			    CLHEP::HepRandomEngine *hre) const {
  
  MTDSimHitData chargeColl, toa1, toa2;

  for(MTDSimHitDataAccumulator::const_iterator it=input.begin();
      it!=input.end();
      it++) {
    
    chargeColl.fill(0.f);
    toa1.fill(0.f);
    toa2.fill(0.f);
    for(size_t i=0; i<it->second.hit_info[0].size(); i++) {

      // --- Fluctuate the total number of photo-electrons
      float Npe = CLHEP::RandPoissonQ::shoot(hre, (it->second).hit_info[0][i]);
      if ( Npe < EnergyThreshold_ ) continue;


      // --- Get the time of arrival and add a channel time offset
      float finalToA1 = (it->second).hit_info[1][i] + ChannelTimeOffset_;
      float finalToA2 = (it->second).hit_info[1][i] + ChannelTimeOffset_;
      
      if (  smearChannelTimeOffset_ > 0. ){
	float timeSmearing = CLHEP::RandGaussQ::shoot(hre, 0., smearChannelTimeOffset_);
	finalToA1 += timeSmearing;
	finalToA2 += timeSmearing;
      }


      // --- Calculate and add the time walk
      std::array<float, 3> times = btlPulseShape_.timeAtThr(Npe*Npe_to_V_/ReferencePulseNpe_, 
							    TimeThreshold1_*Npe_to_V_, 
							    TimeThreshold2_*Npe_to_V_);


      // --- If the pulse amplitude is smaller than TimeThreshold2, the trigger does not fire
      if (times[1] == 0.) continue;

      finalToA1 += times[0];
      finalToA2 += times[1];


      // --- Add time smearing due to the photo-electron statistics
      finalToA1 += CLHEP::RandGaussQ::shoot(hre, 0., ScintillatorDecayTime_*sigma_pe(TimeThreshold1_,Npe));
      finalToA2 += CLHEP::RandGaussQ::shoot(hre, 0., ScintillatorDecayTime_*sigma_pe(TimeThreshold2_,Npe));


      while(finalToA1 < 0.f)  finalToA1 += 25.f;
      while(finalToA1 > 25.f) finalToA1 -= 25.f;
      toa1[i]=finalToA1;

      while(finalToA2 < 0.f)  finalToA2 += 25.f;
      while(finalToA2 > 25.f) finalToA2 -= 25.f;
      toa2[i]=finalToA2;

      chargeColl[i] = Npe*Npe_to_pC_;
      
    }

    //run the shaper to create a new data frame
    BTLDataFrame rawDataFrame( it->first );    
    runTrivialShaper(rawDataFrame,chargeColl,toa1,toa2);
    updateOutput(output,rawDataFrame);
    
  }
    
}

  
void BTLElectronicsSim::runTrivialShaper(BTLDataFrame &dataFrame, 
					 const mtd::MTDSimHitData& chargeColl,
					 const mtd::MTDSimHitData& toa1,
					 const mtd::MTDSimHitData& toa2) const {
    bool debug = debug_;
#ifdef EDM_ML_DEBUG  
  for(int it=0; it<(int)(chargeColl.size()); it++) debug |= (chargeColl[it]>adcThreshold_fC_);
#endif
    
  if(debug) edm::LogVerbatim("BTLElectronicsSim") << "[runTrivialShaper]" << std::endl;
  
  //set new ADCs 
  for(int it=0; it<(int)(chargeColl.size()); it++) {

    if ( chargeColl[it] == 0. ) continue;

    //brute force saturation, maybe could to better with an exponential like saturation      
    const uint32_t adc=std::floor( std::min(chargeColl[it],adcSaturation_MIP_) / adcLSB_MIP_ );
    const uint32_t tdc_time1=std::floor( toa1[it] / toaLSB_ns_ );
    const uint32_t tdc_time2=std::floor( toa2[it] / toaLSB_ns_ );
    BTLSample newSample;
    newSample.set(chargeColl[it] > adcThreshold_MIP_,false,tdc_time2,tdc_time1,adc);
    dataFrame.setSample(it,newSample);

    if(debug) edm::LogVerbatim("BTLElectronicsSim") << adc << " (" 
						    << chargeColl[it] << "/" 
						    << adcLSB_MIP_ << ") ";
  }

  if(debug) { 
    std::ostringstream msg;
    dataFrame.print(msg);
    edm::LogVerbatim("BTLElectronicsSim") << msg.str() << std::endl;
  } 
}
  
void BTLElectronicsSim::updateOutput(BTLDigiCollection &coll,
				     const BTLDataFrame& rawDataFrame) const {
  int itIdx(9);
  if(rawDataFrame.size()<=itIdx+2) return;
  
  BTLDataFrame dataFrame( rawDataFrame.id() );
  dataFrame.resize(dfSIZE);
  bool putInEvent(false);
  for(int it=0;it<dfSIZE; ++it) {    
    dataFrame.setSample(it, rawDataFrame[itIdx-2+it]);
    if(it==2) putInEvent = rawDataFrame[itIdx-2+it].threshold(); 
  }

  if(putInEvent) {
    coll.push_back(dataFrame);    
  }
}

float BTLElectronicsSim::sigma_pe(const float& Q, const float& R) const {
  
  float OneOverR = 1./R;
  // --- This is Eq. (17) from Nucl. Instr. Meth. A 564 (2006) 185
  float sigma = OneOverR * sqrt( Q + 2.*Q*(Q+1.)*OneOverR + 
				 Q*(Q+1.)*(6.*Q+11)*OneOverR*OneOverR +
				 Q*(Q+1.)*(Q+2.)*(2.*Q+5.)*OneOverR*OneOverR*OneOverR );
  return sigma;

}



