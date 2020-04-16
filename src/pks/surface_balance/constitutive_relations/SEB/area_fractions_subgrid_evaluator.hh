/* -*-  mode: c++; indent-tabs-mode: nil -*- */
/*
  License: see $ATS_DIR/COPYRIGHT
  Author: Ethan Coon (ecoon@ornl.gov)
*/

//! A subgrid model for determining the area fraction of land, water, and snow within a grid cell with subgrid microtopography.

/*!

  Uses the subgrid equation from Jan et al WRR 2018 for volumetric or
  effective ponded depth to determine the area of water, then heuristically
  places snow on top of that surface.

Requires the following dependencies:

* `"maximum ponded depth key`" ``[string]`` **DOMAIN-maximum_ponded_depth**
         The name of del_max, the max microtopography value.
* `"excluded volume key`" ``[string]`` **DOMAIN-excluded_volume**
         The name of del_excluded, the integral of the microtopography.
* `"pressure key`" ``[string]`` **DOMAIN-pressure**
         The name of the pressure on the surface.

  
  NOTE: this evaluator simplifies the situation by assuming constant
  density.  This make it so that ice and water see the same geometry per
  unit pressure, which isn't quite true thanks to density differences.
  However, we hypothesize that these differences, on the surface (unlike in
  the subsurface) really don't matter much. --etc
         
*/

#ifndef AMANZI_SURFACE_BALANCE_AREA_FRACTIONS_SUBGRID_EVALUATOR_HH_
#define AMANZI_SURFACE_BALANCE_AREA_FRACTIONS_SUBGRID_EVALUATOR_HH_

#include "Factory.hh"
#include "secondary_variable_field_evaluator.hh"

namespace Amanzi {
namespace SurfaceBalance {

class AreaFractionsSubgridEvaluator : public SecondaryVariableFieldEvaluator {

 public:
  explicit
  AreaFractionsSubgridEvaluator(Teuchos::ParameterList& plist);
  AreaFractionsSubgridEvaluator(const AreaFractionsSubgridEvaluator& other) = default;

  virtual Teuchos::RCP<FieldEvaluator> Clone() const {
    return Teuchos::rcp(new AreaFractionsSubgridEvaluator(*this));
  }

  virtual void EnsureCompatibility(const Teuchos::Ptr<State>& S);
  
 protected:
  // Required methods from SecondaryVariableFieldEvaluator
  virtual void EvaluateField_(const Teuchos::Ptr<State>& S,
          const Teuchos::Ptr<CompositeVector>& result);
  virtual void EvaluateFieldPartialDerivative_(const Teuchos::Ptr<State>& S,
          Key wrt_key, const Teuchos::Ptr<CompositeVector>& result) {
    Exceptions::amanzi_throw("NotImplemented: AreaFractionsSubgridEvaluator currently does not provide derivatives.");
  }

 protected:

  // TODO: put these functions into a model and share them across evaluators:
  //  - this one
  //  - overland_subgrid_water_content_evaluator
  //  - volumetric_ponded_depth evaluator --etc
  double f_(double delta, double del_max, double del_ex) {
    return delta >= del_max ? delta - del_ex:
        std::pow(delta/del_max, 2) * (2*del_max - 3*del_ex)
        + std::pow(delta/del_max,3) * (2*del_ex - del_max);
  }
  double f_prime_(double delta, double del_max, double del_ex) {
    return delta >= del_max ? 1 :
        2 * delta/del_max * (2*del_max - 3*del_ex) / del_max
        + 3 * std::pow(delta/del_max,2) * (2*del_ex - del_max) / del_max;
  }

  Key domain_, domain_snow_;
  Key ponded_depth_key_, snow_depth_key_, vol_snow_depth_key_;
  Key delta_max_key_, delta_ex_key_;
  double rho_liq_;
  double snow_subgrid_transition_;
  double min_area_;

 private:
  static Utils::RegisteredFactory<FieldEvaluator,AreaFractionsSubgridEvaluator> reg_;

};

} //namespace
} //namespace

#endif
