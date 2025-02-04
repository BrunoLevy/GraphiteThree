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



extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

namespace OGF {
    namespace GOMLua {

	/**
	 * \brief Converts a lua object into a value
	 * \tparam T type of the value
	 * \param[in] L a pointer to the Lua state
	 * \param[in] index the index of the object in thestack
	 * \param[out] result a reference to the read value
	 * \retval true if the conversion was successful
	 * \retval false otherwise (LUA type did not match)
	 */
	template <class T> inline bool lua_tographiteveccomp(
	    lua_State* L, int index, T& result
	) {
	    geo_argused(L);
	    geo_argused(index);
	    geo_argused(result);
	    geo_assert_not_reached;
	}

	template<> inline bool lua_tographiteveccomp<double>(
	    lua_State* L, int index, double& result
	) {
	    if(lua_type(L,index) == LUA_TNUMBER) {
		result = lua_tonumber(L,index);
		return true;
	    }
	    return false;
	}

	template<> inline bool lua_tographiteveccomp<Numeric::int32>(
	    lua_State* L, int index, Numeric::int32& result
	) {
	    if(lua_type(L,index) == LUA_TNUMBER && lua_isinteger(L,index)) {
		result = GEO::Numeric::int32(lua_tointeger(L,index));
		return true;
	    }
	    return false;
	}


	/**
	 * \brief Converts a lua object into a Graphite vec2,vec3 or vec4
	 * \tparam N dimension of the vector
	 * \tparam T type of the components
	 * \param[in] L a pointer to the Lua state
	 * \param[in] index the index of the object in thestack
	 * \param[out] result a reference to the converted vector
	 * \retval true if the conversion was successful
	 * \retval false otherwise (mtype does not match, or object on
	 *  the stack is not an integer-indexed table of numbers of the
	 *  correct size).
	 */
	template<unsigned int N, class T> inline bool lua_tographitevec(
	    lua_State* L, int index, ::GEO::vecng<N,T>& result
	) {
	    if(!lua_istable(L,index)) {
		return false;
	    }

	    index_t cur = 0;
	    bool ok = true;

	    for(lua_Integer i=1; lua_geti(L,index,i) != LUA_TNIL; ++i) {
		if(cur < N) {
		    ok = ok && lua_tographiteveccomp(L,-1,result[cur]);
		}
		++cur;
		lua_pop(L,1);
	    }
	    lua_pop(L,1); // lua_geti() pushes smthg on the stack
	    // even for the last round of the loop !

	    return(ok && cur == index_t(N));
	}

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
	    if(!lua_tographitevec(L, index, V)) {
		return false;
	    }
	    result.set_value(V);
	    return true;
	}

	/***************************************************/

	/**
	 * \brief Converts a python object into a Graphite mat2, mat3 or mat4
	 * \tparam N dimension of the vector
	 * \tparam T type of the coefficients
	 * \param[in] L a pointer to the Lua state
	 * \param[in] index the index of the object in thestack
	 * \param[out] result a reference to the converted matrix
	 * \retval true if the conversion was successful
	 * \retval false otherwise (mtype does not match, or object
	 *  is not a table of the correct size).
	 */
	template<unsigned int N, class T> inline bool lua_tographitemat(
	    lua_State* L, int index, ::GEO::Matrix<N,T>& result
	) {

	    if(!lua_istable(L,index)) {
		return false;
	    }

	    index_t cur = 0;
	    bool ok = true;

	    for(lua_Integer i=1; lua_geti(L,index,i) != LUA_TNIL; ++i) {
		::GEO::vecng<N,T> row;
		if(cur < N) {
		    if(lua_tographitevec(L,-1,row)) {
			for(index_t j=0; j<index_t(N); ++j) {
			    result(cur,j) = row[j];
			}
		    } else {
			ok = false;
		    }
		}
		++cur;
		lua_pop(L,1);
	    }
	    lua_pop(L,1); // lua_geti() pushes smthg on the stack
	    // even for the last round of the loop !

	    return(ok && cur == index_t(N));
	}


	/***************************************************/

	/**
	 * \brief Converts a python object into a Graphite mat2, mat3 or mat4
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
	    if(!lua_tographitemat(L,index,M)) {
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
	    return lua_tographitevec(L, index, result);
	}

	/********************************************************************/

	template <class T> inline void lua_pushveccomp(lua_State* L, T val) {
	    geo_argused(L);
	    geo_argused(val);
	    geo_assert_not_reached;
	}

	template<> inline void lua_pushveccomp(lua_State* L, double val) {
	    lua_pushnumber(L, val);
	}

	template<> inline void lua_pushveccomp(
	    lua_State* L, Numeric::int32 val
	) {
	    lua_pushinteger(L, lua_Integer(val));
	}

	template <unsigned int N, class T> inline void lua_pushvec(
	    lua_State* L, const ::GEO::vecng<N,T>& V
	) {
	    lua_createtable(L, int(N), 0);
	    for(index_t i=0; i<index_t(N); ++i) {
		lua_pushveccomp(L,V[i]);
		lua_seti(L,-2,lua_Integer(i+1)); // indices start from 1 in Lua
	    }
	}

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

	template <unsigned int N, class T> inline void lua_pushmat(
	    lua_State* L, const ::GEO::Matrix<N,T>& M
	) {
	    lua_createtable(L, int(N), 0);
	    for(index_t i=0; i<index_t(N); ++i) {
		lua_createtable(L, int(N), 0);
		for(index_t j=0; j<index_t(N); ++j) {
		    lua_pushveccomp(L,M(i,j));
		    lua_seti(L,-2,lua_Integer(j+1)); // Idces start from 1 in Lua
		}
		lua_seti(L,-2,lua_Integer(i+1)); // Idem...
	    }
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
