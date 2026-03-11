/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 Bruno Levy
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

#include "generate_luawrap.h"
#include <OGF/gom/codegen/codegen.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/types/gom_implementation.h>

#include "gom_lang.h"

// Swig includes need to be AFTER OGF includes
// else it causes a problem under Windows.
#include <swig/Modules/swigmod.h>
#include <swig/CParse/cparse.h>

/****************************************************************************/

static const char* luawrap_header = R"(// Generated with gomgen

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GEOGRAM_USE_BUILTIN_DEPS
#include <geogram/third_party/lua/lua.h>
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
#else
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#endif

#ifdef __cplusplus
}
#endif

#define LUAWRAP_DECLARE_GLOBAL_CONSTANT(L,C) \
    lua_pushinteger(L,C);                    \
    lua_setglobal(L,#C)

struct LuawrapArg {
   LuawrapArg() : initialized(false), type_OK(true) {  }
   bool initialized;
   bool type_OK;
};

template <class T> struct LuawrapArg_integer : public LuawrapArg {
   LuawrapArg_integer(lua_State* L, int idx) { get(L,idx); }
   LuawrapArg_integer(const T& value_in) {
       value = value_in; initialized=true; get(L,idx);
   }
   void get(lua_State* L, int idx) {
      if(!lua_isnoneornil(L,idx)) {
         if(lua_isinteger(L,idx)) {
            value = T(lua_tointeger(idx));
            initialized = true;
         } else {
            type_OK = false;
         }
      }
   }
   T value;
};

template <class T> struct LuawrapArg_number : public LuawrapArg {
   LuawrapArg_number(lua_State* L, int idx) { get(L,idx); }
   LuawrapArg_number(const T& value_in) {
       value = value_in; initialized=true; get(L,idx);
   }
   void get(lua_State* L, int idx) {
      if(!lua_isnoneornil(L,idx)) {
         if(lua_isnumber(L,idx)) {
            value = T(lua_tonumber(idx));
            initialized = true;
         } else {
            type_OK = false;
         }
      }
   }
   T value;
};

template <class T> struct LuawrapArg_boolean : public LuawrapArg {
   LuawrapArg_boolean(lua_State* L, int idx) { get(L,idx); }
   LuawrapArg_boolean(const T& value_in) {
       value = value_in; initialized=true; get(L,idx);
   }
   void get(lua_State* L, int idx) {
      if(!lua_isnoneornil(L,idx)) {
         if(lua_isboolean(L,idx)) {
            value = T(lua_toboolean(idx));
            initialized = true;
         } else {
            type_OK = false;
         }
      }
   }
   T value;
};

template <class T> struct LuawrapArg_string : public LuawrapArg {
   LuawrapArg_string(lua_State* L, int idx) { get(L,idx); }
   LuawrapArg_string(const T& value_in) {
       value = value_in; initialized=true; get(L,idx);
   }
   void get(lua_State* L, int idx) {
      if(!lua_isnoneornil(L,idx)) {
         if(lua_isstring(L,idx)) {
            value = T(lua_tostring(idx));
            initialized = true;
         } else {
            type_OK = false;
         }
      }
   }
   T value;
};


)";

/****************************************************************************/

namespace {
    using namespace OGF;

    class LuaWrapGenerator {
    public:
	LuaWrapGenerator(std::ostream& out) : out_(out) {
	    prefix_ = "gomgenerated_";

	    // List of integer-like types
	    integer_types_.push_back(ogf_meta<int>::type());
	    integer_types_.push_back(ogf_meta<long>::type());
	    integer_types_.push_back(ogf_meta<unsigned int>::type());
	    integer_types_.push_back(ogf_meta<unsigned long>::type());
	    integer_types_.push_back(ogf_meta<GEO::index_t>::type());
	    integer_types_.push_back(ogf_meta<OGF::index_t>::type());
	    integer_types_.push_back(ogf_meta<size_t>::type());

	    // Pre-declare all integer types, char* and bool* used in ImGui

	    integer_types_.push_back(
		ogf_declare_builtin_type<signed char>("signed char")
	    );
	    integer_types_.push_back(
		ogf_declare_builtin_type<unsigned char>("unsigned char")
	    );
	    integer_types_.push_back(
		ogf_declare_builtin_type<short>("short")
	    );
	    integer_types_.push_back(
		ogf_declare_builtin_type<unsigned short>("unsigned short")
	    );
	    integer_types_.push_back(
		ogf_declare_builtin_type<long long>("long long")
	    );
	    integer_types_.push_back(
		ogf_declare_builtin_type<unsigned long long>(
		    "unsigned long long"
		)
	    );

	    ogf_declare_pointer_type<char*>("char*");
	    ogf_declare_pointer_type<bool*>("bool*");
	    ogf_declare_pointer_type<int*>("int*");
	    ogf_declare_pointer_type<float*>("float*");
	    ogf_declare_pointer_type<double*>("double*");

	    out_ << luawrap_header << std::endl;
	}


	/**
	 * \brief Tests whether a MetaType corresponds to an integer type
	 * \param[in] mtype a pointer to the MetaType
	 * \retval true if mtype corresponds to a (signed or not) integer of
	 *  any bitlength
	 * \retval false otherwise
	 */
	bool type_is_integer_like(MetaType* mtype) const {
	    if(dynamic_cast<MetaEnum*>(mtype) != nullptr) {
		return true;
	    }
	    return(
		std::find(integer_types_.begin(), integer_types_.end(), mtype) !=
		integer_types_.end()
	    );
	}

	/**
	 * \brief Tests whether a MetaType corresponds to an integer type
	 * \param[in] mtype a pointer to the MetaType
	 * \retval true if mtype corresponds to an integer type or a
	 *   floating-point type
	 * \retval false otherwise
	 */
	bool type_is_number_like(MetaType* mtype) const {
	    if(mtype == nullptr) {
		return false;
	    }
	    return type_is_integer_like(mtype) ||
		mtype->name() == "float" ||
		mtype->name() == "double" ;
	}

	/**
	 * \brief Tests whether a MetaType corresponds to an integer type
	 * \param[in] mtype a pointer to the MetaType
	 * \retval true if mtype corresponds to a C++ string or C string (char*)
	 * \retval false otherwise
	 */
	bool type_is_string_like(MetaType* mtype) const {
	    if(mtype == nullptr) {
		return false;
	    }
	    return (mtype->name() == "std::string" || mtype->name() == "char*");
	}

	/**
	 * \brief Tests whether a type is supported by the Lua wrapping mechanism
	 * \retval true if the type is supported
	 * \retval false otherwise, then no wrapper can be generated
	 *  for a function if the function uses this type as an argument
	 *  or as its return type
	 */
	bool check_type(const std::string& type_name) const {
	    if(type_name == "ImVec2" || type_name == "ImVec4") {
		return true;
	    }
	    if(type_name == "void*") {
		return false;
	    }
	    MetaType* mtype = Meta::instance()->resolve_meta_type(type_name);
	    if(mtype == nullptr) {
		return false;
	    }
	    if(dynamic_cast<MetaClass*>(mtype) != nullptr) {
		return false;
	    }
	    return true;
	}


	/**
	 * \brief Tests whether a wrapper can be generated for a function
	 * \param[in] mmethod the MetaMethod that corresponds to the function
	 * \param[in] report if set, report all parameters, return type, and
	 *  whether they can be handled by a wrapper
	 * \retval true if a wrapper can be generated for a method
	 * \retval false otherwise
	 */
	bool check_types(MetaMethod* mmethod, bool report=false) {
	    bool OK = true;
	    for(index_t i=0; i<mmethod->nb_args(); ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		if(report) {
		    Logger::out("GomGen")
			<< "   arg: " << marg->name()
			<< ":" << marg->type_name()
			<< " " << (check_type(marg->type_name()) ? "OK" : "KO")
			<< std::endl;
		}
		OK = OK && check_type(marg->type_name());
	    }
	    if(mmethod->return_type_name() != "void") {
		if(report) {
		    Logger::out("GomGen")
			<< "   ret type: " << mmethod->return_type_name()
			<< " " << (
			    check_type(mmethod->return_type_name()) ? "OK" : "KO"
			)
			<< std::endl;
		}
		OK = OK && check_type(mmethod->return_type_name());
	    }
	    if(OK) {
		for(index_t i=0; i<mmethod->nb_args(); ++i) {
		    used_type(mmethod->ith_arg(i)->type_name());
		}
		if(mmethod->return_type_name() != "void") {
		    used_type(mmethod->return_type_name());
		}
	    }
	    return OK;
	}

	/**
	 * \brief Gets the prototype of a MetaMethod
	 * \param[in] mmethod the MetaMethod
	 * \return a string with the return type, name, argument types, names
	 *  and optional default values.
	 */
	std::string get_prototype(MetaMethod* mmethod) {
	    std::string proto = mmethod->return_type_name() + " ";
	    proto += mmethod->container_meta_class()->name() + "::" +
		mmethod->name() + "(";
	    for(index_t i=0; i<mmethod->nb_args(); ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		proto += (marg->type_name() + " " + marg->name());
		if(marg->has_default_value()) {
		    proto += ("=" + marg->default_value().as_string());
		}
		if(i != mmethod->nb_args()-1) {
		    proto += ", ";
		}
	    }
	    proto += ")";
	    return proto;
	}

	void generate_wrapper(MetaMethod* mmethod) {
	    out_ << "   int " << mmethod->name() << "(lua_State* L) {"
		 << std::endl;

	    // prototype has a string (used to display error messages)
	    if(true || mmethod->nb_args() != 0) {
		out_ << "      static const char* proto = \""
		     << get_prototype(mmethod)
		     << "\";" << std::endl;
	    }

	    // Generate a local variable for each argument
	    // and initialize it with the default value.
	    // Each argument XXX has a twin bool XXX_initialized variable
	    // keeping track of the initialized state.
	    for(index_t i=0; i<mmethod->nb_args(); ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		std::string type_name = marg->type_name();
		bool is_pointer = String::string_ends_with(type_name,"*");
		if(type_name != "char*") {
		    type_name = String::remove_suffix(type_name, "*");
		}
		out_ << "      " << type_name << " " << marg->name();

		if(!is_pointer && marg->has_default_value()) {
		    std::string default_value =
			marg->default_value().as_string();
		    if(type_is_string_like(marg->type())) {
			default_value = String::quote(default_value);
		    }
		    out_ << " = " << default_value;
		} else if(type_name == "char*") {
		    out_ << " = nullptr";
		}
		out_ << ";" << std::endl;
		if(marg->has_default_value()) {
		    out_ << "      bool " << marg->name()
			 << "_initialized = true;" << std::endl;
		} else {
		    out_ << "      bool " << marg->name()
			 << "_initialized = false;" << std::endl;
		}
		out_ << "      bool " << marg->name()
		     << "_type_OK = true;" << std::endl;
	    }

	    // Now get the arguments from lua
	    int stackptr = 1;
	    for(index_t i=0; i<mmethod->nb_args(); ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		std::string type_name = marg->type_name();
		bool is_pointer = String::string_ends_with(type_name,"*");
		if(type_name != "char*") {
		    type_name = String::remove_suffix(type_name, "*");
		}
		MetaType* mtype = Meta::instance()->resolve_meta_type(type_name);
		geo_assert(mtype != nullptr);

		if(type_name == "ImVec2" || type_name == "ImVec4") {
		} else {
		    std::string lua_type = "";
		    if(type_is_integer_like(mtype)) {
			lua_type = "integer";
		    } else if(type_is_number_like(mtype)) {
			lua_type = "number";
		    } else if(type_is_string_like(mtype)) {
			lua_type = "string";
		    } else if(mtype->name() == "bool") {
			lua_type = "bool";
		    } else {
			Logger::warn("GomGen") << "Unknown type: "
					       << mtype->name()
					       << std::endl;
		    }
		    if(lua_type != "") {
			out_ << "      "
			     << "LUAWRAP_GET_" << lua_type << "_ARG("
			     << type_name << "," << marg->name() << ","
			     << stackptr
			     << ");" << std::endl;
		    }

		    stackptr++;
		}
	    }

	    out_ << "   }" << std::endl << std::endl;
	}

	/**
	 * \brief Generates wrappers for all functions in a namespace
	 * \param[in] name_space the name of the namespace, as a string
	 */
	void generate_wrappers(const std::string& name_space) {
	    prefix_ += name_space;
	    out_ << "namespace " << prefix_ << "_wrappers {"
		 << std::endl << std::endl;
	    MetaClass* mclass = Meta::instance()->resolve_meta_class(name_space);
	    if(mclass == nullptr) {
		Logger::err("Gom::CodeGen") << "Did not find namespace:"
					    << name_space
					    << std::endl;
		return;
	    }
	    index_t N = index_t(mclass->nb_members());
	    for(index_t i=0; i<N; ++i) {
		MetaMember* mmember = mclass->ith_member(i);
		MetaMethod* mmethod = dynamic_cast<MetaMethod*>(mmember);
		if(mmethod != nullptr) {
		    bool OK = check_types(mmethod);
		    std::string proto = get_prototype(mmethod);
		    Logger::out("GomGen") << (OK ? "OK " : "KO ") << proto
					  << std::endl;
		    if(OK) {
			generate_wrapper(mmethod);
		    } else {
			check_types(mmethod,true);
		    }
		}
	    }
	    out_ << "} // namespace " << prefix_ << "_wrappers" << std::endl;
	}

	void generate_consts() {
	    out_ << "void " << prefix_
		 << "_register_constants(lua_State* L) {"
		 << std::endl;
	    for(MetaType* mtype: used_types_) {
		MetaEnum* menum = dynamic_cast<MetaEnum*>(mtype);
		if(menum != nullptr) {
		    Logger::out("GomGen") << "Enum " << menum->name()
					  << std::endl;
		    for(index_t i=0; i<menum->nb_values(); ++i) {
			out_ << "   LUAWRAP_DECLARE_GLOBAL_CONSTANT(L,"
			     << menum->ith_name(i)
			     << ");" << std::endl;
		    }
		}
	    }
	    out_ << "}" << std::endl;
	}

	/**
	 * \brief Mark a type as used
	 * \details All enums and bitmasks with corresponding enums will
	 *  have the corresponding symbolic values generated as constants.
	 * \param[in] type_name a string with the type name
	 */
	void used_type(const std::string& type_name) {
	    MetaType* mtype = Meta::instance()->resolve_meta_type(type_name);
	    if(mtype != nullptr) {
		used_types_.insert(mtype);
	    }
	    // In Dear Imgui, some bitfields are typedefed as integer types,
	    // with the corresponding constants declared in an enum with
	    // a trailing underscore (we need to also generate the constants
	    // for these ones).
	    mtype = Meta::instance()->resolve_meta_type(type_name + "_");
	    if(mtype != nullptr) {
		used_types_.insert(mtype);
	    }
	}

    private:
	/**
	 * \brief The stream where generated code is output
	 */
	std::ostream& out_;

	/**
	 * \brief string prepended to all generated names
	 */
	std::string prefix_;

	/**
	 * \brief all integer-like types, signed and unsigned, with various
	 * bitlength
	 */
	std::vector<MetaType*> integer_types_;

	/**
	 * \brief all the types used as argument or return types by the
	 *  generated functions
	 */
	std::set<MetaType*> used_types_;
    };
}

/****************************************************************************/

void generate_luawrap(
    Language* lang, const std::vector<std::string>& sources,
    DOH* cpps, std::ofstream& out, int argc, char** argv,
    const std::vector<std::string>& include_path,
    const std::string& input_path,
    const std::string& output_path,
    const std::string& package_name
) {
    using namespace OGF;

    ::Node *top = Swig_cparse(cpps);
    Swig_process_types(top);
    Swig_default_allocators(top);

    LuaWrapGenerator generator(out);

    if (top) {
	if (Swig_contract_mode_get()) {
	    Swig_contracts(top);
	}
	lang->top(top);

	generator.generate_wrappers("ImGui");
	generator.generate_consts();
    }
}
