/*
 *  GXML/Graphite: Geometry and Graphics Programming Library + Utilities
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
 

#include <OGF/gom_gom/common/common.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/basic/modules/module.h>
#include <string>

namespace OGF {
    
/****************************************************************/
    
    void gom_gom_libinit::initialize() {
        Logger::out("Init") << "<gom_gom>" << std::endl;
        //_____________________________________________________________

        gom_package_initialize(gom) ;        

        //_____________________________________________________________

        Module* module_info = new Module ;
        module_info->set_name("gom_gom") ;
        module_info->set_vendor("OGF") ;
        module_info->set_version("3-1.x") ;
        module_info->set_is_system(true);        
        module_info->set_info("meta-information for the OGF Object Model") ;
        Module::bind_module("gom_gom", module_info) ;

        Logger::out("Init") << "</gom_gom>" << std::endl;
    }
    
    void gom_gom_libinit::terminate() {
        Logger::out("Init") << "<~gom_gom>" << std::endl;

        //_____________________________________________________________



        //_____________________________________________________________

        Module::unbind_module("gom_gom") ;

        Logger::out("Init") << "</~gom_gom>" << std::endl;        
    }
    
// You should not need to modify this file below that point.
    
/****************************************************************/
    
    gom_gom_libinit::gom_gom_libinit() {
        increment_users() ;
    }

    gom_gom_libinit::~gom_gom_libinit() {
        decrement_users() ;
    }
    
    void gom_gom_libinit::increment_users() {
        // If I do not do that, gom_gom is
        //  initialized before gom under Windows
        // TODO: fix that
        gom_libinit::increment_users() ;

        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which 
        // may cause duplicate initialization of libraries.
        count_++ ;
        if(count_ == 1) {
            initialize() ;
        }
    }
    
    void gom_gom_libinit::decrement_users() {
        count_-- ;
        if(count_ == 0) {
            terminate() ;
        }
        // See increment_users()
        // TODO: fix that
        gom_libinit::decrement_users() ;
    }
    
    int gom_gom_libinit::count_ = 0 ;
    
}

// The initialization and termination functions
// are also declared using C linkage in order to 
// enable dynamic linking of modules.

extern "C" void GOM_GOM_API OGF_gom_gom_initialize(void);
extern "C" void GOM_GOM_API OGF_gom_gom_initialize() {
    OGF::gom_gom_libinit::increment_users() ;
}

extern "C" void GOM_GOM_API OGF_gom_gom_terminate(void);
extern "C" void GOM_GOM_API OGF_gom_gom_terminate() {
    OGF::gom_gom_libinit::decrement_users() ;
}


