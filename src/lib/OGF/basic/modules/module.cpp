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

#include <OGF/basic/modules/module.h>
#include <OGF/basic/modules/modmgr.h>

namespace OGF {

//____________________________________________________________________________

    Module::Module() {
        name_       = "unknown";
        vendor_     = "unknown";
        version_    = "unknown";
        is_dynamic_ = false;
        is_system_ = false;
        info_       = "unknown";
        // TODO: dependencies ...
    }

    Module::~Module() {
    }
    
    bool Module::bind_module(
        const std::string& module_name, Module* module
    ) {
        return ModuleManager::instance()->bind_module(module_name, module);
    }
    
    bool Module::unbind_module(const std::string& module_name) {
        return ModuleManager::instance()->unbind_module(module_name);
    }
    
    Module* Module::resolve_module(const std::string& module_name) {
        return ModuleManager::instance()->resolve_module(module_name);
    }

}
