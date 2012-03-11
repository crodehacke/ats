/*
  This is the flow component of the Amanzi code.
  License: BSD
  Authors: Konstantin Lipnikov (version 2) (lipnikov@lanl.gov)
*/

#include "Epetra_FECrsGraph.h"
#include "matrix_mfd.hh"

namespace Amanzi {
namespace AmanziFlow {

// main computational methods

/* ******************************************************************
 * Calculate elemental inverse mass matrices.
 * WARNING: The original Aff_ matrices are destroyed.
 ****************************************************************** */
void MatrixMFD::CreateMFDmassMatrices(MFD_method method,
        std::vector<WhetStone::Tensor>& K) {
  int dim = mesh_->space_dimension();
  WhetStone::MFD3D mfd(mesh_);
  AmanziMesh::Entity_ID_List faces;
  std::vector<int> dirs;

  Aff_cells_.clear();
  for (int c=0; c != K.size(); ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
    int nfaces = faces.size();

    Teuchos::SerialDenseMatrix<int, double> Bff(nfaces, nfaces);

    if (method == MFD_HEXAHEDRA_MONOTONE) {
      if ((nfaces == 6 && dim == 3) || (nfaces == 4 && dim == 2)) {
        mfd.darcy_mass_inverse_hex(c, K[c], Bff);
      } else {
        mfd.darcy_mass_inverse(c, K[c], Bff);
      }
    } else {
      mfd.darcy_mass_inverse(c, K[c], Bff);
    }

    Aff_cells_.push_back(Bff);
  }
}


/* ******************************************************************
 * Calculate elemental stiffness matrices.
 ****************************************************************** */
void MatrixMFD::CreateMFDstiffnessMatrices(MFD_method method,
        std::vector<WhetStone::Tensor>& K, const CompositeVector& K_faces) {
  int dim = mesh_->space_dimension();
  WhetStone::MFD3D mfd(mesh_);
  AmanziMesh::Entity_ID_List faces;
  std::vector<int> dirs;

  Aff_cells_.clear();
  Afc_cells_.clear();
  Acf_cells_.clear();
  Acc_cells_.clear();

  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  for (int c=0; c != ncells; ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
    int nfaces = faces.size();

    Teuchos::SerialDenseMatrix<int, double> Bff(nfaces, nfaces);
    Epetra_SerialDenseVector Bcf(nfaces), Bfc(nfaces);

    if (method == MFD_HEXAHEDRA_MONOTONE) {
      if ((nfaces == 6 && dim == 3) || (nfaces == 4 && dim == 2)) {
        mfd.darcy_mass_inverse_hex(c, K[c], Bff);
      } else {
        mfd.darcy_mass_inverse(c, K[c], Bff);
        //mfd.darcy_mass_inverse_diagonal(c, K[c], Bff);
      }
    } else {
      mfd.darcy_mass_inverse(c, K[c], Bff);
    }

    for (int n=0; n != nfaces; ++n)
      for (int m=0; m != nfaces; ++m) Bff(m, n) *= K_faces("face",faces[m]);

    double matsum = 0.0;  // elimination of mass matrix
    for (int n=0; n != nfaces; ++n) {
      double rowsum = 0.0, colsum = 0.0;
      for (int m=0; m != nfaces; ++m) {
        colsum += Bff(m, n);
        rowsum += Bff(n, m);
      }
      Bcf(n) = -colsum;
      Bfc(n) = -rowsum;
      matsum += colsum;
    }

    Aff_cells_.push_back(Bff);  // This the only place where memory can be allocated.
    Afc_cells_.push_back(Bfc);
    Acf_cells_.push_back(Bcf);
    Acc_cells_.push_back(matsum);
  }
}


/* ******************************************************************
 * May be used in the future.
 ****************************************************************** */
void MatrixMFD::RescaleMFDstiffnessMatrices(const Epetra_Vector& old_scale,
        const Epetra_Vector& new_scale) {

  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  for (int c=0; c != ncells; ++c) {
    Teuchos::SerialDenseMatrix<int, double>& Bff = Aff_cells_[c];
    Epetra_SerialDenseVector& Bcf = Acf_cells_[c];

    int n = Bff.numRows();
    double scale = old_scale[c] / new_scale[c];

    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) Bff(i, j) *= scale;
      Bcf(i) *= scale;
    }
    Acc_cells_[c] *= scale;
  }
}


/* ******************************************************************
 * Simply allocates memory.
 ****************************************************************** */
void MatrixMFD::CreateMFDrhsVectors() {
  Ff_cells_.clear();
  Fc_cells_.clear();

  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  AmanziMesh::Entity_ID_List faces;
  std::vector<int> dirs;

  for (int c=0; c != ncells; ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
    int nfaces = faces.size();

    Epetra_SerialDenseVector Ff(nfaces);  // Entries are initilaized to 0.0.
    double Fc = 0.0;

    Ff_cells_.push_back(Ff);
    Fc_cells_.push_back(Fc);
  }
}

/* ******************************************************************
 *  Create work vectors for Apply-ing the operator from/to Epetra_Vectors
 * (Supervectors) instead of CompositeVectors -- for use with AztecOO
 ****************************************************************** */
void MatrixMFD::InitializeSuperVecs(const CompositeVector& sample) {
  vector_x_ = Teuchos::rcp(new CompositeVector(sample));
  vector_y_ = Teuchos::rcp(new CompositeVector(sample));
  supermap_ = vector_x_->supermap();
}

/* ******************************************************************
 * Applies boundary conditions to elemental stiffness matrices and
 * creates elemental rigth-hand-sides.
 ****************************************************************** */
void MatrixMFD::ApplyBoundaryConditions(std::vector<Matrix_bc>& bc_markers,
        std::vector<double>& bc_values) {
  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  AmanziMesh::Entity_ID_List faces;
  std::vector<int> dirs;

  for (int c=0; c != ncells; ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
    int nfaces = faces.size();

    Teuchos::SerialDenseMatrix<int, double>& Bff = Aff_cells_[c];  // B means elemental.
    Epetra_SerialDenseVector& Bfc = Afc_cells_[c];
    Epetra_SerialDenseVector& Bcf = Acf_cells_[c];

    Epetra_SerialDenseVector& Ff = Ff_cells_[c];
    double& Fc = Fc_cells_[c];

    for (int n=0; n != nfaces; ++n) {
      int f=faces[n];
      if (bc_markers[f] == MATRIX_BC_DIRICHLET) {
        for (int m=0; m != nfaces; ++m) {
          Ff[m] -= Bff(m, n) * bc_values[f];
          Bff(n, m) = Bff(m, n) = 0.0;
        }
        Fc -= Bcf(n) * bc_values[f];
        Bcf(n) = Bfc(n) = 0.0;

        Bff(n, n) = 1.0;
        Ff[n] = bc_values[f];
      } else if (bc_markers[f] == MATRIX_BC_FLUX) {
        Ff[n] -= bc_values[f] * mesh_->face_area(f);
      }
    }
  }
}


/* ******************************************************************
 * Initialize Trilinos matrices. It must be called only once.
 * If matrix is non-symmetric, we generate transpose of the matrix
 * block Afc_ to reuse cf_graph; otherwise, pointer Afc_ = Acf_.
 ****************************************************************** */
void MatrixMFD::SymbolicAssembleGlobalMatrices() {
  const Epetra_Map& cmap = mesh_->cell_map(false);
  const Epetra_Map& fmap = mesh_->face_map(false);
  const Epetra_Map& fmap_wghost = mesh_->face_map(true);

  int avg_entries_row = (mesh_->space_dimension() == 2) ? MFD_QUAD_FACES : MFD_HEX_FACES;
  Epetra_CrsGraph cf_graph(Copy, cmap, fmap_wghost, avg_entries_row, false);  // FIX (lipnikov@lanl.gov)
  Epetra_FECrsGraph ff_graph(Copy, fmap, 2*avg_entries_row);

  AmanziMesh::Entity_ID_List faces;
  std::vector<int> dirs;
  int faces_LID[MFD_MAX_FACES];  // Contigious memory is required.
  int faces_GID[MFD_MAX_FACES];

  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  for (int c=0; c != ncells; ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
    int nfaces = faces.size();

    for (int n=0; n != nfaces; ++n) {
      faces_LID[n] = faces[n];
      faces_GID[n] = fmap_wghost.GID(faces_LID[n]);
    }
    cf_graph.InsertMyIndices(c, nfaces, faces_LID);
    ff_graph.InsertGlobalIndices(nfaces, faces_GID, nfaces, faces_GID);
  }
  cf_graph.FillComplete(fmap, cmap);
  ff_graph.GlobalAssemble();  // Symbolic graph is complete.

  // create global matrices
  Acc_ = Teuchos::rcp(new Epetra_Vector(cmap));
  Acf_ = Teuchos::rcp(new Epetra_CrsMatrix(Copy, cf_graph));
  Aff_ = Teuchos::rcp(new Epetra_FECrsMatrix(Copy, ff_graph));
  Sff_ = Teuchos::rcp(new Epetra_FECrsMatrix(Copy, ff_graph));
  Aff_->GlobalAssemble();
  Sff_->GlobalAssemble();

  if (flag_symmetry_) Afc_ = Acf_;
  else Afc_ = Teuchos::rcp(new Epetra_CrsMatrix(Copy, cf_graph));

  std::vector<std::string>> names(2);
  names[0] = "cell"; names[1] = "face";

  std::vector<AmanziMesh::Entity_kind>> locations(2);
  locations[0] = AmanziMesh::CELL; locations[1] = AmanziMesh::FACE;

  rhs_ = Teuchos::rcp(new CompositeVector(mesh_, names, locations, 1, true);
}


/* ******************************************************************
 * Convert elemental mass matrices into stiffness matrices and
 * assemble them into four global matrices.
 * We need an auxiliary GHOST-based vector to assemble the RHS.
 ****************************************************************** */
void MatrixMFD::AssembleGlobalMatrices() {
  Aff_->PutScalar(0.0);

  const Epetra_Map& fmap_wghost = mesh_->face_map(true);
  AmanziMesh::Entity_ID_List faces;
  std::vector<int> dirs;
  int faces_LID[MFD_MAX_FACES];
  int faces_GID[MFD_MAX_FACES];

  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  for (int c=0; c != ncells; ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
    int nfaces = faces.size();

    for (int n=0; n != nfaces; ++n) {
      faces_LID[n] = faces[n];
      faces_GID[n] = fmap_wghost.GID(faces_LID[n]);
    }
    (*Acc_)[c] = Acc_cells_[c];
    (*Acf_).ReplaceMyValues(c, nfaces, Acf_cells_[c].Values(), faces_LID);
    (*Aff_).SumIntoGlobalValues(nfaces, faces_GID, Aff_cells_[c].values());

    if (!flag_symmetry_)
      (*Afc_).ReplaceMyValues(c, nfaces, Afc_cells_[c].Values(), faces_LID);
  }
  (*Aff_).GlobalAssemble();

  // We repeat some of the loops for code clarity.
  for (int c=0; c != ncells; ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
    int nfaces = faces.size();

    (*rhs)("cell",c) = Fc_cells_[c];

    for (int n=0; n != nfaces; ++n) {
      int f = faces[n];
      (*rhs)("face",f) += Ff_cells_[c][n];
    }
  }
  rhs->GatherGhostedToMaster("face", Add);
}


/* ******************************************************************
 * Compute the face Schur complement of 2x2 block matrix.
 ****************************************************************** */
void MatrixMFD::ComputeSchurComplement(std::vector<Matrix_bc>& bc_markers,
        std::vector<double>& bc_values) {
  Sff_->PutScalar(0.0);

  AmanziMesh::Entity_ID_List faces_LID;
  std::vector<int> dirs;
  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);

  for (int c=0; c != ncells; ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces_LID, &dirs);
    int nfaces = faces_LID.size();
    Epetra_SerialDenseMatrix Schur(nfaces, nfaces);

    Epetra_SerialDenseVector& Bcf = Acf_cells_[c];
    Epetra_SerialDenseVector& Bfc = Afc_cells_[c];

    for (int n=0; n != nfaces; ++n) {
      for (int m=0; m != nfaces; ++m) {
        Schur(n, m) = Aff_cells_[c](n, m) - Bfc[n] * Bcf[m] / (*Acc_)[c];
      }
    }

    for (int n=0; n != nfaces; ++n) {  // Symbolic boundary conditions
      int f=faces_LID[n];
      if (bc_markers[f] == MFD_BC_DIRICHLET) {
        for (int m=0; m != nfaces; ++m) Schur(n, m) = Schur(m, n) = 0.0;
        Schur(n, n) = 1.0;
      }
    }

    Epetra_IntSerialDenseVector faces_GID(nfaces);
    for (int n=0; n != nfaces; ++n) faces_GID[n] = (*Acf_).ColMap().GID(faces_LID[n]);
    (*Sff_).SumIntoGlobalValues(faces_GID, Schur);
  }
  (*Sff_).GlobalAssemble();
}

/* ******************************************************************
 * Parallel matvec product A * X.
 ****************************************************************** */
int Matrix_MFD::Apply(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const {
  vector_x_->CopyFromSuperVector(*X(0));
  Apply(*vector_x_, vector_y_);
  vector_y_->CopyToSuperVector(*Y(0));
}

int Matrix_MFD::ApplyInverse(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const {
  vector_x_->CopyFromSuperVector(*X(0));
  ApplyInverse(*vector_x_, vector_y_);
  vector_y_->CopyToSuperVector(*Y(0));
}

/* ******************************************************************
 * Parallel matvec product A * X.
 ****************************************************************** */
void MatrixMFD::Apply(const CompositeVector& X,
                     const Teuchos::RCP<CompositeVector>& Y) const {
  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  int nvectors = X.num_dofs(0);

  int ierr;

  // Face unknowns:  Yf = Aff_ * Xf + Afc_ * Xc
  ierr = (*Aff_).Multiply(false, *X.ViewComponent("face",false),
                          *Y->ViewComponent("face", false));

  Epetra_MultiVector Tf(*Y->ViewComponent("face", false));
  ierr |= (*Afc_).Multiply(true, *X.ViewComponent("cell",false), Tf);  // Afc_ is kept in transpose form
  Y->ViewComponent("face",false)->Update(1.0, Tf, 1.0);

  // Cell unknowns:  Yc = Acf_ * Xf + Acc_ * Xc
  ierr |= (*Acf_).Multiply(false, *X.ViewComponent("face", false),
                           *Y->ViewComponent("cell", false));  // It performs the required parallel communications.
  ierr |= Y->ViewComponent("cell", false)->Multiply(1.0, *Acc_, *X.ViewComponent("cell", false), 1.0);

  if (ierr) {
    Errors::Message msg("MatrixMFD::Apply has failed to calculate Y = inv(A) * X.");
    Exceptions::amanzi_throw(msg);
  }
}


/* ******************************************************************
 * The OWNED cell-based and face-based d.o.f. are packed together into
 * the X and Y Epetra vectors, with the cell-based in the first part.
 *
 * WARNING: When invoked by AztecOO the arguments X and Y may be
 * aliased: possibly the same object or different views of the same
 * underlying data. Thus, we do not assign to Y until the end.
 *
 * NOTE however that this is broken for AztecOO since we use CompositeVectors,
 * not Epetra_MultiVectors.
 *
 ****************************************************************** */
void MatrixMFD::Apply(const CompositeVector& X,
                     const Teuchos::RCP<CompositeVector>& Y) const {
  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  int nvectors = X.num_dofs(0);

  // Temporary cell and face vectors.
  Epetra_MultiVector Tc(*Y->ViewComponent("cell", false));
  Epetra_MultiVector Tf(*Y->ViewComponent("face", false));

  // FORWARD ELIMINATION:  Tf = Xf - Afc_ inv(Acc_) Xc
  int ierr;
  ierr  = Tc.ReciprocalMultiply(1.0, *Acc_, *X.ViewComponent("cell", false), 0.0);
  ierr |= (*Afc_).Multiply(true, Tc, Tf);  // Afc_ is kept in transpose form
  Tf.Update(1.0, *X.ViewComponent("face", false), -1.0);

  // Solve the Schur complement system Sff_ * Yf = Tf.
  ml_prec_->ApplyInverse(Tf, *Y->ViewComponent("face", false));

  // BACKWARD SUBSTITUTION:  Yc = inv(Acc_) (Xc - Acf_ Yf)
  ierr |= (*Acf_).Multiply(false, *Y->ViewComponent("face", false), Tc);  // It performs the required parallel communications.
  Tc.Update(1.0, *X.ViewComponent("cell", false), -1.0);
  ierr |= Y->ViewComponent("cell", false)->ReciprocalMultiply(1.0, *Acc_, Tc, 0.0);

  if (ierr) {
    Errors::Message msg("MatrixMFD::ApplyInverse has failed in calculating y = A*x.");
    Exceptions::amanzi_throw(msg);
  }
}


/* ******************************************************************
 * Linear algebra operations with matrices: r = f - A * x
 ****************************************************************** */
void MatrixMFD::ComputeResidual(const CompositeVector& solution,
          const Teuchos::RCP<CompositeVector>& residual) {
  Apply(solution, residual);
  residual.Update(1.0, *rhs, -1.0);
}


/* ******************************************************************
 * Linear algebra operations with matrices: r = A * x - f
 ****************************************************************** */
void MatrixMFD::ComputeNegativeResidual(const CompositeVector& solution,
        const Teuchos::RCP<CompositeVector>& residual) {
  Apply(solution, residual);
  residual.Update(-1.0, *rhs, 1.0);
}


/* ******************************************************************
 * Initialization of the preconditioner
 ****************************************************************** */
void MatrixMFD::InitMLPreconditioner(Teuchos::ParameterList& ml_plist) {
  ml_plist_ = ml_plist;
  ml_prec_ = new ML_Epetra::MultiLevelPreconditioner(*Sff_, ml_plist_, false);
}


/* ******************************************************************
 * Rebuild ML preconditioner.
 ****************************************************************** */
void MatrixMFD::UpdateMLPreconditioner() {
  if (ml_prec_->IsPreconditionerComputed()) ml_prec_->DestroyPreconditioner();
  ml_prec_->SetParameterList(ml_plist_);
  ml_prec_->ComputePreconditioner();
}



/* ******************************************************************
 * WARNING: Routines requires original mass matrices (Aff_cells_), i.e.
 * before boundary conditions were imposed.
 *
 * WARNING: Since diffusive flux is not continuous, we derive it only
 * once (using flag) and in exactly the same manner as in routine
 * Flow_PK::addGravityFluxes_DarcyFlux.
 ****************************************************************** */
void MatrixMFD::DeriveFlux(const CompositeVector& solution,
                           const Teuchos::RCP<CompositeVector>& flux) {

  AmanziMesh::Entity_ID_List faces;
  std::vector<double> dp;
  std::vector<int> dirs;

  int ncells = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  int nfaces_owned = mesh_->num_entities(AmanziMesh::FACE, AmanziMesh::OWNED);
  std::vector<int> flag(solution->ViewComponent("face", true)->MyLength(), 0);

  for (int c=0; c != ncells; ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
    int nfaces = faces.size();

    dp.resize(nfaces);
    for (int n=0; n != nfaces; ++n) {
      int f = faces[n];
      dp[n] = solution("cell", c) - solution("face", f);
    }

    for (int n=0; n != nfaces; ++n) {
      int f = faces[n];
      if (f < nfaces_owned && !flag[f]) {
        double s = 0.0;
        for (int m=0; m != nfaces; ++m) s += Aff_cells_[c](n, m) * dp[m];
        (*flux)(f) = s * dirs[n];
        flag[f] = 1;
      }
    }
  }
}


/* ******************************************************************
 * Derive Darcy velocity in cells.
 * WARNING: It cannot be consistent with the Darcy flux.
 ****************************************************************** */
void MatrixMFD::DeriveCellVelocity(const CompositeVector& flux,
        const Teuchos::RCP<CompositeVector>& velocity) const {

  Teuchos::LAPACK<int, double> lapack;

  int dim = mesh_->space_dimension();
  Teuchos::SerialDenseMatrix<int, double> matrix(dim, dim);
  double rhs_cell[dim];

  AmanziMesh::Entity_ID_List faces;
  std::vector<int> dirs;

  int ncells_owned = mesh_->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);
  for (int c=0; c != ncells_owned; ++c) {
    mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
    int nfaces = faces.size();

    for (int i=0; i != dim; ++i) rhs_cell[i] = 0.0;
    matrix.putScalar(0.0);

    for (int n=0; n != nfaces; ++n) {  // populate least-square matrix
      int f = faces[n];
      const AmanziGeometry::Point& normal = mesh_->face_normal(f);
      double area = mesh_->face_area(f);

      for (int i=0; i != dim; ++i) {
        rhs_cell[i] += normal[i] * flux(f);
        matrix(i,i) += normal[i] * normal[i];
        for (int j=i+1; j != dim; ++j) {
          matrix(j,i) = matrix(i,j) += normal[i] * normal[j];
        }
      }
    }

    int info;
    lapack.POSV('U', dim, 1, matrix.values(), dim, rhs_cell, dim, &info);

    for (int i=0; i<dim; i++) (*velocity)(i,c) = rhs_cell[i];
  }
}

}  // namespace AmanziFlow
}  // namespace Amanzi
