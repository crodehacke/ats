/* -*-  mode: c++; indent-tabs-mode: nil -*- */

/* -------------------------------------------------------------------------
ATS

License: see $ATS_DIR/COPYRIGHT
Author: Ethan Coon

Hacky TC model with constant values as a function of temperature, requires the
sutra model for permafrost WRM to also be used.  This only exists to support
the INTERFROST comparison.

Usage:

  <ParameterList name="Thermal Conductivity Model">
    <Parameter name="Thermal Conductivity Type" type="string" value="sutra hacked"/>
    <Parameter name="thermal conductivity of frozen" type="double" value=""/>
    <Parameter name="thermal conductivity of mushy" type="double" value=""/>
    <Parameter name="thermal conductivity of unfrozen" type="double" value=""/>
    <Parameter name="residual saturation" type="double" value=""/>
  </ParameterList>

Units: ????
------------------------------------------------------------------------- */

#ifndef PK_ENERGY_RELATIONS_THERMAL_CONDUCTIVITY_THREEPHASE_SUTRA_HACKEED_HH_
#define PK_ENERGY_RELATIONS_THERMAL_CONDUCTIVITY_THREEPHASE_SUTRA_HACKEED_HH_

#include "Teuchos_ParameterList.hpp"

#include "Factory.hh"
#include "thermal_conductivity_threephase.hh"

namespace Amanzi {
namespace Energy {

class ThermalConductivityThreePhaseSutraHacked : public ThermalConductivityThreePhase {

public:
  ThermalConductivityThreePhaseSutraHacked(Teuchos::ParameterList& plist);

  double ThermalConductivity(double porosity, double sat_liq, double sat_ice, double temp);

private:
  void InitializeFromPlist_();

  Teuchos::ParameterList plist_;

  double k_frozen_;
  double k_unfrozen_;
  double k_mushy_;
  double sr_;

private:
  static Utils::RegisteredFactory<ThermalConductivityThreePhase,
                                  ThermalConductivityThreePhaseSutraHacked> factory_;

};

}
}

#endif
