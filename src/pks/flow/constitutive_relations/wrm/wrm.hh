/* -*-  mode: c++; indent-tabs-mode: nil -*- */

/*
  A linear sat-pc curve, plus a constant rel perm, makes the system linear, so
  nonlinear solver should always converge in one step.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#ifndef AMANZI_FLOWRELATIONS_WRM_
#define AMANZI_FLOWRELATIONS_WRM_

#include "Teuchos_ParameterList.hpp"

namespace Amanzi {
namespace Flow {

const int FLOW_WRM_MUALEM = 1;
const int FLOW_WRM_BURDINE = 2;

const int FLOW_WRM_ONE = 3;


class WRM {

public:
  virtual ~WRM() {}

  // required methods from the base class
  virtual double k_relative(double saturation) = 0;
  virtual double d_k_relative(double saturation) = 0;
  virtual double saturation(double pc) = 0;
  virtual double d_saturation(double pc) = 0;
  virtual double capillaryPressure(double saturation) = 0;
  virtual double d_capillaryPressure(double saturation) = 0;
  virtual double residualSaturation() = 0;

};

typedef double(WRM::*KRelFn)(double pc);

} //namespace
} //namespace

#endif
