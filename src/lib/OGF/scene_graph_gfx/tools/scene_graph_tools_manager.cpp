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

#include <OGF/scene_graph_gfx/tools/scene_graph_tools_manager.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/basic/modules/modmgr.h>

#include <geogram_gfx/gui/application.h>

namespace OGF {

    SceneGraphToolsManager* SceneGraphToolsManager::instance_;

    SceneGraphToolsManager::SceneGraphToolsManager(
	Node* renderer
    ) : current_tools_manager_(nullptr),
        rendering_context_(nullptr) {

	ogf_assert(instance_== nullptr);
	instance_ = this;

        scene_graph_ = SceneGraphLibrary::instance()->scene_graph();
        ogf_assert(scene_graph_ != nullptr);

	renderer_ = renderer;

        // ogf_assert(renderer_ != nullptr);
        /* I removed this assertion to be able to create a Tools manager
         * without knowing the renderer_
         * As it is not often used only in grob_pan.cpp for "quite ugly"
         *  stuff so ...
         * I think it's not a big deal
         * Useful in the Gocad plugin for Graphite.
         * Asserts where renderer_ is needed are added.
         */

        scene_graph_->connect_signal_to_slot(
            "current_object_changed", this, "current_object"
        );

        focus_.load_identity();

        SceneGraphLibrary::instance()->set_scene_graph_tools_manager(this);
    }

    SceneGraphToolsManager::~SceneGraphToolsManager() {
       ogf_assert(instance_ == this);
       instance_ = nullptr;
    }


    /* To be able to set the rendering context after the creation of
     * the Tools manager
     * Used when the constructor without the renderer_id is used to build
     * the object
     */
    void SceneGraphToolsManager::set_rendering_context(
        OGF::RenderingContext* rendering_context
    ) {
        rendering_context_ = rendering_context;
    }


    RenderingContext* SceneGraphToolsManager::rendering_context() {
        if( rendering_context_ != nullptr ) {
            return rendering_context_;
        }
        // else the behaviour of the function is not changed
        std::string result_str;
        ogf_assert( renderer_ != nullptr );
        bool has_rendering_context = renderer_->get_property(
            "rendering_context", result_str
        );
        ogf_assert(has_rendering_context);
        RenderingContext* result;
        ogf_convert_from_string(result_str, result);
        return result;
    }

    void SceneGraphToolsManager::update_tool_icon(const std::string& icon) {
        tool_icon_changed(icon);
    }

    void SceneGraphToolsManager::focus(const mat4& value) {
        focus_ = value;
    }

    void SceneGraphToolsManager::tool(const std::string& value) {
        Object* main = scene_graph_->get_application();
	if(
	    main != nullptr &&
	    main->meta_class()->find_property("picked_grob") != nullptr
	) {
	    Any nullptr_any;
	    main->set_property("picked_grob", nullptr_any);
	}
        if(current_tools_manager_ != nullptr) {
            current_tools_manager_->set_tool(value);
        }
    }

    Tool* SceneGraphToolsManager::current() const {
        if(current_tools_manager_ == nullptr) {
            return nullptr;
        }
        return current_tools_manager_->current_tool();
    }

    ToolsManager* SceneGraphToolsManager::current_tools_manager() const {
        return current_tools_manager_;
    }


    void SceneGraphToolsManager::current_object(
        const std::string& name
    ) {

        std::string last_tool_name;

        if(name.length() == 0) {
            if(current_tools_manager_ != nullptr) {
                Tool* cur_tool = current_tools_manager_->current_tool();
                if(
                    cur_tool != nullptr &&
                    cur_tool->meta_class()->has_custom_attribute("category") &&
                    cur_tool->meta_class()->custom_attribute_value("category")
                    == "viewer"
                ) {
                    last_tool_name = cur_tool->meta_class()->name();
                }
                current_tools_manager_->deactivate();
                current_tools_manager_ = nullptr;
            }
            return;
        }

        Grob* current_object = scene_graph_->resolve(name);
        ogf_assert(current_object != nullptr);

        std::string new_grob_class_name = current_object->meta_class()->name();
        ToolsManager* new_tools_manager = resolve_tools_manager(
            new_grob_class_name
        );
        ogf_assert(new_tools_manager != nullptr);

        if(current_tools_manager_ == new_tools_manager) {
            if(current_tools_manager_ != nullptr) {
                current_tools_manager_->set_grob(current_object);
            }
        } else {
            if(current_tools_manager_ != nullptr) {
                current_tools_manager_->deactivate();
            }
            current_tools_manager_ = new_tools_manager;
            current_tools_manager_->activate(current_object);

            if(last_tool_name.length() != 0) {
                current_tools_manager_->set_tool(last_tool_name);
            } else {
                Tool* cur_tool = current_tools_manager_->current_tool();
                if(
                    cur_tool != nullptr &&
                    cur_tool->meta_class()->has_custom_attribute("icon")
                ) {
                    tool_icon_changed(
                        "tools/" +
                        cur_tool->meta_class()->custom_attribute_value("icon")
                    );
                }
            }
        }
    }


    ToolsManager* SceneGraphToolsManager::resolve_tools_manager(
        const std::string& grob_class_name
    ) {
        auto it = tools_.find(grob_class_name);
        if(it == tools_.end()) {
            RenderingContext* context = rendering_context();
            tools_[grob_class_name] = new ToolsManager(
                this,
                grob_class_name,
                context
            );
        }
        it = tools_.find(grob_class_name);
        ogf_assert(it != tools_.end());
        return it->second;
    }


    void SceneGraphToolsManager::grab(const RayPick& ev) {
	Application::instance()->lock_updates();
        if(current_tools_manager_ != nullptr) {
            current_tools_manager_->grab(ev);
        }
	Application::instance()->unlock_updates();
    }

    void SceneGraphToolsManager::drag(const RayPick& ev) {
        if(current_tools_manager_ != nullptr) {
            current_tools_manager_->drag(ev);
        }
    }

    void SceneGraphToolsManager::release(const RayPick& ev) {
	Application::instance()->lock_updates();
        if(current_tools_manager_ != nullptr) {
            current_tools_manager_->release(ev);
        }
	Application::instance()->unlock_updates();
    }
}
