/* -*-  mode: c++; indent-tabs-mode: nil -*- */

/*
  ATS

  EOS for liquid ice.  See the permafrost physical properties notes for
  references and documentation of this EOS at:

  http://software.lanl.gov/ats/trac

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#include "eos_ice.hh"

namespace Amanzi {
namespace Relations {

EOSIce::EOSIce(Teuchos::ParameterList& eos_plist) :
    eos_plist_(eos_plist),

    ka_(916.724),
    kb_(-0.147143),
    kc_(-0.000238095),

    kT0_(273.15),

    kalpha_(1.0e-10),
    kp0_(1.0e5) {
  InitializeFromPlist_();
};

double EOSIce::MassDensity(std::vector<double>& params) {
  //AMANZI_ASSERT (params.size() >= 2);
  double T = params[0];
  double p = params[1];
                                                
  p = std::max(p, 101325.);

  double dT = T - kT0_;
  double rho1bar = ka_ + (kb_ + kc_*dT)*dT;
  return rho1bar * (1.0 + kalpha_*(p - kp0_));
};


double EOSIce::DMassDensityDT(std::vector<double>& params) {
  //AMANZI_ASSERT (params.size() >= 2);

  double T = params[0];
  double p = params[1];
                                                   
  p = std::max(p, 101325.);

  double dT = T - kT0_;
  double rho1bar = kb_ + 2.0*kc_*dT;
  return rho1bar * (1.0 + kalpha_*(p - kp0_));
};


double EOSIce::DMassDensityDp(std::vector<double>& params) {
  //AMANZI_ASSERT (params.size() >= 2);
  double T = params[0];
  double p = params[1];

  if (p < 101325.) {
    return 0.;
  } else {
    double T = params[0];
    double p = params[1];          
    double dT = T - kT0_;
    double rho1bar = ka_ + (kb_ + kc_*dT)*dT;
    return rho1bar * kalpha_;
  }

};


void EOSIce::InitializeFromPlist_() {
  if (eos_plist_.isParameter("Molar mass of ice [kg/mol]")) {
    M_ = eos_plist_.get<double>("Molar mass of ice [kg/mol]");
  } else {
    M_ = eos_plist_.get<double>("Molar mass of ice [g/mol]", 18.0153)*1e-3;
  }
};

} // namespace
} // namespace
