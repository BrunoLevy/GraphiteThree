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
#include "luawrap_runtime.h"
)";

/****************************************************************************/

namespace {
    using namespace OGF;

    class LuaWrapGenerator {
    public:
	LuaWrapGenerator(std::ostream& out) : out_(out) {
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
	    ogf_declare_pointer_type<const char*>("const char*");
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
	    return (
		mtype->name() == "std::string" ||
		mtype->name() == "char*" ||
		mtype->name() == "const char*"
	    );
	}

	/**
	 * \brief Tests whether a type is supported by the Lua wrapping mechanism
	 * \retval true if the type is supported
	 * \retval false otherwise, then no wrapper can be generated
	 *  for a function if the function uses this type as an argument
	 *  or as its return type
	 */
	bool check_type(const std::string& type_name) const {
	    if(
		type_name == "ImVec2" || type_name == "ImVec4" ||
		type_name == "ImTextureRef"
	    ) {
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

	std::string type_to_lua_type(MetaType* mtype) {
	    std::string lua_type = "";
	    if(type_is_integer_like(mtype)) {
		lua_type = "lua_Integer";
	    } else if(type_is_number_like(mtype)) {
		lua_type = "lua_Number";
	    } else if(type_is_string_like(mtype)) {
		lua_type = "const char*";
	    } else if(mtype->name() == "bool") {
		lua_type = "bool";
	    } else {
		lua_type = mtype->name();
		if(
		    mtype->name() != "ImVec2" &&
		    mtype->name() != "ImVec4" &&
		    mtype->name() != "ImTextureRef"
		) {
		    Logger::warn("GomGen") << "Unknown type: "
					   << mtype->name()
					   << std::endl;
		}
	    }
	    return lua_type;
	}


	void generate_wrapper(MetaMethod* mmethod) {

	    out_ << "   int " << mmethod->name()
		 << "([[maybe_unused]] lua_State* L) {"
		 << std::endl;

	    // prototype has a string (used to display error messages)
	    if(mmethod->nb_args() != 0) {
		out_ << "      static const char* proto = \""
		     << get_prototype(mmethod)
		     << "\";" << std::endl;
	    }

	    // get argument information
	    // - arg_is_pointer: an argument passed by address and returned
	    // - arg_type: C++ type of the argument
	    // - arg_default_value: default value as a string or "" if not any

	    bool has_pointers = false;
	    vector<bool> arg_is_pointer;
	    vector<MetaType*> arg_type;
	    vector<std::string> arg_default_value;

	    for(index_t i=0; i<mmethod->nb_args(); ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		std::string type_name = marg->type_name();
		bool is_pointer = false;
		if(type_name == "char*") {
		    type_name = "const char*"; // quick and dirty,
  		                               // TODO: fix lang instead
		} else {
		    is_pointer = OGF::String::string_ends_with(type_name,"*");
		    type_name = OGF::String::remove_suffix(type_name, "*");
		}
		std::string default_value;
		if(is_pointer) {
		    if(marg->has_default_value()) {
			default_value = "NullPointer()";
		    } else {
			default_value = "UninitializedPointer()";
		    }
		} else if(marg->has_default_value()) {
		    default_value = marg->default_value().as_string();
		    if(
			type_is_string_like(marg->type()) &&
			default_value != "NULL" // TODO: smthg cleaner
		    ) {
			default_value = OGF::String::quote(default_value);
		    }
		}
		MetaType* mtype = Meta::instance()->resolve_meta_type(type_name);
		geo_assert(mtype != nullptr);

		arg_is_pointer.push_back(is_pointer);
		has_pointers = has_pointers || is_pointer;
		arg_type.push_back(mtype);
		arg_default_value.push_back(default_value);
	    }

	    // Generate code to declare, initialize and fetch arguments

	    int stackptr = 1;
	    for(index_t i=0; i<mmethod->nb_args(); ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		MetaType* mtype = arg_type[i];
		std::string type_name = mtype->name();
		std::string default_value = arg_default_value[i];
		std::string lua_type = type_to_lua_type(mtype);
		out_ << "      " << "Arg<" << type_name;
		if(lua_type != type_name) {
		    out_ << "," << lua_type;
		}
		out_ << ">" << " " << marg->name() << "("
		     <<  "L," << stackptr;
		if(default_value != "") {
		    out_ << "," << default_value;
		}
		out_ << ");" << std::endl;
		if(type_name == "ImVec2") {
		    stackptr += 2;
		} else if(type_name == "ImVec4") {
		    stackptr += 4;
		} else {
		    stackptr++;
		}
	    }

	    // Check arguments
	    if(mmethod->nb_args() > 0) {
		out_ << "      LUAWRAP_CHECK_ARGS(";
		for(index_t i=0; i<mmethod->nb_args(); ++i) {
		    out_ << mmethod->ith_arg(i)->name();
		    if(i != mmethod->nb_args()-1) {
			out_ << ", ";
		    }
		}
		out_ << ");" << std::endl;
	    }

	    // Call function
	    out_ << "      ";
	    if(mmethod->return_type_name() != "void") {
		std::string ret_type = mmethod->return_type_name();
		if(ret_type == "char*") {
		    ret_type = "const char*";
		}
		MetaType* mret_type =
		    Meta::instance()->resolve_meta_type(ret_type);
		geo_assert(mret_type != nullptr);
		std::string lua_rettype = type_to_lua_type(mret_type);
		if(ret_type == lua_rettype) {
		    out_ << "Arg<" << ret_type << "> retval = ";
		} else {
		    out_ << "Arg<" << ret_type << "," << lua_rettype << ">"
			 << " retval = ";
		}
	    }
	    out_ << "ImGui::" << mmethod->name() << "(";
	    for(index_t i=0; i<mmethod->nb_args(); ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		if(i != 0) {
		    out_ << ", ";
		}
		if(arg_is_pointer[i]) {
		    out_ << marg->name() << ".pointer()";
		} else {
		    out_ << marg->name() << ".value";
		}
	    }
	    out_ << ");" << std::endl;

	    // Push result and pointer values
	    if(has_pointers || mmethod->return_type_name() != "void") {
		out_ << "      int prevtop = lua_gettop(L);" << std::endl;
		if(mmethod->return_type_name() != "void") {
		    out_ << "      retval.push(L);" << std::endl;
		}
		for(index_t i=0; i<mmethod->nb_args(); ++i) {
		    if(arg_is_pointer[i]) {
			out_ << "      " << mmethod->ith_arg(i)->name()
			     << ".push_if_set(L);" << std::endl;
		    }
		}
		out_ << "      return lua_gettop(L)-prevtop;" << std::endl;
	    } else {
		out_ << "      return 0;" << std::endl;
	    }

	    out_ << "   }" << std::endl << std::endl;
	}

	/**
	 * \brief Generates wrappers for all functions in a namespace
	 * \param[in] name_space the name of the namespace, as a string
	 */
	void generate_wrappers(const std::string& name_space) {
	    name_space_ = name_space;
	    prefix_ = name_space + "_lua_wrappers";
	    out_ << "namespace " << prefix_ << " {"
		 << std::endl;
	    out_ << "   using namespace LuaWrap;" << std::endl << std::endl;
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

	void generate_register_func() {
	    out_ << "void " << prefix_ << "_register(lua_State* L) {"
		 << std::endl;


	    // Retreive or create imgui table if it does not already exist,
	    // Keep the table on the top of the stack
	    out_ << "   lua_getglobal(L, \"imgui\");" << std::endl;
	    out_ << "   if(lua_isnil(L,-1)) {" << std::endl;
	    out_ << "      lua_pop(L,1); " << std::endl;
	    out_ << "      lua_newtable(L); " << std::endl;
	    out_ << "      lua_pushvalue(L, -1);" << std::endl;
	    out_ << "      lua_setglobal(L, \"imgui\");" << std::endl;
            out_ << "   } " << std::endl << std::endl;

	    out_ << "   using namespace " << prefix_ << ";" << std::endl;

	    // Register functions
	    MetaClass* mclass =
		Meta::instance()->resolve_meta_class(name_space_);
	    index_t N = index_t(mclass->nb_members());
	    for(index_t i=0; i<N; ++i) {
		MetaMember* mmember = mclass->ith_member(i);
		MetaMethod* mmethod = dynamic_cast<MetaMethod*>(mmember);
		if(mmethod != nullptr && check_types(mmethod)) {
		    out_ << "   LUAWRAP_DECLARE_FUNCTION(L," << mmethod->name()
			 << ");" << std::endl;
		}
	    }

	    // Pop the imgui table from the top of the stack
	    out_ << "   lua_pop(L,1);" << std::endl << std::endl;


	    // Register enums
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
	 * \brief namespace of the generated functions
	 */
	std::string name_space_;

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
	generator.generate_register_func();
    }
}
