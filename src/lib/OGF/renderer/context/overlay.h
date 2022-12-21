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

    /**
     * \brief A display list that memorizes simple graphic primitives to
     *  be displayed over the 3D rendering window.
     * \details Used by some tools. We annot directly use ImDrawList API,
     *  because mouse event handler of the 3D window are called completely 
     *  independelty of ImGui, so we need a "protocol" to communicate
     *  between the two worlds.
     */
    class RENDERER_API Overlay {
    public:
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
         *  segment. One may use Tool::project_point()
         */
        void segment(vec2 p1, vec2 p2, Color color, double thickness=1.0);
        void rect(vec2 p1, vec2 p2, Color color, double thickness=1.0);
        void fillrect(vec2 p1, vec2 p2, Color color);
        void circle(vec2 p1, double R, Color color, double thickness=1.0);
        void fillcircle(vec2 p1, double R, Color color);
        
    private:
        enum PrimitiveType {
            OVL_SEGMENT, OVL_RECT, OVL_CIRCLE
        };
        struct Primitive {
            PrimitiveType type;
            float x1,y1,x2,y2,R;
            float thickness;
            Numeric::uint32 color;
            bool filled;
        };
        vector<Primitive> primitives_;
    };
    
}

#endif

