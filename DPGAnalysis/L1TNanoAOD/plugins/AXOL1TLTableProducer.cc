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

class AXOL1TLTableProducer : public::edm::stream::EDProducer<> {
public:
  explicit AXOL1TLTableProducer(const edm::ParameterSet&);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override {};
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override {};

  const edm::EDGetTokenT<float> axoToken_;
  const std::string axoName_;
};

AXOL1TLTableProducer::AXOL1TLTableProducer(const edm::ParameterSet& iConfig)
  : axoToken_(consumes(iConfig.getParameter<edm::InputTag>("axoSrc"))),
    axoName_(iConfig.getParameter<std::string>("axoName"))
{
  produces<nanoaod::FlatTable>("AXOScore");
}

void AXOL1TLTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  edm::Handle<float> axoScore;
  iEvent.getByToken(axoToken_, axoScore);
  
  auto axoTable = std::make_unique<nanoaod::FlatTable>(1, axoName_, true);
  axoTable->addColumnValue<float>("AXOScore", *axoScore, "");

  iEvent.put(std::move(axoTable), "AXOScore");
}

void AXOL1TLTableProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<edm::InputTag>("axoSrc", edm::InputTag{"axol1tlProducerv4", "AXOScore"});
  desc.add<std::string>("axoName", "axol1tl_v4");

  descriptions.addWithDefaultLabel(desc);
}

DEFINE_FWK_MODULE(AXOL1TLTableProducer);
