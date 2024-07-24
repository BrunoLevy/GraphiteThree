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

#include <OGF/scene_graph/tools/tools_manager.h>
#include <OGF/scene_graph/tools/grob_pan.h>
#include <OGF/scene_graph/tools/grob_select.h>
#include <OGF/scene_graph/types/scene_graph_tools_manager.h>
#include <OGF/scene_graph/grob/grob.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/types/connection.h>
#include <OGF/gom/interpreter/interpreter.h>

#include <geogram/basic/environment.h>

namespace OGF {

    ToolsManager::ToolsManager(
        SceneGraphToolsManager* manager,
        const std::string& grob_class_name, 
        RenderingContext* context
    ) : manager_(manager),
        grob_class_name_(grob_class_name), 
        rendering_context_(context), 
        grob_(nullptr)
    {
        current_tool_ = nullptr ;
        set_tool("OGF::GrobPan") ;
    }
    
    ToolsManager::~ToolsManager() {
    }

    void ToolsManager::set_grob(Grob* grob) {
        grob_ = grob ;
        if(current_tool_ != nullptr) {
            current_tool_->reset() ;
            if(current_tool_->meta_class()->has_custom_attribute("message")) {
                status(
                    current_tool_->meta_class()->custom_attribute_value(
                        "message"
                    )
                );
            }
        }
    }

    void ToolsManager::activate(Grob* grob) {
        set_grob(grob) ;
    }
    
    void ToolsManager::deactivate() {
        grob_ = nullptr ;
        status(" ") ;
    }

    Tool* ToolsManager::resolve_tool(const std::string& tool_class_name) {
        auto it = tools_.find(tool_class_name) ;
        if(it == tools_.end()) {
            ArgList args ;
	    args.create_arg("parent",this);
            Tool* tool = dynamic_cast<Tool*>(
                manager()->scene_graph()->interpreter()->create(
                    tool_class_name, args
                ) 
            );
	    if(tool == nullptr) {
		Logger::err("Tool")
		    << tool_class_name << " No such tool class name"
		    << std::endl;
		return resolve_tool("OGF::GrobPan");
	    }
            ogf_assert(tool != nullptr) ;
            tools_[tool_class_name] = tool ;
            it = tools_.find(tool_class_name) ;
            ogf_assert(it != tools_.end()) ;
        }
        return it->second ;
    }

    void ToolsManager::set_tool(const std::string& value) {
        current_tool_ = resolve_tool(value) ;
        current_tool_->reset() ;
        if(current_tool_->meta_class()->has_custom_attribute("message")) {
            status(
                current_tool_->meta_class()->custom_attribute_value("message")
            ) ;
        }
        if(current_tool_->meta_class()->has_custom_attribute("icon")) {
            manager()->update_tool_icon(
                "tools/" +
                current_tool_->meta_class()->custom_attribute_value("icon")
            ) ;
        }
    }

    void ToolsManager::configure_tool(const std::string& value) {
        current_tool_ = resolve_tool(value) ;
        current_tool_->reset() ;
        current_tool_->configure() ; 
    }

    void ToolsManager::status(const std::string& value) { 
        manager_->status(value) ; 
    }

    void ToolsManager::grab(const RayPick& ray) {
        if(current_tool_ != nullptr) {
            current_tool_->grab(ray) ;
        }
    }
    
    void ToolsManager::drag(const RayPick& ray) {
        if(current_tool_ != nullptr) {
            current_tool_->drag(ray) ;
        }
    }
    
    void ToolsManager::release(const RayPick& ray) {
        if(current_tool_ != nullptr) {
            current_tool_->release(ray) ;
            if(
                dynamic_cast<GrobPan*>(current_tool_) == nullptr 
            ) {
                current_tool_->object()->update();
            }
        }
    }

    RenderingContext* ToolsManager::rendering_context() const {
	if(rendering_context_ == nullptr) {
	    rendering_context_ = manager_->rendering_context();
	}
	return rendering_context_;
    }
}
