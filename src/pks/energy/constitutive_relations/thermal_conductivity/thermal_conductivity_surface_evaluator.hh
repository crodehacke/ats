/* -*-  mode: c++; indent-tabs-mode: nil -*- */

/*
  Interface for a thermal conductivity model with two phases.

  License: BSD
  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#ifndef AMANZI_ENERGY_RELATIONS_TC_SURFACE_EVALUATOR_HH_
#define AMANZI_ENERGY_RELATIONS_TC_SURFACE_EVALUATOR_HH_

#include "secondary_variable_field_evaluator.hh"

namespace Amanzi {
namespace Energy {

class ThermalConductivitySurfaceEvaluator :
    public SecondaryVariableFieldEvaluator {

 public:
  // constructor format for all derived classes
  ThermalConductivitySurfaceEvaluator(Teuchos::ParameterList& plist);
  ThermalConductivitySurfaceEvaluator(const ThermalConductivitySurfaceEvaluator& other);

  Teuchos::RCP<FieldEvaluator> Clone() const;

  // Required methods from SecondaryVariableFieldModel
  virtual void EvaluateField_(const Teuchos::Ptr<State>& S,
          const Teuchos::Ptr<CompositeVector>& result);
  virtual void EvaluateFieldPartialDerivative_(const Teuchos::Ptr<State>& S,
          Key wrt_key, const Teuchos::Ptr<CompositeVector>& result);

 protected:
  // dependencies
  Key uf_key_;
  Key height_key_;

  double K_liq_;
  double K_ice_;
  double min_K_;
};

} // namespace
} // namespace

#endif
