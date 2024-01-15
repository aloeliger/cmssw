#ifndef L1Trigger_Phase2L1GT_L1GTSingleCollectionCut_h
#define L1Trigger_Phase2L1GT_L1GTSingleCollectionCut_h

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "DataFormats/L1Trigger/interface/P2GTCandidate.h"

#include "L1Trigger/Phase2L1GT/interface/L1GTScales.h"

#include "L1GTOptionalParam.h"

#include <functional>
#include <optional>
#include <cstdint>

namespace l1t {

  template <typename T, typename K>
  inline std::vector<T> getParamVector(const std::string& name,
                                       const edm::ParameterSet& config,
                                       std::function<T(K)> conv) {
    const std::vector<K>& values = config.getParameter<std::vector<K>>(name);
    std::vector<T> convertedValues(values.size());
    for (std::size_t i = 0; i < values.size(); i++) {
      convertedValues[i] = conv(values[i]);
    }
    return convertedValues;
  }

  class L1GTSingleCollectionCut {
  public:
    L1GTSingleCollectionCut(const edm::ParameterSet& config,
                            const edm::ParameterSet& lutConfig,
                            const L1GTScales& scales)
        : scales_(scales),
          tag_(config.getParameter<edm::InputTag>("tag")),
          minPt_(getOptionalParam<int, double>(
              "minPt", config, [&scales](double value) { return scales.to_hw_pT(value); })),
          maxPt_(getOptionalParam<int, double>(
              "maxPt", config, [&scales](double value) { return scales.to_hw_pT(value); })),
          minEta_(getOptionalParam<int, double>(
              "minEta", config, [&scales](double value) { return scales.to_hw_eta(value); })),
          maxEta_(getOptionalParam<int, double>(
              "maxEta", config, [&scales](double value) { return scales.to_hw_eta(value); })),
          minPhi_(getOptionalParam<int, double>(
              "minPhi", config, [&scales](double value) { return scales.to_hw_phi(value); })),
          maxPhi_(getOptionalParam<int, double>(
              "maxPhi", config, [&scales](double value) { return scales.to_hw_phi(value); })),
          minZ0_(getOptionalParam<int, double>(
              "minZ0", config, [&scales](double value) { return scales.to_hw_z0(value); })),
          maxZ0_(getOptionalParam<int, double>(
              "maxZ0", config, [&scales](double value) { return scales.to_hw_z0(value); })),
          minScalarSumPt_(getOptionalParam<int, double>(
              "minScalarSumPt", config, [&scales](double value) { return scales.to_hw_pT(value); })),
          maxScalarSumPt_(getOptionalParam<int, double>(
              "maxScalarSumPt", config, [&scales](double value) { return scales.to_hw_pT(value); })),
          minQualityScore_(getOptionalParam<unsigned int>("minQualityScore", config)),
          maxQualityScore_(getOptionalParam<unsigned int>("maxQualityScore", config)),
          qualityFlags_(getOptionalParam<unsigned int>("qualityFlags", config)),
          minAbsEta_(getOptionalParam<int, double>(
              "minAbsEta", config, [&scales](double value) { return scales.to_hw_eta(value); })),
          maxAbsEta_(getOptionalParam<int, double>(
              "maxAbsEta", config, [&scales](double value) { return scales.to_hw_eta(value); })),
          minIsolationPT_(getOptionalParam<int, double>(
              "minRelIsolationPT",
              config,
              [&scales](double value) { return scales.to_hw_isolationPT(value); })),
          maxIsolationPT_(getOptionalParam<int, double>(
              "maxRelIsolationPT",
              config,
              [&scales](double value) { return scales.to_hw_isolationPT(value); })),
          minRelIsolationPT_(getOptionalParam<int, double>(
              "minRelIsolationPT",
              config,
              [&scales](double value) { return scales.to_hw_relative_isolationPT(value); })),
          maxRelIsolationPT_(getOptionalParam<int, double>(
              "maxRelIsolationPT",
              config,
              [&scales](double value) { return scales.to_hw_relative_isolationPT(value); })),
          regionsAbsEtaLowerBounds_(getParamVector<int, double>(
              "regionsAbsEtaLowerBounds", config, [&scales](double value) { return scales.to_hw_eta(value); })),
          regionsMinPt_(getParamVector<int, double>(
              "regionsMinPt", config, [&scales](double value) { return scales.to_hw_pT(value); })),
          regionsMaxRelIsolationPT_(getParamVector<int, double>(
              "regionsMaxRelIsolationPT",
              config,
              [&scales](double value) { return scales.to_hw_relative_isolationPT(value); })),
          regionsQualityFlags_(config.getParameter<std::vector<unsigned int>>("regionsQualityFlags")),
          minPrimVertDz_(getOptionalParam<int, double>(
              "minPrimVertDz", config, [&scales](double value) { return scales.to_hw_z0(value); })),
          maxPrimVertDz_(getOptionalParam<int, double>(
              "maxPrimVertDz", config, [&scales](double value) { return scales.to_hw_z0(value); })),
          primVertex_(getOptionalParam<unsigned int>("primVertex", config)) {}

    bool checkObject(const P2GTCandidate& obj) const {
      bool result = true;

      result &= minPt_ ? (obj.hwPT() > minPt_) : true;
      result &= maxPt_ ? (obj.hwPT() < maxPt_) : true;

      result &= minEta_ ? (obj.hwEta() > minEta_) : true;
      result &= maxEta_ ? (obj.hwEta() < maxEta_) : true;

      result &= minPhi_ ? (obj.hwPhi() > minPhi_) : true;
      result &= maxPhi_ ? (obj.hwPhi() < maxPhi_) : true;

      result &= minZ0_ ? (obj.hwZ0() > minZ0_) : true;
      result &= maxZ0_ ? (obj.hwZ0() < maxZ0_) : true;

      result &= minAbsEta_ ? (abs(obj.hwEta()) > minAbsEta_) : true;
      result &= maxAbsEta_ ? (abs(obj.hwEta()) < maxAbsEta_) : true;

      result &= minScalarSumPt_ ? (obj.hwSca_sum() > minScalarSumPt_) : true;
      result &= maxScalarSumPt_ ? (obj.hwSca_sum() < minScalarSumPt_) : true;

      result &= minQualityScore_ ? (obj.hwQualityScore() > minQualityScore_) : true;
      result &= maxQualityScore_ ? (obj.hwQualityScore() < maxQualityScore_) : true;
      result &= qualityFlags_ ? (obj.hwQualityFlags().to_uint() & qualityFlags_.value()) == qualityFlags_ : true;

      result &= minIsolationPT_ ? (obj.hwSca_sum() > minScalarSumPt_) : true;
      result &= maxIsolationPT_ ? (obj.hwSca_sum() < minScalarSumPt_) : true;

      result &= minRelIsolationPT_ ? obj.hwIsolationPT().to_int64() << scales_.isolation_shift() >
                                         static_cast<int64_t>(minRelIsolationPT_.value()) * obj.hwPT().to_int64()
                                   : true;
      result &= maxRelIsolationPT_ ? obj.hwIsolationPT().to_int64() << scales_.isolation_shift() <
                                         static_cast<int64_t>(maxRelIsolationPT_.value()) * obj.hwPT().to_int64()
                                   : true;

      result &= regionsAbsEtaLowerBounds_.empty() ? true : checkEtaDependentCuts(obj);
      return result;
    }

    bool checkPrimaryVertices(const P2GTCandidate& obj, const P2GTCandidateCollection& primVertCol) const {
      if (!minPrimVertDz_ && !maxPrimVertDz_) {
        return true;
      } else {
        if (!primVertex_) {
          throw cms::Exception("Configuration")
              << "When a min/max primary vertex cut is set the primary vertex to cut\n"
              << "on has to be specified with with config parameter \'primVertex\'. ";
        }
        if (primVertex_ < primVertCol.size()) {
          uint32_t dZ = abs(obj.hwZ0() - primVertCol[primVertex_.value()].hwZ0());

          bool result = true;
          result &= minPrimVertDz_ ? dZ > minPrimVertDz_ : true;
          result &= maxPrimVertDz_ ? dZ < maxPrimVertDz_ : true;
          return result;
        } else {
          return false;
        }
      }
    }

    static void fillPSetDescription(edm::ParameterSetDescription& desc) {
      desc.add<edm::InputTag>("tag");
      desc.addOptional<double>("minPt");
      desc.addOptional<double>("maxPt");
      desc.addOptional<double>("minEta");
      desc.addOptional<double>("maxEta");
      desc.addOptional<double>("minPhi");
      desc.addOptional<double>("maxPhi");
      desc.addOptional<double>("minZ0");
      desc.addOptional<double>("maxZ0");
      desc.addOptional<double>("minScalarSumPt");
      desc.addOptional<double>("maxScalarSumPt");
      desc.addOptional<unsigned int>("minQualityScore");
      desc.addOptional<unsigned int>("maxQualityScore");
      desc.addOptional<unsigned int>("qualityFlags");
      desc.add<std::vector<unsigned int>>("regions", {});
      desc.addOptional<double>("minAbsEta");
      desc.addOptional<double>("maxAbsEta");
      desc.addOptional<double>("minIsolationPT");
      desc.addOptional<double>("maxIsolationPT");
      desc.addOptional<double>("minRelIsolationPT");
      desc.addOptional<double>("maxRelIsolationPT");
      desc.add<std::vector<double>>("regionsAbsEtaLowerBounds", {});
      desc.add<std::vector<double>>("regionsMinPt", {});
      desc.add<std::vector<double>>("regionsMaxRelIsolationPT", {});
      desc.add<std::vector<unsigned int>>("regionsQualityFlags", {});
      desc.addOptional<double>("minPrimVertDz");
      desc.addOptional<double>("maxPrimVertDz");
      desc.addOptional<unsigned int>("primVertex");
    }

    const edm::InputTag& tag() const { return tag_; }

  private:
    bool checkEtaDependentCuts(const P2GTCandidate& obj) const {
      bool res = true;

      unsigned int index;
      index = atIndex(obj.hwEta());
      res &= regionsMinPt_.empty() ? true : obj.hwPT() > regionsMinPt_[index];
      res &= regionsMaxRelIsolationPT_.empty()
                 ? true
                 : obj.hwIsolationPT().to_int64() << scales_.isolation_shift() <
                       static_cast<int64_t>(regionsMaxRelIsolationPT_[index]) * obj.hwPT().to_int64();
      res &= regionsQualityFlags_.empty()
                 ? true
                 : (obj.hwQualityFlags().to_uint() & regionsQualityFlags_[index]) == regionsQualityFlags_[index];
      return res;
    }

    unsigned int atIndex(int objeta) const {
      // Function that checks at which index the eta of the object is
      // If object abs(eta) < regionsAbsEtaLowerBounds_[0] the function returns the last index,
      // Might be undesired behaviour
      for (unsigned int i = 0; i < regionsAbsEtaLowerBounds_.size() - 1; i++) {
        if (std::abs(objeta) >= regionsAbsEtaLowerBounds_[i] && std::abs(objeta) < regionsAbsEtaLowerBounds_[i + 1]) {
          return i;
        }
      }
      return regionsAbsEtaLowerBounds_.size() - 1;
    }

  private:
    const L1GTScales scales_;
    const edm::InputTag tag_;
    const std::optional<int> minPt_;
    const std::optional<int> maxPt_;
    const std::optional<int> minEta_;
    const std::optional<int> maxEta_;
    const std::optional<int> minPhi_;
    const std::optional<int> maxPhi_;
    const std::optional<int> minZ0_;
    const std::optional<int> maxZ0_;
    const std::optional<int> minScalarSumPt_;
    const std::optional<int> maxScalarSumPt_;
    const std::optional<unsigned int> minQualityScore_;
    const std::optional<unsigned int> maxQualityScore_;
    const std::optional<unsigned int> qualityFlags_;
    const std::optional<int> minAbsEta_;
    const std::optional<int> maxAbsEta_;
    const std::optional<int> minIsolationPT_;
    const std::optional<int> maxIsolationPT_;
    const std::optional<int> minRelIsolationPT_;
    const std::optional<int> maxRelIsolationPT_;
    const std::vector<int> regionsAbsEtaLowerBounds_;
    const std::vector<int> regionsMinPt_;
    const std::vector<int> regionsMaxRelIsolationPT_;
    const std::vector<unsigned int> regionsQualityFlags_;
    const std::optional<int> minPrimVertDz_;
    const std::optional<int> maxPrimVertDz_;
    const std::optional<unsigned int> primVertex_;
  };

}  // namespace l1t

#endif  // L1Trigger_Phase2L1GT_L1GTSingleCollectionCut_h
