#include "DataFormats/L1Trigger/interface/P2GTCandidate.h"

#include "DataFormats/L1Trigger/interface/TkJetWord.h"
#include "DataFormats/L1Trigger/interface/VertexWord.h"

#include "DataFormats/L1TMuonPhase2/interface/SAMuon.h"
#include "DataFormats/L1TMuonPhase2/interface/TrackerMuon.h"

#include "DataFormats/L1TParticleFlow/interface/PFJet.h"
#include "DataFormats/L1TCorrelator/interface/TkEm.h"
#include "DataFormats/L1TCorrelator/interface/TkElectron.h"

#include "DataFormats/L1TParticleFlow/interface/gt_datatypes.h"

#include "DataFormats/L1Trigger/interface/EtSum.h"

#include <type_traits>

namespace l1t {
  P2GTCandidate::P2GTCandidate() {}

  P2GTCandidate::P2GTCandidate(const VertexWord& obj)
      : hwZ0_(obj.z0Word().V.to_int() * 5),
        hwQual_(obj.qualityWord().V.to_int()),
        hwSum_pT_pv_(obj.multiplicityWord().V.to_int()),
        hwNumber_of_tracks_in_pv_(obj.multiplicityWord().V.to_int()),
        hwNumber_of_tracks_not_in_pv_(obj.inverseMultiplicityWord().V.to_int()),
        objectType_(GTTPrimaryVert) {}

  P2GTCandidate::P2GTCandidate(const TkJetWord& obj, ObjectType objectType)
      : hwPT_(obj.ptWord().V.to_int()),
        hwPhi_(obj.glbPhiWord().V.to_int()),
        hwEta_(obj.glbEtaWord().V.to_int()),
        hwZ0_(obj.z0Word().V.to_int() << 7),
        hwNumber_of_tracks_(obj.ntWord().V.to_int()),
        objectType_(objectType) {
    static_assert(std::is_same<std::result_of<decltype (&TkJetWord::ptWord)(TkJetWord*)>::type,
                               ap_ufixed<16, 11, AP_TRN, AP_SAT>>::value);
    static_assert(std::result_of<decltype (&TkJetWord::ptWord)(TkJetWord*)>::type::width == 16);
    static_assert(std::result_of<decltype (&TkJetWord::glbPhiWord)(TkJetWord*)>::type::width == 13);
    static_assert(std::result_of<decltype (&TkJetWord::glbEtaWord)(TkJetWord*)>::type::width == 14);
    static_assert(std::result_of<decltype (&TkJetWord::z0Word)(TkJetWord*)>::type::width == 10);
    static_assert(std::result_of<decltype (&TkJetWord::ntWord)(TkJetWord*)>::type::width == 5);
  }

  P2GTCandidate::P2GTCandidate(const SAMuon& obj, ObjectType objectType)
      : hwPT_(obj.hwPt()),
        hwPhi_(obj.hwPhi()),
        hwEta_(obj.hwEta()),
        hwZ0_(obj.hwZ0() << 7),
        hwQual_(obj.hwQual()),
        hwCharge_(obj.hwCharge()),
        hwD0_(obj.hwD0()),
        objectType_(objectType) {}

  P2GTCandidate::P2GTCandidate(const TrackerMuon& obj)
      : hwPT_(obj.hwPt()),
        hwPhi_(obj.hwPhi()),
        hwEta_(obj.hwEta()),
        hwZ0_(obj.hwZ0() << 7),
        hwIso_(obj.hwIso()),
        hwQual_(obj.hwQual()),
        hwCharge_(obj.hwCharge()),
        hwD0_(obj.hwD0()),
        hwBeta_(obj.hwBeta()),
        objectType_(GMTTkMuons) {}

  P2GTCandidate::P2GTCandidate(const PFJet& obj) {
    l1gt::Jet gtJet = l1gt::Jet::unpack(const_cast<PFJet&>(obj).encodedJet());
    hwPT_ = gtJet.v3.pt.V.to_int();
    hwPhi_ = gtJet.v3.phi.V.to_int();
    hwEta_ = gtJet.v3.eta.V.to_int();
    hwZ0_ = gtJet.z0.V.to_int() << 7;
    objectType_ = CL2Jets;
  }

  P2GTCandidate::P2GTCandidate(const TkEm& obj) {
    l1gt::Photon gtPhoton = l1gt::Photon::unpack_ap(const_cast<TkEm&>(obj).egBinaryWord<96>());
    hwPT_ = gtPhoton.v3.pt.V.to_int();
    hwPhi_ = gtPhoton.v3.phi.V.to_int();
    hwEta_ = gtPhoton.v3.eta.V.to_int();
    hwIso_ = gtPhoton.isolation.V.to_int();
    hwQual_ = gtPhoton.quality.V.to_int();
    objectType_ = CL2Photons;
  }

  P2GTCandidate::P2GTCandidate(const TkElectron& obj) {
    l1gt::Electron gtElectron = l1gt::Electron::unpack_ap(const_cast<TkElectron&>(obj).egBinaryWord<96>());
    hwPT_ = gtElectron.v3.pt.V.to_int();
    hwPhi_ = gtElectron.v3.phi.V.to_int();
    hwEta_ = gtElectron.v3.eta.V.to_int();
    hwZ0_ = gtElectron.z0.V.to_int() << 7;
    hwIso_ = gtElectron.isolation.V.to_int();
    hwQual_ = gtElectron.quality.V.to_int();
    hwCharge_ = gtElectron.charge.V.to_int();
    objectType_ = CL2Electrons;
  }

  P2GTCandidate::P2GTCandidate(const EtSum& met) {
    l1gt::Sum sum{true /* valid */, met.pt(), met.phi() / l1gt::Scales::ETAPHI_LSB, 0 /* scalar sum */};

    hwPT_ = sum.vector_pt.V.to_int();
    hwPhi_ = sum.vector_phi.V.to_int();
    hwSca_sum_ = sum.scalar_pt.V.to_int();
    objectType_ = CL2EtSum;
  }

  P2GTCandidate::P2GTCandidate(const EtSum& ht, const EtSum& mht) {
    l1gt::Sum sum{true /* valid */, mht.pt(), mht.phi() / l1gt::Scales::ETAPHI_LSB, ht.pt()};

    hwPT_ = sum.vector_pt.V.to_int();
    hwPhi_ = sum.vector_phi.V.to_int();
    hwSca_sum_ = sum.scalar_pt.V.to_int();
    objectType_ = CL2HtSum;
  }

  bool P2GTCandidate::operator==(const P2GTCandidate& rhs) const {
    return hwPT_ == rhs.hwPT_ && hwPhi_ == rhs.hwPhi_ && hwEta_ == rhs.hwEta_ && hwZ0_ == rhs.hwZ0_ &&
           hwIso_ == rhs.hwIso_ && hwQual_ == rhs.hwQual_ && hwCharge_ == rhs.hwCharge_ && hwD0_ == rhs.hwD0_ &&
           hwBeta_ == rhs.hwBeta_ && hwMass_ == rhs.hwMass_ && hwIndex_ == rhs.hwIndex_ &&
           hwSeed_pT_ == rhs.hwSeed_pT_ && hwSeed_z0_ == rhs.hwSeed_z0_ && hwSca_sum_ == rhs.hwSca_sum_ &&
           hwNumber_of_tracks_ == rhs.hwNumber_of_tracks_ && hwSum_pT_pv_ == rhs.hwSum_pT_pv_ &&
           hwType_ == rhs.hwType_ && hwNumber_of_tracks_in_pv_ == rhs.hwNumber_of_tracks_in_pv_ &&
           hwNumber_of_tracks_not_in_pv_ == rhs.hwNumber_of_tracks_not_in_pv_;
  }

  bool P2GTCandidate::operator!=(const P2GTCandidate& rhs) const {
    return hwPT_ != rhs.hwPT_ && hwPhi_ != rhs.hwPhi_ && hwEta_ != rhs.hwEta_ && hwZ0_ != rhs.hwZ0_ &&
           hwIso_ != rhs.hwIso_ && hwQual_ != rhs.hwQual_ && hwCharge_ != rhs.hwCharge_ && hwD0_ != rhs.hwD0_ &&
           hwBeta_ != rhs.hwBeta_ && hwMass_ != rhs.hwMass_ && hwIndex_ != rhs.hwIndex_ &&
           hwSeed_pT_ != rhs.hwSeed_pT_ && hwSeed_z0_ != rhs.hwSeed_z0_ && hwSca_sum_ != rhs.hwSca_sum_ &&
           hwNumber_of_tracks_ != rhs.hwNumber_of_tracks_ && hwSum_pT_pv_ != rhs.hwSum_pT_pv_ &&
           hwType_ != rhs.hwType_ && hwNumber_of_tracks_in_pv_ != rhs.hwNumber_of_tracks_in_pv_ &&
           hwNumber_of_tracks_not_in_pv_ != rhs.hwNumber_of_tracks_not_in_pv_;
  };
}  // namespace l1t
