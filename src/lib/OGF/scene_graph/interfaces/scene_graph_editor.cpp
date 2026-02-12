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

#include <OGF/scene_graph/interfaces/scene_graph_editor.h>

namespace OGF {

    SceneGraphEditor::SceneGraphEditor() {
    }

    SceneGraphEditor::~SceneGraphEditor() {
    }

    void SceneGraphEditor::set_current(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->set_current_object(grob_name);
    }

    void SceneGraphEditor::delete_object(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	scene_graph()->delete_current_object();
    }

    void SceneGraphEditor::delete_current() {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->delete_current_object();
    }

    void SceneGraphEditor::delete_selected() {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	std::vector<std::string> to_delete;
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    Grob* grob = scene_graph()->ith_child(i);
	    if(grob != nullptr && grob->get_selected()) {
		to_delete.push_back(grob->name());
	    }
	}
	for(auto& name: to_delete) {
	    delete_object(name);
	}
    }

    void SceneGraphEditor::delete_all() {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->clear();
    }

    void SceneGraphEditor::show(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	Grob* grob = scene_graph()->resolve(grob_name);
	grob->set_visible(true);
    }

    void SceneGraphEditor::show_selected() {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    Grob* grob = scene_graph()->ith_child(i);
	    if(grob->get_selected()) {
		grob->set_visible(true);
	    }
	}
    }

    void SceneGraphEditor::show_all() {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    scene_graph()->ith_child(i)->set_visible(true);
	}
    }

    void SceneGraphEditor::hide(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	Grob* grob = scene_graph()->resolve(grob_name);
	grob->set_visible(false);
    }

    void SceneGraphEditor::hide_selected() {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    Grob* grob = scene_graph()->ith_child(i);
	    if(grob->get_selected()) {
		grob->set_visible(false);
	    }
	}
    }

    void SceneGraphEditor::hide_all() {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
	    scene_graph()->ith_child(i)->set_visible(false);
	}
    }

    void SceneGraphEditor::move_up(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	scene_graph()->move_current_up();
    }

    void SceneGraphEditor::move_down(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	scene_graph()->move_current_down();
    }

    void SceneGraphEditor::rename(
	const GrobName& grob_name, const std::string& new_name
    ) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	if(scene_graph()->current() != nullptr) {
	    scene_graph()->current()->rename(new_name);
	}
    }

    Grob* SceneGraphEditor::duplicate(const GrobName& grob_name) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return nullptr;
	}
	scene_graph()->set_current_object(grob_name);
	if(scene_graph()->current() != nullptr) {
	    return scene_graph()->duplicate_current();
	}
	return nullptr;
    }

    Grob* SceneGraphEditor::create_object(
	const GrobClassName& class_name, const std::string& name
    ) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return nullptr;
	}
        Grob* new_grob = scene_graph()->create_object(class_name);
        if(new_grob == nullptr) {
            Logger::err("SceneGraphEditor")
                << "Could not create object" << std::endl;
            return nullptr;
        }
        if(name.length() > 0) {
            new_grob->rename(name);
        }
        scene_graph()->set_current_object(new_grob->name());
	return new_grob;
    }

    void SceneGraphEditor::copy_graphic_properties_to_all(
	const GrobName& grob_name
    ) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	Object* shd_mgr = scene_graph()->get_scene_graph_shader_manager();
	if(scene_graph()->current() != nullptr && shd_mgr != nullptr) {
	    ArgList args;
	    args.create_arg("visible_only", false);
	    args.create_arg("selected_only", false);
	    shd_mgr->invoke_method("apply_to_scehe_graph",args);
	}
    }

    void SceneGraphEditor::copy_graphic_properties_to_visible(
	const GrobName& grob_name
    ) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	Object* shd_mgr = scene_graph()->get_scene_graph_shader_manager();
	if(scene_graph()->current() != nullptr && shd_mgr != nullptr) {
	    ArgList args;
	    args.create_arg("visible_only", true);
	    args.create_arg("selected_only", false);
	    shd_mgr->invoke_method("apply_to_scehe_graph",args);
	}
    }

    void SceneGraphEditor::copy_graphic_properties_to_selected(
	const GrobName& grob_name
    ) {
	if(scene_graph() == nullptr) {
	    Logger::err("SceneGraphEditor") << "No SceneGraph" << std::endl;
	    return;
	}
	scene_graph()->set_current_object(grob_name);
	Object* shd_mgr = scene_graph()->get_scene_graph_shader_manager();
	if(scene_graph()->current() != nullptr && shd_mgr != nullptr) {
	    ArgList args;
	    args.create_arg("visible_only", false);
	    args.create_arg("selected_only", true);
	    shd_mgr->invoke_method("apply_to_scehe_graph",args);
	}
    }

    /*************************************************/

    void SceneGraphEditor::backup_current() {
	current_bkp_ = scene_graph()->get_current_object();
    }

    void SceneGraphEditor::restore_current() {
	if(scene_graph()->is_bound(current_bkp_)) {
	    scene_graph()->set_current_object(current_bkp_);
	} else if(scene_graph()->get_nb_children() > 0) {
	    scene_graph()->set_current_object(
		scene_graph()->ith_child(0)->name()
	    );
	}
	current_bkp_ = "";
    }

}
