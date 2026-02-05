
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
 */


#include <OGF/WarpDrive/common/common.h>
#include <OGF/basic/modules/module.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/scene_graph/types/scene_graph_library.h>

#include <OGF/WarpDrive/commands/mesh_grob_transport_commands.h>
#include <OGF/WarpDrive/commands/mesh_grob_martingale_commands.h>
#include <OGF/WarpDrive/commands/voxel_grob_transport_commands.h>

#include <OGF/WarpDrive/interfaces/mesh_grob_transport_interface.h>

#include <OGF/WarpDrive/shaders/voronoi_mesh_grob_shader.h>
#include <OGF/WarpDrive/shaders/transport_mesh_grob_shader.h>

#include <OGF/WarpDrive/IO/xyzw_serializer.h>

#include <OGF/WarpDrive/shaders/cosmo_mesh_grob_shader.h>
#include <OGF/WarpDrive/shaders/aniso_mesh_grob_shader.h>
// [includes insertion point] (do not delete this line, ModuleMaker depends on it)

namespace OGF {

    void WarpDrive_libinit::initialize() {

        Logger::out("Init") << "<WarpDrive>" << std::endl;

        //_____________________________________________________________

        gom_package_initialize(WarpDrive) ;

        ogf_register_grob_commands<OGF::MeshGrob,OGF::MeshGrobTransportCommands>();
        ogf_register_grob_commands<OGF::MeshGrob,OGF::MeshGrobMartingaleCommands>();
	ogf_register_grob_interface<OGF::MeshGrob, OGF::MeshGrobTransport>();

        ogf_register_grob_commands<OGF::VoxelGrob,OGF::VoxelGrobTransportCommands>();

	ogf_register_grob_shader<OGF::MeshGrob,OGF::VoronoiMeshGrobShader>();
	ogf_register_grob_shader<OGF::MeshGrob,OGF::TransportMeshGrobShader>();

        geo_register_MeshIOHandler_creator(XYZWSerializer, "xyzw");
        MeshGrob::register_geogram_file_extensions();


        ogf_register_grob_shader<OGF::MeshGrob,CosmoMeshGrobShader>();
        ogf_register_grob_shader<OGF::MeshGrob,AnisoMeshGrobShader>();
        // [source insertion point] (do not delete this line, ModuleMaker depends on it)

        // Insert package initialization stuff here ...

        //_____________________________________________________________

        Module* module_info = new Module;
        module_info->set_name("WarpDrive");
        module_info->set_vendor("OGF");
        module_info->set_version("3.0");
        module_info->set_info("New package, created by Graphite Development Tools");
        Module::bind_module("WarpDrive", module_info);

        Logger::out("Init") << "</WarpDrive>" << std::endl;
    }

    void WarpDrive_libinit::terminate() {
        Logger::out("Init") << "<~WarpDrive>" << std::endl;
        //_____________________________________________________________

        // Insert package termination stuff here ...

        //_____________________________________________________________

        Module::unbind_module("WarpDrive");
        Logger::out("Init") << "</~WarpDrive>" << std::endl;
    }

    WarpDrive_libinit::WarpDrive_libinit() {
        increment_users();
    }

    WarpDrive_libinit::~WarpDrive_libinit() {
        decrement_users();
    }

    void WarpDrive_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }

    void WarpDrive_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }

    int WarpDrive_libinit::count_ = 0;
}

// The initialization and termination functions
// are also declared using C linkage in order to
// enable dynamic linking of modules.

extern "C" void WarpDrive_API OGF_WarpDrive_initialize(void);
extern "C" void WarpDrive_API OGF_WarpDrive_initialize() {
    OGF::WarpDrive_libinit::increment_users();
}

extern "C" void WarpDrive_API OGF_WarpDrive_terminate(void);
extern "C" void WarpDrive_API OGF_WarpDrive_terminate() {
    OGF::WarpDrive_libinit::decrement_users();
}
