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
#include <OGF/gom/reflection/meta_namespace.h>
#include <OGF/gom/types/gom_implementation.h>

#include "gom_lang.h"

// Swig includes need to be AFTER OGF includes
// else it causes a problem under Windows.
#include <swig/Modules/swigmod.h>
#include <swig/CParse/cparse.h>

/****************************************************************************/

namespace {
    using namespace OGF;

    class LuaWrapGenerator {
    public:
	LuaWrapGenerator(std::ostream& out) : out_(out) {

	    // List of integer-like types
	    integer_types_.push_back(ogf_meta<int>::type());
	    integer_types_.push_back(ogf_meta<long>::type());
	    integer_types_.push_back(ogf_meta<unsigned short>::type());
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

	    ogf_declare_pointer_type<const char*>("const char*");
	    ogf_declare_pointer_type<const bool*>("const bool*");
	    ogf_declare_pointer_type<const int*>("const int*");
	    ogf_declare_pointer_type<const float*>("const float*");
	    ogf_declare_pointer_type<const double*>("const double*");

	    unsupported_types_.insert("char*");
	    unsupported_types_.insert("void*");
	    unsupported_types_.insert("ImVec2*");
	    unsupported_types_.insert("ImVec4*");
	    unsupported_types_.insert("const ImVec2*");
	    unsupported_types_.insert("const ImVec4*");

	    supported_types_.insert("const char*");
	    supported_types_.insert("ImVec2");
	    supported_types_.insert("ImVec4");
	    supported_types_.insert("ImTextureRef");

	    nb_components_["ImVec2"] = 2;
	    nb_components_["ImVec4"] = 4;
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
	    if(unsupported_types_.find(type_name) != unsupported_types_.end()) {
		return false;
	    }

	    if(supported_types_.find(type_name) != supported_types_.end()) {
		return true;
	    }

 	    if(GEO::String::string_starts_with(type_name, "const ")) {
		return check_type(
		    GEO::String::remove_prefix(type_name, "const ")
		);
	    }

	    if(GEO::String::string_ends_with(type_name, "*")) {
		return (
		    OGF::Meta::instance()->resolve_meta_type(type_name) !=
		    nullptr
		);
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
	 * \brief Tests whether a function is printf-like
	 * \details A function is printf-like if its two last arguments
	 *  are a format string (const char*) and the ellipsis
	 */
	bool is_printf_like(MetaMethod* mmethod) {
	    index_t N = index_t(mmethod->nb_args());
	    if(N < 2) {
		return false;
	    }
	    return (
		(mmethod->ith_arg(N-2)->type_name() == "const char*") &&
		(mmethod->ith_arg(N-1)->type_name() == "...")
	    );
	}

	/**
	 * \brief Gets the number of valid arguments
	 * \details Ignores the arguments at the end of the list that
	 *  have invalid types but that have default values
	 */
	index_t nb_valid_args(MetaMethod* mmethod) {
	    if(is_printf_like(mmethod)) {
		return index_t(mmethod->nb_args()-1);
	    }
	    index_t N = index_t(mmethod->nb_args());
	    while(
		N > 0 &&
		!check_type(mmethod->ith_arg(N-1)->type_name()) &&
		mmethod->ith_arg(N-1)->has_default_value()
	    ) {
		--N;
	    }
	    return N;
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
	    index_t N = nb_valid_args(mmethod);
	    for(index_t i=0; i<N; ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		bool ok =
		    ((i == mmethod->nb_args()-1) && is_printf_like(mmethod)) ||
		    check_type(marg->type_name());
		if(report) {
		    Logger::out("GomGen")
			<< "   arg: " << marg->name()
			<< ":" << marg->type_name()
			<< " " << (ok ? "ok" : "ko")
			<< std::endl;
		}
		OK = OK && ok;
	    }
	    if(mmethod->return_type_name() != "void") {
		if(report) {
		    Logger::out("GomGen")
			<< "   ret type: " << mmethod->return_type_name()
			<< " " << (
			    check_type(mmethod->return_type_name()) ? "ok" : "ko"
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
	    if(mmethod->name()[0] == '_') {
		Logger::out("GomGen")
		    << mmethod->name() << ":"
		    << "skipping function starting with _"
		    << std::endl;
		OK = false;
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
	    index_t N = nb_valid_args(mmethod);
	    for(index_t i=0; i<N; ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		proto += (marg->type_name() + " " + marg->name());
		if(marg->has_default_value()) {
		    proto += ("=" + marg->default_value().as_string());
		}
		if(i != N-1) {
		    proto += ", ";
		}
	    }
	    proto += ")";
	    return proto;
	}

	/**
	 * \brief Gets the Lua type associated with a C++ type
	 * \param[in] mtype the C++ type, as a MetaType
	 * \return the lua type name as a string
	 */
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
	    }
	    return lua_type;
	}

	/**
	 * \brief Gets the number of components associated with a type
	 * \param[in] mtype the C++ type, as a MetaType
	 * \return the number of values that will be consumed on the Lua
	 *  stack to initialize a variable of that type. It is almost always
	 *  1, except for ImVec2 (2) and ImVec4(4).
	 */
	int type_nb_components(MetaType* mtype) {
	    auto it = nb_components_.find(mtype->name());
	    if(it == nb_components_.end()) {
		return 1;
	    }
	    return it->second;
	}

	/**
	 * \brief Generates a wrapper for a function
	 * \param[in] mmethod the function, as a MetaMethod
	 * \details if the container is a class, then an additional
	 *  parameter is generated for the "this" pointer.
	 */
	void generate_wrapper(MetaMethod* mmethod) {
	    MetaClass* mclass = mmethod->container_meta_class();
	    bool is_in_class = (
		dynamic_cast<OGF::MetaNamespace*>(mclass) == nullptr
	    );

	    // get argument information
	    // - nb_args: an additional arg is generated if container is a class
	    // - arg_name: name of the argument, as a string
	    // - arg_is_pointer: if argument passed by address and returned
	    // - arg_type: C++ type of the argument
	    // - arg_default_value: default value as a string or "" if not any

	    bool has_pointers = false;
	    vector<std::string> arg_name;
	    vector<bool> arg_is_pointer;
	    vector<MetaType*> arg_type;
	    vector<std::string> arg_default_value;
	    index_t mmethod_nb_args = nb_valid_args(mmethod);
	    index_t nb_args = mmethod_nb_args;

	    if(is_in_class) {
		++nb_args;
		arg_name.push_back("self");
		arg_is_pointer.push_back(false);
		arg_type.push_back(
		    Meta::instance()->resolve_meta_type(mclass->name()+"*")
		);
		arg_default_value.push_back("");
	    }
	    bool printf_like = is_printf_like(mmethod);
	    for(index_t i=0; i<mmethod_nb_args; ++i) {
		MetaArg* marg = mmethod->ith_arg(i);
		std::string type_name = marg->type_name();
		bool is_pointer = false;

		if(!OGF::String::string_starts_with(type_name, "const ")) {
		    is_pointer = OGF::String::string_ends_with(type_name,"*");
		    if(is_pointer) {
			// If type is a pointer to a struct or class, then
			// keep type as is (no special pointer management
			// mechanism).
			type_name = OGF::String::remove_suffix(type_name, "*");
			MetaClass* mclass =
			    Meta::instance()->resolve_meta_class(type_name);
			if(mclass != nullptr) {
			    is_pointer = false;
			    type_name = type_name + "*";
			}
		    }
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
		if(mtype == nullptr) {
		    Logger::err("GomGen") << "FATAL: did not find type:"
					  << type_name
					  << std::endl;
		}
		geo_assert(mtype != nullptr);

		arg_name.push_back(marg->name());
		arg_is_pointer.push_back(is_pointer);
		has_pointers = has_pointers || is_pointer;
		arg_type.push_back(mtype);
		arg_default_value.push_back(default_value);
	    }


	    out_ << "   static int " << mmethod->name();
	    if(nb_args == 0 && mmethod->return_type_name() == "void") {
		out_ << "(lua_State*) {"
		     << std::endl;
	    } else {
		out_ << "(lua_State* L) {"
		     << std::endl;
	    }

	    // prototype as a string (used to display error messages)
	    if(nb_args != 0) {
		out_ << "      static const char* proto = \""
		     << get_prototype(mmethod)
		     << "\";" << std::endl;
	    }

	    // Generate code to declare, initialize and fetch arguments
	    int stackptr = 1;
	    for(index_t i=0; i<nb_args; ++i) {
		MetaType* mtype = arg_type[i];
		std::string type_name = mtype->name();
		std::string default_value = arg_default_value[i];
		std::string lua_type = type_to_lua_type(mtype);
		out_ << "      " << "Arg<" << type_name;
		if(lua_type != type_name) {
		    out_ << "," << lua_type;
		}
		out_ << ">" << " " << arg_name[i] << "("
		     <<  "L," << stackptr;
		if(default_value != "") {
		    out_ << "," << default_value;
		}
		out_ << ");" << std::endl;
		stackptr += type_nb_components(mtype);
	    }

	    // Check arguments
	    if(nb_args > 0) {
		out_ << "      LUAWRAP_CHECK_ARGS(";
		for(index_t i=0; i<nb_args; ++i) {
		    out_ << arg_name[i];
		    if(i != nb_args-1) {
			out_ << ", ";
		    }
		}
		out_ << ");" << std::endl;
	    }

	    // Call function
	    out_ << "      ";
	    if(mmethod->return_type_name() != "void") {
		std::string ret_type = mmethod->return_type_name();
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
	    index_t first_arg = 0;
	    if(!is_in_class) {
		out_ << mclass->name() << "::" << mmethod->name() << "(";
	    } else {
		out_ << "self.value->" << mmethod->name() << "(";
		++first_arg;
	    }
	    for(index_t i=first_arg; i<nb_args; ++i) {
		if(i != first_arg) {
		    out_ << ", ";
		}
		if(i == nb_args - 1 && printf_like) {
		    out_ << "\"%s\"," << arg_name[i] << ".value";
		} else if(arg_is_pointer[i]) {
		    out_ << arg_name[i] << ".pointer()";
		} else {
		    out_ << arg_name[i] << ".value";
		}
	    }
	    out_ << ");" << std::endl;

	    // Push result and pointer values
	    if(has_pointers || mmethod->return_type_name() != "void") {
		out_ << "      int prevtop = lua_gettop(L);" << std::endl;
		if(mmethod->return_type_name() != "void") {
		    out_ << "      retval.push(L);" << std::endl;
		}
		for(index_t i=first_arg; i<nb_args; ++i) {
		    if(arg_is_pointer[i]) {
			out_ << "      " << arg_name[i]
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

	/**
	 * \brief Generates the function that registers the wrappers and
	 *  constants to the Lua interpreter
	 */
	void generate_register_func() {
	    out_ << "void " << "Load" << name_space_
		 << "Bindings(lua_State* L) {"
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

	/**
	 * \brief Explicitly unsupported types.
	 */
	std::set<std::string> unsupported_types_;

	/**
	 * \brief Explicitly supported types.
	 */
	std::set<std::string> supported_types_;

	/**
	 * \brief Maps type names to number of components
	 * \details 2 for ImVec2, 4 for ImVec4, 1 if not in the table
	 */
	std::map<std::string, int> nb_components_;
    };
}

/****************************************************************************/

void generate_luawrap(
    Language* lang, const std::vector<std::string>& sources,
    DOH* cpps, std::ofstream& out, int argc, char** argv,
    const std::vector<std::string>& include_path,
    const std::string& input_path,
    const std::string& output_path,
    const std::string& package_name,
    const std::string& input_scope
) {
    using namespace OGF;

    geo_argused(sources);
    geo_argused(package_name);

    ::Node *top = Swig_cparse(cpps);
    Swig_process_types(top);
    Swig_default_allocators(top);

    out << "#include \"luawrap_runtime.h\"" << std::endl << std::endl;

    out << "// GOMGEN automatically generated code" << std::endl;
    out << "// Do not edit." << std::endl;
    out << std::endl;
    out << "// Command line:" << std::endl;
    for(int i=0; i<argc; ++i) {
	out << "//  " << argv[i] << std::endl;
    }
    out << std::endl;
    out << std::endl;
    out << "// Include path:" << std::endl;
    for(GEO::index_t i=0; i<include_path.size(); ++i) {
	out << "//   " << include_path[i] << std::endl;
    }
    out << "// Input path:" << std::endl;
    out << "//   " << input_path << std::endl;
    out << "// Output file:" << std::endl;
    out << "//   " << output_path << std::endl;
    out << std::endl;
    out << std::endl;

    LuaWrapGenerator generator(out);

    if (top) {
	if (Swig_contract_mode_get()) {
	    Swig_contracts(top);
	}
	lang->top(top);
	generator.generate_wrappers(input_scope);
	out << std::endl << std::endl;
	generator.generate_register_func();
    }
}
