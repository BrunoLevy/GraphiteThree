
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
 


#ifndef H_OGF_DEVEL_COMMANDS_SCENE_GRAPH_DEVEL_COMMANDS_H
#define H_OGF_DEVEL_COMMANDS_SCENE_GRAPH_DEVEL_COMMANDS_H

#include <OGF/devel/common/common.h>
#include <OGF/devel/types/properties.h>
#include <OGF/scene_graph/commands/scene_graph_commands.h>

/**
 * \file OGF/devel/commands/scene_graph_devel_commands.h
 * \brief Graphite developpers commands.
 */

namespace OGF {

    /**
     * \brief Commands to create new plugins, Grob class names,
     *  Tool, Commands and Shader.
     */
    gom_class SceneGraphDevelCommands : public SceneGraphCommands {
    public:
        /**
         * \brief SceneGraphDevelCommands constructor.
         */
        SceneGraphDevelCommands();

        /**
         * \brief SceneGraphDevelCommands destructor.
         */
        virtual ~SceneGraphDevelCommands();

    gom_slots:
        /**
         * \brief Loads a plugin.
         * \param[in] plugin_name the name of the plugin
         */
        void load_plugin(const std::string& plugin_name);

        /**
         * \brief Creates C++ skeleton sources for a new plugin.
         * \param[in] plugin_name the name of the plugin
         * \param[in] author your name
         * \param[in] contact your contact info (only used to be included 
         *  in the generated headers, you can leave blank if you wish)
         * \param[in] autoload if true, the plugin will be loaded next time
         *  Graphite is started (once the plugin is compiled)
         */
        void create_plugin(
            const std::string& plugin_name,
            const std::string& author = "Nicolas Bourbaki",
            const std::string& contact = "",
            bool autoload = true
        );

        /**
         * \brief Creates a new Commands class in a plugin.
         * \param[in] plugin_name the name of the plugin
         * \param[in] type the Grob class the commands should be attached to
         * \param[in] name for instance "FooBar" 
         *  for "OGF::MeshGrobFooBarCommands"
         */
        void create_commands(
            const DynamicModuleName& plugin_name,
            const GrobClassName& type, 
            const std::string& name
        );

        /**
         * \brief Creates a new Shader class in a plugin.
         * \param[in] plugin_name the name of the plugin
         * \param[in] type the Grob class the shader should be attached to
         * \param[in] name for instance "FooBar" for "OGF::MeshGrobFooBarShader"
         * \param[in] base_class_name optional base class for the shader
         *  (with the "OGF::" prefix)
         */
        void create_shader(
            const DynamicModuleName& plugin_name, 
            const GrobClassName& type,
            const std::string& name,
            const std::string& base_class_name=""
        );

        /**
         * \brief Creates a new tool in a plugin.
         * \param[in] plugin_name the name of the plugin
         * \param[in] type the Grob class the shader should be attached to
         * \param[in] name for instance "FooBar" for "OGF::MeshGrobFooBarTool"
         * \param[in] base_class_name optional base class for the shader
         *  (with the "OGF::" prefix)
         */
        void create_tool(
            const DynamicModuleName& plugin_name,
            const GrobClassName& type,
            const std::string& name,            
            const std::string& base_class_name = ""
        );

        /**
         * \brief Creates a new Grob class in a plugin.
         * \param[in] plugin_name the name of the plugin
         * \param[in] grob_name the name of the Grob class
         * \param[in] file_extension optional extension of the files used to
         *  save/load objects with this Grob class, without the "."
         * \param[in] base_class_name optional base class (with the "OGF::"
         *  prefix)
         */
        void create_grob(
            const DynamicModuleName& plugin_name,
            const std::string& grob_name,
            const std::string& file_extension = "",
            const std::string& base_class_name = ""
        );
        
        /**
         * \brief Creates a new class in a plugin.
         * \param[in] plugin_name the name of the plugin
         * \param[in] subdirectory relative path to the subdirectory where 
         *  the class files should be created
         * \param[in] class_name the name of the class to be created
         */
        void create_class(
            const DynamicModuleName& plugin_name,
            const std::string& subdirectory,
            const std::string& class_name
        );

        /**
         * \brief Creates a new file in a plugin.
         * \param[in] plugin_name the name of the plugin
         * \param[in] subdirectory relative path to the subdirectory where 
         *  the class files should be created
         * \param[in] name the name of the file to be created, without extension
         */
        void create_file(
            const DynamicModuleName& plugin_name,
            const std::string& subdirectory,
            const std::string& name
        );

        
    };
}

#endif


