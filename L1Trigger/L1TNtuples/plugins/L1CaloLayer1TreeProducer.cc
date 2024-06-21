//Andrew's way of testing the Calo Layer 1 Unpacker
#include <memory>
#include <iostream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/L1CaloTrigger/interface/L1CaloCollections.h"
#include "DataFormats/L1TCalorimeter/interface/CaloTower.h"

#include "L1Trigger/L1TNtuples/interface/L1AnalysisCaloRegionDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisCaloLayer1DataFormat.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TTree.h"

class L1CaloLayer1TreeProducer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public: 
  explicit L1CaloLayer1TreeProducer(const edm::ParameterSet&);
  ~L1CaloLayer1TreeProducer() override;

private:
  //void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  //void endJob() override;

public:
  L1Analysis::L1AnalysisCaloRegionDataFormat* l1CaloRegionData_;
  L1Analysis::L1AnalysisCaloLayer1DataFormat* l1CaloTowerData_;

private:
  const edm::EDGetTokenT<L1CaloRegionCollection> regionToken_;
  const edm::EDGetTokenT<l1t::CaloTowerBxCollection> towerToken_;

  edm::Service<TFileService> fs_;
  TTree* tree_;
};

L1CaloLayer1TreeProducer::L1CaloLayer1TreeProducer(const edm::ParameterSet& iConfig)
  : regionToken_(consumes<L1CaloRegionCollection>(iConfig.getUntrackedParameter<edm::InputTag>("regionToken"))),
    towerToken_(consumes<l1t::CaloTowerBxCollection>(iConfig.getUntrackedParameter<edm::InputTag>("towerToken"))){
  usesResource(TFileService::kSharedResource);

  l1CaloRegionData_ = new L1Analysis::L1AnalysisCaloRegionDataFormat();
  l1CaloTowerData_ = new L1Analysis::L1AnalysisCaloLayer1DataFormat();

  tree_ = fs_->make<TTree>("L1CaloLayer1Tree", "L1CaloLayer1Tree");
  tree_->Branch("CaloRegions", "L1Analysis::L1AnalysisCaloRegionDataFormat", &l1CaloRegionData_, 32000, 3);
  tree_->Branch("CaloTowers", "L1Analysis::L1AnalysisCaloLayer1DataFormat", &l1CaloTowerData_, 32000, 3);
  }


void L1CaloLayer1TreeProducer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  l1CaloRegionData_->Reset();
  l1CaloTowerData_->Reset();
  
  edm::Handle<L1CaloRegionCollection> regions;
  iEvent.getByToken(regionToken_, regions);

  if(regions.isValid()) {
    for(const auto& itr: *regions) {
      l1CaloRegionData_->regionEnergies[itr.gctPhi()][itr.gctEta()-4] = itr.et(); //4 is subtracted off the eta to account for the 4+4 forward/backward HF regions which should be handled separaetely
      //std::cout<<"Ntuplizing region iPhi: "<<itr.gctPhi()<<" iEta: "<<(itr.gctEta()-4)<<std::dec<<std::endl;
    }
  }
  else {
    edm::LogWarning("L1Ntuple")<<"Could not find appropriate Calo Layer 1 Regions Input. It will not be filled";
  }

  edm::Handle<l1t::CaloTowerBxCollection> towers;
  iEvent.getByToken(towerToken_, towers);

  if(towers.isValid()) {
      for (int ibx = towers->getFirstBX(); ibx <= towers->getLastBX(); ++ibx) {
	for(const auto& itr: *towers) {
	  if (itr.hwPt() <= 0)
	    continue;
	  l1CaloTowerData_->ibx.push_back(ibx);

	  l1CaloTowerData_->et.push_back(itr.pt());
	  l1CaloTowerData_->eta.push_back(itr.eta());
	  l1CaloTowerData_->phi.push_back(itr.phi());

	  l1CaloTowerData_->iet.push_back(itr.hwPt());
	  l1CaloTowerData_->ieta.push_back(itr.hwEta());
	  l1CaloTowerData_->iphi.push_back(itr.hwPhi());
	  l1CaloTowerData_->iem.push_back(itr.hwEtEm());
	  l1CaloTowerData_->ihad.push_back(itr.hwEtHad());
	  l1CaloTowerData_->iratio.push_back(itr.hwEtRatio());
	  l1CaloTowerData_->iqual.push_back(itr.hwQual());

	  l1CaloTowerData_->nTower++;
	  
	}
      }

  }
  else {
    edm::LogWarning("L1Ntuples")<<"Could not find appropriate Calo Layer 1 Towers Input. It will not be filled.";
  }
  
  tree_->Fill();
}


L1CaloLayer1TreeProducer::~L1CaloLayer1TreeProducer() {
  delete l1CaloRegionData_;
  delete l1CaloTowerData_;
}

DEFINE_FWK_MODULE(L1CaloLayer1TreeProducer);

