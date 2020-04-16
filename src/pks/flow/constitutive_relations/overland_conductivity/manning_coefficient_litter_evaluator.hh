/*
  The manning coefficient with variable litter evaluator is an algebraic evaluator of a given model.

  Generated via evaluator_generator with:
Manning's coefficient that varies based on litter thickness and ponded depth.
    
  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#ifndef AMANZI_FLOW_MANNING_COEFFICIENT_LITTER_EVALUATOR_HH_
#define AMANZI_FLOW_MANNING_COEFFICIENT_LITTER_EVALUATOR_HH_

#include "Teuchos_RCP.hpp"

#include "Factory.hh"
#include "secondary_variable_field_evaluator.hh"

namespace Amanzi {
namespace Flow {
namespace Relations {

class ManningCoefficientLitterModel;

typedef std::vector<Teuchos::RCP<ManningCoefficientLitterModel> > ManningCoefList;
typedef std::pair<Teuchos::RCP<Functions::MeshPartition>, ManningCoefList> ManningCoefPartition;

  
class ManningCoefficientLitterEvaluator : public SecondaryVariableFieldEvaluator {

 public:
  explicit
  ManningCoefficientLitterEvaluator(Teuchos::ParameterList& plist);
  ManningCoefficientLitterEvaluator(const ManningCoefficientLitterEvaluator& other);

  virtual Teuchos::RCP<FieldEvaluator> Clone() const;

  // Required methods from SecondaryVariableFieldEvaluator
  virtual void EvaluateField_(const Teuchos::Ptr<State>& S,
          const Teuchos::Ptr<CompositeVector>& result);
  virtual void EvaluateFieldPartialDerivative_(const Teuchos::Ptr<State>& S,
          Key wrt_key, const Teuchos::Ptr<CompositeVector>& result);

  Teuchos::RCP<ManningCoefPartition> get_models() { return models_; }

 protected:
  void InitializeFromPlist_();

  Key ld_key_;
  Key pd_key_;
  Teuchos::RCP<ManningCoefPartition> models_;

 private:
  static Utils::RegisteredFactory<FieldEvaluator,ManningCoefficientLitterEvaluator> reg_;

};

  
// Non-member factory
Teuchos::RCP<ManningCoefPartition>
createManningCoefPartition(Teuchos::ParameterList& plist);

  
} //namespace
} //namespace
} //namespace

#endif
