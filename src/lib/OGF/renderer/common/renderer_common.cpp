/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
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
 

#include <OGF/renderer/common/common.h>
#include <OGF/basic/modules/module.h>
#include <geogram_gfx/basic/common.h>

namespace OGF {
    
/****************************************************************/
    
    void renderer_libinit::initialize() {
        Logger::out("Init") << "<renderer>" << std::endl;
        //_____________________________________________________________

        Module* module_info = new Module ;
        module_info->set_name("renderer") ;
        module_info->set_vendor("OGF") ;
        module_info->set_version("3-1.x") ;
        module_info->set_info("3D rendering abstraction layer") ;
        module_info->set_is_system(true);        
        Module::bind_module("renderer", module_info) ;

        //_____________________________________________________________



        //_____________________________________________________________

        Logger::out("Init") << "</renderer>" << std::endl;
    }
    
    void renderer_libinit::terminate() {
        Logger::out("Init") << "<~renderer>" << std::endl;

        //_____________________________________________________________

        
        //_____________________________________________________________

        Module::unbind_module("renderer") ;
        
        Logger::out("Init") << "</~renderer>" << std::endl;
    }
    
// You should not need to modify this file below that point.
    
/****************************************************************/
    
    renderer_libinit::renderer_libinit() {
        increment_users() ;
    }

    renderer_libinit::~renderer_libinit() {
        decrement_users() ;
    }
    
    void renderer_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which 
        // may cause duplicate initialization of libraries.
        count_++ ;
        if(count_ == 1) {
            initialize() ;
        }
    }
    
    void renderer_libinit::decrement_users() {
        count_-- ;
        if(count_ == 0) {
            terminate() ;
        }
    }
    
    int renderer_libinit::count_ = 0 ;
    
}

// The initialization and termination functions
// are also declared using C linkage in order to 
// enable dynamic linking of modules.

extern "C" void RENDERER_API OGF_renderer_initialize(void);
extern "C" void RENDERER_API OGF_renderer_initialize() {
    OGF::renderer_libinit::increment_users() ;
}

extern "C" void RENDERER_API OGF_renderer_terminate(void);
extern "C" void RENDERER_API OGF_renderer_terminate() {
    OGF::renderer_libinit::decrement_users() ;
}


