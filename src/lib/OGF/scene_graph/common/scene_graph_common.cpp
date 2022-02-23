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
 

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph/grob/grob.h>
#include <OGF/scene_graph/grob/composite_grob.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/properties.h>
#include <OGF/scene_graph/shaders/shader.h>
#include <OGF/scene_graph/shaders/shader_manager.h>
#include <OGF/scene_graph/types/properties.h>
#include <OGF/scene_graph/commands/commands.h>
#include <OGF/scene_graph/commands/scene_graph_commands.h>
#include <OGF/scene_graph/types/scene_graph_shader_manager.h>
#include <OGF/scene_graph/types/scene_graph_tools_manager.h>
#include <OGF/scene_graph/tools/tool.h>
#include <OGF/scene_graph/tools/tools_manager.h>
#include <OGF/scene_graph/tools/grob_pan.h>
#include <OGF/scene_graph/tools/grob_select.h>
#include <OGF/scene_graph/tools/grob_light.h>
#include <OGF/scene_graph/full_screen_effects/ambient_occlusion.h>
#include <OGF/scene_graph/full_screen_effects/unsharp_masking.h>

#include <OGF/basic/modules/module.h>

#include <geogram/basic/environment.h>

namespace OGF {
    
/****************************************************************/
    
    void scene_graph_libinit::initialize() {
        Logger::out("Init") << "<scene_graph>" << std::endl;
        //_____________________________________________________________

	
        gom_package_initialize(scene_graph);
        SceneGraphLibrary::initialize();

	ogf_register_abstract_grob_type<Grob>();
	ogf_register_grob_tool<Grob, GrobPan>();
	ogf_register_grob_tool<Grob, GrobSelect>();
	ogf_register_grob_tool<Grob, GrobLight>();
	
	ogf_register_abstract_grob_type<CompositeGrob>();	

	// SceneGraph is not an abstract class, but I do not want
	// it to appear as an option in the 'create object' menu.
        ogf_register_abstract_grob_type<SceneGraph>();
	ogf_register_grob_commands<SceneGraph, SceneGraphSceneCommands>();
        ogf_register_grob_read_file_extension<SceneGraph>("graphite");
        ogf_register_grob_read_file_extension<SceneGraph>("graphite_ascii");
        ogf_register_grob_read_file_extension<SceneGraph>("aln");        
        ogf_register_grob_write_file_extension<SceneGraph>("graphite");
        ogf_register_grob_write_file_extension<SceneGraph>("graphite_ascii");

        ogf_register_full_screen_effect<PlainFullScreenEffect>("Plain");
        ogf_register_full_screen_effect<AmbientOcclusion>("SSAO");        
        ogf_register_full_screen_effect<UnsharpMasking>();        
        
        //_____________________________________________________________

        Module* module_info = new Module;
        module_info->set_name("scene_graph");
        module_info->set_vendor("OGF");
        module_info->set_version("3-1.x");
        module_info->set_is_system(true);                        
        module_info->set_info(
            "base classes for 3d objects, tools, commands and shaders"
        );
        Module::bind_module("scene_graph", module_info);


        Logger::out("Init") << "</scene_graph>" << std::endl;
    }
    
    void scene_graph_libinit::terminate() {
        
        Logger::out("Init") << "<~scene_graph>" << std::endl;

        //_____________________________________________________________

        SceneGraphLibrary::terminate();

        //_____________________________________________________________

        Module::unbind_module("scene_graph");

        Logger::out("Init") << "</~scene_graph>" << std::endl;
    }
    
// You should not need to modify this file below that point.
    
/****************************************************************/
    
    scene_graph_libinit::scene_graph_libinit() {
        increment_users();
    }

    scene_graph_libinit::~scene_graph_libinit() {
        decrement_users();
    }
    
    void scene_graph_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which 
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }
    
    void scene_graph_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }
    
    int scene_graph_libinit::count_ = 0;
    
}

// The initialization and termination functions
// are also declared using C linkage in order to 
// enable dynamic linking of modules.

extern "C" void SCENE_GRAPH_API OGF_scene_graph_initialize(void);
extern "C" void SCENE_GRAPH_API OGF_scene_graph_initialize() {
    OGF::scene_graph_libinit::increment_users();
}

extern "C" void SCENE_GRAPH_API OGF_scene_graph_terminate(void);
extern "C" void SCENE_GRAPH_API OGF_scene_graph_terminate() {
    OGF::scene_graph_libinit::decrement_users();
}


