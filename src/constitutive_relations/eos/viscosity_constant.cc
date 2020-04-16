/* -*-  mode: c++; indent-tabs-mode: nil -*- */

/*
  ATS

  Constant viscosity EOS, defaults to reasonable values for water.

  http://software.lanl.gov/ats/trac

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#include "viscosity_constant.hh"

namespace Amanzi {
namespace Relations {

ViscosityConstant::ViscosityConstant(Teuchos::ParameterList& visc_plist) :
    visc_plist_(visc_plist) {
  InitializeFromPlist_();
};

void ViscosityConstant::InitializeFromPlist_() {
  // defaults to water
  visc_ = visc_plist_.get<double>("viscosity [kg/m-s]", 8.9e-4);
};

} // namespace
} // namespace
