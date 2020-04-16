
/* -*-  mode: c++; indent-tabs-mode: nil -*- */

/*
  No special limits as p_surf -> p_atm.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#ifndef AMANZI_FLOWRELATIONS_ONE_UNFROZEN_FRACTION_KR_MODEL_
#define AMANZI_FLOWRELATIONS_ONE_UNFROZEN_FRACTION_KR_MODEL_

#include "Teuchos_ParameterList.hpp"
#include "dbc.hh"
#include "Factory.hh"
#include "surface_relperm_model.hh"

namespace Amanzi {
namespace Flow {

class OneUFRelPermModel : public SurfaceRelPermModel {
 public:
  OneUFRelPermModel(Teuchos::ParameterList& list);

  virtual bool TemperatureDependent() { return true; }

  virtual double SurfaceRelPerm(double uf, double h);

  virtual double DSurfaceRelPermDUnfrozenFraction(double uf, double h) {
    AMANZI_ASSERT(0);
    return 0.;
  }

  virtual double DSurfaceRelPermDPondedDepth(double uf, double h) {
    AMANZI_ASSERT(0);
    return 0.;
  }

 protected:
  Teuchos::ParameterList plist_;

  int alpha_; // must be an even integer
  const double pi_;
  double h_cutoff_up_, h_cutoff_dn_;

 private:
  static Utils::RegisteredFactory<SurfaceRelPermModel,OneUFRelPermModel> reg_;
  
};

} // namespace
} // namespace

#endif

