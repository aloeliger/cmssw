#include "Geometry/HGCalCommonData/interface/HGCalCellUV.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <iostream>
#include <vector>
#include <algorithm>

HGCalCellUV::HGCalCellUV(double waferSize, double separation, int32_t nFine, int32_t nCoarse) {
  HGCalCell hgcalcell(waferSize, nFine, nCoarse);
  ncell_[0] = nFine;
  ncell_[1] = nCoarse;
  for (int k = 0; k < 2; ++k) {
    cellX_[k] = waferSize / (3 * ncell_[k]);
    cellY_[k] = 0.5 * sqrt3_ * cellX_[k];
    cellXTotal_[k] = (waferSize + separation) / (3 * ncell_[k]);
    cellY_[k] = 0.5 * sqrt3_ * cellXTotal_[k];
  }

  // Fill up the placement vectors
  for (int placement = 0; placement < HGCalCell::cellPlacementTotal; ++placement) {
    // Fine cells
    for (int iu = 0; iu < 2 * ncell_[0]; ++iu) {
      for (int iv = 0; iv < 2 * ncell_[0]; ++iv) {
        int u = (placement < HGCalCell::cellPlacementExtra) ? iv : iu;
        int v = (placement < HGCalCell::cellPlacementExtra) ? iu : iv;
        if (((v - u) < ncell_[0]) && (u - v) <= ncell_[0]) {
          cellPosFine_[placement][std::pair<int, int>(u, v)] = hgcalcell.HGCalCellUV2XY1(u, v, placement, 0);
        }
      }
    }
    // Coarse cells
    for (int iu = 0; iu < 2 * ncell_[1]; ++iu) {
      for (int iv = 0; iv < 2 * ncell_[1]; ++iv) {
        int u = (placement < HGCalCell::cellPlacementExtra) ? iv : iu;
        int v = (placement < HGCalCell::cellPlacementExtra) ? iu : iv;
        if (((v - u) < ncell_[1]) && (u - v) <= ncell_[1]) {
          cellPosCoarse_[placement][std::pair<int, int>(u, v)] = hgcalcell.HGCalCellUV2XY1(u, v, placement, 1);
        }
      }
    }
  }
}

std::pair<int32_t, int32_t> HGCalCellUV::HGCalCellUVFromXY0(
    double xloc, double yloc, int32_t placement, int32_t type, bool extend, bool debug) {
  //--- Reverse transform to placement=0, if placement index ≠ 6
  double xloc1 = (placement >= 6) ? xloc : -xloc;
  int rot = placement % 6;
  const std::vector<double> fcos = {1.0, 0.5, -0.5, -1.0, -0.5, 0.5};
  const std::vector<double> fsin = {0.0, sqrt3By2_, sqrt3By2_, 0.0, -sqrt3By2_, -sqrt3By2_};
  double xprime = xloc1 * fcos[rot] - yloc * fsin[rot];
  double yprime = xloc1 * fsin[rot] + yloc * fcos[rot];
  double x = xprime;
  double y = yprime;

  //--- Calculate coordinates in u,v,w system
  const double sin60(sqrt3By2_), cos60(0.5);
  double u = x * sin60 + y * cos60;
  double v = -x * sin60 + y * cos60;
  double w = y;

  //--- Rounding in u, v, w coordinates
  int iu = std::floor(u / cellY_[type]) + 3 * (ncell_[type]) - 1;
  int iv = std::floor(v / cellY_[type]) + 3 * (ncell_[type]);
  int iw = std::floor(w / cellY_[type]) + 1;

  int isv = (iu + iw) / 3;
  int isu = (iv + iw) / 3;

  //--- Taking care of extending cells
  if ((iu + iw) < 0) {
    isu = (iv + iw + 1) / 3;
    isv = 0;
  } else if (isv - isu > ncell_[type] - 1) {
    isu = (iv + iw + 1) / 3;
    isv = (iu + iw - 1) / 3;
  } else if (isu > 2 * ncell_[type] - 1) {
    isu = 2 * ncell_[type] - 1;
    isv = (iu + iw - 1) / 3;
  }
  if (debug)
    edm::LogVerbatim("HGCalGeom") << "HGCalCellUVFromXY0: Input " << xloc << ":" << yloc << ":" << extend << " Output "
                                  << isu << ":" << isv;
  return std::make_pair(isu, isv);
}

std::pair<int32_t, int32_t> HGCalCellUV::HGCalCellUVFromXY1(
    double xloc, double yloc, int32_t placement, int32_t type, bool extend, bool debug) {
  //--- Using multiple inequalities to find (u, v)
  //--- Reverse transform to placement=0, if placement index ≠ 7
  double x = (placement >= 6) ? xloc : -1 * xloc;
  const std::vector<double> fcos = {0.5, 1.0, 0.5, -0.5, -1.0, -0.5};
  const std::vector<double> fsin = {-sqrt3By2_, 0.0, sqrt3By2_, sqrt3By2_, 0.0, -sqrt3By2_};
  double y = x * fsin[placement % 6] + yloc * fcos[placement % 6];
  x = x * fcos[placement % 6] - yloc * fsin[placement % 6];

  int32_t u(-100), v(-100);
  int N_ = (type != 0) ? ncell_[1] : ncell_[0];
  double r = (type != 0) ? cellY_[1] : cellY_[0];
  double l1 = (y / r) + N_ - 1.0;
  int l2 = std::floor((0.5 * y + 0.5 * x / sqrt3_) / r + N_ - 4.0 / 3.0);
  int l3 = std::floor((x / sqrt3_) / r + N_ - 4.0 / 3.0);
  double l4 = (y + sqrt3_ * x) / (2 * r) + 2 * N_ - 2;
  double l5 = (y - sqrt3_ * x) / (2 * r) - N_;
  double u1 = (y / r) + N_ + 1.0;
  int u2 = std::ceil((0.5 * y + 0.5 * x / sqrt3_) / r + N_ + 2.0 / 3.0);
  int u3 = std::ceil((x / sqrt3_) / r + N_);
  double u4 = l4 + 2;
  double u5 = l5 + 2;

  for (int ui = l2 + 1; ui < u2; ui++) {
    for (int vi = l3 + 1; vi < u3; vi++) {
      int c1 = 2 * ui - vi;
      int c4 = ui + vi;
      int c5 = ui - 2 * vi;
      if ((c1 < u1) && (c1 > l1) && (c4 < u4) && (c4 > l4) && (c5 < u5) && (c5 > l5)) {
        u = ui;
        v = vi;
      }
    }
  }

  //--- Taking care of extending cells
  if (v == -1) {
    if (y < (2 * u - v - N_) * r) {
      v += 1;
    } else {
      u += 1;
      v += 1;
    }
  }
  if (v - u == N_) {
    if ((y + sqrt3_ * x) < ((u + v - 2 * N_ + 1) * 2 * r)) {
      v += -1;
    } else {
      u += 1;
    }
  }
  if (u == 2 * N_) {
    if ((y - sqrt3_ * x) < ((u - 2 * v + N_ - 1) * 2 * r)) {
      u += -1;
    } else {
      u += -1;
      v += -1;
    }
  }
  if (debug)
    edm::LogVerbatim("HGCalGeom") << "HGCalCellUVFromXY1: Input " << xloc << ":" << yloc << ":" << extend << " Output "
                                  << u << ":" << v;
  return std::make_pair(u, v);
}

std::pair<int, int> HGCalCellUV::HGCalCellUVFromXY2(
    double xloc, double yloc, int32_t placement, int32_t type, bool extend, bool debug) {
  if (type != 0)
    return HGCalCellUVFromXY2(
        xloc, yloc, ncell_[1], cellX_[1], cellY_[1], cellXTotal_[1], cellY_[1], cellPosCoarse_[placement], extend, debug);
  else
    return HGCalCellUVFromXY2(
        xloc, yloc, ncell_[0], cellX_[0], cellY_[0], cellXTotal_[0], cellY_[0], cellPosFine_[placement], extend, debug);
}

std::pair<int, int> HGCalCellUV::HGCalCellUVFromXY2(double xloc,
                                                    double yloc,
                                                    int n,
                                                    double cellX,
                                                    double cellY,
                                                    double cellXTotal,
                                                    double cellYTotal,
                                                    std::map<std::pair<int, int>, std::pair<double, double> >& cellPos,
                                                    bool extend,
                                                    bool debug) {
  std::pair<int, int> uv = std::make_pair(-1, -1);
  std::map<std::pair<int, int>, std::pair<double, double> >::const_iterator itr;
  for (itr = cellPos.begin(); itr != cellPos.end(); ++itr) {
    double delX = std::abs(xloc - (itr->second).first);
    double delY = std::abs(yloc - (itr->second).second);
    if ((delX < cellX) && (delY < cellY)) {
      if ((delX < (0.5 * cellX)) || (delY < (2.0 * cellY - sqrt3_ * delX))) {
        uv = itr->first;
        break;
      }
    }
  }
  if ((uv.first < 0) && extend) {
    for (itr = cellPos.begin(); itr != cellPos.end(); ++itr) {
      double delX = std::abs(xloc - (itr->second).first);
      double delY = std::abs(yloc - (itr->second).second);
      if ((delX < cellXTotal) && (delY < cellYTotal)) {
        if ((delX < (0.5 * cellXTotal)) || (delY < (2.0 * cellYTotal - sqrt3_ * delX))) {
          uv = itr->first;
          break;
        }
      }
    }
  }
  if (debug)
    edm::LogVerbatim("HGCalGeom") << "HGCalCellUVFromXY2: Input " << xloc << ":" << yloc << ":" << extend << " Output "
                                  << uv.first << ":" << uv.second;
  return uv;
}

std::pair<int32_t, int32_t> HGCalCellUV::HGCalCellUVFromXY3(
    double xloc, double yloc, int32_t placement, int32_t type, bool extend, bool debug) {
  //--- Using Cube coordinates to find the (u, v)
  //--- Reverse transform to placement=0, if placement index ≠ 6
  double xloc1 = (placement >= 6) ? xloc : -xloc;
  int rot = placement % 6;
  const std::vector<double> fcos = {1.0, 0.5, -0.5, -1.0, -0.5, 0.5};
  const std::vector<double> fsin = {0.0, sqrt3By2_, sqrt3By2_, 0.0, -sqrt3By2_, -sqrt3By2_};
  double xprime = xloc1 * fcos[rot] - yloc * fsin[rot];
  double yprime = xloc1 * fsin[rot] + yloc * fcos[rot];
  double x = xprime + cellX_[type];
  double y = yprime;

  x = x / cellX_[type];
  y = y / cellY_[type];

  double cu = 2 * x / 3;
  double cv = -x / 3 + y / 2;
  double cw = -x / 3 - y / 2;

  int iu = std::round(cu);
  int iv = std::round(cv);
  int iw = std::round(cw);

  if (iu + iv + iw != 0) {
    double arr[] = {std::abs(cu - iu), std::abs(cv - iv), std::abs(cw - iw)};
    int i = std::distance(arr, std::max_element(arr, arr + 3));

    switch (i) {
      case 0:
        iu = (std::round(cu) == std::floor(cu)) ? std::ceil(cu) : std::floor(cu);
        break;
      case 1:
        iv = (std::round(cv) == std::floor(cv)) ? std::ceil(cv) : std::floor(cv);
        break;
      case 2:
        iw = (std::round(cw) == std::floor(cw)) ? std::ceil(cw) : std::floor(cw);
        break;
    }
  }

  //--- Taking care of extending cells
  int u(ncell_[type] + iv), v(ncell_[type] - 1 - iw);
  double xcell = (1.5 * (v - u) + 0.5) * cellX_[type];
  double ycell = (v + u - 2 * ncell_[type] + 1) * cellY_[type];
  if (v == -1) {
    if ((yprime - sqrt3_ * xprime) < (ycell - sqrt3_ * xcell)) {
      v += 1;
    } else {
      u += 1;
      v += 1;
    }
  }
  if (v - u == ncell_[type]) {
    if (yprime < ycell) {
      v += -1;
    } else {
      u += 1;
    }
  }
  if (u == 2 * ncell_[type]) {
    if ((yprime + sqrt3_ * xprime) > (ycell + sqrt3_ * xcell)) {
      u += -1;
    } else {
      u += -1;
      v += -1;
    }
  }

  if (debug)
    edm::LogVerbatim("HGCalGeom") << "HGCalCellUVFromXY3: Input " << xloc << ":" << yloc << ":" << extend << " Output "
                                  << iu << ":" << iv;
  return std::make_pair(u, v);
}
