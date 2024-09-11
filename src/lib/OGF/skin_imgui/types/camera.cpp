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

#include <OGF/skin_imgui/types/camera.h>
#include <OGF/skin_imgui/types/application.h>
#include <OGF/skin_imgui/widgets/render_area.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph_gfx/shaders/scene_graph_shader_manager.h>
#include <OGF/gom/interpreter/interpreter.h>

namespace OGF {

    Camera::Camera(Application* app) : application_(app) {
    }

    Camera::~Camera() {
    }

    SceneGraphShaderManager* Camera::scene_graph_shader_manager() const {
	return dynamic_cast<SceneGraphShaderManager*>(
	    SceneGraphLibrary::instance()->scene_graph()->get_scene_graph_shader_manager()
	);
    }

    void Camera::set_auto_focus(bool value) {
	scene_graph_shader_manager()->set_auto_focus(value);
	application_->get_render_area()->update();
    }

    bool Camera::get_auto_focus() const {
	return scene_graph_shader_manager()->get_auto_focus();
    }

    void Camera::set_draw_selected_only(bool value) {
	scene_graph_shader_manager()->set_draw_selected_only(value);
	application_->get_render_area()->update();
    }

    bool Camera::get_draw_selected_only() const {
	return scene_graph_shader_manager()->get_draw_selected_only();
    }

    void Camera::set_effect(const FullScreenEffectName& effect) {
	scene_graph_shader_manager()->set_effect(effect);
	application_->get_render_area()->update();
    }

    const FullScreenEffectName& Camera::get_effect() const {
	return scene_graph_shader_manager()->get_effect();
    }

    FullScreenEffect* Camera::get_effect_object() const {
	return scene_graph_shader_manager()->current_effect();
    }

    void Camera::set_bkg_color_1(const Color& value) {
	application_->get_render_area()->set_background_color_1(value);
    }

    const Color& Camera::get_bkg_color_1() const {
	return application_->get_render_area()->get_background_color_1();
    }

    void Camera::set_bkg_color_2(const Color& value) {
	application_->get_render_area()->set_background_color_2(value);
    }

    const Color& Camera::get_bkg_color_2() const {
	return application_->get_render_area()->get_background_color_2();
    }

    void Camera::set_clipping(ClippingConfig value) {
	application_->get_render_area()->set_clipping_config(value);
    }

    ClippingConfig Camera::get_clipping() const {
	return application_->get_render_area()->get_clipping_config();
    }

    mat4 Camera::get_lighting_matrix() const {
	return application_->get_render_area()->get_lighting_matrix();
    }

    void Camera::set_lighting_matrix(const mat4& value) {
	application_->get_render_area()->set_lighting_matrix(value);
    }
}
