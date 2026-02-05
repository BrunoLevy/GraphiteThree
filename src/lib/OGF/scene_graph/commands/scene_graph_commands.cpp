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


#include <OGF/scene_graph/commands/scene_graph_commands.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/gom/reflection/meta_type.h>

#include <geogram/basic/command_line.h>

namespace OGF {

    SceneGraphCommands::SceneGraphCommands() {
    }

    SceneGraphCommands::~SceneGraphCommands() {
    }

    //_______________________________________________________________________

    SceneGraphSceneCommands::SceneGraphSceneCommands() {
    }

    SceneGraphSceneCommands::~SceneGraphSceneCommands() {
    }

    void SceneGraphSceneCommands::create_object(
        const GrobClassName& type, const std::string& name
    ) {
        Grob* new_grob = scene_graph()->create_object(type);
        if(new_grob == nullptr) {
            Logger::err("SceneGraph")
                << "Could not create object" << std::endl;
            return;
        }
        if(name.length() > 0) {
            new_grob->rename(name);
        }
        scene_graph()->set_current_object(new_grob->name());
    }

    void SceneGraphSceneCommands::rename_current(const std::string& new_name) {
        Grob* g = scene_graph()->current();
        if(g == nullptr) {
            Logger::err("SceneGraph")
                << "There is no current object" << std::endl;
            return;
        }
        scene_graph()->current()->rename(new_name);
        // Note: g->name() may be different from new_name if there was
        // a name collision.
        scene_graph()->set_current_object(g->name());
    }

    void SceneGraphSceneCommands::delete_current() {
        scene_graph()->delete_current_object();
    }

    void SceneGraphSceneCommands::duplicate_current(){
        scene_graph()->duplicate_current();
    }

    void SceneGraphSceneCommands::delete_all() {
        scene_graph()->scene_graph()->set_current_object("");
        scene_graph()->disable_signals();
        scene_graph()->clear();
        scene_graph()->enable_signals();
        scene_graph()->update_values();
        scene_graph()->update();
    }

    void SceneGraphSceneCommands::display_current_dimensions() {
        Grob* current = scene_graph()->current();
        if(current != nullptr) {
            Box3d bbox = current->bbox();
            double w = bbox.xyz_max[0] - bbox.xyz_min[0];
            double h = bbox.xyz_max[1] - bbox.xyz_min[1];
            double d = bbox.xyz_max[2] - bbox.xyz_min[2];
            Logger::status()
                << "Min: "
                << bbox.xyz_min[0] << " "
                << bbox.xyz_min[1] << " "
                << bbox.xyz_min[2]
                << " Max: "
                << bbox.xyz_max[0] << " "
                << bbox.xyz_max[1] << " "
                << bbox.xyz_max[2]
                << " Sizes: "
                << w << " " << h << " " << d
                << std::endl;
        } else {
            Logger::err("Dimensions")
                << "No object" << std::endl;
        }
    }

    void SceneGraphSceneCommands::display_current_attributes() {
        if(scene_graph()->current() == nullptr) {
            Logger::err("Attributes") << "No object" << std::endl;
            return;
        }
        const ArgList& attributes = scene_graph()->current()->attributes();
        for(index_t i=0; i<attributes.nb_args(); ++i) {
            Logger::out("Attributes")
                << "Grob attribute " << attributes.ith_arg_name(i)
                << ":"
                << attributes.ith_arg_type(i)->name()
                << "="
                << attributes.ith_arg_value(i).as_string()
                << std::endl;
        }
    }

    void SceneGraphSceneCommands::set_parameter(
        const std::string& name, const std::string& value
    ) {
        GEO::CmdLine::set_arg(name, value);
    }

    void SceneGraphSceneCommands::list_parameters() {
        Logger::out("Geogram")
            << "Note: Parameters are displayed in the console"
            << std::endl;
        GEO::CmdLine::show_usage("",true);
    }

    void SceneGraphSceneCommands::enable_verbose() {
        GEO::Logger::instance()->set_minimal(false);
    }

    void SceneGraphSceneCommands::disable_verbose() {
        GEO::Logger::instance()->set_minimal(true);
    }

}
