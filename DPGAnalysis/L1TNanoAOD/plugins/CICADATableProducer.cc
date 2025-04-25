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

#include "DataFormats/L1CaloTrigger/interface/CICADA.h"

class CICADATableProducer : public edm::stream::EDProducer<> {
public:
  explicit CICADATableProducer(const edm::ParameterSet&);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override {};
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override {};

  const edm::EDGetTokenT<l1t::CICADABxCollection> cicadaToken_;
  const std::string cicadaName_;
};

CICADATableProducer::CICADATableProducer(const edm::ParameterSet& iConfig)
  : cicadaToken_(consumes(iConfig.getParameter<edm::InputTag>("cicadaSrc"))),
    cicadaName_(iConfig.getParameter<std::string>("cicadaName"))
{
  produces<nanoaod::FlatTable>("CICADAScore");
}

void CICADATableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  edm::Handle<l1t::CICADABxCollection> cicadaScore;
  iEvent.getByToken(cicadaToken_, cicadaScore);

  auto CICADATable = std::make_unique<nanoaod::FlatTable>(1, cicadaName_, true);
  CICADATable->addColumnValue<float>("CICADAScore", cicadaScore->at(0, 0), "");

  iEvent.put(std::move(CICADATable), "CICADAScore");
}

void CICADATableProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<edm::InputTag>("cicadaSrc", edm::InputTag{"simCaloStage2Layer1Summary", "CICADAScore"});
  desc.add<string>("cicadaName", "CICADA");

  descriptions.addWithDefaultLabel(desc);
}

DEFINE_FWK_MODULE(CICADATableProducer);
