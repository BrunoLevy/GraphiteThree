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

#include "generate_gom.h"
#include <OGF/gom/codegen/codegen.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/types/gom_implementation.h>

#include "gom_lang.h"

// Swig includes need to be AFTER OGF includes
// else it causes a problem under Windows.
#include <swig/Modules/swigmod.h>
#include <swig/CParse/cparse.h>

void generate_gom(
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

    if (top) {
	if (Swig_contract_mode_get()) {
	    Swig_contracts(top);
	}
	lang->top(top);

	{
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

	    for(size_t i =0; i<sources.size(); ++i) {
		out << "#include <" << sources[i] << ">" << std::endl;
	    }
	    out << "#include <OGF/gom/types/gom_implementation.h>"
		<< std::endl;
	    out << std::endl;
	    out << std::endl;


	    out << "#ifdef GEO_COMPILER_GCC" << std::endl;
	    out << "#pragma GCC diagnostic ignored \"-Wunused-parameter\""
		<< std::endl;
	    out << "#pragma GCC optimize (\"O0\")"
		<< std::endl;
	    out << "#endif" << std::endl;
	    out << std::endl;

	    out << "#ifdef GEO_COMPILER_MSVC" << std::endl;
	    out << "#pragma warning(disable: 4100)" << std::endl;
	    out << "#endif" << std::endl;
	    out << std::endl;

	    out << "#ifdef GEO_COMPILER_CLANG" << std::endl;
	    out << "#pragma GCC diagnostic ignored \"-Wweak-vtables\""
		<< std::endl;
	    out << "#pragma GCC diagnostic ignored \"-Wmissing-prototypes\""
		<< std::endl;
	    out << "#pragma GCC diagnostic ignored \"-Wunused-parameter\""
		<< std::endl;
	    out << "#endif" << std::endl;

	    out << std::endl;
	    out << std::endl;

	    const std::vector<OGF::MetaClass*> classes =
		get_swig_gom_generated_classes();

	    OGF::GomCodeGenerator generator;
	    OGF::Logger::out("Gom::CodeGen") << ">>" << std::endl;
	    // generator.generate(out, classes, get_package_name(input_path));
	    generator.generate(out, classes, package_name);
	}
    }
}
