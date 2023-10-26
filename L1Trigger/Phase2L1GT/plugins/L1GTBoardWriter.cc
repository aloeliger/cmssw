/**
 * BoardDataWriter for validation with hardware. Currently only writing the algo bits is implemented.
 **/

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "L1Trigger/DemonstratorTools/interface/BoardDataWriter.h"
#include "L1Trigger/DemonstratorTools/interface/utilities.h"

#include "FWCore/Utilities/interface/EDGetToken.h"

#include "DataFormats/L1Trigger/interface/P2GTAlgoBlock.h"

#include "ap_int.h"

#include <vector>
#include <algorithm>
#include <string>

using namespace l1t;

class L1GTBoardWriter : public edm::one::EDAnalyzer<> {
public:
  explicit L1GTBoardWriter(const edm::ParameterSet&);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  const std::vector<unsigned int> channels_;
  const std::vector<unsigned long long> algoBitMask_;
  const edm::EDGetTokenT<P2GTAlgoBlockCollection> algoBlocksToken_;
  l1t::demo::BoardDataWriter boardDataWriter_;

  std::map<l1t::demo::LinkId, std::vector<ap_uint<64>>> linkData_;
  std::size_t tmuxCounter_;
};

L1GTBoardWriter::L1GTBoardWriter(const edm::ParameterSet& config)
    : channels_(config.getParameter<std::vector<unsigned int>>("channels")),
      algoBitMask_(config.getParameter<std::vector<unsigned long long>>("algoBitMask")),
      algoBlocksToken_(consumes<P2GTAlgoBlockCollection>(config.getParameter<edm::InputTag>("algoBlocksTag"))),
      boardDataWriter_(
          l1t::demo::parseFileFormat(config.getParameter<std::string>("patternFormat")),
          config.getParameter<std::string>("outputFilename"),
          config.getParameter<std::string>("outputFileExtension"),
          9,
          2,
          config.getParameter<unsigned int>("maxLines"),
          [](const std::vector<unsigned int>& channels) {
            l1t::demo::BoardDataWriter::ChannelMap_t channelMap;
            for (unsigned int channel : channels) {
              channelMap.insert({l1t::demo::LinkId{"Algos", channel}, {l1t::demo::ChannelSpec{2, 0, 0}, {channel}}});
            }
            return channelMap;
          }(channels_)),
      linkData_(),
      tmuxCounter_(0) {}

void L1GTBoardWriter::analyze(const edm::Event& event, const edm::EventSetup& iSetup) {
  const P2GTAlgoBlockCollection& algoBlocks = event.get(algoBlocksToken_);

  auto algoBlockIt = algoBlocks.begin();
  auto algoMaskIt = algoBitMask_.begin();

  for (unsigned int channel : channels_) {
    if (tmuxCounter_ == 0) {
      linkData_[{"Algos", channel}] = std::vector<ap_uint<64>>(18, 0);
    }

    for (std::size_t word = 0; word < 9; word++) {
      ap_uint<64> mask = algoMaskIt != algoBitMask_.end() ? *algoMaskIt++ : ~static_cast<unsigned long long>(0);

      for (std::size_t idx = 0; idx < 64 && algoBlockIt != algoBlocks.end(); idx++) {
        linkData_[{"Algos", channel}][word + tmuxCounter_ * 9].set(
            idx, algoBlockIt->decisionBeforeBxMaskAndPrescale() && mask.bit(idx));
        algoBlockIt++;
      }
    }
  }

  if (tmuxCounter_ == 1) {
    boardDataWriter_.addEvent(l1t::demo::EventData(linkData_));
  }

  tmuxCounter_ = (tmuxCounter_ + 1) % 2;
}

void L1GTBoardWriter::endJob() {
  if (tmuxCounter_ == 1) {
    boardDataWriter_.addEvent(l1t::demo::EventData(linkData_));
  }

  boardDataWriter_.flush();
}

void L1GTBoardWriter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("outputFilename");
  desc.add<std::string>("outputFileExtension", "txt");
  desc.add<edm::InputTag>("algoBlocksTag");
  desc.add<std::vector<unsigned int>>("channels");
  desc.add<std::vector<unsigned long long>>("algoBitMask", {});
  desc.add<unsigned int>("maxLines", 1024);
  desc.add<std::string>("patternFormat", "EMPv2");

  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(L1GTBoardWriter);
