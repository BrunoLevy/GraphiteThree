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

#include <OGF/scene_graph/tools/tool.h>
#include <OGF/scene_graph/tools/tools_manager.h>
#include <OGF/scene_graph/types/scene_graph_tools_manager.h>

namespace OGF {

    //_________________________________________________________________________

    SceneGraph* Tool::scene_graph() {
	return tools_manager_->manager()->scene_graph();
    }
    
    void Tool::grab(const RayPick&) {
    }
    
    void Tool::drag(const RayPick&) {
    }
    
    void Tool::release(const RayPick&) {
    }

    void Tool::reset() {
    }

    void Tool::configure() {
    }

    RenderingContext* Tool::rendering_context() const {
        return tools_manager_->rendering_context() ;
    }
    
    Grob* Tool::object() const {
        return tools_manager_->object() ;
    }
    
    const mat4& Tool::focus() const {
        return tools_manager_->manager()->get_focus() ;
    }

    void Tool::status(const std::string& value) { 
        tools_manager_->status(value) ; 
    }

    /*******************************************************/

    MultiTool::MultiTool(ToolsManager* mgr) : Tool(mgr) {
    }
    
    void MultiTool::grab(const RayPick& ev) {
	if(ev.button < 1 || ev.button > 3) {
	    return;
	}
        if(!tools_[ev.button - 1].is_null()) {
            tools_[ev.button - 1]->grab(ev) ;
        }
    }
    
    void MultiTool::drag(const RayPick& ev) {
	if(ev.button < 1 || ev.button > 3) {
	    return;
	}
        if(!tools_[ev.button - 1].is_null()) {
            tools_[ev.button - 1]->drag(ev) ;
        }
    }
    
    void MultiTool::release(const RayPick& ev) {
	if(ev.button < 1 || ev.button > 3) {
	    return;
	}
        if(!tools_[ev.button - 1].is_null()) {
            tools_[ev.button - 1]->release(ev) ;
        }
    }
    
    void MultiTool::reset() {
        for(int i=0; i<3; i++) {
            if(!tools_[i].is_null()) {
                tools_[i]->reset() ;
            }
        }
    }

    void MultiTool::set_tool(int button, Tool* tool) {
        ogf_assert(button >= 1 && button <= 3) ;
        tools_[button - 1] = tool ;
    }

    Tool* MultiTool::get_tool(int button) const {
        ogf_assert(button >= 1 && button <= 3) ;
        return tools_[button - 1] ;
    }

    //_______________________________________________________________________

}
