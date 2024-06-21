#ifndef L1Trigger_L1Ntuples_L1AnalysisCaloLayer1DataFormat_h
#define L1Trigger_L1Ntuples_L1AnalysisCaloLayer1DataFormat_h

#include <vector>

namespace L1Analysis {
  struct L1AnalysisCaloLayer1DataFormat {
    L1AnalysisCaloLayer1DataFormat() { Reset(); };
    ~L1AnalysisCaloLayer1DataFormat() {};

    void Reset() {
      nTower = 0;
      ieta.clear();
      iphi.clear();
      iet.clear();
      iem.clear();
      ihad.clear();
      iratio.clear();
      iqual.clear();
      et.clear();
      eta.clear();
      phi.clear();
    }

    void Init() {}

    int nTower;
    std::vector<int> ibx;
    std::vector<int> ieta;
    std::vector<int> iphi;
    std::vector<int> iet;
    std::vector<int> iem;
    std::vector<int> ihad;
    std::vector<int> iratio;
    std::vector<int> iqual;
    std::vector<float> et;
    std::vector<float> eta;
    std::vector<float> phi;
  };
}

#endif
