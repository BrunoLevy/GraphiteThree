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

#include <OGF/voxel/common/common.h>
#include <OGF/basic/modules/module.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/voxel/grob/voxel_grob.h>
#include <OGF/voxel/commands/voxel_grob_attributes_commands.h>
#include <OGF/voxel/interfaces/voxel_grob_editor_interface.h>

#include <geogram/basic/command_line.h>

namespace OGF {
    void voxel_libinit::initialize() {
        Logger::out("Init") << "<voxel>" << std::endl;

        //**************************************************************

        gom_package_initialize(voxel) ;

        ogf_register_grob_type<VoxelGrob>();
	ogf_register_grob_read_file_extension<VoxelGrob>("vox");
	ogf_register_grob_write_file_extension<VoxelGrob>("vox");
        ogf_register_grob_commands<VoxelGrob,VoxelGrobAttributesCommands>();
        ogf_register_grob_interface<VoxelGrob,VoxelGrobEditor>();

        //**************************************************************

        Module* module_info = new Module;
        module_info->set_name("voxel");
        module_info->set_vendor("OGF");
        module_info->set_is_system(true);
        module_info->set_version("3-1.x");
        module_info->set_info(
            "Voxel grids object and manipulation"
        );
        Module::bind_module("voxel", module_info);

        Logger::out("Init") << "</voxel>" << std::endl;
    }

    void voxel_libinit::terminate() {
        Logger::out("Init") << "<~voxel>" << std::endl;

        //**************************************************************

        //**************************************************************

        Module::unbind_module("voxel");
        Logger::out("Init") << "</~voxel>" << std::endl;
    }

    voxel_libinit::voxel_libinit() {
        increment_users();
    }
    voxel_libinit::~voxel_libinit() {
        decrement_users();
    }

    void voxel_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }

    void voxel_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }

    int voxel_libinit::count_ = 0;
}
// The initialization and termination functions
// are also declared using C linkage in order to
// enable dynamic linking of modules.

extern "C" void VOXEL_API OGF_voxel_initialize(void);
extern "C" void VOXEL_API OGF_voxel_initialize() {
    OGF::voxel_libinit::increment_users();
}

extern "C" void VOXEL_API OGF_voxel_terminate(void);
extern "C" void VOXEL_API OGF_voxel_terminate() {
    OGF::voxel_libinit::decrement_users();
}
