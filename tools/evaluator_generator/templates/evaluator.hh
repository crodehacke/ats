/*
  The {evalNameString} evaluator is an algebraic evaluator of a given model.

  Generated via evaluator_generator with:
{docDict}
    
  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#ifndef AMANZI_{namespaceCaps}_{evalNameCaps}_EVALUATOR_HH_
#define AMANZI_{namespaceCaps}_{evalNameCaps}_EVALUATOR_HH_

#include "Factory.hh"
#include "secondary_variable_field_evaluator.hh"

namespace Amanzi {{
namespace {namespace} {{
namespace Relations {{

class {evalClassName}Model;

class {evalClassName}Evaluator : public SecondaryVariableFieldEvaluator {{

 public:
  explicit
  {evalClassName}Evaluator(Teuchos::ParameterList& plist);
  {evalClassName}Evaluator(const {evalClassName}Evaluator& other);

  virtual Teuchos::RCP<FieldEvaluator> Clone() const;

  // Required methods from SecondaryVariableFieldEvaluator
  virtual void EvaluateField_(const Teuchos::Ptr<State>& S,
          const Teuchos::Ptr<CompositeVector>& result);
  virtual void EvaluateFieldPartialDerivative_(const Teuchos::Ptr<State>& S,
          Key wrt_key, const Teuchos::Ptr<CompositeVector>& result);

  Teuchos::RCP<{evalClassName}Model> get_model() {{ return model_; }}

 protected:
  void InitializeFromPlist_();

{keyDeclarationList}

  Teuchos::RCP<{evalClassName}Model> model_;

 private:
  static Utils::RegisteredFactory<FieldEvaluator,{evalClassName}Evaluator> reg_;

}};

}} //namespace
}} //namespace
}} //namespace

#endif
