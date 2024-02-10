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
#include <OGF/skin/types/application_base.h>

namespace OGF {

    //_________________________________________________________________________

    SceneGraph* Tool::scene_graph() {
	return tools_manager_->manager()->scene_graph();
    }
    
    void Tool::grab(const RayPick&) {
        ApplicationBase::instance()->save_state();
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

    vec2 Tool::project_point(vec3 p) const {
        Shader* shd         = object()->get_shader();
        GLdouble* modelview = shd->latest_modelview();
        GLdouble* project   = shd->latest_project();
        GLint*    viewport  = shd->latest_viewport();
        double X,Y,Z;
        glupProject(
            p.x, p.y, p.z,
            modelview,
            project,
            viewport,
            &X, &Y, &Z
        );

        // Once again, Y axis is flipped... 
        double y0 = double(viewport[1]);
        double h  = double(viewport[3]);
        Y -= y0;
        Y  = h-1-Y;
        Y += y0;
        
        return vec2(X,Y);
    }

    vec2 Tool::ndc_to_dc(vec2 p) const {
        Shader* shd = object()->get_shader();
        GLint* viewport = shd->latest_viewport();
        double x = 0.5*(p.x+1.0);
        double y = 0.5*(p.y+1.0);
        double x0 = double(viewport[0]);
        double y0 = double(viewport[1]);
        double w  = double(viewport[2]);
        double h  = double(viewport[3]);
        // Here, Y axis is not flipped (???)
        // (check convention for GLFW events)
        return vec2(
            x * w + x0,
            y * h + y0
        );
    }
    
    const mat4& Tool::focus() const {
        return tools_manager_->manager()->get_focus() ;
    }

    void Tool::status(const std::string& value) { 
        tools_manager_->status(value) ; 
    }

    void Tool::set_tooltip(const std::string& text) {
        // get Application through high level gom object API to avoid creating
        // a SceneGraph -> GUI dependency
        Object* main = scene_graph()->interpreter()->resolve_object("main");
        if(main != nullptr) {
            main->set_property("tooltip", text);
        }
    }

    void Tool::reset_tooltip() {
        // get Application through high level gom object API to avoid creating
        // a SceneGraph -> GUI dependency
        Object* main = scene_graph()->interpreter()->resolve_object("main");
        if(main != nullptr) {
            main->set_property("tooltip", "");
        }
    }
    
    /*******************************************************/

    MultiTool::MultiTool(ToolsManager* mgr) : Tool(mgr) {
    }
    
    void MultiTool::grab(const RayPick& ev) {
	if(ev.button >= MOUSE_BUTTONS_NB) {
	    return;
	}
        if(!tools_[ev.button].is_null()) {
            tools_[ev.button]->grab(ev) ;
        }
    }
    
    void MultiTool::drag(const RayPick& ev) {
	if(ev.button >= MOUSE_BUTTONS_NB) {
	    return;
	}
        if(!tools_[ev.button].is_null()) {
            tools_[ev.button]->drag(ev) ;
        }
    }
    
    void MultiTool::release(const RayPick& ev) {
	if(ev.button >= MOUSE_BUTTONS_NB) {
	    return;
	}
        if(!tools_[ev.button].is_null()) {
            tools_[ev.button]->release(ev) ;
        }
    }
    
    void MultiTool::reset() {
        for(int i=0; i<MOUSE_BUTTONS_NB; ++i) {
            if(!tools_[i].is_null()) {
                tools_[i]->reset() ;
            }
        }
    }

    void MultiTool::set_tool(int button, Tool* tool) {
        geo_assert(button < MOUSE_BUTTONS_NB);
        tools_[button] = tool ;
    }

    Tool* MultiTool::get_tool(int button) const {
        geo_assert(button < MOUSE_BUTTONS_NB);        
        return tools_[button] ;
    }

    //_______________________________________________________________________

}
