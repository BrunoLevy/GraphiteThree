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

#include <OGF/scene_graph/interfaces/scene_graph_graphics.h>
#include <OGF/scene_graph/types/scene_graph_library.h>

namespace OGF {

    SceneGraphGraphicsInterface::SceneGraphGraphicsInterface() {
    }

    SceneGraphGraphicsInterface::~SceneGraphGraphicsInterface() {
    }

    void SceneGraphGraphicsInterface::show_object(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    return;
	}
	Grob* grob = scene_graph()->resolve(grob_name);
	if(grob != nullptr) {
	    grob->set_visible(true);
	}
    }

    void SceneGraphGraphicsInterface::show_only(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    Grob* grob = scene_graph()->ith_child(i);
	    grob->set_visible(grob->name() == std::string(grob_name));
	}
    }

    void SceneGraphGraphicsInterface::show_selected() {
	if(scene_graph() == nullptr) {
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    Grob* grob = scene_graph()->ith_child(i);
	    if(grob->get_selected()) {
		grob->set_visible(true);
	    }
	}
    }

    void SceneGraphGraphicsInterface::show_all() {
	if(scene_graph() == nullptr) {
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    scene_graph()->ith_child(i)->set_visible(true);
	}
    }

    void SceneGraphGraphicsInterface::hide_object(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    return;
	}
	Grob* grob = scene_graph()->resolve(grob_name);
	if(grob != nullptr) {
	    grob->set_visible(false);
	}
    }

    void SceneGraphGraphicsInterface::hide_selected() {
	if(scene_graph() == nullptr) {
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    Grob* grob = scene_graph()->ith_child(i);
	    if(grob->get_selected()) {
		grob->set_visible(false);
	    }
	}
    }

    void SceneGraphGraphicsInterface::hide_all() {
	if(scene_graph() == nullptr) {
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    scene_graph()->ith_child(i)->set_visible(false);
	}
    }


    void SceneGraphGraphicsInterface::copy_object_properties_to_all(
	const GrobName& grob_name
    ) {
	if(scene_graph() == nullptr) {
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	Object* shd_mgr = scene_graph()->get_scene_graph_shader_manager();
	if(scene_graph()->current() != nullptr && shd_mgr != nullptr) {
	    ArgList args;
	    args.create_arg("visible_only", false);
	    args.create_arg("selected_only", false);
	    shd_mgr->invoke_method("apply_to_scene_graph",args);
	}
    }

    void SceneGraphGraphicsInterface::copy_object_properties_to_visible(
	const GrobName& grob_name
    ) {
	if(scene_graph() == nullptr) {
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	Object* shd_mgr = scene_graph()->get_scene_graph_shader_manager();
	if(scene_graph()->current() != nullptr && shd_mgr != nullptr) {
	    ArgList args;
	    args.create_arg("visible_only", true);
	    args.create_arg("selected_only", false);
	    shd_mgr->invoke_method("apply_to_scene_graph",args);
	}
    }

    void SceneGraphGraphicsInterface::copy_object_properties_to_selected(
	const GrobName& grob_name
    ) {
	if(scene_graph() == nullptr) {
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	Object* shd_mgr = scene_graph()->get_scene_graph_shader_manager();
	if(scene_graph()->current() != nullptr && shd_mgr != nullptr) {
	    ArgList args;
	    args.create_arg("visible_only", false);
	    args.create_arg("selected_only", true);
	    shd_mgr->invoke_method("apply_to_scene_graph",args);
	}
    }

    void SceneGraphGraphicsInterface::set_object_shader(
	const GrobName& grobname, const std::string& shader_name
    ) {
	if(scene_graph() == nullptr) {
	    return;
	}
	Grob* grob = scene_graph()->resolve(grobname);
	if(grob == nullptr) {
	    return;
	}
        std::string shader_classname =
            SceneGraphLibrary::instance()->shader_user_to_classname(
                grob->meta_class()->name(), shader_name
            );
	ArgList properties;
	grob->set_shader_and_shader_properties(shader_classname, properties);
    }

}
