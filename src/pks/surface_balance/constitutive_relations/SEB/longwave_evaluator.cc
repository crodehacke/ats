/* -*-  mode: c++; indent-tabs-mode: nil -*- */
/*
  License: see $ATS_DIR/COPYRIGHT
  Author: Ethan Coon (ecoon@ornl.gov)
*/

//! Evaluates incoming longwave radiation from rel humidity and air temperature.

/*!

Requires the following dependencies:

* `"air temperature key`" ``[string]`` **DOMAIN-air_temperature**
* `"relative humicity key`" ``[string]`` **DOMAIN-relative_humidity**
         
*/

#include "Key.hh"
#include "seb_physics_defs.hh"
#include "seb_physics_funcs.hh"
#include "longwave_evaluator.hh"


namespace Amanzi {
namespace SurfaceBalance {

LongwaveEvaluator::LongwaveEvaluator(Teuchos::ParameterList& plist) :
    SecondaryVariableFieldEvaluator(plist)
{
  auto domain = Keys::getDomain(my_key_);
  air_temp_key_ = Keys::readKey(plist, domain, "air temperature", "air_temperature");
  dependencies_.insert(air_temp_key_);
  rel_hum_key_ = Keys::readKey(plist, domain, "relative humidity", "relative_humidity");
  dependencies_.insert(rel_hum_key_);

  stephB_ = SEBPhysics::ModelParams().stephB;
}

// Required methods from SecondaryVariableFieldEvaluator
void
LongwaveEvaluator::EvaluateField_(const Teuchos::Ptr<State>& S,
        const Teuchos::Ptr<CompositeVector>& result)
{
  const auto& air_temp = *S->GetFieldData(air_temp_key_)->ViewComponent("cell", false);
  const auto& rel_hum = *S->GetFieldData(rel_hum_key_)->ViewComponent("cell", false);
  auto& res = *result->ViewComponent("cell", false);

  for (int c=0; c!=res.MyLength(); ++c) {
    res[0][c] = SEBPhysics::CalcIncomingLongwave(air_temp[0][c], rel_hum[0][c], stephB_);
  }  
}

} //namespace
} //namespace

