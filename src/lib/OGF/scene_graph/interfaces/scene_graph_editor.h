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

#ifndef H_OGF_SCENE_GRAPH_INTERFACES_SCENE_GRAPH_EDIT_H
#define H_OGF_SCENE_GRAPH_INTERFACES_SCENE_GRAPH_EDIT_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/commands/commands.h>
#include <OGF/scene_graph/types/scene_graph.h>

namespace OGF {

    /**
     * \brief All SceneGraph modifications go through this Interface.
     * \details Calls are logged to the history if called from user interaction.
     */
    gom_class SCENE_GRAPH_API SceneGraphEditor : public Interface {
    public:
	/**
	 * \brief SceneGraphEditor constructor.
	 */
	SceneGraphEditor();

	/**
	 * \brief SceneGraphEditor destrutor.
	 */
	~SceneGraphEditor() override;

	/**
	 * \brief Gets the wrapped MeshGrob.
	 * \return a pointer to the MeshGrob or nullptr.
	 */
	SceneGraph* scene_graph() const {
	    return dynamic_cast<SceneGraph*>(grob());
	}

      gom_slots:

	void set_current(const GrobName& grob_name);
	void delete_object(const GrobName& grob_name);
	void delete_current();
	void delete_selected();
	void delete_all();

	void show(const GrobName& grob_name);
	void show_selected();
	void show_all();

	void hide(const GrobName& grob_name);
	void hide_selected();
	void hide_all();

	void move_up(const GrobName& grob_name);
	void move_down(const GrobName& grob_name);
	void rename(const GrobName& grob_name, const std::string& new_name);
	Grob* duplicate(const GrobName& grob_name);
	Grob* create_object(
	    const GrobClassName& class_name = "OGF::MeshGrob",
	    const std::string& name = "new_object"
	);

	void copy_graphic_properties_to_all(const GrobName& grob_name);
	void copy_graphic_properties_to_visible(const GrobName& grob_name);
	void copy_graphic_properties_to_selected(const GrobName& grob_name);

    protected:
	void backup_current();
	void restore_current();

    private:
	std::string current_bkp_;
    };


}

#endif
