/* -*-  mode: c++; indent-tabs-mode: nil -*- */

/* -------------------------------------------------------------------------
ATS

License: see $ATS_DIR/COPYRIGHT
Author: Ethan Coon

Simple model of three-phase thermal conductivity, based upon:

- Interpolation between saturated and dry conductivities via a Kersten number.
- Power-law Kersten number.

See ATS process model documentation's permafrost model for details.

Usage:

  <ParameterList name="Thermal Conductivity Model">
    <Parameter name="Thermal Conductivity Type" type="string" value="three-phase wet/dry"/>

    <Parameter name="thermal conductivity, wet" type="double" value=""/>
    <Parameter name="thermal conductivity, dry" type="double" value=""/>

    <Parameter name="epsilon" type="double" value="1.e-10"/>
    <Parameter name="unsaturated alpha" type="double" value="1.0"/>
  </ParameterList>

Units: ????
------------------------------------------------------------------------- */

#ifndef PK_ENERGY_RELATIONS_THERMAL_CONDUCTIVITY_THREEPHASE_WETDRY_HH_
#define PK_ENERGY_RELATIONS_THERMAL_CONDUCTIVITY_THREEPHASE_WETDRY_HH_

#include "Teuchos_ParameterList.hpp"

#include "Factory.hh"
#include "thermal_conductivity_threephase.hh"

namespace Amanzi {
namespace Energy {

class ThermalConductivityThreePhaseWetDry : public ThermalConductivityThreePhase {

public:
  ThermalConductivityThreePhaseWetDry(Teuchos::ParameterList& plist);

  double ThermalConductivity(double porosity, double sat_liq, double sat_ice, double temp);
  double DThermalConductivity_DPorosity(double porosity, double sat_liq, double sat_ice, double temp);
  double DThermalConductivity_DSaturationLiquid(double porosity, double sat_liq, double sat_ice, double temp);
  double DThermalConductivity_DSaturationIce(double porosity, double sat_liq, double sat_ice, double temp);
  double DThermalConductivity_DTemperature(double porosity, double sat_liq, double sat_ice, double temp);

private:
  void InitializeFromPlist_();

  Teuchos::ParameterList plist_;

  double eps_;
  double alpha_f_;
  double alpha_u_;
  double k_sat_u_;
  double k_dry_;
  double beta_sat_f_; 
private:
  static Utils::RegisteredFactory<ThermalConductivityThreePhase,
                                  ThermalConductivityThreePhaseWetDry> factory_;

};

} // namespace
} // namespace

#endif
