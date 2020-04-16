/* -*-  mode: c++; indent-tabs-mode: nil -*- */

// -----------------------------------------------------------------------------
// ATS
//
// License: see $ATS_DIR/COPYRIGHT
// Author: Ethan Coon (ecoon@lanl.gov)
//
// Scheme for taking coefficients for div-grad operators from cells to
// faces.
// -----------------------------------------------------------------------------

#ifndef AMANZI_UPWINDING_GRAVITYFLUX_SCHEME_
#define AMANZI_UPWINDING_GRAVITYFLUX_SCHEME_

#include "Epetra_Vector.h"
#include "Tensor.hh"

#include "upwinding.hh"

namespace Amanzi {

class State;
class CompositeVector;

namespace Operators {

class UpwindGravityFlux : public Upwinding {

public:

  UpwindGravityFlux(std::string pkname,
                    std::string cell_coef,
                    std::string face_coef,
                    const Teuchos::RCP<std::vector<WhetStone::Tensor> > K);

  void Update(const Teuchos::Ptr<State>& S,
              const Teuchos::Ptr<Debugger>& db=Teuchos::null);


  void CalculateCoefficientsOnFaces(
        const CompositeVector& cell_coef,
        const Epetra_Vector& gravity,
        const Teuchos::Ptr<CompositeVector>& face_coef);

  virtual std::string
  CoefficientLocation() { return "upwind: face"; }

private:

  std::string pkname_;
  std::string cell_coef_;
  std::string face_coef_;

  Teuchos::RCP<std::vector<WhetStone::Tensor> > K_;
};

} // namespace
} // namespace

#endif
