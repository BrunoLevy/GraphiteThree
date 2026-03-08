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

    bool check_type(const std::string& type_name) {
	OGF::MetaType* mtype = OGF::Meta::instance()->resolve_meta_type(
	    type_name
	);
	if(mtype == nullptr) {
	    return false;
	}
	if(
	    type_name != "ImVec2" && type_name != "ImVec4" &&
	    dynamic_cast<OGF::MetaClass*>(mtype) != nullptr
	) {
	    return false;
	}
	return true;
    }

    bool check_types(OGF::MetaMethod* mmethod, bool report=false) {
	bool OK = true;
	for(GEO::index_t i=0; i<mmethod->nb_args(); ++i) {
	    OGF::MetaArg* marg = mmethod->ith_arg(i);
	    if(report) {
		GEO::Logger::out("GomGen")
		    << "   arg: " << marg->name()
		    << ":" << marg->type_name()
		    << " " << (check_type(marg->type_name()) ? "OK" : "KO")
		    << std::endl;
	    }
	    OK = OK && check_type(marg->type_name());
	}
	if(mmethod->return_type_name() != "void") {
	    if(report) {
		GEO::Logger::out("GomGen")
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

    void show_imgui() {
	OGF::MetaClass* mclass =
	    OGF::Meta::instance()->resolve_meta_class("ImGui");
	if(mclass == nullptr) {
	    std::cerr << "  DID NOT FIND ImGui" << std::endl;
	    return;
	}
	GEO::index_t N = GEO::index_t(mclass->nb_members());
	for(GEO::index_t i=0; i<N; ++i) {
	    OGF::MetaMember* mmember = mclass->ith_member(i);
	    OGF::MetaMethod* mmethod = dynamic_cast<OGF::MetaMethod*>(mmember);
	    if(mmethod != nullptr) {
		bool OK = check_types(mmethod);
		std::string proto;
		if(true) {
		    proto += (mmethod->return_type_name() + " ");
		    proto += (mmethod->name() + "(");
		    for(GEO::index_t i=0; i<mmethod->nb_args(); ++i) {
			OGF::MetaArg* marg = mmethod->ith_arg(i);
			proto += (marg->type_name() + " " + marg->name());
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
		GEO::Logger::out("GomGen")
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
    Node *top = Swig_cparse(cpps);
    Swig_process_types(top);
    Swig_default_allocators(top);

    // Pre-declare all integer types, char* and bool* used in ImGui
    OGF::ogf_declare_builtin_type<signed char>("signed char");
    OGF::ogf_declare_builtin_type<unsigned char>("unsigned char");
    OGF::ogf_declare_builtin_type<short>("short");
    OGF::ogf_declare_builtin_type<unsigned short>("unsigned short");
    OGF::ogf_declare_builtin_type<long long>("long long");
    OGF::ogf_declare_builtin_type<unsigned long long>("unsigned long long");
    OGF::ogf_declare_pointer_type<char*>("char*");
    OGF::ogf_declare_pointer_type<bool*>("bool*");
    OGF::ogf_declare_pointer_type<int*>("int*");
    OGF::ogf_declare_pointer_type<float*>("float*");
    OGF::ogf_declare_pointer_type<double*>("double*");

    if (top) {
	if (Swig_contract_mode_get()) {
	    Swig_contracts(top);
	}
	lang->top(top);

	show_imgui();
    }
}
