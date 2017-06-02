#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include <FWCore/MessageLogger/interface/MessageLogger.h>

#include <RecoTracker/MeasurementDet/interface/MeasurementTracker.h>
#include <RecoTracker/Record/interface/CkfComponentsRecord.h>
#include "TrackingTools/DetLayers/interface/NavigationSchool.h"
#include "RecoTracker/Record/interface/NavigationSchoolRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "MagneticField/VolumeGeometry/interface/MagVolumeOutsideValidity.h"
#include "DataFormats/GeometrySurface/interface/PlaneBuilder.h"


#include "TrackingTools/GeomPropagators/interface/AnalyticalPropagator.h"
#include "TrackPropagation/RungeKutta/interface/defaultRKPropagator.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"

#include "TrackingTools/KalmanUpdators/interface/Chi2MeasurementEstimator.h"
#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"

#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHit.h"


namespace {
  inline
  Surface::RotationType rotation( const GlobalVector& zDir)
  {
    GlobalVector zAxis = zDir.unit();
    GlobalVector yAxis( zAxis.y(), -zAxis.x(), 0); 
    GlobalVector xAxis = yAxis.cross( zAxis);
    return Surface::RotationType( xAxis, yAxis, zAxis);
  }
  
}


struct MSData {
  int stid;
  int lid;
  float zi;
  float zo;
  float uerr;
  float verr;
};
inline
std::ostream & operator<<(std::ostream & os, MSData d) {
  os <<  d.stid<<'>' <<d.lid <<'|'<<d.zi<<'/'<<d.zo<<':'<<d.uerr<<'/'<<d.verr;
  return os;
}


constexpr unsigned short packLID(unsigned int id, unsigned int od) {  return (id<<8) | od ;}
constexpr std::tuple<unsigned short, unsigned short> unpackLID(unsigned short lid) { return std::make_tuple(lid>>8, lid&255);}



constexpr unsigned int nLmBins() { return 12*10;}                                                                         
constexpr float lmBin() { return 0.1f;}

class MSParam {
public:
  struct Elem {
    float vi;
    float vo;
    float uerr;
    float verr;
  };
  
  using Data = std::unordered_map<unsigned short,std::array<std::vector<Elem>,nLmBins()>>;
  
  Data data;
  
};

inline
std::ostream & operator<<(std::ostream & os, MSParam::Elem d) {
  os <<d.vi<<'/'<<d.vo<<':'<<d.uerr<<'/'<<d.verr;
  return os;
}


MSParam msParam;


class MSGalore final : public edm::EDAnalyzer {
 public:
  explicit MSGalore(const edm::ParameterSet&);
  
  
 private:
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  
  std::string theMeasurementTrackerName;
  std::string theNavigationSchoolName;
};


MSGalore::MSGalore(const edm::ParameterSet& iConfig): 
  theMeasurementTrackerName(iConfig.getParameter<std::string>("measurementTracker"))
  ,theNavigationSchoolName(iConfig.getParameter<std::string>("navigationSchool")){}


void MSGalore::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  
  //get the measurementtracker
  edm::ESHandle<MeasurementTracker> measurementTracker;
  edm::ESHandle<NavigationSchool>   navSchool;
  
  iSetup.get<CkfComponentsRecord>().get(theMeasurementTrackerName, measurementTracker);
  iSetup.get<NavigationSchoolRecord>().get(theNavigationSchoolName, navSchool);
  
  auto const & searchGeom = *(*measurementTracker).geometricSearchTracker();
  
  /*
    auto const & geom = *(TrackerGeometry const *)(*measurementTracker).geomTracker();
    auto const & dus = geom.detUnits(); 
    auto firstBarrel = geom.offsetDU(GeomDetEnumerators::tkDetEnum[1]);
    auto firstForward = geom.offsetDU(GeomDetEnumerators::tkDetEnum[2]);
    std::cout << "number of dets " << dus.size() << std::endl;
    std::cout << "Bl/Fw loc " << firstBarrel<< '/' << firstForward << std::endl;
  */
  
  edm::ESHandle<MagneticField> magfield;
  iSetup.get<IdealMagneticFieldRecord>().get(magfield);
  
  edm::ESHandle<Propagator>             propagatorHandle;
  iSetup.get<TrackingComponentsRecord>().get("PropagatorWithMaterial", propagatorHandle);
  auto const & ANprop = *propagatorHandle;  
  
  // error (very very small)
  ROOT::Math::SMatrixIdentity id;
  AlgebraicSymMatrix55 C(id);
  C *= 1.e-16;
  
  CurvilinearTrajectoryError err(C);
  
  Chi2MeasurementEstimator estimator(30.,-3.0,0.5,2.0,0.5,1.e12);  // same as defauts....
  KFUpdator kfu;
  LocalError he(0.001*0.001,0,0.002*0.002);
  
  /*
    auto bsz = searchGeom.pixelBarrelLayers().size();
    auto fsz = searchGeom.posPixelForwardLayers().size();
    auto nl=bsz+fsz-2;
    DetLayer const * layers[nl];
    for (decltype(bsz) i=0;i<bsz-1;++i) layers[i]=searchGeom.pixelBarrelLayers()[i];
    for (decltype(fsz) i=0;i<fsz-1;++i) layers[i+bsz-1]=searchGeom.posPixelForwardLayers()[i];
  */
  
  
  // loop over lambdas
  bool debug = false;
  float tl=0;
  for (decltype(nLmBins()) ib=0; ib<nLmBins(); ++ib, tl+=lmBin()) {
    
    float pt = 1.0f;
    float tanlmd = tl; // 0.1f;
    auto sinth2 = 1.f/(1.f+tanlmd*tanlmd);
    auto sinth = std::sqrt(sinth2);
    // auto costh = tanlmd*sinth;
    auto overp = sinth/pt; 
    auto pz = pt*tanlmd;
    
    // debug= (tl>2.34f && tl<2.55f);
    if(debug)  std::cout << tl << ' ' << pz << ' ' << 1./overp << std::endl;
    
    
    
    float lastzz=-18;
    // float lastbz=-18;
    bool goFw=false;
    std::string loc=" Barrel";
    for (int iz=0;iz<2; ++iz) {
      if (iz>0) goFw=true;
      for (float zz=lastzz; zz<18.1; zz+=0.2) {
	float z = zz;
	GlobalPoint startingPosition(0,0,z);
	
	constexpr int maxLayers=6;
	
	std::vector<MSData> mserr[maxLayers][maxLayers];
	
	// define propagation from/to
	std::function<void(int, TrajectoryStateOnSurface, DetLayer const *, float, int)>  
        propagate = [&](int from, TrajectoryStateOnSurface tsos, DetLayer const * layer, float z1, int stid) {
	  
	  for (auto il=from+1; il<maxLayers; ++il) {
	    
	    auto compLayers = (*navSchool).nextLayers(*layer,*tsos.freeState(),alongMomentum);
	    std::stable_sort(compLayers.begin(),compLayers.end(),[](auto a, auto  b){return a->seqNum()<b->seqNum();});
	    layer = nullptr;
	    for(auto it : compLayers){
	      if (it->basicComponents().empty()) {
		//this should never happen. but better protect for it
		std::cout <<"a detlayer with no components: I cannot figure out a DetId from this layer. please investigate." << std::endl;
		continue;
	      }
	      if (debug) std::cout << il << (it->isBarrel() ? " Barrel" : " Forward") << " layer " << it->seqNum() << " SubDet " << it->subDetector()<< std::endl;
	      auto const & detWithState = it->compatibleDets(tsos,ANprop,estimator);
	      if(!detWithState.size()) { 
		if(debug) std::cout << "no det on this layer" << it->seqNum() << std::endl; 
		continue;
	      }
	      layer = it;
	      auto did = detWithState.front().first->geographicalId();
	      if (debug) std::cout << "arrived at " << int(did) << std::endl;
	      tsos = detWithState.front().second;
	      if(debug) std::cout << tsos.globalPosition() << ' ' << tsos.localError().positionError() << std::endl;
	      
	      
	      // save errs
	      auto globalError = ErrorFrameTransformer::transform(tsos.localError().positionError(), tsos.surface());
	      auto gp = tsos.globalPosition();
	      float r = gp.perp();
	      float errorPhi = std::sqrt(float(globalError.phierr(gp))); 
	      float errorR = std::sqrt(float(globalError.rerr(gp)));
	      float errorZ = std::sqrt(float(globalError.czz()));
	      float xerr = overp*errorPhi;
	      float zerr = overp*(layer->isBarrel() ? errorZ : errorR); 
	      auto zo = layer->isBarrel() ? gp.z() : r;
	      //  std::cout << tanlmd << ' ' << z1 << ' ' << it->seqNum() << ':' << xerr <<'/'<<zerr << std::endl;    
	      mserr[from][il-1].emplace_back(MSData{stid,it->seqNum(),z1,zo,xerr,zerr});
	      
	      if (from==0) {
	        // spawn prop from this layer
	        // std::cout << tsos.globalPosition() << ' ' << tsos.globalDirection() << ' ' << tsos.localError().positionError() << std::endl;
	        // constrain it to this location (relevant for layer other than very first)
	        SiPixelRecHit::ClusterRef pref;
	        SiPixelRecHit   hitpx(tsos.localPosition(),he,1.,*detWithState.front().first,pref);
	        auto tsosl = kfu.update(tsos, hitpx);
	        // std::cout << tsosl.globalPosition() << ' ' << tsosl.globalDirection() << ' ' << tsosl.localError().positionError() << std::endl;
	        auto z1l = layer->isBarrel() ? tsos.globalPosition().z() : tsos.globalPosition().perp();
	        auto stidl = layer->seqNum();
	        propagate(il,tsosl,layer,z1l, stidl);
              }
	      break;
	    }  // loop on compLayers
	    if (!layer) break;
	    // if (!goFw) lastbz=z1;
	    if(from==0) lastzz=zz;
	    
	  } // layer loop
	  
	  if (debug && mserr[from][from].empty()) {
	    std::cout << "tl " << tanlmd << loc << ' ' <<from<< std::endl;
	    for (auto il=from; il<maxLayers; ++il) { std::cout << il << ' ';
	      for ( auto const & e : mserr[from][il]) std::cout << e<<' '; //  << '-' <<e.uerr*sinth <<'/'<<e.verr*sinth <<' ';
	      std::cout << std::endl;  
	    }
	  }
	}; // propagate
	  
	float phi = 0.0415f;
	for (int ip=0;ip<5; ++ip) {
	  phi += (3.14159f/6.f)/5.f;
	  GlobalVector startingMomentum(pt*std::sin(phi),pt*std::cos(phi),pz);
	  
	  // make TSOS happy
	  //Define starting plane
	  PlaneBuilder pb;
	  auto rot = rotation(startingMomentum);
	  auto startingPlane = pb.plane( startingPosition, rot);
	  
	  TrajectoryStateOnSurface startingStateP( GlobalTrajectoryParameters(startingPosition,
									      startingMomentum, 1, magfield.product()),
						   err, *startingPlane);
	  auto tsos0 = startingStateP;
	  
	  DetLayer const * layer0 = searchGeom.pixelBarrelLayers()[0];
	  if (goFw) layer0 = searchGeom.posPixelForwardLayers()[0];
	    int stid0 = layer0->seqNum();
	    
	    {
	      auto it = layer0;
	      if(debug) std::cout << "first layer " << (it->isBarrel() ? " Barrel" : " Forward") << " layer " << it->seqNum() << " SubDet " << it->subDetector()<< std::endl;
	    }
	    
	    auto const & detWithState = layer0->compatibleDets(tsos0,ANprop,estimator);
	    if(!detWithState.size()) {
	      if(debug) std::cout << "no det on first layer" << layer0->seqNum() << std::endl;
	      continue;
	    }
	    tsos0 = detWithState.front().second;
	    if(debug) std::cout << "arrived at " << int(detWithState.front().first->geographicalId()) << ' ' << tsos0.globalPosition() << ' ' << tsos0.localError().positionError() << std::endl;
	    
	    // for barrel
	    float z1l = tsos0.globalPosition().z();
	    if (goFw) {
	      loc = " Forward";
	      z1l = tsos0.globalPosition().perp();
	    }
	    
	    propagate(0,tsos0,layer0,z1l,stid0);
	    
	} // loop on phi
	
	  // fill
	for (auto from=0; from<maxLayers; ++from)
	  for (auto il=from; il<maxLayers; ++il) {
	    if (mserr[from][il].empty()) continue;
	    auto stid=mserr[from][il].front().stid;     
	    auto lid=mserr[from][il].front().lid;
	    auto zi = mserr[from][il].front().zi;
	    float xerr =0;
	    float zerr =0;
	    float zo=0;
	    for (auto const & e : mserr[from][il]) {
	      if (e.stid!=stid) continue;
	      lid = std::min(lid,e.lid);
	    }
	    float nn=0;
	    for (auto const & e : mserr[from][il]) {
	      if (e.stid!=stid || lid!=e.lid) continue;
	      xerr+=e.uerr; zerr+=e.verr; zo+=e.zo; ++nn;
	    }
	    xerr/=nn; zerr/=nn; zo/=nn;
	    auto iid = packLID(stid,lid);
	      auto & md = msParam.data[iid][ib];
	      if (md.empty()
		  || std::abs(xerr-md.back().uerr)>0.2f*xerr
		  || std::abs(zerr-md.back().verr)>0.2f*zerr
		  ) md.emplace_back(MSParam::Elem{zi,zo,xerr,zerr});
	  } // loop on layers
	
	}} // loop on z
    
    // std::cout << tanlmd << ' ' << lastbz << std::endl;
  } // loop  on tanLa
  
    // sort
  for (auto & e : msParam.data)
    for (auto & d : e.second) std::stable_sort(d.begin(),d.end(),[](auto a,auto  b){return a.vo<b.vo;});
  
  // test MSParam
  { 
    std::cout << "\n\nProduced MSParam" << std::endl;
    unsigned short f,t;
    for (auto const & e : msParam.data) {
      std::tie(f,t) = unpackLID(e.first);
      std::cout << "from/to " << f << "->" << t << std::endl;
      for(auto const & d : e.second) {
	std::cout << d.size() << ' ';
      }
      std::cout << std::endl;
    }
    
    int lst[] = {0,1,29};
    for (auto st : lst)
      for (int il=1; il<5;++il) {
	int ll = st+il;
	if (st==0 && il==5) ll=29;
	auto id = packLID(st,ll);
	std::cout << "from/to " << st << "->" << ll <<std::endl;
	auto const & d = msParam.data[id];  // we should use find
	int lb=0; 
	for(auto const & ld : d) {
	  lb++;
	  if (ld.empty()) continue;
	  std::cout << lb << ": ";
	  for(auto const & e : ld) std::cout << e << ' ';
	  std::cout<<std::endl;
	} 
      }
  }
  
  
  }

//define this as a plug-in
DEFINE_FWK_MODULE(MSGalore);

