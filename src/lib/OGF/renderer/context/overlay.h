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

#ifndef H_OGF_RENDERER_CONTEXT_OVERLAY_H
#define H_OGF_RENDERER_CONTEXT_OVERLAY_H

#include <OGF/renderer/common/common.h>
#include <geogram/image/color.h>

namespace OGF {

    class RenderingContext;

    /**
     * \brief A display list that memorizes simple graphic primitives to
     *  be displayed over the 3D rendering window.
     * \details Used by some tools. We cannot directly use ImDrawList API,
     *  because mouse event handler of the 3D window are called completely
     *  independelty of ImGui, so we need a "protocol" to communicate
     *  between the two worlds.
     *  All functions use OpenGL device coordinates, that is, in pixels,
     *  with origin in lower left corner.
     */
    class RENDERER_API Overlay {
    public:

        Overlay(
	    RenderingContext* rendering_context
	) : rendering_context_(rendering_context) {
        }

        /**
         * \brief Removes all primitives to be displayed.
         */
        void clear();

        /**
         * \brief Plays back this overlay to the current ImGui context.
         * \details This function needs to be called right before
         *  ImGui::Render(). It is called by ApplicationImpl::draw_gui()
         *  in skin_imgui.
         */
        void playback();

        /**
         * \brief Adds a segment to the display list.
         * \param[in] p1 , p2 the 2D coordinates of the extremities of the
         *  segment. One may use Tool::project_point() to obtain them from
         *  real-world coordinates. Origin is lower-left corner.
         * \param[in] color the color, with alpha
         * \param[in] thickness line thickness (ImGui can draw antialiased
         *  thick lines).
         */
        void segment(vec2 p1, vec2 p2, Color color, double thickness=1.0);

        /**
         * \brief Adds a rectangle to the display list.
         * \param[in] p1 , p2 the 2D coordinates of two corners of the
         *  rectangle. One may use Tool::project_point() to obtain them from
         *  real-world coordinates. Origin is lower-left corner.
         * \param[in] color the color, with alpha
         * \param[in] thickness line thickness (ImGui can draw antialiased
         *  thick lines).
         */
        void rect(vec2 p1, vec2 p2, Color color, double thickness=1.0);

        /**
         * \brief Adds a filled rectangle to the display list.
         * \param[in] p1 , p2 the 2D coordinates of two corners of the
         *  rectangle. One may use Tool::project_point() to obtain them from
         *  real-world coordinates. Origin is lower-left corner.
         * \param[in] color the color, with alpha
         */
        void fillrect(vec2 p1, vec2 p2, Color color);

        /**
         * \brief Adds a circle to the display list.
         * \param[in] p1 the 2D coordinates of the center of the
         *  circle. One may use Tool::project_point() to obtain them from
         *  real-world coordinates. Origin is lower-left corner.
         * \param[in] R the radius of the circle
         * \param[in] color the color, with alpha
         * \param[in] thickness line thickness (ImGui can draw antialiased
         *  thick lines).
         */
        void circle(vec2 p1, double R, Color color, double thickness=1.0);

        /**
         * \brief Adds a filled circle to the display list.
         * \param[in] p1 the 2D coordinates of the center of the
         *  circle. One may use Tool::project_point() to obtain them from
         *  real-world coordinates. Origin is lower-left corner.
         * \param[in] R the radius of the circle
         * \param[in] color the color, with alpha
         */
        void fillcircle(vec2 p1, double R, Color color);

        /**
         * \brief Adds a filled triangle to the display list.
         * \param[in] p1 , p2 , p3 the 2D coordinates of two corners of the
         *  rectangle. One may use Tool::project_point() to obtain them from
         *  real-world coordinates. Origin is lower-left corner.
         * \param[in] color the color, with alpha
         */
        void filltriangle(vec2 p1, vec2 p2, vec2 p3, Color color);

        /**
         * \brief Adds a filled triangle to the display list.
         * \param[in] p1 , p2 , p3 , p4 the 2D coordinates of two corners of the
         *  rectangle. One may use Tool::project_point() to obtain them from
         *  real-world coordinates. Origin is lower-left corner.
         * \param[in] color the color, with alpha
         */
        void fillquad(vec2 p1, vec2 p2, vec2 p3, vec2 p4, Color color);


	/**
	 * \brief Converts Dear ImGui coordinates to OpenGL coordinate
	 * \param[in] XY_imgui a point in imgui coordinates, with
	 *   origin at top-left corner and possibly retina-screen scaling
	 * \return the corresponding point in OpenGL Window coordinates,
	 *   with origin at bottom-left.
	 */
	vec2f imgui_to_GL(const vec2f& XY_imgui) const;

	/**
	 * \brief Converts OpenGL coordinates to Dear ImGui coordinate
	 * \param[in] XY_GL a point in OpenGL Window coordinates, with
	 *   origin at bottom-left corner.
	 * \return the corresponding point in Dear ImGui coordinates,
	 *   with origin at top-left and possibly retina-screen scaling.
	 */
	vec2f GL_to_imgui(const vec2f& XY_GL) const;

	/**
	 * \brief Converts OpenGL coordinates to Dear ImGui coordinate
	 * \param[in] XY_GL a point in OpenGL Window coordinates, with
	 *   origin at bottom-left corner, in double-precision.
	 * \return the corresponding point in Dear ImGui coordinates,
	 *   with origin at top-left and possibly retina-screen scaling.
	 */
	vec2f GL_to_imgui(const vec2& XY_GL) const {
	    return GL_to_imgui(vec2f(float(XY_GL.x), float(XY_GL.y)));
	}

    private:
        enum PrimitiveType {
            OVL_SEGMENT =0,
            OVL_RECT    =1,
            OVL_CIRCLE  =2,
            OVL_TRIANGLE=3,
            OVL_QUAD    =4
        };
        struct Primitive {
            PrimitiveType type;
	    vec2f p[4];
	    float R;
            float thickness;
            Numeric::uint32 color;
            bool filled;
        };

	void add_primitive(Primitive& prim);
        vector<Primitive> primitives_;
	RenderingContext* rendering_context_;
    };

}

#endif
