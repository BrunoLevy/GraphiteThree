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

namespace {
    using namespace OGF;


    class LuaWrapGenerator {
    public:
	LuaWrapGenerator() {
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
	 * \param[in] type_name a string with the type name, for instance
	 *  "unsigned int", "signed char", "long long int".
	 * \retval true if mtype corresponds to a (signed or not) integer of
	 *  any bitlength
	 * \retval false otherwise
	 */
	bool type_is_integer_like(const std::string& type_name) const {
	    MetaType* mtype = Meta::instance()->resolve_meta_type(type_name);
	    return type_is_integer_like(mtype);
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
	 * \brief decorates a type name with additional information
	 * \param[in] type_name a type name
	 * \return type_name plus some additional information:
	 *   - "(I)" for integer-like or enum types
	 */
	std::string show_type_name(const std::string& type_name) const {
	    if(type_is_integer_like(type_name)) {
		return type_name + "(I)";
	    }
	    return type_name;
	}


	/**
	 * \brief Generates wrappers for all functions in a namespace
	 * \param[in] name_space the name of the namespace, as a string
	 */
	void generate_wrappers(const std::string& name_space) {
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
		    std::string proto;
		    if(true) {
			proto += (
			    show_type_name(mmethod->return_type_name()) + " "
			);
			proto += (mmethod->name() + "(");
			for(index_t i=0; i<mmethod->nb_args(); ++i) {
			    MetaArg* marg = mmethod->ith_arg(i);
			    proto += (
				show_type_name(marg->type_name()) + " " +
				marg->name()
			    );
			    if(marg->has_default_value()) {
				proto += ("=" +
					  marg->default_value().as_string()
					 );
			    }
			    if(i != mmethod->nb_args()-1) {
				proto += ",";
			    }
			}
			proto += ")";
		    } else {
			proto = mmethod->name();
		    }
		    Logger::out("GomGen") << (OK ? "OK " : "KO ") << proto
					  << std::endl;
		    if(!OK) {
			check_types(mmethod,true);
		    }
		}
	    }
	}

	void generate_consts() {
	    for(MetaType* mtype: used_types_) {
		MetaEnum* menum = dynamic_cast<MetaEnum*>(mtype);
		if(menum != nullptr) {
		    Logger::out("GomGen") << " Enum " << menum->name()
					  << std::endl;
		    for(index_t i=0; i<menum->nb_values(); ++i) {
			Logger::out("GomGen") << "   "
					      << menum->ith_name(i)
					      << std::endl;
		    }
		}
	    }
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

    LuaWrapGenerator generator;

    if (top) {
	if (Swig_contract_mode_get()) {
	    Swig_contracts(top);
	}
	lang->top(top);

	generator.generate_wrappers("ImGui");
	generator.generate_consts();
    }
}
