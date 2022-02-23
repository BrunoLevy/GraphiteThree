/*
 *  GXML/Graphite: Geometry and Graphics Programming Library + Utilities
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

#include <OGF/skin_imgui/types/rendering_context.h>
#include <OGF/skin_imgui/types/application.h>

namespace OGF {

    void SkinImGUIRenderingContext::begin_frame() {
	if(width_ == 0 || height_ == 0) {
	    return;
	}
	//   To prevent the logger to trigger global
	// redraw while we are doing rendering ops.
	Application::instance()->lock_updates();
	if(!FBO_.initialized()) {
	    GEO_CHECK_GL();	    	    		
	    FBO_.initialize(get_width(), get_height(), true, GL_RGBA, false);
	    initialized_ = true;
	    GEO_CHECK_GL();	    	    		
	}
	GEO_CHECK_GL();
	FBO_.bind_as_framebuffer();
	RenderingContext::begin_frame();
    }

    void SkinImGUIRenderingContext::end_frame() {
	RenderingContext::end_frame();
	GEO_CHECK_GL();	    	    	    
	FBO_.unbind();
	GEO_CHECK_GL();
	Application::instance()->unlock_updates();	
    }

    void SkinImGUIRenderingContext::resize(index_t w, index_t h) {
	RenderingContext::resize(w,h);
	if(FBO_.initialized()) {
	    FBO_.resize(w,h);
	}
    }
	
    void SkinImGUIRenderingContext::draw_last_frame() {
	glDisable(GL_DEPTH_TEST);	    
	glActiveTexture(GL_TEXTURE0);
	FBO_.bind_as_texture();
	GEO_CHECK_GL();	    	    	    
	glViewport(0,0,GLint(get_width()),GLint(get_height()));
	GEO_CHECK_GL();	    	    	    
	draw_unit_textured_quad();
	GEO_CHECK_GL();	    	    	    
	FBO_.unbind();
	glEnable(GL_DEPTH_TEST);	    	    
    }

    void SkinImGUIRenderingContext::snapshot(Image* image, bool make_current) {
	// Bind FBO as framebuffer only if not already bound.
	// Binding if already bound would work, but this would
	// also overwrite the previously backed up binding state,
	// thus wreaking the state of the system (all rendering
	// operations would be redirected to the FBO, not good !)
	bool need_to_bind = !FBO_.is_bound_as_framebuffer();
	if(make_current && need_to_bind) {
	    FBO_.bind_as_framebuffer();
	}
	RenderingContext::snapshot(image, make_current);
	if(make_current && need_to_bind) {
	    FBO_.unbind();
	}
    }

}


