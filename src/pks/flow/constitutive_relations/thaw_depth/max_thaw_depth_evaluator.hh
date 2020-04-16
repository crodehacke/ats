/* -*-  mode: c++; indent-tabs-mode: nil -*- */

/*
  The max thaw depth evaluator gets the thaw depth and computes the maximum thaw depth over time.

  Authors: Ahmad Jan (jana@ornl.gov)
*/

#ifndef AMANZI_FLOWRELATIONS_MAX_THAWDEPTH_EVALUATOR_
#define AMANZI_FLOWRELATIONS_MAX_THAWDEPTH_EVALUATOR_

#include "Factory.hh"
//#include "thaw_depth_evaluator.hh"
#include "secondary_variable_field_evaluator.hh"

namespace Amanzi {
namespace Flow {

class MaxThawDepthEvaluator : public SecondaryVariableFieldEvaluator {

public:
  explicit
  MaxThawDepthEvaluator(Teuchos::ParameterList& plist);
  MaxThawDepthEvaluator(const MaxThawDepthEvaluator& other);
  Teuchos::RCP<FieldEvaluator> Clone() const;
  
protected:
  // Required methods from SecondaryVariableFieldEvaluator
  virtual void EvaluateField_(const Teuchos::Ptr<State>& S,
                              const Teuchos::Ptr<CompositeVector>& result);
  //virtual void EnsureCompatibility(const Teuchos::Ptr<State>& S);

  virtual void EvaluateFieldPartialDerivative_(const Teuchos::Ptr<State>& S,
                                               Key wrt_key, const Teuchos::Ptr<CompositeVector>& result);
  Key td_key_;
  double threshold_td_;
private:

  static Utils::RegisteredFactory<FieldEvaluator,MaxThawDepthEvaluator> reg_;  
  //  static std::vector<double> base_thawdepth;
  //static bool init_flag;
};
  
} //namespace
} //namespace 

#endif
