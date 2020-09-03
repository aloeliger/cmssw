#include "SimG4CMS/Tracker/interface/TrackerG4SimHitNumberingScheme.h"
#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4VTouchable.hh"
#include "G4TouchableHistory.hh"
#include "G4VSensitiveDetector.hh"

//#define DEBUG

TrackerG4SimHitNumberingScheme::TrackerG4SimHitNumberingScheme(const GeometricDet& det)
    : alreadySet_(false), geomDet_(&det) {}

void TrackerG4SimHitNumberingScheme::buildAll() {
  if (alreadySet_)
    return;
  alreadySet_ = true;

  G4Navigator* theStdNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
  G4Navigator theNavigator;
  theNavigator.SetWorldVolume(theStdNavigator->GetWorldVolume());

  std::vector<const GeometricDet*> allSensitiveDets;
  geomDet_->deepComponents(allSensitiveDets);
  edm::LogVerbatim("TrackerSimInfoNumbering")
      << " TouchableTo History: got " << allSensitiveDets.size() << " sensitive detectors from GeometricDet.";

  for (auto& theSD : allSensitiveDets) {
    auto const& t = theSD->translation();
    edm::LogVerbatim("TrackerSimInfoNumbering") << "::buildAll" << theSD->geographicalID().rawId() << "\t" << t;
    theNavigator.LocateGlobalPointAndSetup(G4ThreeVector(t.x(), t.y(), t.z()));
    G4TouchableHistory* hist = theNavigator.CreateTouchableHistory();
    assert(!!hist);
    TrackerG4SimHitNumberingScheme::Nav_Story st;
    touchToNavStory(hist, st);

    for (const std::pair<int, std::string>& p : st)
      edm::LogVerbatim("TrackerSimInfoNumbering") << "Nav_Story\t" << p.first << "\t" << p.second;

    directMap_[st] = theSD->geographicalID();

    LogDebug("TrackerSimDebugNumbering") << " INSERTING LV " << hist->GetVolume()->GetLogicalVolume()->GetName()
                                         << " SD: "
                                         << hist->GetVolume()->GetLogicalVolume()->GetSensitiveDetector()->GetName()
                                         << " Now size is " << directMap_.size();
    delete hist;
  }
  edm::LogVerbatim("TrackerSimInfoNumbering")
      << " TrackerG4SimHitNumberingScheme: mapped " << directMap_.size() << " detectors to Geant4.";

  if (directMap_.size() != allSensitiveDets.size()) {
    edm::LogError("TrackerSimInfoNumbering") << " ERROR: GeomDet sensitive detectors do not match Geant4 ones.";
    throw cms::Exception("TrackerG4SimHitNumberingScheme::buildAll")
        << " cannot resolve structure of tracking sensitive detectors";
  }
}

void TrackerG4SimHitNumberingScheme::touchToNavStory(const G4VTouchable* v,
                                                     TrackerG4SimHitNumberingScheme::Nav_Story& st) {
  std::vector<int> debugint;
  std::vector<std::string> debugstring;

  int levels = v->GetHistoryDepth();

  for (int k = 0; k <= levels; ++k) {
    if (v->GetVolume(k)->GetLogicalVolume()->GetName() != "TOBInactive") {
      st.emplace_back(
          std::pair<int, std::string>(v->GetVolume(k)->GetCopyNo(), v->GetVolume(k)->GetLogicalVolume()->GetName()));
      debugint.emplace_back(v->GetVolume(k)->GetCopyNo());
      debugstring.emplace_back(v->GetVolume(k)->GetLogicalVolume()->GetName());
    }
  }

  for (const int& i : debugint)
    edm::LogVerbatim("TrackerSimInfoNumbering") << " G4 TrackerG4SimHitNumberingScheme " << i;
  for (const std::string& s : debugstring)
    edm::LogVerbatim("TrackerSimInfoNumbering") << " " << s;
}

unsigned int TrackerG4SimHitNumberingScheme::g4ToNumberingScheme(const G4VTouchable* v) {
  if (alreadySet_ == false) {
    buildAll();
  }
  TrackerG4SimHitNumberingScheme::Nav_Story st;
  touchToNavStory(v, st);

  dumpG4VPV(v);
  edm::LogVerbatim("TrackerSimInfoNumbering") << " Returning: " << directMap_[st];

  return directMap_[st];
}

void TrackerG4SimHitNumberingScheme::dumpG4VPV(const G4VTouchable* v) {
  int levels = v->GetHistoryDepth();

  edm::LogVerbatim("TrackerSimInfoNumbering") << " NAME : " << v->GetVolume()->GetLogicalVolume()->GetName();
  for (int k = 0; k <= levels; k++) {
    edm::LogVerbatim("TrackerSimInfoNumbering")
        << " Hist: " << v->GetVolume(k)->GetLogicalVolume()->GetName() << " Copy " << v->GetVolume(k)->GetCopyNo();
  }
}
