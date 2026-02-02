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

#include <OGF/scene_graph_gfx/shaders/scene_graph_shader_manager.h>
#include <OGF/scene_graph_gfx/shaders/shader_manager.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/skin/application_base.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/basic/modules/modmgr.h>

#include <geogram_gfx/basic/GLSL.h>
#include <geogram/basic/stopwatch.h>

namespace OGF {

    SceneGraphShaderManager::SceneGraphShaderManager()
	: current_object_(nullptr) {

        scene_graph_ = SceneGraphLibrary::instance()->scene_graph();
        ogf_assert(scene_graph_ != nullptr);

        current_object_ = nullptr;

        scene_graph_->connect_signal_to_slot(
            "current_object_changed", this, "current_object"
        );

        auto_focus_ = false;
        focus_.load_identity();
        draw_selected_only_ = false;
        highlight_selected_ = false;

	FullScreenEffect* default_FSE = new PlainFullScreenEffect(scene_graph_);
	effects_["OGF::PlainFullScreenEffect"] = default_FSE;
        effect_ = default_FSE;

        SceneGraphLibrary::instance()->set_scene_graph_shader_manager(this);
    }

    SceneGraphShaderManager::~SceneGraphShaderManager() {
    }

    void SceneGraphShaderManager::shader(const std::string& shader_name_in) {

        if(current_object_ == nullptr || shader_name_in.length() == 0) {
            return;
        }

        std::string shader_name =
            SceneGraphLibrary::instance()->shader_user_to_classname(
                current_object_->meta_class()->name(),
                shader_name_in
            );

        ShaderManager* shader_mgr = resolve_shader_manager(current_object_);
        if(shader_mgr == nullptr) {
            return;
        }

        shader_mgr->set_shader(shader_name);
        last_shader_ = shader_name;
    }

    void SceneGraphShaderManager::full_screen_effect(
        const std::string& full_screen_effect_name_in
    ) {
	user_effect_name_ = full_screen_effect_name_in;

        const std::string& full_screen_effect_name =
            SceneGraphLibrary::instance()->
            full_screen_effect_user_to_classname(full_screen_effect_name_in);

        FullScreenEffect_var& effect = effects_[
            full_screen_effect_name
        ];

        if(effect.is_null()) {
            ArgList args;
	    args.create_arg("scene_graph", scene_graph_);
            effect = dynamic_cast<FullScreenEffect*>(
                interpreter()->create(
                    full_screen_effect_name, args
                    )
                );
	    if(effect.is_null()) {
		Logger::warn("Effects")
		    << full_screen_effect_name << ": no such effect"
		    << std::endl;
		effect = dynamic_cast<FullScreenEffect*>(
		    interpreter()->create(
			"OGF::PlainFullScreenEffect", args
                    )
                );
		user_effect_name_ = "Plain";
	    }
        }

        effect_ = effect;

        if(RenderingContext::current()!= nullptr) {
	    RenderingContext::current()->set_full_screen_effect(
                effect->implementation()
            );
        }

	effect->update();
    }

    void SceneGraphShaderManager::set_highlight_selected(bool value) {
       highlight_selected_ = value;
       scene_graph_->update();
    }

    void SceneGraphShaderManager::set_draw_selected_only(bool value) {
       draw_selected_only_ = value;
       scene_graph_->update();
    }

    void SceneGraphShaderManager::set_effect(
	const FullScreenEffectName& effect
    ) {
	full_screen_effect(effect);
    }

    const FullScreenEffectName& SceneGraphShaderManager::get_effect() const {
	return user_effect_name_;
    }


    void SceneGraphShaderManager::current_object(
        const std::string& name
    ) {
        if(name.length() == 0) {
            current_object_ = nullptr;
            last_shader_ = "";
            return;
        }

        current_object_ = scene_graph_->resolve(name);
        ogf_assert(current_object_ != nullptr);

        bool ok = false;
        std::string shader_names;
        ok = SceneGraphLibrary::instance()->get_value(
            current_object_->meta_class()->name() + "_shaders", shader_names
        );
        ogf_assert(ok);

        ShaderManager* shader_mgr = resolve_shader_manager(current_object_);

        if(auto_focus_) {
            update_focus();
        }

        if(highlight_selected_ && shader_mgr->current_shader() != nullptr) {
            shader_mgr->current_shader()->blink();
	    // TODO... wait, update camera
            shader_mgr->current_shader()->blink();
        } else {
            if(draw_selected_only_ || auto_focus_) {
                scene_graph_->update();
            }
        }
    }

    ShaderManager* SceneGraphShaderManager::resolve_shader_manager(Grob* grob) {
        ogf_assert(grob != nullptr);
        ShaderManager* result = dynamic_cast<ShaderManager*>(
            grob->get_shader_manager()
        );
        if(result == nullptr) {
            result = new ShaderManager(grob, this);
            grob->set_shader_manager(result);
        }
        ogf_assert(result != nullptr);

        if(result->current_shader() == nullptr) {
            grob->scene_graph()->disable_signals();
            result->set_shader(
                SceneGraphLibrary::instance()->shader_user_to_classname(
                    grob->meta_class()->name(),
                    "Plain"
                )
            );
            grob->scene_graph()->enable_signals();
        }
        return result;
    }

    Shader* SceneGraphShaderManager::current() {
        Grob* current_grob = scene_graph_->current();
        if(current_grob == nullptr) {
            return nullptr;
        }
        ShaderManager* mgr = resolve_shader_manager(current_grob);
        return mgr->current_shader();
    }

    FullScreenEffect* SceneGraphShaderManager::current_effect() {
	return effect_;
    }


    void SceneGraphShaderManager::update_focus() {

        Grob* grob_focus = nullptr;
        if(auto_focus_ && current_object_ != nullptr) {
            grob_focus = current_object_;
        } else {
            grob_focus = scene_graph_;
        }
        Box3d box = grob_focus->world_bbox();
        if(!box.initialized()) {
            box.add_point(vec3(0,0,0));
            box.add_point(vec3(1,1,1));
        }

        vec3 center = box.center();
        double radius  = box.radius();

        double s = 1.0;
        if(radius != 0.0) {
            s = 1.0 / radius;
        }

        focus_.load_identity();
        focus_(0,0) = s;
        focus_(1,1) = s;
        focus_(2,2) = s;
        focus_(3,0) = -s * center.x;
        focus_(3,1) = -s * center.y;
        focus_(3,2) = -s * center.z;
        focus_changed(focus_);
    }

    void SceneGraphShaderManager::draw() {
	//  Some shaders may trigger new events, thus preventing the
	// code that purges the event loop from exiting.
	if(ApplicationBase::is_stopping()) {
	    return;
	}

        //   If a full screen effect was just set and we did
        // not know the rendering context already, then
        // memorize it, fire another update event and exit,
        // so that rendering will be performed at the next
        // update with everything setup correctly.
        if(
            effect_ != nullptr &&
            RenderingContext::current()->get_full_screen_effect() !=
            effect_->implementation()
        ) {
	    RenderingContext::current()->set_full_screen_effect(
                effect_->implementation()
            );
	    if(scene_graph_->get_render_area() != nullptr) {
		scene_graph_->get_render_area()->invoke_method("update");
	    }
            return;
        }


	if(!scene_graph_->get_visible()) {
	    return;
	}

        glupMatrixMode(GLUP_MODELVIEW_MATRIX);
        glupPushMatrix();
        glupMultMatrix(focus_);

        if(draw_selected_only_) {
            if(current_object_ != nullptr) {
                glupPushMatrix();
                glupMultMatrix(
                    current_object_->get_obj_to_world_transform()
                );

                ShaderManager* shader_mgr =
                    resolve_shader_manager(current_object_);

                if(shader_mgr != nullptr) {
                    shader_mgr->draw();
                }

                glupPopMatrix();
            }
        } else {
            for(index_t i=0; i<scene_graph_->get_nb_children(); i++) {
                Grob* cur = scene_graph_->ith_child(i);
                if(cur != nullptr && (cur->get_visible())) {

                    glupPushMatrix();
                    glupMultMatrix(cur->get_obj_to_world_transform());

                    ShaderManager* shader_mgr = resolve_shader_manager(cur);
                    if(shader_mgr != nullptr) {
                        shader_mgr->draw();
                    }
                    glupPopMatrix();
                }
            }
        }
        glupPopMatrix();
    }

    void SceneGraphShaderManager::pick_object() {
        if(
	    RenderingContext::current() == nullptr ||
	    !RenderingContext::current()->initialized()
	) {
            return;
        }

	if(scene_graph_->current() == nullptr) {
	    std::cerr << "NO CURRENT" << std::endl;
	}

        glupMatrixMode(GLUP_MODELVIEW_MATRIX);
        glupPushMatrix();
        glupMultMatrix(focus_);

	for(index_t i=0; i<scene_graph_->get_nb_children(); i++) {
	    Grob* cur = scene_graph_->ith_child(i);
	    if(draw_selected_only_ && cur != scene_graph_->current()) {
		continue;
	    }
	    if(cur != nullptr && cur->get_visible()) {
		glupPushMatrix();
		glupMultMatrix(cur->get_obj_to_world_transform());
		Shader* shader =  dynamic_cast<Shader*>(cur->get_shader());
		if(shader != nullptr) {
		    shader->pick_object(i);
		}
		glupPopMatrix();
	    }
	}

        glupPopMatrix();
    }

    void SceneGraphShaderManager::get_grob_shader(
        Grob* grob, std::string& classname, ArgList& args, bool pointers
    ) {
        classname = "";
        args.clear();
        ShaderManager* mgr = resolve_shader_manager(grob);
        if(mgr == nullptr) {
            return;
        }
        Shader* shader = mgr->current_shader();
        if(shader == nullptr) {
            return;
        }
        classname = shader->meta_class()->name();
        std::vector<MetaProperty*> meta_props;
        shader->meta_class()->get_properties(meta_props);
        for(unsigned int i=0; i<meta_props.size(); i++) {
            MetaProperty* mprop = meta_props[i];
            const std::string& mprop_type =
                mprop->container_meta_class()->name();
            if(
                mprop->read_only() ||
                mprop_type == "Object" ||
                mprop_type == "Node"
            ) {
                continue;
            }
            if(!pointers && mprop_type[mprop_type.length()-1] == '*') {
                continue;
            }
            std::string name = mprop->name();
            std::string value;
            shader->get_property(name, value);
            args.create_arg(name, value);
        }
    }

    void SceneGraphShaderManager::set_grob_shader(
        Grob* grob, const std::string& classname, const ArgList& args
    ) {
        ShaderManager* mgr = resolve_shader_manager(grob);
        if(mgr == nullptr) {
	    return;
	}
        mgr->set_shader(classname);
        Shader* shader = mgr->current_shader();
        if(shader != nullptr) {
            shader->disable_signals();
            shader->set_properties(args);
            shader->enable_signals();
        }
    }

    Interpreter* SceneGraphShaderManager::interpreter() {
       return scene_graph_->interpreter();
    }

    void SceneGraphShaderManager::apply_to_scene_graph(
	bool visible_only, bool selected_only
    ) {
        if(current_object_ == nullptr) {
            return;
        }

        std::string shader_class_name;
        ArgList shader_properties;
        get_grob_shader(current_object_, shader_class_name, shader_properties);

        scene_graph_->disable_signals();

        for(index_t i=0; i<scene_graph_->get_nb_children(); i++) {
            Grob* cur_grob = scene_graph_->ith_child(i);
            if(
                (cur_grob == current_object_) ||
                (cur_grob->meta_class() != current_object_->meta_class()) ||
                (visible_only && !cur_grob->get_visible()) ||
		(selected_only && !cur_grob->get_selected())
            ) {
                continue;
            }
            set_grob_shader(cur_grob, shader_class_name, shader_properties);
        }

        scene_graph_->enable_signals();
        scene_graph_->update();
    }

}
