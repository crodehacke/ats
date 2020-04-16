/* -*-  mode: c++; indent-tabs-mode: nil -*- */
/*
  License: see $ATS_DIR/COPYRIGHT
  Authors: Ahmad Jan (jana@ornl.gov)
           Ethan Coon (ecoon@ornl.gov)
*/

//! Determine the volumetric ponded depth from ponded depth and subgrid parameters.
/*!

* `"maximum relief key`" ``[string]`` **DOMAIN-maximum_relief**
         The name of del_max, the max microtopography value.
* `"excluded volume key`" ``[string]`` **DOMAIN-excluded_volume**
         The name of del_excluded, the integral of the microtopography.
* `"ponded depth key`" ``[string]`` **DOMAIN-ponded_depth**
         The true height of the water surface.

*/

#include "volumetric_height_evaluator.hh"

namespace Amanzi {
namespace Flow {


VolumetricHeightEvaluator::VolumetricHeightEvaluator(Teuchos::ParameterList& plist) :
     SecondaryVariableFieldEvaluator(plist)
{
  Key domain = Keys::getDomain(my_key_);

  // dependencies
  pd_key_ = Keys::readKey(plist_, domain, "ponded depth key", "ponded_depth");
  dependencies_.insert(pd_key_);

  delta_max_key_ = Keys::readKey(plist_, domain, "microtopographic relief", "microtopographic_relief"); 
  dependencies_.insert(delta_max_key_);
  
  delta_ex_key_ = Keys::readKey(plist_, domain, "excluded volume", "excluded_volume");
  dependencies_.insert(delta_ex_key_);
}


void VolumetricHeightEvaluator::EvaluateField_(const Teuchos::Ptr<State>& S,
        const Teuchos::Ptr<CompositeVector>& result)
{
  auto& res = *result->ViewComponent("cell",false);
  const auto& pd = *S->GetFieldData(pd_key_)->ViewComponent("cell",false);
  const auto& del_max = *S->GetFieldData(delta_max_key_)->ViewComponent("cell",false);
  const auto& del_ex = *S->GetFieldData(delta_ex_key_)->ViewComponent("cell",false);

  for (int c=0; c!=res.MyLength(); ++c){
    res[0][c] = f_(pd[0][c], del_max[0][c], del_ex[0][c]);
  }
}


void VolumetricHeightEvaluator::EvaluateFieldPartialDerivative_(const Teuchos::Ptr<State>& S,
        Key wrt_key, const Teuchos::Ptr<CompositeVector>& result)
{
  auto& res = *result->ViewComponent("cell",false);
  const auto& pd = *S->GetFieldData(pd_key_)->ViewComponent("cell",false);
  const auto& del_max = *S->GetFieldData(delta_max_key_)->ViewComponent("cell",false);
  const auto& del_ex = *S->GetFieldData(delta_ex_key_)->ViewComponent("cell",false);

  if (wrt_key == pd_key_) {
    for (int c=0; c!=res.MyLength(); ++c){
      res[0][c] = f_prime_(pd[0][c], del_max[0][c], del_ex[0][c]);
    }
  } else {
    Errors::Message msg("VolumetricHeightEvaluator: Not Implemented: no derivatives implemented other than ponded depth.");
    Exceptions::amanzi_throw(msg);
  }
}


} //namespace
} //namespace
