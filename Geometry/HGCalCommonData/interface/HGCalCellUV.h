#ifndef Geometry_HGCalCommonData_HGCalCellUV_h
#define Geometry_HGCalCommonData_HGCalCellUV_h

#include <cmath>
#include <cstdint>
#include <iterator>
#include <map>
#include "Geometry/HGCalCommonData/interface/HGCalCell.h"

class HGCalCellUV {
public:
  HGCalCellUV(double waferSize, double separation, int32_t nFine, int32_t nCoarse);

  std::pair<int32_t, int32_t> HGCalCellUVFromXY0(double xloc, double yloc, int32_t placement, int32_t type, bool extend, bool debug);

  std::pair<int32_t, int32_t> HGCalCellUVFromXY1(double xloc, double yloc, int32_t placement, int32_t type, bool extend, bool debug);

  std::pair<int32_t, int32_t> HGCalCellUVFromXY2(double xloc, double yloc, int32_t placement, int32_t type, bool extend, bool debug);
  
  std::pair<int32_t, int32_t> HGCalCellUVFromXY3(double xloc, double yloc, int32_t placement, int32_t type, bool extend, bool debug);

private:
  std::pair<int32_t, int32_t> HGCalCellUVFromXY2(double xloc, double yloc, int ncell, double cellX, double cellY, double cellXTotal, double cellYTotal, std::map<std::pair<int, int>, std::pair<double, double> >& cellPos, bool extend, bool debug);

  const double sqrt3_ = std::sqrt(3.0);
  const double sqrt3By2_ = (0.5 * std::sqrt(3.0));

  int32_t ncell_[2];
  double cellX_[2], cellY_[2], cellXTotal_[2], cellYTotal_[2], waferSize;

  std::map<std::pair<int32_t, int32_t>, std::pair<double, double> > cellPosFine_[HGCalCell::cellPlacementTotal],  cellPosCoarse_[HGCalCell::cellPlacementTotal];
};

#endif
