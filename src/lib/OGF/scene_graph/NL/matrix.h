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
 
#ifndef H_OGF_SCENE_GRAPH_NL_MATRIX_H
#define H_OGF_SCENE_GRAPH_NL_MATRIX_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/gom/types/object.h>

struct NLMatrixStruct;
typedef NLMatrixStruct* NLMatrix;

namespace OGF {
    namespace NL {

	class Vector;

	gom_class SCENE_GRAPH_API Matrix : public Object {
	  public:

	    /**
	     * \brief Internal format of a matrix.
	     */
	    enum Format { Dynamic, CRS, Factorized };

	    /**
	     * \brief Factorization methods.
	     */
	    enum Factorization { SuperLU, SuperLU_perm, SuperLU_sym, Cholmod };

	    /**
	     * \brief Iterative Solvers.
	     */
	    enum IterativeSolver { CG, GMRES, BiCGSTAB };

	    /**
	     * \brief Preconditioners.
	     */
	    enum Preconditioner { None, Jacobi, SSOR};
	    
	    /**
	     * \brief Matrix constructor.
	     * \param[in] m number of rows.
	     * \param[in] n number of columns.
	     */
	    Matrix(index_t m, index_t n);

	    /**
	     * \brief Matrix destructor.
	     */
	     ~Matrix() override;

	  gom_properties:
	    /**
	     * \brief Gets the number of rows.
	     * \return the number of rows.
	     */
	    index_t get_m() const;

	    /**
	     * \brief Gets the number of columns.
	     * \return the number of columns.
	     */
	    index_t get_n() const;

	    /**
	     * \brief Gets the format of the matrix.
	     * \retval Dynamic is the default format. Supports add_coefficient()
	     * \retval CRS for Compressed Row Storage. 
	     * \retval Factorized for solving linear systems.
	     */
	    Format get_format() const;
	    
	  gom_slots:
	    
	    /**
	     * \brief Adds a coefficient.
	     * \details If the coefficient already exists, a is added
	     *  to its previous value.
	     * \param[in] i row index
	     * \param[in] j column index
	     * \param[in] a value to be added
	     */
	    void add_coefficient(index_t i, index_t j, double a);

	    /**
	     * \brief Computes a matrix vector product.
	     * \param[in] x the vector to be multiplied. Should have
	     *  n elements.
	     * \param[out] y the result. Should have m elements.
	     */
	    void mult(const Vector* x, Vector* y) const;

	    /**
	     * \brief Compresses the matrix from the dynamic format to 
	     *  the CRS format.
	     * \details When the matrix is compressed, matrix vector product
	     *  are faster, but add_coefficient() can no longer be called.
	     */
	    void compress();

	    /**
	     * \brief Factorizes the matrix.
	     * \details This depends on OpenNL extensions that may be 
	     *  unsupported on the architecture.
	     * \param[in] factorization one of 
	     *   - SuperLU      : plain SuperLU algorithm
	     *   - SuperLU_perm : SuperLU with permutation
	     *   - SuperLU_sym  : SuperLU with permutation for symmetric matrix
	     *   - CHOLMOD      : modified Cholesky (for symmetric matrix)
	     */
	    Matrix* factorize(Factorization factorization) const;

	    /**
	     * \brief Solves a linear system using an iterative solver.
	     * \param[in] b the right hand side
	     * \param[out] x the solution
	     * \param[in] solver one of CG, GMRES, BiCGSTAB
	     * \param[in] precond one of None, Jacobi, SSOR
	     * \param[in] max_iter maximum number of iterations
	     * \param[in] eps convergence criterion. Iterations are stopped
	     *  whenever \f$ \| Ax - b\| / \| b \| \f$ is smaller than 
	     *  eps.
	     */
	    void solve_iterative(
		const Vector* b, Vector* x,
		IterativeSolver solver = BiCGSTAB,
		Preconditioner precond = None,
		index_t max_iter=1000, double eps = 1e-3
	    );

	    /**
	     * \brief Solves a linear system using a direct solver.
	     * \param[in] b the right hand side
	     * \param[out] x the solution
	     * \param[in] factorization one of 
	     *   SuperLU, SuperLU_perm, SuperLU_sym, Cholmod
	     */
	    void solve_direct(
		const Vector* b, Vector* x,
		Factorization factorization = SuperLU_perm
	    );

	    /**
	     * \brief Solves a linear system with a symmetric matrix 
	     *  using default paramerers.
	     * \param[in] b the right hand side
	     * \param[out] x the solution
	     * \param[in] direct_solver if true, 
	     *   tentatively use a direct solver.
	     */
	    void solve_symmetric(
		const Vector* b, Vector* x,
		bool direct_solver = false
	    );

	  public:
	    /**
	     * \brief Gets the underlying OpenNL implementation.
	     * \return a pointer to the NLMatrix.
	     */
	     NLMatrix implementation() const {
		return impl_;
	    }
	    
	  protected:

	    /**
	     * \brief Matrix constructor.
	     * \param[in] impl a pointer to the OpenNL implementation.
	     * \details For internal use only.
	     */
	    Matrix(NLMatrix impl) : impl_(impl) {
	    }
	    
	  private:
	    NLMatrix impl_;
	};
	
    }
}

#endif
