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
 *  (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#ifndef H_OGF_SCENE_GRAPH_INTERFACES_SCENE_GRAPH_GRAPHICS_H
#define H_OGF_SCENE_GRAPH_INTERFACES_SCENE_GRAPH_GRAPHICS_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/commands/commands.h>
#include <OGF/scene_graph/types/scene_graph.h>

namespace OGF {

    /**
     * \brief All SceneGraph modifications related with graphics
     *  go through this Interface.
     * \details Calls are logged to the history if called
     *  from user interaction.
     */
    gom_class SCENE_GRAPH_API SceneGraphGraphicsInterface : public Interface {
    public:
	/**
	 * \brief SceneGraphSelectionInterface constructor.
	 */
	SceneGraphGraphicsInterface();

	/**
	 * \brief SceneGraphEditor destrutor.
	 */
	~SceneGraphGraphicsInterface() override;

    gom_slots:

	/**
	 * \brief Makes an object visible.
	 * \details Ignored in terminal applications.
	 * \param[in] grob the object to be shown or its name
	 */
	void show_object(const GrobName& grob);

	/**
	 * \brief Makes an object visible and hides all the other ones.
	 * \details Ignored in terminal applications.
	 * \param[in] grob the object to be shown or its name
	 */
	void show_only(const GrobName& grob);

	/**
	 * \brief Makes all selected objects visible.
	 * \details Ignored in terminal applications.
	 */
	void show_selected();

	/**
	 * \brief Makes all objects visible.
	 * \details Ignored in terminal applications.
	 */
	void show_all();

	/**
	 * \brief Makes an object invisible.
	 * \details Ignored in terminal applications.
	 * \param[in] grob the object to be hidden or its name
	 */
	void hide_object(const GrobName& grob);

	/**
	 * \brief Makes all selected objects invisible.
	 * \details Ignored in terminal applications.
	 */
	void hide_selected();

	/**
	 * \brief Makes all objects invisible.
	 * \details Ignored in terminal applications.
	 */
	void hide_all();

	/**
	 * \brief Copies graphic properties of an object to all objects
	 * \details Ignored in terminal applications
	 * \param[in] grob the object or its name
	 */
	void copy_object_properties_to_all(const GrobName& grob);

	/**
	 * \brief Copies graphic properties of an object to all visible objects
	 * \details Ignored in terminal applications
	 * \param[in] grob the object or its name
	 */
	void copy_object_properties_to_visible(const GrobName& grob);

	/**
	 * \brief Copies graphic properties of an object to
	 *  all selected objects
	 * \details Ignored in terminal applications
	 * \param[in] grob the object or its name
	 */
	void copy_object_properties_to_selected(const GrobName& grob);

	/**
	 * \brief Sets the shader for an object
	 * \details Ignored in terminal applications
	 * \param[in] grob the object or its name
	 * \param[in] user_name the user name of the shader, that is,
	 *   without the "OGF::" prefix
	 */
	void set_object_shader(
	    const GrobName& grob, const std::string& user_name
	);
    };

}

#endif
