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
#include <OGF/renderer/context/rendering_context.h>

#ifdef GEO_COMPILER_GCC_FAMILY
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wc++98-compat-pedantic"
#endif
#endif

#include <geogram_gfx/third_party/imgui/imgui.h>
#include <geogram_gfx/third_party/imgui/imgui_internal.h>

#if defined(GEO_COMPILER_GCC_FAMILY)
#pragma GCC diagnostic pop
#endif


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
        ImDrawList* L = ImGui::GetForegroundDrawList();
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
                        0.0f, // <- rounding
                        0,    // <- flags
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
            case OVL_TRIANGLE: {
                L->AddTriangleFilled(
                    ImVec2(P.x1, P.y1),
                    ImVec2(P.x2, P.y2),
                    ImVec2(P.x3, P.y3),
                    ImU32(P.color)
                );
            } break;
            case OVL_QUAD: {
                L->AddQuadFilled(
                    ImVec2(P.x1, P.y1),
                    ImVec2(P.x2, P.y2),
                    ImVec2(P.x3, P.y3),
                    ImVec2(P.x4, P.y4),
                    ImU32(P.color)
                );
            } break;
            }
        }
    }

    void Overlay::add_primitive(Primitive& P) {
	// float x0 = ImGui::GetMainViewport()->Pos.x;
	// float y0 = ImGui::GetMainViewport()->Pos.y;
	// x1..x4 += x0; y1..y4 += y0 // do not remember why ??
	float H = float(rendering_context_->get_height());
	float sx = 1.0f/ImGui::GetIO().DisplayFramebufferScale.x;
	float sy = 1.0f/ImGui::GetIO().DisplayFramebufferScale.y;
	P.x1 = sx * P.x1;
	P.y1 = H-1.0f-sy*P.y1;

	P.x2 = sx * P.x2;
	P.y2 = H-1.0f-sy*P.y2;

	P.x3 = sx * P.x3;
	P.y3 = H-1.0f-sy*P.y3;

	P.x4 = sx * P.x4;
	P.y4 = H-1.0f-sy*P.y4;
	primitives_.push_back(P);
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
        add_primitive(P);
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
        add_primitive(P);
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
        add_primitive(P);
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
        add_primitive(P);
    }

    void Overlay::fillcircle(vec2 p1, double R, Color color) {
        Primitive P;
        P.type = OVL_CIRCLE;
        P.x1 = float(p1.x);
        P.y1 = float(p1.y);
        P.R = float(R);
        P.color  = color_to_uint32(color);
        P.filled = true;
        add_primitive(P);
    }

    void Overlay::filltriangle(vec2 p1, vec2 p2, vec2 p3, Color color) {
        Primitive P;
        P.type = OVL_TRIANGLE;
        P.x1 = float(p1.x);
        P.y1 = float(p1.y);
        P.x2 = float(p2.x);
        P.y2 = float(p2.y);
        P.x3 = float(p3.x);
        P.y3 = float(p3.y);
        P.color  = color_to_uint32(color);
        P.filled = true;
        add_primitive(P);
    }

    void Overlay::fillquad(vec2 p1, vec2 p2, vec2 p3, vec2 p4, Color color) {
        Primitive P;
        P.type = OVL_QUAD;
        P.x1 = float(p1.x);
        P.y1 = float(p1.y);
        P.x2 = float(p2.x);
        P.y2 = float(p2.y);
        P.x3 = float(p3.x);
        P.y3 = float(p3.y);
        P.x4 = float(p4.x);
        P.y4 = float(p4.y);
        P.color  = color_to_uint32(color);
        P.filled = true;
        add_primitive(P);
    }

}
