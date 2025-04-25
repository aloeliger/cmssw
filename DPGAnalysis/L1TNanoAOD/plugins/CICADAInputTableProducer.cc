
#include <memory>
#include <iostream>
#include <string>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/NanoAOD/interface/FlatTable.h"

#include "DataFormats/L1CaloTrigger/interface/L1CaloCollections.h"
#include "DataFormats/L1CaloTrigger/interface/L1CaloRegion.h"

class CICADAInputTableProducer : public edm::stream::EDProducer<> {
public:
  explicit CICADAInputTableProducer(const edm::ParameterSet&);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override {};
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override {};

  const edm::EDGetTokenT<L1CaloRegionCollection> regionsToken_;
  const std::string regionsName_;
};

CICADAInputTableProducer::CICADAInputTableProducer(const edm::ParameterSet& iConfig)
  : regionsToken_(consumes(iConfig.getParameter<edm::InputTag>("regionsSrc"))),
    regionsName_(iConfig.getParameter<std::string>("regionsName"))
{
  produces<nanoaod::FlatTable>("Regions");
}

void CICADAInputTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

  using namespace edm;
  
  edm::Handle<L1CaloRegionCollection> regions;
  iEvent.getByToken(regionsToken_, regions);

  std::vector<int> regionieta;
  std::vector<int> regioniphi;
  std::vector<int> regionet;
  std::vector<unsigned short int> taubit;
  std::vector<unsigned short int> egbit;

  regionieta.reserve(18*14); //18x14 regions. May be better to have a dedicated const
  regioniphi.reserve(18*14);
  regionet.reserve(18*14);
  taubit.reserve(18*14);
  egbit.reserve(18*14);
  
  if (regions.isValid()){
    for (const auto& itr : *(regions.product())) {
      regionieta.emplace_back(itr.gctEta()-4); //This zero indexes it to CICADA's way of thinking about the regions, but this is NOT strictly the hardware ieta (offset by 4)
      regioniphi.emplace_back(itr.gctPhi());
      regionet.emplace_back(itr.et());
      taubit.emplace_back((unsigned short int) itr.tauVeto());
      egbit.emplace_back((unsigned short int) itr.overFlow());
    }
  }

  auto regionTable = std::make_unique<nanoaod::FlatTable>(18*14, regionsName_, false);
  regionTable->addColumn<int16_t>("iphi", regioniphi, "");
  regionTable->addColumn<int16_t>("ieta", regionieta, "");
  regionTable->addColumn<int16_t>("et", regionet, "");
  regionTable->addColumn<uint8_t>("taubit", taubit, "");
  regionTable->addColumn<uint8_t>("egbit", egbit, "");

  iEvent.put(std::move(regionTable), "Regions");
}

void CICADAInputTableProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<edm::InputTag>("regionsSrc", edm::InputTag{"caloLayer1Digis"});
  desc.add<std::string>("regionsName", "Regions");

  descriptions.addWithDefaultLabel(desc);
}

DEFINE_FWK_MODULE(CICADAInputTableProducer);
