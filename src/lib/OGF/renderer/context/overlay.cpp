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

#include <OGF/renderer/context/overlay.h>
#include <geogram_gfx/third_party/imgui/imgui.h>

#ifdef __GNUC__
#ifndef __ICC
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#endif

#include <geogram_gfx/third_party/imgui/imgui_internal.h>

namespace {
    using namespace GEO;
    
    inline Numeric::uint32 color_to_uint32(const Color& color) {
        return ImGui::ColorConvertFloat4ToU32(
            ImVec4(
                float(color.r()),
                float(color.g()),
                float(color.b()),
                float(color.a())
            )
        );
    }
}

namespace OGF {

    void Overlay::clear() {
        primitives_.resize(0);
    }

    void Overlay::playback() {
        ImDrawList* L = ImGui::GetBackgroundDrawList();
        for(Primitive& P : primitives_) {
            switch(P.type) {
            case OVL_SEGMENT: {
                L->AddLine(
                    ImVec2(P.x1,P.y1), ImVec2(P.x2,P.y2),
                    ImU32(P.color), P.thickness
                );
            } break;
            case OVL_RECT: {
                if(P.filled) {
                    L->AddRectFilled(
                        ImVec2(std::min(P.x1, P.x2), std::min(P.y1, P.y2)),
                        ImVec2(std::max(P.x1, P.x2), std::max(P.y1, P.y2)),
                        ImU32(P.color)
                    );
                } else {
                    L->AddRect(
                        ImVec2(std::min(P.x1, P.x2), std::min(P.y1, P.y2)),
                        ImVec2(std::max(P.x1, P.x2), std::max(P.y1, P.y2)),
                        ImU32(P.color),
                        P.thickness
                    );
                }
            } break;
            case OVL_CIRCLE: {
                if(P.filled) {
                    L->AddCircleFilled(
                        ImVec2(P.x1, P.y1),
                        P.R,
                        ImU32(P.color)
                    );
                } else {
                    L->AddCircle(
                        ImVec2(P.x1, P.y1),
                        P.R,
                        ImU32(P.color),
                        0, // <- num_segments
                        P.thickness
                    );
                }
            } break;
            }
        }
    }
        
    void Overlay::segment(vec2 p1, vec2 p2, Color color, double thickness) {
        Primitive P;
        P.type = OVL_SEGMENT;
        P.x1 = float(p1.x);
        P.y1 = float(p1.y);
        P.x2 = float(p2.x);
        P.y2 = float(p2.y);
        P.color = color_to_uint32(color);
        P.thickness = float(thickness);
        primitives_.push_back(P);
    }

    void Overlay::rect(vec2 p1, vec2 p2, Color color, double thickness) {
        Primitive P;
        P.type = OVL_RECT;
        P.x1 = float(p1.x);
        P.y1 = float(p1.y);
        P.x2 = float(p2.x);
        P.y2 = float(p2.y);
        P.color = color_to_uint32(color);
        P.thickness = float(thickness);
        P.filled = false;
        primitives_.push_back(P);
    }

    void Overlay::fillrect(vec2 p1, vec2 p2, Color color) {
        Primitive P;
        P.type = OVL_RECT;
        P.x1 = float(p1.x);
        P.y1 = float(p1.y);
        P.x2 = float(p2.x);
        P.y2 = float(p2.y);
        P.color = color_to_uint32(color);
        P.filled = true;
        primitives_.push_back(P);
    }

    void Overlay::circle(vec2 p1, double R, Color color, double thickness) {
        Primitive P;
        P.type = OVL_CIRCLE;
        P.x1 = float(p1.x);
        P.y1 = float(p1.y);
        P.R = float(R);
        P.color = color_to_uint32(color);
        P.thickness = float(thickness);
        P.filled = false;
        primitives_.push_back(P);
    }

    void Overlay::fillcircle(vec2 p1, double R, Color color) {
        Primitive P;
        P.type = OVL_CIRCLE;
        P.x1 = float(p1.x);
        P.y1 = float(p1.y);
        P.R = float(R);
        P.color  = color_to_uint32(color);
        P.filled = true;
        primitives_.push_back(P);
    }

}
