
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
 *  Contact for this Plugin: Bruno Levy - Bruno.Levy@inria.fr
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


#include <OGF/mesh_gfx/common/common.h>
#include <OGF/basic/modules/module.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/scene_graph/types/scene_graph_library.h>

#include <OGF/mesh_gfx/shaders/mesh_grob_shader.h>
#include <OGF/mesh_gfx/shaders/pdb_mesh_grob_shader.h>
#include <OGF/mesh_gfx/shaders/param_mesh_grob_shader.h>

#include <OGF/mesh_gfx/tools/mesh_grob_facet_tools.h>
#include <OGF/mesh_gfx/tools/mesh_grob_component_tools.h>
#include <OGF/mesh_gfx/tools/mesh_grob_border_tools.h>
#include <OGF/mesh_gfx/tools/mesh_grob_selection_tools.h>
#include <OGF/mesh_gfx/tools/mesh_grob_edge_tools.h>
#include <OGF/mesh_gfx/tools/mesh_grob_paint_tools.h>

#include <OGF/mesh_gfx/commands/mesh_grob_visibility_commands.h>

namespace OGF {

    void mesh_gfx_libinit::initialize() {

        Logger::out("Init") << "<mesh_gfx>" << std::endl;

        //**************************************************************

        gom_package_initialize(mesh_gfx) ;

        ogf_register_grob_shader<MeshGrob,PlainMeshGrobShader>();
        ogf_register_grob_shader<MeshGrob,ExplodedViewMeshGrobShader>();
        ogf_register_grob_shader<MeshGrob,ParamMeshGrobShader>();
        ogf_register_grob_shader<MeshGrob,PDBMeshGrobShader>();

        ogf_register_grob_tool<MeshGrob,MeshGrobGlueUnglueEdges>();
        ogf_register_grob_tool<MeshGrob,MeshGrobZipUnzipEdges>();
        ogf_register_grob_tool<MeshGrob,MeshGrobConnectDisconnectEdges>();

        ogf_register_grob_tool<MeshGrob,MeshGrobEditCenterVertex>();
        ogf_register_grob_tool<MeshGrob,MeshGrobRemoveIncidentFacets>();
        ogf_register_grob_tool<MeshGrob,MeshGrobEditHole>();
        ogf_register_grob_tool<MeshGrob,MeshGrobTransformFacet>();
        ogf_register_grob_tool<MeshGrob,MeshGrobEditFacetEdge>();

        ogf_register_grob_tool<MeshGrob,MeshGrobKeepOrRemoveComponent>();
        ogf_register_grob_tool<MeshGrob,MeshGrobTransformComponent>();
        ogf_register_grob_tool<MeshGrob,MeshGrobCopyComponent>();
        ogf_register_grob_tool<MeshGrob,MeshGrobFlipComponent>();

        ogf_register_grob_tool<MeshGrob,MeshGrobSelectUnselectVertex>();

        ogf_register_grob_tool<MeshGrob,MeshGrobEditEdge>();

        ogf_register_grob_tool<MeshGrob,MeshGrobPaint>();
        ogf_register_grob_tool<MeshGrob,MeshGrobPaintRect>();
        ogf_register_grob_tool<MeshGrob,MeshGrobPaintFreeform>();
        ogf_register_grob_tool<MeshGrob,MeshGrobPaintConnected>();
        ogf_register_grob_tool<MeshGrob,MeshGrobProbe>();
        ogf_register_grob_tool<MeshGrob,MeshGrobRuler>();


	ogf_register_grob_commands<MeshGrob,MeshGrobVisibilityCommands>();

        //**************************************************************

        Module* module_info = new Module;
        module_info->set_name("mesh_gfx");
        module_info->set_vendor("Bruno Levy - Bruno.Levy@inria.fr");
        module_info->set_version("3-1.x");
        module_info->set_info("shaders and tools for MeshGrob");
        Module::bind_module("mesh_gfx", module_info);

        Logger::out("Init") << "</mesh_gfx>" << std::endl;
    }

    void mesh_gfx_libinit::terminate() {
        Logger::out("Init") << "<~mesh_gfx>" << std::endl;

        //*************************************************************

        //*************************************************************

        Module::unbind_module("mesh_gfx");
        Logger::out("Init") << "</~mesh_gfx>" << std::endl;
    }

    mesh_gfx_libinit::mesh_gfx_libinit() {
        increment_users();
    }

    mesh_gfx_libinit::~mesh_gfx_libinit() {
        decrement_users();
    }

    void mesh_gfx_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }

    void mesh_gfx_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }

    int mesh_gfx_libinit::count_ = 0;
}

// The initialization and termination functions
// are also declared using C linkage in order to
// enable dynamic linking of modules.

extern "C" void MESH_GFX_API OGF_mesh_gfx_initialize(void);
extern "C" void MESH_GFX_API OGF_mesh_gfx_initialize() {
    OGF::mesh_gfx_libinit::increment_users();
}

extern "C" void MESH_GFX_API OGF_mesh_gfx_terminate(void);
extern "C" void MESH_GFX_API OGF_mesh_gfx_terminate() {
    OGF::mesh_gfx_libinit::decrement_users();
}
