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
	geo_argused(grob_name);
    }

    void SceneGraphEditor::delete_object(const GrobName& grob_name) {
	geo_argused(grob_name);
    }

    void SceneGraphEditor::delete_current() {
    }

    void SceneGraphEditor::delete_selected() {
    }

    void SceneGraphEditor::delete_all() {
    }

    void SceneGraphEditor::show(const GrobName& grob_name) {
	geo_argused(grob_name);
    }

    void SceneGraphEditor::show_selected() {
    }

    void SceneGraphEditor::show_all() {
    }

    void SceneGraphEditor::hide(const GrobName& grob_name) {
	geo_argused(grob_name);
    }

    void SceneGraphEditor::hide_selected() {
    }

    void SceneGraphEditor::hide_all() {
    }

    void SceneGraphEditor::move_up(const GrobName& grob_name) {
	geo_argused(grob_name);
    }

    void SceneGraphEditor::move_down(const GrobName& grob_name) {
	geo_argused(grob_name);
    }

    void SceneGraphEditor::rename(
	const GrobName& grob_name, const std::string& new_name
    ) {
	geo_argused(grob_name);
	geo_argused(new_name);
    }

    Grob* SceneGraphEditor::duplicate(const GrobName& grob_name) {
	geo_argused(grob_name);
	return nullptr;
    }

    Grob* SceneGraphEditor::create_object(
	const GrobClassName& class_name, const std::string& name
    ) {
	geo_argused(class_name);
	geo_argused(name);
	return nullptr;
    }

    void SceneGraphEditor::copy_graphic_properties_to_all(
	const GrobName& grob_name
    ) {
	geo_argused(grob_name);
    }

    void SceneGraphEditor::copy_graphic_properties_to_visible(
	const GrobName& grob_name
    ) {
	geo_argused(grob_name);
    }

    void SceneGraphEditor::copy_graphic_properties_to_selected(
	const GrobName& grob_name
    ) {
	geo_argused(grob_name);
    }


}
