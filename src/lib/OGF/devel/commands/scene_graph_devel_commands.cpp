
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2005 INRIA - Project ALICE
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

#include <OGF/devel/commands/scene_graph_devel_commands.h>
#include <OGF/devel/types/module_maker.h>
#include <OGF/scene_graph/skin/preferences.h>
#include <OGF/basic/modules/modmgr.h>


namespace OGF {

    SceneGraphDevelCommands::SceneGraphDevelCommands() {
    }

    SceneGraphDevelCommands::~SceneGraphDevelCommands() {
    }

    void SceneGraphDevelCommands::load_plugin(const std::string& plugin_name) {
        ModuleManager::instance()->load_module(plugin_name);
    }

    void SceneGraphDevelCommands::create_plugin(
        const std::string& plugin_name,
        const std::string& author,
        const std::string& contact,
        bool autoload
    ) {
        std::string author_info = author;
        if(contact != "") {
            author_info = author_info + " - " + contact;
        }
        ModuleMaker mm;
        if(!mm.create_module(plugin_name, author_info)) {
            Logger::err("ModuleMaker") << "Could not create plugin "
                                       << plugin_name
                                       << std::endl;
            return;
        }
        if(autoload) {
            // Test if module is already in the preferences (do not add
            // it twice...)
            std::string modules_str = Environment::instance()->get_value(
                "modules"
            );
            std::vector<std::string> modules;
            String::split_string(modules_str, ';', modules);
            for(index_t i=0; i<modules.size(); ++i) {
                if(modules[i] == plugin_name) {
                    return;
                }
            }
	    if(modules_str != "") {
		modules_str += ";";
	    }
	    modules_str += plugin_name;
	    Environment::instance()->set_value("modules", modules_str);
            Preferences::save_preferences();
        }
    }

    void SceneGraphDevelCommands::create_commands(
        const DynamicModuleName& plugin_name,
        const GrobClassName& type,
        const std::string& name
    ) {
        ModuleMaker mm;
        mm.create_commands(plugin_name, type, name);
    }

    void SceneGraphDevelCommands::create_scene_graph_commands(
        const DynamicModuleName& plugin_name,
        const std::string& name
    ) {
        ModuleMaker mm;
        mm.create_commands(plugin_name, "OGF::SceneGraph", name);
    }

    void SceneGraphDevelCommands::create_shader(
        const DynamicModuleName& plugin_name,
        const GrobClassName& type,
        const std::string& name,
        const std::string& base_class_name
    ) {
        ModuleMaker mm;
        mm.create_shader(plugin_name, type, name, base_class_name);
    }

    void SceneGraphDevelCommands::create_tool(
        const DynamicModuleName& plugin_name, const GrobClassName& type,
        const std::string& name, const std::string& base_class_name
    ) {
        ModuleMaker mm;
        mm.create_tool(plugin_name, type, name, base_class_name);
    }

    void SceneGraphDevelCommands::create_grob(
        const DynamicModuleName& plugin_name,
        const std::string& grob_name,
        const std::string& file_extension,
        const std::string& base_class_name
    ) {
        ModuleMaker mm;
        mm.create_grob(
            plugin_name,
            grob_name,
            file_extension,
            base_class_name
        );
    }

    void SceneGraphDevelCommands::create_class(
        const DynamicModuleName& plugin_name, const std::string& subdirectory,
        const std::string& class_name
    ) {
        ModuleMaker mm;
        mm.create_class(plugin_name, subdirectory, class_name);
    }

    void SceneGraphDevelCommands::create_file(
        const DynamicModuleName& plugin_name, const std::string& subdirectory,
        const std::string& name
    ) {
        ModuleMaker mm;
        mm.create_file(plugin_name, subdirectory, name);
    }

}
