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
 *   (non-GPL) libraries:  Qt, SuperLU, WildMagic and CGAL
 */
 

#include <OGF/luagrob/shaders/lua_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>

namespace OGF {

    LuaGrobShader::LuaGrobShader(
        LuaGrob* grob
    ) : Shader(grob) {
       no_grob_update_ = true;
    }
        
    LuaGrobShader::~LuaGrobShader() { 
    }
    
    void LuaGrobShader::blink() {
    }

    void LuaGrobShader::draw() {
    }

    void LuaGrobShader::pick_object(index_t object_id) {
        geo_argused(object_id);
    }

    /*********************************************************************/
    
    PlainLuaGrobShader::PlainLuaGrobShader(
        LuaGrob* grob
    ) :
        LuaGrobShader(grob)
    {
        picking_ = false;
        clipping_ = true;
        lighting_ = true;

        box_style_.visible = true;
        box_style_.color   = Color(0.0,0.0,0.0,1.0);
        box_style_.width   = 2;
    }
    
    PlainLuaGrobShader::~PlainLuaGrobShader() {
    }

    void PlainLuaGrobShader::draw() {
        GLUPboolean clipping_backup = glupIsEnabled(GLUP_CLIPPING);
        
        if(!clipping_) {
            glupDisable(GLUP_CLIPPING);
            glDisable(GL_CLIP_PLANE0);
        }
        
        if(lua_grob()->graphics_are_locked()) {
            return;
        }

        if(lua_grob()->dirty()) {
            lua_grob()->up_to_date();
        }
        
        
        if(box_style_.visible) {
            draw_wireframe_box();
        }

	if(lua_grob()->shader_OK()) {
	    lua_grob()->execute_shader_command("draw()");
	}
	
        if(!clipping_ && clipping_backup) {
            glupEnable(GLUP_CLIPPING);
            glEnable(GL_CLIP_PLANE0);
        }
    }

    void PlainLuaGrobShader::draw_wireframe_box() {
	const Box3d& box = lua_grob()->bbox();

	double x1 = box.x_min();
	double y1 = box.y_min();
	double z1 = box.z_min();

	double x2 = box.x_max();
	double y2 = box.y_max();
	double z2 = box.z_max();	
	
	
        vec3 p000(x1,y1,z1);
        vec3 p100(x2,y1,z1);
        vec3 p010(x1,y2,z1);
        vec3 p001(x1,y1,z2);
        vec3 p011(x1,y2,z2);
        vec3 p101(x2,y1,z2);
        vec3 p110(x2,y2,z1);
        vec3 p111(x2,y2,z2);
        
        glupDisable(GLUP_VERTEX_COLORS);
        glupDisable(GLUP_TEXTURING);
        glupSetColor4dv(GLUP_MESH_COLOR, box_style_.color.data());
        glupSetMeshWidth(GLUPint(box_style_.width));
            
        glupBegin(GLUP_LINES);
        glupVertex(p000);
        glupVertex(p100);
        glupVertex(p100);
        glupVertex(p110);
        glupVertex(p110);
        glupVertex(p010);
        glupVertex(p010);            
        glupVertex(p000);
        glupVertex(p001);
        glupVertex(p101);
        glupVertex(p101);
        glupVertex(p111);
        glupVertex(p111);
        glupVertex(p011);
        glupVertex(p011);            
        glupVertex(p001);
        glupVertex(p000);
        glupVertex(p001);
        glupVertex(p100);
        glupVertex(p101);
        glupVertex(p110);
        glupVertex(p111);
        glupVertex(p010);            
        glupVertex(p011);
        glupEnd();
    }
    
    void PlainLuaGrobShader::pick_object(
	index_t object_id
    ) {
        geo_argused(object_id);
        // TODO
        picking_ = true;
        draw();
        picking_ = false;
    }

    void PlainLuaGrobShader::blink() {
        // TODO
        update();
    }
}

