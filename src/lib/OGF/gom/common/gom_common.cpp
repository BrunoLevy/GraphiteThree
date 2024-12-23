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


#include <OGF/gom/common/common.h>
#include <OGF/gom/types/gom.h>
#include <OGF/gom/types/gom_implementation.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/basic/modules/module.h>
#include <geogram/basic/geometry.h>
#include <string>

namespace OGF {

/****************************************************************/

    void gom_libinit::initialize() {
        Logger::out("Init") << "<gom>" << std::endl;
        //_____________________________________________________________


        Meta::initialize() ;

        ogf_declare_builtin_type<void>("void");
        ogf_declare_builtin_type<bool>("bool");
        ogf_declare_builtin_type<int>("int");
        ogf_declare_builtin_type<long>("long");
        ogf_declare_builtin_type<unsigned int>("unsigned int");
        ogf_declare_builtin_type<unsigned long>("unsigned long");
        ogf_declare_builtin_type<float>("float");
        ogf_declare_builtin_type<double>("double");
        ogf_declare_builtin_type<std::string>("std::string");
        ogf_declare_pointer_type<Memory::pointer>("GEO::Memory::pointer");
        ogf_declare_pointer_type<Memory::pointer>("OGF::Memory::pointer");
        ogf_declare_pointer_type<void*>("void*");
        ogf_declare_pointer_type<std::nullptr_t>("nullptr_t");

        ogf_declare_builtin_type<index_t>("GEO::index_t");
        ogf_declare_builtin_type<index_t>("OGF::index_t");
        ogf_declare_builtin_type<size_t>("size_t");

        ogf_declare_builtin_type<Numeric::uint8>("OGF::Numeric::uint8");
        ogf_declare_builtin_type<Numeric::int8>("OGF::Numeric::int8");
        ogf_declare_builtin_type<Numeric::uint16>("OGF::Numeric::uint16");
        ogf_declare_builtin_type<Numeric::int16>("OGF::Numeric::int16");
        ogf_declare_builtin_type<Numeric::uint32>("OGF::Numeric::uint32");
        ogf_declare_builtin_type<Numeric::int32>("OGF::Numeric::int32");
        ogf_declare_builtin_type<Numeric::uint64>("OGF::Numeric::uint64");
        ogf_declare_builtin_type<Numeric::int64>("OGF::Numeric::int64");
        ogf_declare_builtin_type<Numeric::float32>("OGF::Numeric::float32");
        ogf_declare_builtin_type<Numeric::float64>("OGF::Numeric::float64");

	ogf_declare_builtin_type<vec2>("OGF::vec2");
	ogf_declare_builtin_type<vec3>("OGF::vec3");
	ogf_declare_builtin_type<vec4>("OGF::vec4");

	ogf_declare_builtin_type<vec2i>("OGF::vec2i");
	ogf_declare_builtin_type<vec3i>("OGF::vec3i");
	ogf_declare_builtin_type<vec4i>("OGF::vec4i");

	ogf_declare_builtin_type<mat2>("OGF::mat2");
	ogf_declare_builtin_type<mat3>("OGF::mat3");
	ogf_declare_builtin_type<mat4>("OGF::mat4");

        //_____________________________________________________________

        Module* module_info = new Module ;
        module_info->set_name("gom") ;
        module_info->set_vendor("OGF") ;
        module_info->set_version("3-1.x") ;
        module_info->set_is_system(true);
        module_info->set_info("OGF Object Model and reflection API") ;
        Module::bind_module("gom", module_info) ;

        Logger::out("Init") << "</gom>" << std::endl;
    }

    void gom_libinit::terminate() {
        Logger::out("Init") << "<~gom>" << std::endl;

        //_____________________________________________________________

        Meta::terminate();
	Interpreter::terminate();

        //_____________________________________________________________

        Module::unbind_module("gom") ;

        Logger::out("Init") << "</~gom>" << std::endl;
    }

// You should not need to modify this file below that point.

/****************************************************************/

    gom_libinit::gom_libinit() {
        increment_users() ;
    }

    gom_libinit::~gom_libinit() {
        decrement_users() ;
    }

    void gom_libinit::increment_users() {
        // If I do not do that, gom is
        //  initialized before basic under Windows
        // TODO: fix that
        basic_libinit::increment_users() ;

        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which
        // may cause duplicate initialization of libraries.
        count_++ ;
        if(count_ == 1) {
            initialize() ;
        }
    }

    void gom_libinit::decrement_users() {
        count_-- ;
        if(count_ == 0) {
            terminate() ;
        }
        // See increment_users()
        // TODO: fix that
        basic_libinit::decrement_users() ;
    }

    int gom_libinit::count_ = 0 ;

}

// The initialization and termination functions
// are also declared using C linkage in order to
// enable dynamic linking of modules.

extern "C" void GOM_API OGF_gom_initialize(void);
extern "C" void GOM_API OGF_gom_initialize() {
    OGF::gom_libinit::increment_users() ;
}

extern "C" void GOM_API OGF_gom_terminate(void);
extern "C" void GOM_API OGF_gom_terminate() {
    OGF::gom_libinit::decrement_users() ;
}
