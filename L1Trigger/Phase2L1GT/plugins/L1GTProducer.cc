#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/Common/interface/View.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/global/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/L1Trigger/interface/P2GTCandidate.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"

#include "DataFormats/L1Trigger/interface/TkJetWord.h"
#include "DataFormats/L1Trigger/interface/VertexWord.h"

#include "DataFormats/L1TMuonPhase2/interface/SAMuon.h"
#include "DataFormats/L1TMuonPhase2/interface/TrackerMuon.h"

#include "DataFormats/L1TParticleFlow/interface/PFJet.h"
#include "DataFormats/L1TCorrelator/interface/TkEmFwd.h"
#include "DataFormats/L1TCorrelator/interface/TkEm.h"
#include "DataFormats/L1TCorrelator/interface/TkElectronFwd.h"
#include "DataFormats/L1TCorrelator/interface/TkElectron.h"
#include "DataFormats/L1TParticleFlow/interface/PFTau.h"

#include "DataFormats/L1Trigger/interface/EtSum.h"

#include <vector>
#include <array>
#include <string>

using namespace l1t;

class L1GTProducer : public edm::global::EDProducer<> {
public:
  explicit L1GTProducer(const edm::ParameterSet &);
  ~L1GTProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions &);

private:
  void produce(edm::StreamID, edm::Event &, const edm::EventSetup &) const override;

  const edm::EDGetTokenT<TkJetWordCollection> gttPromptJetToken_;
  const edm::EDGetTokenT<TkJetWordCollection> gttDisplacedJetToken_;
  const edm::EDGetTokenT<VertexWordCollection> gttPrimaryVertexToken_;

  const edm::EDGetTokenT<SAMuonCollection> gmtSaPromptMuonToken_;
  const edm::EDGetTokenT<SAMuonCollection> gmtSaDisplacedMuonToken_;
  const edm::EDGetTokenT<TrackerMuonCollection> gmtTkMuonToken_;

  const edm::EDGetTokenT<PFJetCollection> cl2JetToken_;
  const edm::EDGetTokenT<TkEmCollection> cl2PhotonToken_;
  const edm::EDGetTokenT<TkElectronCollection> cl2ElectronToken_;
  const edm::EDGetTokenT<PFTauCollection> cl2TauToken_;
  const edm::EDGetTokenT<std::vector<l1t::EtSum>> cl2EtSumToken_;
  const edm::EDGetTokenT<std::vector<l1t::EtSum>> cl2HtSumToken_;
};

L1GTProducer::L1GTProducer(const edm::ParameterSet &config)
    : gttPromptJetToken_(consumes<TkJetWordCollection>(config.getParameter<edm::InputTag>("GTTPromptJets"))),
      gttDisplacedJetToken_(consumes<TkJetWordCollection>(config.getParameter<edm::InputTag>("GTTDisplacedJets"))),
      gttPrimaryVertexToken_(consumes<VertexWordCollection>(config.getParameter<edm::InputTag>("GTTPrimaryVert"))),
      gmtSaPromptMuonToken_(consumes<SAMuonCollection>(config.getParameter<edm::InputTag>("GMTSaPromptMuons"))),
      gmtSaDisplacedMuonToken_(consumes<SAMuonCollection>(config.getParameter<edm::InputTag>("GMTSaDisplacedMuons"))),
      gmtTkMuonToken_(consumes<TrackerMuonCollection>(config.getParameter<edm::InputTag>("GMTTkMuons"))),
      cl2JetToken_(consumes<PFJetCollection>(config.getParameter<edm::InputTag>("CL2Jets"))),
      cl2PhotonToken_(consumes<TkEmCollection>(config.getParameter<edm::InputTag>("CL2Photons"))),
      cl2ElectronToken_(consumes<TkElectronCollection>(config.getParameter<edm::InputTag>("CL2Electrons"))),
      cl2TauToken_(consumes<PFTauCollection>(config.getParameter<edm::InputTag>("CL2Taus"))),
      cl2EtSumToken_(consumes<std::vector<l1t::EtSum>>(config.getParameter<edm::InputTag>("CL2EtSum"))),
      cl2HtSumToken_(consumes<std::vector<l1t::EtSum>>(config.getParameter<edm::InputTag>("CL2HtSum"))) {
  produces<P2GTCandidateCollection>("GTTPromptJets");
  produces<P2GTCandidateCollection>("GTTDisplacedJets");
  produces<P2GTCandidateCollection>("GTTPrimaryVert");

  produces<P2GTCandidateCollection>("GMTSaPromptMuons");
  produces<P2GTCandidateCollection>("GMTSaDisplacedMuons");
  produces<P2GTCandidateCollection>("GMTTkMuons");

  produces<P2GTCandidateCollection>("CL2Jets");
  produces<P2GTCandidateCollection>("CL2Photons");
  produces<P2GTCandidateCollection>("CL2Electrons");
  produces<P2GTCandidateCollection>("CL2Taus");
  produces<P2GTCandidateCollection>("CL2EtSum");
  produces<P2GTCandidateCollection>("CL2HtSum");
}

void L1GTProducer::fillDescriptions(edm::ConfigurationDescriptions &description) {
  edm::ParameterSetDescription desc;
  desc.addOptional<edm::InputTag>("GTTPromptJets");
  desc.addOptional<edm::InputTag>("GTTDisplacedJets");
  desc.addOptional<edm::InputTag>("GTTPrimaryVert");

  desc.addOptional<edm::InputTag>("GMTSaPromptMuons");
  desc.addOptional<edm::InputTag>("GMTSaDisplacedMuons");
  desc.addOptional<edm::InputTag>("GMTTkMuons");

  desc.addOptional<edm::InputTag>("CL2Jets");
  desc.addOptional<edm::InputTag>("CL2Photons");
  desc.addOptional<edm::InputTag>("CL2Electrons");
  desc.addOptional<edm::InputTag>("CL2Taus");
  desc.addOptional<edm::InputTag>("CL2EtSum");
  desc.addOptional<edm::InputTag>("CL2HtSum");

  description.addWithDefaultLabel(desc);
}

template <typename T, std::size_t MAX_COLLECTION_SIZE = 12, P2GTCandidate::ObjectType type = P2GTCandidate::Undefined>
static void produceByToken(const std::string &productName, const edm::EDGetTokenT<T> &token, edm::Event &event) {
  std::unique_ptr<P2GTCandidateCollection> outputCollection = std::make_unique<P2GTCandidateCollection>();
  edm::Handle<T> inputCollection;
  if (event.getByToken(token, inputCollection)) {
    std::size_t idx = 0;
    for (const auto &object : *inputCollection) {
      if (idx >= MAX_COLLECTION_SIZE) {
        break;
      }

      if constexpr (type != P2GTCandidate::Undefined) {
        outputCollection->emplace_back(P2GTCandidate(object, type));
      } else {
        outputCollection->emplace_back(P2GTCandidate(object));
      }
      ++idx;
    }
  }

  event.put(std::move(outputCollection), productName);
}

static void produceCl2HtSum(const std::string &productName,
                            const edm::EDGetTokenT<std::vector<l1t::EtSum>> &token,
                            edm::Event &event) {
  std::unique_ptr<P2GTCandidateCollection> outputCollection = std::make_unique<P2GTCandidateCollection>();
  edm::Handle<std::vector<EtSum>> htCollection = event.getHandle(token);
  outputCollection->emplace_back(P2GTCandidate(htCollection->at(0), htCollection->at(1)));
  event.put(std::move(outputCollection), productName);
}

void L1GTProducer::produce(edm::StreamID, edm::Event &event, const edm::EventSetup &setup) const {
  produceByToken<TkJetWordCollection, 12, P2GTCandidate::GTTPromptJets>("GTTPromptJets", gttPromptJetToken_, event);
  produceByToken<TkJetWordCollection, 12, P2GTCandidate::GTTDisplacedJets>(
      "GTTDisplacedJets", gttDisplacedJetToken_, event);
  produceByToken<VertexWordCollection, 10>("GTTPrimaryVert", gttPrimaryVertexToken_, event);

  produceByToken<SAMuonCollection, 12, P2GTCandidate::GMTSaPromptMuons>(
      "GMTSaPromptMuons", gmtSaPromptMuonToken_, event);
  produceByToken<SAMuonCollection, 12, P2GTCandidate::GMTSaDisplacedMuons>(
      "GMTSaDisplacedMuons", gmtSaDisplacedMuonToken_, event);
  produceByToken<TrackerMuonCollection>("GMTTkMuons", gmtTkMuonToken_, event);

  produceByToken<PFJetCollection>("CL2Jets", cl2JetToken_, event);
  produceByToken<TkEmCollection>("CL2Photons", cl2PhotonToken_, event);
  produceByToken<PFTauCollection>("CL2Taus", cl2TauToken_, event);
  produceByToken<TkElectronCollection>("CL2Electrons", cl2ElectronToken_, event);
  produceByToken<std::vector<EtSum>, 1>("CL2EtSum", cl2EtSumToken_, event);
  produceCl2HtSum("CL2HtSum", cl2HtSumToken_, event);
}

DEFINE_FWK_MODULE(L1GTProducer);
