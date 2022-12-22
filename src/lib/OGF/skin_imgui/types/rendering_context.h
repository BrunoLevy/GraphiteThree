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
 

#ifndef H_SKIN_IMGUI_RENDERING_CONTEXT_H
#define H_SKIN_IMGUI_RENDERING_CONTEXT_H

#include <OGF/skin_imgui/common/common.h>
#include <OGF/renderer/context/rendering_context.h>

#include <geogram/image/color.h>
#include <geogram_gfx/GLUP/GLUP.h>

namespace OGF {
    
    /**
     * \brief A class derived from RenderingContext that interfaces 
     *  the Application class with Graphite.
     * \details Uses a FrameBufferObject so that the GUI can be refreshed
     *  without needing to redraw 3D content.
     */
    class SkinImGUIRenderingContext : public RenderingContext {
    public:

	/**
	 * \brief SkinImGUIRenderingContext constructor.
	 * \param[in] glup_context the GLUP context to be used
	 *  for rendering.
	 */
	SkinImGUIRenderingContext(
	    GLUPcontext glup_context
	) : RenderingContext(glup_context) {
	}

	/**
	 * \copydoc RenderingContext::begin_frame()
	 */
	void begin_frame() override;

	/**
	 * \copydoc RenderingContext::end_frame()
	 */
	void end_frame() override;

	/**
	 * \copydoc RenderingContext::resize()
	 */
	void resize(index_t w, index_t h) override;

	/**
	 * \brief Draws the frame that was rendered in the FBO.
	 */
	void draw_last_frame();

	/**
	 * \brief Copies the content of this RenderingContext
	 *  to an Image.
	 * \param[out] image a pointer to the image. It should have
	 *  RGB or RGBA storage, and have the same size as this
	 *  RenderingContext.
	 * \param[in] hide_gui if true, gui elements are not captured
	 *  (only the content of the frame buffer object), else the
	 *  current OpenGL framebuffer is copied (with the GUI if it
	 *  called after GUI refresh).
	 */
	void snapshot(Image* image, bool hide_gui=true);

    private:
	FrameBufferObject FBO_;
    };

}

#endif
