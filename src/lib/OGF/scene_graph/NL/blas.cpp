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

#include <OGF/scene_graph/NL/blas.h>
#include <OGF/scene_graph/NL/vector.h>
#include <OGF/gom/reflection/meta.h>
#include <geogram/NL/nl_blas.h>
#include <geogram/basic/logger.h>

namespace {
    using namespace OGF;
    using namespace OGF::NL;

    bool check_vector_type(
	const Vector* V, const char* func_name, const char* arg_name
    ) {
	if(V->get_element_meta_type() != ogf_meta<double>::type()) {
	    Logger::err("NL") << func_name << " arg " << arg_name
			      << " is a vector of "
			      << V->get_element_meta_type()->name()
			      << " (expected double)"
			      << std::endl;
	    return false;
	}
	return true;
    }
}

namespace OGF {
    namespace NL {
	
	Blas::Blas() {
	    impl_ = nlHostBlas();
	}
	
	Blas::~Blas() {
	}

	void Blas::copy(const Vector* x, Vector* y) {
	    if(!check_vector_type(x, "Blas::copy", "x")) {
		return;
	    }
	    if(!check_vector_type(y, "Blas::copy", "y")) {
		return;
	    }
	    if(x->nb_elements() != y->nb_elements()) {
		Logger::err("NL") << "Blas::copy "
				  << "Vectors x and y have different sizes"
				  << std::endl;
		return;
	    }
	    NLint n = NLint(x->get_nb_elements());
	    impl_->Dcopy(
		impl_, n, x->data_double(), 1, y->data_double(), 1
	    );
	}

	void Blas::scal(double a, Vector* x) {
	    if(!check_vector_type(x, "Blas::scal", "x")) {
		return;
	    }
	    NLint n = NLint(x->get_nb_elements());
	    impl_->Dscal(
		impl_, n, a, x->data_double(), 1
	    );
	}

	double Blas::dot(const Vector* x, const Vector* y) {
	    if(!check_vector_type(x, "Blas::dot", "x")) {
		return 0.0;
	    }
	    if(!check_vector_type(x, "Blas::dot", "y")) {
		return 0.0;
	    }
	    if(x->nb_elements() != y->nb_elements()) {
		Logger::err("NL") << "Blas::dot "
				  << "Vectors x and y have different sizes"
				  << std::endl;
		return 0.0;
	    }
	    NLint n = NLint(x->nb_elements());
	    return impl_->Ddot(
		impl_, n, x->data_double(), 1, y->data_double(), 1
	    );
	}

	double Blas::nrm2(const Vector* x) {
	    if(!check_vector_type(x, "Blas::nrm2", "x")) {
		return 0.0;
	    }
	    NLint n = NLint(x->nb_elements());
	    return impl_->Dnrm2(
		impl_, n, x->data_double(), 1
	    );
	}

	void Blas::axpy(double a, const Vector* x, Vector* y) {
	    if(!check_vector_type(x, "Blas::axpy", "x")) {
		return;
	    }
	    if(!check_vector_type(x, "Blas::axpy", "y")) {
		return;
	    }
	    if(x->nb_elements() != y->nb_elements()) {
		Logger::err("NL") << "Blas::axpy "
				  << "Vectors x and y have different sizes"
				  << std::endl;
		return;
	    }
	    NLint n = NLint(x->nb_elements());
	    impl_->Daxpy(
		impl_, n, a, x->data_double(), 1, y->data_double(), 1
	    );
	    
	}
    }
}
