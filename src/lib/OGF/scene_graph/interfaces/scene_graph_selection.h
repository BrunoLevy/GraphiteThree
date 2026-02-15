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

#ifndef H_OGF_SCENE_GRAPH_INTERFACES_SCENE_GRAPH_SELECTION_H
#define H_OGF_SCENE_GRAPH_INTERFACES_SCENE_GRAPH_SELECTION_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/interfaces/scene_graph_interface.h>

namespace OGF {

    /**
     * \brief All SceneGraph modifications related with selection
     *  go through this Interface.
     * \details Calls are logged to the history if called
     *  from user interaction.
     */
    gom_class SCENE_GRAPH_API SceneGraphSelectionInterface
	: public SceneGraphInterface {
    public:
	/**
	 * \brief SceneGraphSelectionInterface constructor.
	 */
	SceneGraphSelectionInterface();

	/**
	 * \brief SceneGraphEditor destrutor.
	 */
	~SceneGraphSelectionInterface() override;

	/**
	 * \brief Gets the wrapped MeshGrob.
	 * \return a pointer to the MeshGrob or nullptr.
	 */
	SceneGraph* scene_graph() const {
	    return dynamic_cast<SceneGraph*>(grob());
	}

    gom_slots:
	/**
	 * \brief Adds an object to the selection
	 * \param[in] grob the object or its name
	 */
	void select_object(const GrobName& grob);

	/**
	 * \brief Removes an object from the selection
	 * \param[in] grob the object or its name
	 */
	void unselect_object(const GrobName& grob);

	/**
	 * \brief Toggles the selection flag for an object
	 * \param[in] grob the object or its name
	 */
	void toggle_selection(const GrobName& grob);

	/**
	 * \brief Adds all objects to selection.
	 */
	void select_all();

	/**
	 * \brief Removes all objects from selection.
	 */
	void clear_selection();

	/**
	 * \brief Extends selection
	 * \details Selects all objects between current object and grob
	 */
	void extend_selection(const GrobName& grob);

	/**
	 * \brief Gets the number of selected objects
	 */
	index_t nb_selected() const;
    };

}

#endif
