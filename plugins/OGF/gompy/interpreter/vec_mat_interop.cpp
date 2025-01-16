/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
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

#include <OGF/gompy/interpreter/vec_mat_interop.h>
#include <OGF/gom/types/any.h>
#include <OGF/gom/reflection/meta.h>
#include <geogram/basic/matrix.h>
#include <geogram/basic/vecg.h>

namespace {
    using namespace OGF;

    /**
     * \brief Converts a Python object into a value
     * \tparam T type of the value
     * \param[in] obj a pointer to a Python object
     * \param[out] result a reference to the read value
     * \retval true if the conversion was successful
     * \retval false otherwise (LUA type did not match)
     */
    template <class T> inline bool python_tographiteveccomp(
	PyObject* obj, T& result
    ) {
	geo_argused(obj);
	geo_argused(result);
	geo_assert_not_reached;
    }


    template <> inline bool python_tographiteveccomp<double>(
	PyObject* obj, double& result
    ) {
	if(PyFloat_Check(obj)) {
	    result = PyFloat_AsDouble(obj);
	    return true;
	} else if(PyLong_Check(obj)) {
	    result = double(PyLong_AsLong(obj));
	    return true;
	}
	return false;
    }

    template <> inline bool python_tographiteveccomp<Numeric::int32>(
	PyObject* obj, Numeric::int32& result
    ) {
	if(PyLong_Check(obj)) {
	    result = Numeric::int32(PyLong_AsLong(obj));
	    return true;
	}
	return false;
    }

    /**
     * \brief Converts a python object into a Graphite vec2,vec3 or vec4
     * \tparam N dimension of the vector
     * \tparam T type of the coefficients
     * \param[in] obj a pointer to the Python object
     * \param[out] result a reference to the converted vector
     * \retval true if the conversion was successful
     * \retval false otherwise (mtype does not match, or object
     *  is not a list of the correct size).
     */
    template<unsigned int N, class T> inline bool python_tographitevec(
	PyObject* obj, ::GEO::vecng<N,T>& result
    ) {
	if(!PyList_Check(obj)) {
	    return false;
	}
	index_t n = index_t(PyList_Size(obj));
	if(n != index_t(N)) {
	    return false;
	}
	for(index_t i=0; i<n; ++i) {
	    PyObject* coord_obj = PyList_GetItem(obj,i);
	    if(!python_tographiteveccomp(coord_obj, result[i])) {
		return false;
	    }
	}
	return true;
    }

    /**
     * \brief Converts a python object into a Graphite vec2,vec3 or vec4
     * \tparam N dimension of the vector
     * \tparam T type of the coefficients
     * \param[in] obj a pointer to the Python object
     * \param[out] result the converted vector as an Any
     * \param[in] mtype the meta-type (vec2, vec3 or vec4)
     * \retval true if the conversion was successful
     * \retval false otherwise (mtype does not match, or object
     *  is not a list of the correct size).
     */
    template<unsigned int N, class T> inline bool python_tographitevec(
	PyObject* obj, Any& result, MetaType* mtype
    ) {
	if(mtype != ogf_meta<::GEO::vecng<N,double> >::type()) {
	    return false;
	}
	::GEO::vecng<N,T> V;
	if(!python_tographitevec(obj,V)) {
	    return false;
	}
	result.set_value(V);
	return true;
    }

    /**
     * \brief Converts a python object into a Graphite mat2, mat3 or mat4
     * \tparam N dimension of the vector
     * \tparam T type of the coefficients
     * \param[in] obj a pointer to the Python object
     * \param[out] result a reference to the converted matrix
     * \retval true if the conversion was successful
     * \retval false otherwise (mtype does not match, or object
     *  is not a list of the correct size).
     */
    template<unsigned int N, class T> inline bool python_tographitemat(
	PyObject* obj, ::GEO::Matrix<N,T>& result
    ) {
	if(!PyList_Check(obj)) {
	    return false;
	}
	index_t n = index_t(PyList_Size(obj));
	if(n != index_t(N)) {
	    return false;
	}
	for(index_t i=0; i<n; ++i) {
	    ::GEO::vecng<N,T> row;
	    PyObject* row_obj = PyList_GetItem(obj,i);
	    if(!python_tographitevec(row_obj, row)) {
		return false;
	    }
	    for(index_t j=0; j<n; ++j) {
		result(i,j) = row[j];
	    }
	}
	return true;
    }

    /**
     * \brief Converts a python object into a Graphite mat2, mat3 or mat4
     * \tparam N dimension of the vector
     * \tparam T type of the coefficients
     * \param[in] obj a pointer to the Python object
     * \param[out] result a reference to the converted matrix as an Any
     * \retval true if the conversion was successful
     * \retval false otherwise (mtype does not match, or object
     *  is not a list of the correct size).
     */
    template<unsigned int N, class T> inline bool python_tographitemat(
	PyObject* obj, Any& result, MetaType* mtype
    ) {
	if(mtype != ogf_meta<::GEO::Matrix<N,double> >::type()) {
	    return false;
	}
	::GEO::Matrix<N,T> M;
	if(!python_tographitemat(obj,M)) {
	    return false;
	}
	result.set_value(M);
	return true;
    }

    /*************************************************************************/

    template<class T> inline PyObject* graphiteveccomp_to_python(
	T value
    ) {
	geo_argused(value);
	geo_assert_not_reached;
    }

    template<> inline PyObject* graphiteveccomp_to_python<double>(
	double value
    ) {
	return PyFloat_FromDouble(value);
    }

    template<> inline PyObject*
    graphiteveccomp_to_python<Numeric::int32>(
	Numeric::int32 value
    ) {
	return PyLong_FromLong(long(value));
    }


    template <unsigned int N, class T> inline PyObject* graphitevec_to_python(
	const ::GEO::vecng<N,T>& V
    ) {
	PyObject* result = PyList_New(N);
	for(index_t i=0; i<N; ++i) {
	    PyList_SetItem(result, i, graphiteveccomp_to_python(V[i]));
	}
	return result;
    }

    template <unsigned int N, class T> inline PyObject* graphitevec_to_python(
	const Any& val
    ) {
	if(val.meta_type() != ogf_meta<::GEO::vecng<N,T> >::type()) {
	    return nullptr;
	}
	::GEO::vecng<N,T> V;
	val.get_value(V);
	return graphitevec_to_python(V);
    }


    template <unsigned int N, class T> inline PyObject* graphitemat_to_python(
	const ::GEO::Matrix<N,T>& M
    ) {
	PyObject* result = PyList_New(N);
	for(index_t i=0; i<index_t(N); ++i) {
	    PyObject* row = PyList_New(N);
	    for(index_t j=0; j<index_t(N); ++j) {
		PyList_SetItem(row, j, graphiteveccomp_to_python(M(i,j)));
	    }
	    PyList_SetItem(result, i, row);
	}
	return result;
    }

    template <unsigned int N, class T> inline PyObject* graphitemat_to_python(
	const Any& val
    ) {
	if(val.meta_type() != ogf_meta<::GEO::Matrix<N,T> >::type()) {
	    return nullptr;
	}
	::GEO::Matrix<N,T> M;
	val.get_value(M);
	return graphitemat_to_python(M);
    }

}

/*****************************************************************************/

namespace OGF {

    namespace GOMPY {

	bool python_to_graphite_mat_vec(
	    PyObject* obj, Any& result, MetaType* mtype
	) {
	    if(!PyList_Check(obj)) {
		return false;
	    }
	    if(python_tographitevec<2,double>(obj, result, mtype)) {
		return true;
	    }
	    if(python_tographitevec<3,double>(obj, result, mtype)) {
		return true;
	    }
	    if(python_tographitevec<4,double>(obj, result, mtype)) {
		return true;
	    }
	    if(python_tographitevec<2,Numeric::int32>(obj, result, mtype)) {
		return true;
	    }
	    if(python_tographitevec<3,Numeric::int32>(obj, result, mtype)) {
		return true;
	    }
	    if(python_tographitevec<4,Numeric::int32>(obj, result, mtype)) {
		return true;
	    }
	    if(python_tographitemat<2,double>(obj, result, mtype)) {
		return true;
	    }
	    if(python_tographitemat<3,double>(obj, result, mtype)) {
		return true;
	    }
	    if(python_tographitemat<4,double>(obj, result, mtype)) {
		return true;
	    }
	    return false;
	}

	/***********************************************************************/

	PyObject* graphite_mat_vec_to_python(const Any& arg) {
	    PyObject* result = nullptr;
	    if(result == nullptr) {
		result = graphitevec_to_python<2,double>(arg);
	    }
	    if(result == nullptr) {
		result = graphitevec_to_python<3,double>(arg);
	    }
	    if(result == nullptr) {
		result = graphitevec_to_python<4,double>(arg);
	    }
	    if(result == nullptr) {
		result = graphitevec_to_python<2,Numeric::int32>(arg);
	    }
	    if(result == nullptr) {
		result = graphitevec_to_python<3,Numeric::int32>(arg);
	    }
	    if(result == nullptr) {
		result = graphitevec_to_python<4,Numeric::int32>(arg);
	    }
	    if(result == nullptr) {
		result = graphitemat_to_python<2,double>(arg);
	    }
	    if(result == nullptr) {
		result = graphitemat_to_python<3,double>(arg);
	    }
	    if(result == nullptr) {
		result = graphitemat_to_python<4,double>(arg);
	    }
	    return result;
	}

    /***********************************************************************/

    }

}
