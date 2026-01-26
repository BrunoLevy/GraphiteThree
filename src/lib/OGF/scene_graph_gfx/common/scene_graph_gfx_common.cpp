
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
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
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Bruno Levy
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 *
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */


#include <OGF/scene_graph_gfx/common/common.h>
#include <OGF/basic/modules/module.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/scene_graph/types/scene_graph_library.h>

#include <OGF/scene_graph_gfx/shaders/shader.h>
#include <OGF/scene_graph_gfx/shaders/shader_manager.h>
#include <OGF/scene_graph_gfx/shaders/scene_graph_shader_manager.h>

#include <OGF/scene_graph_gfx/tools/scene_graph_tools_manager.h>
#include <OGF/scene_graph_gfx/tools/tool.h>
#include <OGF/scene_graph_gfx/tools/tools_manager.h>
#include <OGF/scene_graph_gfx/tools/grob_pan.h>
#include <OGF/scene_graph_gfx/tools/grob_select.h>
#include <OGF/scene_graph_gfx/tools/grob_light.h>
#include <OGF/scene_graph_gfx/tools/grob_focus.h>

#include <OGF/scene_graph_gfx/full_screen_effects/ambient_occlusion.h>
#include <OGF/scene_graph_gfx/full_screen_effects/unsharp_masking.h>

namespace OGF {

    void scene_graph_gfx_libinit::initialize() {

        Logger::out("Init") << "<scene_graph_gfx>" << std::endl;

        //**************************************************************

        gom_package_initialize(scene_graph_gfx) ;

	ogf_register_grob_tool<Grob, GrobPan>();
	ogf_register_grob_tool<Grob, GrobSelect>();
	ogf_register_grob_tool<Grob, GrobLight>();
	ogf_register_grob_tool<Grob, GrobFocus>();

        ogf_register_full_screen_effect<PlainFullScreenEffect>("Plain");
        ogf_register_full_screen_effect<AmbientOcclusion>("SSAO");
        ogf_register_full_screen_effect<UnsharpMasking>();

        //**************************************************************

        Module* module_info = new Module;
        module_info->set_name("scene_graph_gfx");
        module_info->set_vendor("Bruno Levy");
        module_info->set_version("3-1.x");
        module_info->set_info(
                "New package, created by Graphite Development Tools"
        );
        Module::bind_module("scene_graph_gfx", module_info);

        Logger::out("Init") << "</scene_graph_gfx>" << std::endl;
    }

    void scene_graph_gfx_libinit::terminate() {
        Logger::out("Init") << "<~scene_graph_gfx>" << std::endl;

        //*************************************************************

        // Insert package termination stuff here ...

        //*************************************************************

        Module::unbind_module("scene_graph_gfx");
        Logger::out("Init") << "</~scene_graph_gfx>" << std::endl;
    }

    scene_graph_gfx_libinit::scene_graph_gfx_libinit() {
        increment_users();
    }

    scene_graph_gfx_libinit::~scene_graph_gfx_libinit() {
        decrement_users();
    }

    void scene_graph_gfx_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }

    void scene_graph_gfx_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }

    int scene_graph_gfx_libinit::count_ = 0;
}

// The initialization and termination functions
// are also declared using C linkage in order to
// enable dynamic linking of modules.

extern "C" void SCENE_GRAPH_GFX_API OGF_scene_graph_gfx_initialize(void);
extern "C" void SCENE_GRAPH_GFX_API OGF_scene_graph_gfx_initialize() {
    OGF::scene_graph_gfx_libinit::increment_users();
}

extern "C" void SCENE_GRAPH_GFX_API OGF_scene_graph_gfx_terminate(void);
extern "C" void SCENE_GRAPH_GFX_API OGF_scene_graph_gfx_terminate() {
    OGF::scene_graph_gfx_libinit::decrement_users();
}
