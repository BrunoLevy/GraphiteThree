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

#ifndef H_OGF_SCENE_GRAPH_NL_LIBRARY_H
#define H_OGF_SCENE_GRAPH_NL_LIBRARY_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/gom/types/object.h>
#include <OGF/scene_graph/NL/blas.h>

namespace OGF {

    namespace NL {

	class Vector;
	class Matrix;

	/**
	 * \brief Scripting interface to the OpenNL library.
	 */
	gom_class SCENE_GRAPH_API Library : public Object {

	  public:
	    /**
	     * \brief Library constructor.
	     */
	    Library();

	    /**
	     * \brief Library destructor.
	     */
	    virtual ~Library() override;

	  gom_slots:
	    /**
	     * \brief Creates a vector.
	     * \param[in] dim the dimension of the vector.
	     * \return the newly created vector.
	     */
	    Vector* create_vector(index_t dim=0);

	    /**
	     * \brief Creates a matrix.
	     * \param[in] m number of rows
	     * \param[in] n number of columns
	     * \return the newly created matrix.
	     */
	    Matrix* create_matrix(index_t m, index_t n);

	  gom_properties:
	    /**
	     * \brief Gets the local BLAS implementation.
	     * \return a pointer to the BLAS implementation.
	     */
	    Blas* get_blas() const {
		return blas_;
	    }

	  private:
	    Blas_var blas_;
	};
    }
}

#endif
