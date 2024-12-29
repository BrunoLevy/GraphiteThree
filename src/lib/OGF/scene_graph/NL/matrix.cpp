/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 */

#include <OGF/scene_graph/NL/matrix.h>
#include <OGF/scene_graph/NL/vector.h>
#include <OGF/gom/reflection/meta.h>
#include <geogram/NL/nl_matrix.h>
#include <geogram/NL/nl_iterative_solvers.h>
#include <geogram/NL/nl_preconditioners.h>
#include <geogram/basic/logger.h>

namespace {
    using namespace OGF;

    bool init_direct_solver(NL::Matrix::Factorization factorization) {
	switch(factorization) {
	    case NL::Matrix::SuperLU: {
		if(!nlInitExtension("SUPERLU")) {
		    return false;
		}
	    } break;
	    case NL::Matrix::SuperLU_perm: {
		if(!nlInitExtension("SUPERLU")) {
		    return false;
		}
	    } break;
	    case NL::Matrix::SuperLU_sym: {
		if(!nlInitExtension("SUPERLU")) {
		    return false;
		}
	    } break;
	    case NL::Matrix::Cholmod: {
		if(!nlInitExtension("CHOLMOD")) {
		    return false;
		}
	    }
	}
	return true;
    }

    bool check_vector(
	const NL::Matrix* matrix, const NL::Vector* vector,
	bool check_m, bool check_n,
	const char* func_name, const char* arg_name
    ) {
	std::string func_name_full =
	    std::string("OGF::NL::Matrix(")
	    + String::to_string(matrix->get_m())
	    + ","
	    + String::to_string(matrix->get_n())
	    + ")::" + func_name
	    ;

	std::string arg_name_full =
	    std::string(arg_name) + "(" +
	    String::to_string(vector->get_nb_elements()) + ")";

	if(vector->get_element_meta_type() != ogf_meta<double>::type()) {
	    Logger::err("NL") << func_name_full << " " << arg_name_full
			      << " is not a vector of doubles"
			      << std::endl;
	    return false;
	}

	if(check_m && vector->get_nb_elements() != matrix->get_m()) {
	    Logger::err("NL") << func_name_full << " " << arg_name_full
			      << " vector size does not match number of rows"
			      << std::endl;
	    return false;
	}

	if(check_n && vector->get_nb_elements() != matrix->get_n()) {
	    Logger::err("NL") << func_name_full << " " << arg_name_full
			      << " vector size does not match number of columns"
			      << std::endl;
	    return false;
	}

	return true;
    }

}

namespace OGF {
    namespace NL {

	Matrix::Matrix(index_t m, index_t n) {
	    impl_ = nlSparseMatrixNew(m,n,NL_MATRIX_STORE_ROWS);
	}

	Matrix::~Matrix() {
	    nlDeleteMatrix(impl_);
	    impl_ = nullptr;
	}

	index_t Matrix::get_m() const {
	    return index_t(impl_->m);
	}

	index_t Matrix::get_n() const {
	    return index_t(impl_->n);
	}

	Matrix::Format Matrix::get_format() const {
	    if(impl_->type == NL_MATRIX_SPARSE_DYNAMIC) {
		return Dynamic;
	    } else if(impl_->type == NL_MATRIX_CRS) {
		return CRS;
	    }
	    return Factorized;
	}

	void Matrix::add_coefficient(index_t i, index_t j, double a) {
	    if(impl_->type != NL_MATRIX_SPARSE_DYNAMIC) {
		Logger::err("NL")
		    << "Matrix::add_coefficient() called on wrong matrix type"
		    << std::endl;
		return;
	    }
	    if(
		i >= index_t(impl_->m) ||
		j >= index_t(impl_->n)
	    ) {
		Logger::err("NL")
		    << "Matrix(" << impl_->m << "," << impl_->n
		    << ")::add_coefficient(" << i << "," << j
		    << ") index out of bound" << std::endl;
		return;
	    }
	    nlSparseMatrixAdd((NLSparseMatrix*)(impl_), i, j, a);
	}


	void Matrix::add_coefficients(
	    const Vector* I, const Vector* J, const Vector* A
	) {
	    if(
		I->get_element_meta_type()!=ogf_meta<Numeric::uint32>::type() &&
		I->get_element_meta_type()!=ogf_meta<Numeric::int32>::type()
	    ) {
		Logger::err("NL")
		    << "Matrix(" << impl_->m << "," << impl_->n
		    << ")::add_coefficients()"
		    << " I is not an integer vector"
		    << std::endl;
		return;
	    }
	    if(
		J->get_element_meta_type()!=ogf_meta<Numeric::uint32>::type() &&
		J->get_element_meta_type()!=ogf_meta<Numeric::int32>::type()
	    ) {
		Logger::err("NL")
		    << "Matrix(" << impl_->m << "," << impl_->n
		    << ")::add_coefficients()"
		    << " J is not an integer vector"
		    << std::endl;
		return;
	    }
	    if(A->get_element_meta_type()!=ogf_meta<double>::type()) {
		Logger::err("NL")
		    << "Matrix(" << impl_->m << "," << impl_->n
		    << ")::add_coefficients()"
		    << " A is not a vector of doubles"
		    << std::endl;
		return;
	    }
	    if(
		I->dimension() != 1 || J->dimension() != 1 ||
		A->dimension() != 1
	    ) {
		Logger::err("NL")
		    << "Matrix(" << impl_->m << "," << impl_->n
		    << ")::add_coefficients()"
		    << " wrong dimension"
		    << std::endl;
		return;
	    }

	    if(
		I->nb_elements() != J->nb_elements() ||
		J->nb_elements() != A->nb_elements()
	    ) {
		Logger::err("NL")
		    << "Matrix(" << impl_->m << "," << impl_->n
		    << ")::add_coefficients()"
		    << " vector size mismatch"
		    << std::endl;
		return;
	    }

	    const index_t* p_i = reinterpret_cast<index_t*>(I->data());
	    const index_t* p_j = reinterpret_cast<index_t*>(J->data());
	    const double*  p_a = A->data_double();

	    for(index_t k=0; k<I->nb_elements(); ++k) {
		if(
		    p_i[k] >= index_t(impl_->m) ||
		    p_j[k] >= index_t(impl_->n)
		) {
		    Logger::err("NL")
			<< "Matrix(" << impl_->m << "," << impl_->n
			<< ")::add_coefficients()"
			<< " coefficient larger than matrix size"
			<< std::endl;
		    return;
		}
	    }

	    for(index_t k=0; k<I->nb_elements(); ++k) {
		nlSparseMatrixAdd(
		    (NLSparseMatrix*)(impl_), p_i[k], p_j[k], p_a[k]
		);
	    }
	}

	void Matrix::add_coefficients_to_diagonal(const Vector* A) {
	    if(A->get_element_meta_type()!=ogf_meta<double>::type()) {
		Logger::err("NL")
		    << "Matrix(" << impl_->m << "," << impl_->n
		    << ")::add_coefficients()"
		    << " A is not a vector of doubles"
		    << std::endl;
		return;
	    }
	    if(A->dimension() != 1) {
		Logger::err("NL")
		    << "Matrix(" << impl_->m << "," << impl_->n
		    << ")::add_coefficients()"
		    << " wrong dimension"
		    << std::endl;
		return;
	    }
	    if(A->nb_elements() > get_m() || A->nb_elements() > get_n()) {
		Logger::err("NL")
		    << "Matrix(" << impl_->m << "," << impl_->n
		    << ")::add_coefficients()"
		    << " wrong size"
		    << std::endl;
		return;
	    }
	    const double* p_a = A->data_double();
	    for(index_t k=0; k<A->nb_elements(); ++k) {
		nlSparseMatrixAdd(
		    (NLSparseMatrix*)(impl_), k, k, p_a[k]
		);
	    }
	}

	void Matrix::mult(const Vector* x, Vector* y) const {
	    if(!check_vector(this, x, false, true, "mult", "x")) {
		return;
	    }
	    if(!check_vector(this, y, true, false, "mult", "x")) {
		return;
	    }
	    nlMultMatrixVector(
		impl_, x->data_double(), y->data_double()
	    );
	}

	void Matrix::compress() {
	    nlMatrixCompress(&impl_);
	}

	Matrix* Matrix::factorize(Factorization factorization) const {
	    NLMatrix result = nullptr;
	    if(init_direct_solver(factorization)) {
		switch(factorization) {
		    case SuperLU: {
			result = nlMatrixFactorize(impl_, NL_SUPERLU_EXT);
		    } break;
		    case SuperLU_perm: {
			result = nlMatrixFactorize(impl_, NL_PERM_SUPERLU_EXT);
		    } break;
		    case SuperLU_sym: {
			result =
			    nlMatrixFactorize(impl_, NL_SYMMETRIC_SUPERLU_EXT);
		    } break;
		    case Cholmod: {
			result = nlMatrixFactorize(impl_, NL_CHOLMOD_EXT);
		    }
		}
	    }
	    if(result == nullptr) {
		Logger::err("NL") << "Could not factorize matrix"
				  << std::endl;
		return nullptr;
	    }
	    return new Matrix(result);
	}


	void Matrix::solve_iterative(
	    const Vector* b, Vector* x,
	    IterativeSolver solver,
	    Preconditioner precond,
	    index_t max_iter, double eps
	) {
	    if(get_m() != get_n()) {
		Logger::err("NL") << "Matrix::solve_iterative() "
				  << "matrix is not square"
				  << std::endl;
		return;
	    }
	    if(!check_vector(this, b, true, true, "solve_iterative", "b")) {
		return;
	    }
	    if(!check_vector(this, x, true, true, "solve_iterative", "x")) {
		return;
	    }

	    NLMatrix P = nullptr;
	    switch(precond) {
		case None: {
		} break;
		case Jacobi: {
		    P = nlNewJacobiPreconditioner(impl_);
		} break;
		case SSOR: {
		    P = nlNewSSORPreconditioner(impl_, 1.5);
		} break;
	    }


	    NLenum nl_solver = NL_BICGSTAB;
	    switch(solver) {
		case CG: {
		    nl_solver = NL_CG;
		} break;
		case GMRES: {
		    nl_solver = NL_GMRES;
		} break;
		case BiCGSTAB: {
		    nl_solver = NL_BICGSTAB;
		} break;
	    }

	    NLuint inner_iter = 5;

	    Memory::clear(x->data(),sizeof(double)*get_n());


	    nlSolveSystemIterative(
		nlHostBlas(), impl_, P, b->data_double(), x->data_double(),
		nl_solver, eps, max_iter, inner_iter
	    );

	    if(P != nullptr) {
		nlDeleteMatrix(P);
	    }
	}


	void Matrix::solve_direct(
	    const Vector* b, Vector* x,
	    Factorization factorization
	) {
	    if(get_m() != get_n()) {
		Logger::err("NL") << "Matrix::solve_direct() "
				  << "matrix is not square"
				  << std::endl;
		return;
	    }
	    if(!check_vector(this, b, true, true, "solve_iterative", "b")) {
		return;
	    }
	    if(!check_vector(this, x, true, true, "solve_iterative", "x")) {
		return;
	    }

	    Matrix* F = factorize(factorization);
	    if(F == nullptr) {
		return;
	    }
	    F->mult(b,x);
	    delete F;
	}

	void Matrix::solve_symmetric(
	    const Vector* b, Vector* x, bool direct_solver
	) {
	    if(direct_solver) {
		if(nlInitExtension("CHOLMOD")) {
		    solve_direct(b, x, Cholmod);
		    return;
		}
		if(nlInitExtension("SUPERLU")) {
		    solve_direct(b, x, SuperLU_sym);
		    return;
		}
	    }
	    solve_iterative(b,x,CG,Jacobi,NLuint(get_n()*5),1e-3);
	}
    }
}
