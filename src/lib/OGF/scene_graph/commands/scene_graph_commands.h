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

#ifndef H_OGF_SCENE_GRAPH_TYPES_SG_FILE_COMMANDS_H
#define H_OGF_SCENE_GRAPH_TYPES_SG_FILE_COMMANDS_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/commands/commands.h>
#include <OGF/scene_graph/types/scene_graph.h>

/**
 * \file OGF/scene_graph/commands/scene_graph_commands.h
 * \brief classes for Commands that act on the whole SceneGraph. 
 */

namespace OGF {

    /**
     * \brief Base class for Commands that act on the whole SceneGraph.
     */
    gom_attribute(abstract, "true") 
    gom_class SCENE_GRAPH_API SceneGraphCommands : public Commands {
    public:
        /**
         * \brief SceneGraphCommands constructor.
         */
        SceneGraphCommands();


	/**
	 * \brief SceneGraphCommands destructor.
	 */
	 ~SceneGraphCommands() override;
	
        /**
         * \brief Gets the SceneGraph.
         * \return a pointer to the SceneGraph
         */
        SceneGraph* scene_graph() const {
            return dynamic_cast<SceneGraph*>(grob());
        }
     };

    //_________________________________________________________________________

    /**
     * \brief Some commands that act on the whole SceneGraph.
     */
    gom_class SCENE_GRAPH_API SceneGraphSceneCommands :
        public SceneGraphCommands {
    public:
        /**
         * \brief SceneGraphSceneCommands constructor.
         */
        SceneGraphSceneCommands();

	/**
	 * \brief SceneGraphSceneCommands destructor.
	 */
	~SceneGraphSceneCommands();
	
    gom_slots:
        /**
         * \brief Creates a new object.
         * \param[in] type the type of the object to be created.
         * \param[in] name the name of the object to be created.
         */
        void create_object(
            const GrobClassName& type = "OGF::MeshGrob",
            const std::string& name="new_object"
        );

        /**
         * \brief Remanes the current object.
         * \param[in] new_name the new name.
         */
        void rename_current(const std::string& new_name);

        /**
         * \brief Duplicates the current object.
         */
        void duplicate_current();

        /**
         * \brief Deletes the current object.
         */
        void delete_current();

        /**
         * \brief Deletes all the objects of the scene graph.
         */
        void delete_all();

        /**
         * \menu Current
         * \brief Displays the bounding box and dimensions of the
         *  current object.
         */
        void display_current_dimensions();

        /**
         * \menu Current
         * \brief Displays the attributes of the current object.
         */
        void display_current_attributes();
        
        /**
         * \menu System/Parameters
         * \brief Sets a global parameter.
         * \param[in] name name of the parameter.
         * \param[in] value new value for the parameter.
         */
        void set_parameter(
            const std::string& name, const std::string& value
        );

        /**
         * \menu System/Parameters
         * \brief Lists all the parameters and their values.
         */
        void list_parameters();

        /**
         * \menu System/Logger
         * \brief Enables logger messages.
         */
        void enable_verbose();

        /**
         * \menu System/Logger
         * \brief Disables logger messages.
         */
        void disable_verbose();
    };

}

#endif
