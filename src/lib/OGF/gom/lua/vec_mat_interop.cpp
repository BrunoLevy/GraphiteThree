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

#include <OGF/gom/lua/vec_mat_interop.h>
#include <OGF/gom/reflection/meta.h>
#include <geogram/lua/lua_vec_mat.h>

/*
extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}
*/

namespace OGF {
    namespace GOMLua {

	/**
	 * \brief Converts a lua object into a Graphite vec2,vec3 or vec4
	 * \tparam N dimension of the vector
	 * \tparam T type of the components
	 * \param[in] L a pointer to the Lua state
	 * \param[in] index the index of the object in thestack
	 * \param[out] result the converted vector as an Any
	 * \param[in] mtype the meta-type (vec2, vec3 or vec4)
	 * \retval true if the conversion was successful
	 * \retval false otherwise (mtype does not match, or object on
	 *  the stack is not an integer-indexed table of numbers of the
	 *  correct size).
	 */
	template<unsigned int N, class T> inline bool lua_tographitevec(
	    lua_State* L, int index, Any& result, MetaType* mtype
	) {
	    if(mtype != ogf_meta<::GEO::vecng<N,T> >::type()) {
		return false;
	    }
	    GEO::vecng<N,T> V;
	    if(!lua_tovec(L, index, V)) {
		return false;
	    }
	    result.set_value(V);
	    return true;
	}

	/***************************************************/

	/**
	 * \brief Converts a Lua object into a Graphite mat2, mat3 or mat4
	 * \tparam N dimension of the vector
	 * \tparam T type of the coefficients
	 * \param[in] L a pointer to the Lua state
	 * \param[in] index the index of the object in thestack
	 * \param[out] result a reference to the converted matrix as an Any
	 * \retval true if the conversion was successful
	 * \retval false otherwise (mtype does not match, or object
	 *  is not a table of the correct size).
	 */
	template<unsigned int N, class T> inline bool lua_tographitemat(
	    lua_State* L, int index, Any& result, MetaType* mtype
	) {
	    if(mtype != ogf_meta<::GEO::Matrix<N,double> >::type()) {
		return false;
	    }
	    ::GEO::Matrix<N,T> M;
	    if(!lua_tomat(L,index,M)) {
		return false;
	    }
	    result.set_value(M);
	    return true;
	}


	bool lua_to_graphite_mat_vec(
	    lua_State* L, int index, Any& result, MetaType* mtype
	) {

	    if(lua_tographitevec<2,double>(L, index, result, mtype)) {
		return true;
	    }

	    if(lua_tographitevec<3,double>(L, index, result, mtype)) {
		return true;
	    }

	    if(lua_tographitevec<4,double>(L, index, result, mtype)) {
		return true;
	    }

	    if(lua_tographitevec<2,Numeric::int32>(L, index, result, mtype)) {
		return true;
	    }

	    if(lua_tographitevec<3,Numeric::int32>(L, index, result, mtype)) {
		return true;
	    }

	    if(lua_tographitevec<4,Numeric::int32>(L, index, result, mtype)) {
		return true;
	    }

	    if(lua_tographitemat<2,double>(L, index, result, mtype)) {
		return true;
	    }

	    if(lua_tographitemat<3,double>(L, index, result, mtype)) {
		return true;
	    }

	    if(lua_tographitemat<4,double>(L, index, result, mtype)) {
		return true;
	    }

	    return false;
	}

	bool lua_to_graphite_vec2i(lua_State* L, int index, vec2i& result) {
	    return lua_tovec(L, index, result);
	}

	/********************************************************************/

	template <unsigned int N, class T> inline bool lua_pushvec(
	    lua_State* L, const Any& val
	) {
	    if(val.meta_type() != ogf_meta<::GEO::vecng<N,T> >::type()) {
		return false;
	    }
	    ::GEO::vecng<N,T> V;
	    val.get_value(V);
	    lua_pushvec(L,V);
	    return true;
	}

	template <unsigned int N, class T> inline bool lua_pushmat(
	    lua_State* L, const Any& val
	) {
	    if(val.meta_type() != ogf_meta<::GEO::Matrix<N,T> >::type()) {
		return false;
	    }
	    ::GEO::Matrix<N,T> M;
	    val.get_value(M);
	    lua_pushmat(L,M);
	    return true;
	}

	bool push_mat_vec(lua_State* L, const Any& value) {
	    if(lua_pushvec<2,double>(L,value)) {
		return true;
	    }

	    if(lua_pushvec<3,double>(L,value)) {
		return true;
	    }

	    if(lua_pushvec<4,double>(L,value)) {
		return true;
	    }

	    if(lua_pushvec<2,Numeric::int32>(L,value)) {
		return true;
	    }

	    if(lua_pushvec<3,Numeric::int32>(L,value)) {
		return true;
	    }

	    if(lua_pushvec<4,Numeric::int32>(L,value)) {
		return true;
	    }

	    if(lua_pushmat<2,double>(L,value)) {
		return true;
	    }

	    if(lua_pushmat<3,double>(L,value)) {
		return true;
	    }

	    if(lua_pushmat<4,double>(L,value)) {
		return true;
	    }

	    return false;
	}

    }
}
