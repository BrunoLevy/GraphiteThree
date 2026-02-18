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

#include <OGF/scene_graph/interfaces/scene_graph_selection.h>

namespace OGF {

    SceneGraphSelectionInterface::SceneGraphSelectionInterface() {
    }

    SceneGraphSelectionInterface::~SceneGraphSelectionInterface() {
    }

    void SceneGraphSelectionInterface::select_object(
	const GrobName& grob_name
    ) {
	if(scene_graph() == nullptr) {
	    return;
	}
	Grob* grob = scene_graph()->resolve(grob_name);
	if(grob != nullptr) {
	    grob->set_selected(true);
	}
    }

    void SceneGraphSelectionInterface::unselect_object(
	const GrobName& grob_name
    ) {
	if(scene_graph() == nullptr) {
	    return;
	}
	Grob* grob = scene_graph()->resolve(grob_name);
	if(grob != nullptr) {
	    grob->set_selected(false);
	}
    }

    void SceneGraphSelectionInterface::toggle_selection(
	const GrobName& grob_name
    ) {
	if(scene_graph() == nullptr) {
	    return;
	}
	Grob* grob = scene_graph()->resolve(grob_name);
	if(grob != nullptr) {
	    grob->set_selected(!grob->get_selected());
	}
    }

    void SceneGraphSelectionInterface::select_all() {
	if(scene_graph() == nullptr) {
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    scene_graph()->ith_child(i)->set_selected(true);
	}
    }

    void SceneGraphSelectionInterface::clear_selection() {
	if(scene_graph() == nullptr) {
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    scene_graph()->ith_child(i)->set_selected(false);
	}
    }

    void SceneGraphSelectionInterface::extend_selection(const GrobName& grob) {
	if(scene_graph() == nullptr) {
	    return;
	}
	index_t cur_index = NO_INDEX;
	index_t grob_index = NO_INDEX;
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    Grob* cur_grob = scene_graph()->ith_child(i);
	    if(
		cur_grob->name() ==
		std::string(scene_graph()->get_current_object())) {
		cur_index = i;
	    }
	    if(cur_grob->name() == std::string(grob)) {
		grob_index = i;
	    }
	}
	if(cur_index == NO_INDEX || grob_index == NO_INDEX) {
	    return;
	}
	for(
	    index_t i=std::min(cur_index,grob_index);
	    i<=std::max(cur_index,grob_index);
	    ++i
	) {
	    scene_graph()->ith_child(i)->set_selected(true);
	}
    }

    index_t SceneGraphSelectionInterface::nb_selected() const {
	if(scene_graph() == nullptr) {
	    return 0;
	}
	index_t result = 0;
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    if(scene_graph()->ith_child(i)->get_selected()) {
		++result;
	    }
	}
	return result;
    }

}
