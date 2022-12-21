/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
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
 *  Contact: Bruno Levy - levy@loria.fr
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
 *  (non-GPL) libraries: Qt, SuperLU, WildMagic and CGAL
 */
 
#include <OGF/mesh/common/common.h>
#include <OGF/basic/modules/module.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/mesh/grob/mesh_grob.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <OGF/mesh/shaders/pdb_mesh_grob_shader.h>
#include <OGF/mesh/shaders/param_mesh_grob_shader.h>
#include <OGF/mesh/commands/mesh_grob_points_commands.h>
#include <OGF/mesh/commands/mesh_grob_surface_commands.h>
#include <OGF/mesh/commands/mesh_grob_shapes_commands.h>
#include <OGF/mesh/commands/mesh_grob_volume_commands.h>
#include <OGF/mesh/commands/mesh_grob_mesh_commands.h>
#include <OGF/mesh/commands/mesh_grob_attributes_commands.h>
#include <OGF/mesh/commands/mesh_grob_spectral_commands.h>

#include <OGF/mesh/interfaces/mesh_grob_editor_interface.h>

#include <OGF/mesh/tools/mesh_grob_facet_tools.h>
#include <OGF/mesh/tools/mesh_grob_component_tools.h>
#include <OGF/mesh/tools/mesh_grob_border_tools.h>
#include <OGF/mesh/tools/mesh_grob_selection_tools.h>
#include <OGF/mesh/tools/mesh_grob_edge_tools.h>
#include <OGF/mesh/tools/mesh_grob_paint_tools.h>

#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>

// [includes insertion point] (do not delete this line)

namespace OGF {
    void mesh_libinit::initialize() {
        Logger::out("Init") << "<mesh>" << std::endl; 
        //_____________________________________________________________
        gom_package_initialize(mesh) ;
        
        ogf_register_grob_type<MeshGrob>();
        
        ogf_register_grob_shader<MeshGrob,PlainMeshGrobShader>();
        ogf_register_grob_shader<MeshGrob,PDBMeshGrobShader>();
        ogf_register_grob_shader<MeshGrob,ParamMeshGrobShader>();		
        
        ogf_register_grob_commands<MeshGrob,MeshGrobPointsCommands>();
        ogf_register_grob_commands<MeshGrob,MeshGrobSurfaceCommands>(); 
        ogf_register_grob_commands<MeshGrob,MeshGrobShapesCommands>();       
        ogf_register_grob_commands<MeshGrob,MeshGrobVolumeCommands>();
        ogf_register_grob_commands<MeshGrob,MeshGrobMeshCommands>();
        ogf_register_grob_commands<MeshGrob,MeshGrobAttributesCommands>();
        ogf_register_grob_commands<MeshGrob,MeshGrobSpectralCommands>();	

        ogf_register_grob_interface<MeshGrob,MeshGrobEditor>();	
	
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

        ogf_register_grob_tool<MeshGrob,MeshGrobCreateEdge>();	

        ogf_register_grob_tool<MeshGrob,MeshGrobPaint>();
        ogf_register_grob_tool<MeshGrob,MeshGrobPaintRect>();        
        ogf_register_grob_tool<MeshGrob,MeshGrobProbe>();
        ogf_register_grob_tool<MeshGrob,MeshGrobRuler>();
       
        // [source insertion point] (do not delete this line)
        // Insert package initialization stuff here ...
        //_____________________________________________________________
        Module* module_info = new Module;
        module_info->set_name("mesh");
        module_info->set_vendor("OGF");
        module_info->set_is_system(true);        
        module_info->set_version("3-1.x");
        module_info->set_info(
            "Access to Geogram objects from Graphite"
        );
        Module::bind_module("mesh", module_info);

        // Make Graphite aware of the mesh file extensions
        // that Geogram supports
        MeshGrob::register_geogram_file_extensions();

        GEO::CmdLine::import_arg_group("pre");
        GEO::CmdLine::import_arg_group("remesh");
        GEO::CmdLine::import_arg_group("algo");
        GEO::CmdLine::import_arg_group("post");
        GEO::CmdLine::import_arg_group("opt");
        GEO::CmdLine::import_arg_group("co3ne");
        GEO::CmdLine::import_arg_group("tet");
        GEO::CmdLine::import_arg_group("hex");
        GEO::CmdLine::import_arg_group("stat");

        Logger::out("Init") << "</mesh>" << std::endl;         
    }
    
    void mesh_libinit::terminate() {
        Logger::out("Init") << "<~mesh>" << std::endl;                 
        //_____________________________________________________________
        // Insert package termination stuff here ...
        
        //_____________________________________________________________

        Module::unbind_module("mesh");
        Logger::out("Init") << "</~mesh>" << std::endl;        
    }
    
    mesh_libinit::mesh_libinit() {
        increment_users();
    }
    mesh_libinit::~mesh_libinit() {
        decrement_users();
    }
    
    void mesh_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which 
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }
    
    void mesh_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }
    
    int mesh_libinit::count_ = 0;
}
// The initialization and termination functions
// are also declared using C linkage in order to 
// enable dynamic linking of modules.
extern "C" void MESH_API OGF_mesh_initialize(void);
extern "C" void MESH_API OGF_mesh_initialize() {
    OGF::mesh_libinit::increment_users();
}

extern "C" void MESH_API OGF_mesh_terminate(void);
extern "C" void MESH_API OGF_mesh_terminate() {
    OGF::mesh_libinit::decrement_users();
}

