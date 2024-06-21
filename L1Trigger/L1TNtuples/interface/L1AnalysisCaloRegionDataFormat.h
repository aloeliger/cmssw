#ifndef L1Trigger_L1TNtuples_L1AnalysisCaloRegionDataFormat_h
#define L1Trigger_L1TNtuples_L1AnalysisCaloRegionDataFormat_h

namespace L1Analysis{
  struct L1AnalysisCaloRegionDataFormat {
    L1AnalysisCaloRegionDataFormat() {Reset();}
    ~L1AnalysisCaloRegionDataFormat(){};

    void Reset() {
      for(int i = 0; i < 18; ++i) {
	for(int j = 0; j < 14; ++j) {
	  regionEnergies[i][j] = 0;
	}
      }
    }
    void Init() {}

    unsigned short int regionEnergies [18][14]; //Stored in indices of [iPhi][iEta] (adjusted)
  };
}

#endif
