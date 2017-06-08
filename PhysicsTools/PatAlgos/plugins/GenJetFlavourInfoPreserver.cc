/**
  \class    pat::GenJetFlavourInfoPreserver GenJetFlavourInfoPreserver.h "PhysicsTools/JetMCAlgos/interface/GenJetFlavourInfoPreserver.h"
  \brief    Transfers the JetFlavourInfos from the original GenJets to the slimmedGenJets in MiniAOD 
            
  \author   Andrej Saibel, andrej.saibel@cern.ch
*/


#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/JetReco/interface/GenJet.h"

#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/JetReco/interface/JetCollection.h"
#include "SimDataFormats/JetMatching/interface/JetFlavourInfo.h"
#include "SimDataFormats/JetMatching/interface/JetFlavourInfoMatching.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"


namespace pat {

  class GenJetFlavourInfoPreserver : public edm::stream::EDProducer<> {
  public:
    explicit GenJetFlavourInfoPreserver(const edm::ParameterSet & iConfig);
    virtual ~GenJetFlavourInfoPreserver() { }
    
    virtual void produce(edm::Event & iEvent, const edm::EventSetup & iSetup) override;
    
  private:
    const edm::EDGetTokenT<edm::View<reco::GenJet> > genJetsToken_;
    const edm::EDGetTokenT<edm::View<reco::Jet> > slimmedGenJetsToken_;

    const StringCutObjectSelector<reco::GenJet> cut_;
    
    const edm::EDGetTokenT<reco::JetFlavourInfoMatchingCollection> genJetFlavourInfosToken_;
  };

} // namespace

pat::GenJetFlavourInfoPreserver::GenJetFlavourInfoPreserver(const edm::ParameterSet & iConfig) :
    genJetsToken_(consumes<edm::View<reco::GenJet> >(iConfig.getParameter<edm::InputTag>("genJets"))),
    slimmedGenJetsToken_(consumes<edm::View<reco::Jet> >(iConfig.getParameter<edm::InputTag>("slimmedGenJets"))),
    cut_(iConfig.getParameter<std::string>("cut")),
    genJetFlavourInfosToken_(consumes<reco::JetFlavourInfoMatchingCollection>(iConfig.getParameter<edm::InputTag>("genJetFlavourInfos")))
{
    produces<reco::JetFlavourInfoMatchingCollection>();
}

void 
pat::GenJetFlavourInfoPreserver::produce(edm::Event & iEvent, const edm::EventSetup & iSetup) {
    using namespace edm;
    using namespace std;

    Handle<View<reco::GenJet> >      genJets;
    iEvent.getByToken(genJetsToken_, genJets);

    Handle<View<reco::Jet> >      slimmedGenJets;
    iEvent.getByToken(slimmedGenJetsToken_, slimmedGenJets);

    Handle<reco::JetFlavourInfoMatchingCollection> genJetFlavourInfos;
    iEvent.getByToken(genJetFlavourInfosToken_, genJetFlavourInfos);

    auto jetFlavourInfos = std::make_unique<reco::JetFlavourInfoMatchingCollection>(reco::JetRefBaseProd(slimmedGenJets));


    uint slimmedId = 0;

	

    for (View<reco::GenJet>::const_iterator it = genJets->begin(), ed = genJets->end(); it != ed; ++it) {
        if (!cut_(*it)) continue;

        for(reco::JetFlavourInfoMatchingCollection::const_iterator jetInfo  = genJetFlavourInfos->begin(); jetInfo != genJetFlavourInfos->end();++jetInfo){
                
                if((jetInfo - genJetFlavourInfos->begin()) < (it - genJets->begin())) continue;

                else if((jetInfo - genJetFlavourInfos->begin()) > (it - genJets->begin())) continue;

                (*jetFlavourInfos)[slimmedGenJets->refAt(slimmedId)] = reco::JetFlavourInfo(jetInfo->second.getbHadrons(),
                                                                                            jetInfo->second.getcHadrons(),
                                                                                            jetInfo->second.getPartons(), 
                                                                                            jetInfo->second.getLeptons(), 
                                                                                            jetInfo->second.getHadronFlavour(), 
                                                                                            jetInfo->second.getPartonFlavour());

                break;

        }

        slimmedId++; 
    }

    iEvent.put(std::move(jetFlavourInfos));
}

#include "FWCore/Framework/interface/MakerMacros.h"
using namespace pat;
DEFINE_FWK_MODULE(GenJetFlavourInfoPreserver);
