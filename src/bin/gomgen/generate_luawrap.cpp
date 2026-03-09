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

    std::vector<MetaType*> integer_types;

    bool type_is_integer_like(MetaType* mtype) {
	if(dynamic_cast<MetaEnum*>(mtype) != nullptr) {
	    return true;
	}

	if(
	    std::find(integer_types.begin(), integer_types.end(), mtype) !=
	    integer_types.end()
	) {
	    return true;
	}

	return false;
    }

    bool type_is_integer_like(const std::string& type_name) {
	MetaType* mtype = Meta::instance()->resolve_meta_type(type_name);
	return type_is_integer_like(mtype);
    }

    bool check_type(const std::string& type_name) {
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
	return OK;
    }

    std::string show_type_name(const std::string& type_name) {
	if(type_is_integer_like(type_name)) {
	    return type_name + "(I)";
	}
	return type_name;
    }

    void show_imgui() {
	MetaClass* mclass = Meta::instance()->resolve_meta_class("ImGui");
	if(mclass == nullptr) {
	    std::cerr << "  DID NOT FIND ImGui" << std::endl;
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
		    proto += (show_type_name(mmethod->return_type_name()) + " ");
		    proto += (mmethod->name() + "(");
		    for(index_t i=0; i<mmethod->nb_args(); ++i) {
			MetaArg* marg = mmethod->ith_arg(i);
			proto += (
			    show_type_name(marg->type_name()) + " " +
			    marg->name()
			);
			if(marg->has_default_value()) {
			    proto += ("=" + marg->default_value().as_string());
			}
			if(i != mmethod->nb_args()-1) {
			    proto += ",";
			}
		    }
		    proto += ")";
		} else {
		    proto = mmethod->name();
		}
		Logger::out("GomGen")
		    <<  (OK ? "OK " : "KO ")
		    << proto
		    << std::endl;
		if(!OK) {
		    check_types(mmethod,true);
		}
	    }
	}
    }
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

    // List of integer-like types

    integer_types.push_back(ogf_meta<int>::type());
    integer_types.push_back(ogf_meta<long>::type());
    integer_types.push_back(ogf_meta<unsigned int>::type());
    integer_types.push_back(ogf_meta<unsigned long>::type());
    integer_types.push_back(ogf_meta<GEO::index_t>::type());
    integer_types.push_back(ogf_meta<OGF::index_t>::type());
    integer_types.push_back(ogf_meta<size_t>::type());

    // Pre-declare all integer types, char* and bool* used in ImGui

    integer_types.push_back(
	ogf_declare_builtin_type<signed char>("signed char")
    );
    integer_types.push_back(
	ogf_declare_builtin_type<unsigned char>("unsigned char")
    );
    integer_types.push_back(
	ogf_declare_builtin_type<short>("short")
    );
    integer_types.push_back(
	ogf_declare_builtin_type<unsigned short>("unsigned short")
    );
    integer_types.push_back(
	ogf_declare_builtin_type<long long>("long long")
    );
    integer_types.push_back(
	ogf_declare_builtin_type<unsigned long long>("unsigned long long")
    );

    ogf_declare_pointer_type<char*>("char*");
    ogf_declare_pointer_type<bool*>("bool*");
    ogf_declare_pointer_type<int*>("int*");
    ogf_declare_pointer_type<float*>("float*");
    ogf_declare_pointer_type<double*>("double*");

    if (top) {
	if (Swig_contract_mode_get()) {
	    Swig_contracts(top);
	}
	lang->top(top);

	show_imgui();
    }
}
