/* -*-  mode: c++; indent-tabs-mode: nil -*- */

/*
  Evaluates the conductivity of surface flow as a function of ponded
  depth and surface slope using Manning's model.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#ifndef AMANZI_FLOWRELATIONS_MANNING_CONDUCTIVITY_MODEL_
#define AMANZI_FLOWRELATIONS_MANNING_CONDUCTIVITY_MODEL_

#include "Teuchos_ParameterList.hpp"
#include "overland_conductivity_model.hh"

namespace Amanzi {
namespace Flow {

class ManningConductivityModel : public OverlandConductivityModel {
public:
  explicit
  ManningConductivityModel(Teuchos::ParameterList& plist);

  virtual double Conductivity(double depth, double slope, double coef);

  virtual double DConductivityDDepth(double depth, double slope, double coef);

  //Added for the subgrid Model
  virtual double Conductivity(double depth, double slope, double coef, double pd_depth, double frac_cond, double beta);  
  virtual double DConductivityDDepth(double depth, double slope, double coef, double pd_depth, double frac, double beta);

protected:
  Teuchos::ParameterList plist_;

  double slope_regularization_;
  double manning_exp_, beta_exp_;
  double manning_coef_;

};

} // namespace
} // namespace

#endif
