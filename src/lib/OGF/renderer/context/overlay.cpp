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
                    ImVec2(P.p[0].x,P.p[0].y), ImVec2(P.p[1].x,P.p[1].y),
                    ImU32(P.color), P.thickness
                );
            } break;
            case OVL_RECT: {
                if(P.filled) {
                    L->AddRectFilled(
                        ImVec2(
			    std::min(P.p[0].x, P.p[1].x),
			    std::min(P.p[0].y, P.p[1].y)
			),
                        ImVec2(
			    std::max(P.p[0].x, P.p[1].x),
			    std::max(P.p[0].y, P.p[1].y)
			),
                        ImU32(P.color)
                    );
                } else {
                    L->AddRect(
                        ImVec2(
			    std::min(P.p[0].x, P.p[1].x),
			    std::min(P.p[0].y, P.p[1].y)
			),
                        ImVec2(
			    std::max(P.p[0].x, P.p[1].x),
			    std::max(P.p[0].y, P.p[1].y)
			),
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
                        ImVec2(P.p[0].x, P.p[0].y),
                        P.R,
                        ImU32(P.color)
                    );
                } else {
                    L->AddCircle(
                        ImVec2(P.p[0].x, P.p[0].y),
                        P.R,
                        ImU32(P.color),
                        0, // <- num_segments
                        P.thickness
                    );
                }
            } break;
            case OVL_TRIANGLE: {
                L->AddTriangleFilled(
                    ImVec2(P.p[0].x, P.p[0].y),
                    ImVec2(P.p[1].x, P.p[1].y),
                    ImVec2(P.p[2].x, P.p[2].y),
                    ImU32(P.color)
                );
            } break;
            case OVL_QUAD: {
                L->AddQuadFilled(
                    ImVec2(P.p[0].x, P.p[0].y),
                    ImVec2(P.p[1].x, P.p[1].y),
                    ImVec2(P.p[2].x, P.p[2].y),
                    ImVec2(P.p[3].x, P.p[3].y),
                    ImU32(P.color)
                );
            } break;
            }
        }
    }

    void Overlay::add_primitive(Primitive& P) {
	primitives_.push_back(P);
    }

    void Overlay::segment(vec2 p1, vec2 p2, Color color, double thickness) {
        Primitive P;
        P.type = OVL_SEGMENT;
	P.p[0] = GL_to_imgui(p1);
	P.p[1] = GL_to_imgui(p2);
        P.color = color_to_uint32(color);
        P.thickness = float(thickness);
        add_primitive(P);
    }

    void Overlay::rect(vec2 p1, vec2 p2, Color color, double thickness) {
        Primitive P;
        P.type = OVL_RECT;
	P.p[0] = GL_to_imgui(p1);
	P.p[1] = GL_to_imgui(p2);
        P.color = color_to_uint32(color);
        P.thickness = float(thickness);
        P.filled = false;
        add_primitive(P);
    }

    void Overlay::fillrect(vec2 p1, vec2 p2, Color color) {
        Primitive P;
        P.type = OVL_RECT;
	P.p[0] = GL_to_imgui(p1);
	P.p[1] = GL_to_imgui(p2);
        P.color = color_to_uint32(color);
        P.filled = true;
        add_primitive(P);
    }

    void Overlay::circle(vec2 p1, double R, Color color, double thickness) {
        Primitive P;
        P.type = OVL_CIRCLE;
	P.p[0] = GL_to_imgui(p1);
        P.R = float(R);
        P.color = color_to_uint32(color);
        P.thickness = float(thickness);
        P.filled = false;
        add_primitive(P);
    }

    void Overlay::fillcircle(vec2 p1, double R, Color color) {
        Primitive P;
        P.type = OVL_CIRCLE;
	P.p[0] = GL_to_imgui(p1);
        P.R = float(R);
        P.color  = color_to_uint32(color);
        P.filled = true;
        add_primitive(P);
    }

    void Overlay::filltriangle(vec2 p1, vec2 p2, vec2 p3, Color color) {
        Primitive P;
        P.type = OVL_TRIANGLE;
	P.p[0] = GL_to_imgui(p1);
	P.p[1] = GL_to_imgui(p2);
	P.p[2] = GL_to_imgui(p3);
        P.color  = color_to_uint32(color);
        P.filled = true;
        add_primitive(P);
    }

    void Overlay::fillquad(vec2 p1, vec2 p2, vec2 p3, vec2 p4, Color color) {
        Primitive P;
        P.type = OVL_QUAD;
	P.p[0] = GL_to_imgui(p1);
	P.p[1] = GL_to_imgui(p2);
	P.p[2] = GL_to_imgui(p3);
	P.p[3] = GL_to_imgui(p4);
        P.color  = color_to_uint32(color);
        P.filled = true;
        add_primitive(P);
    }

    vec2f Overlay::imgui_to_GL(const vec2f& imgui) const {
	float H = float(rendering_context_->get_height());
	float sx = ImGui::GetIO().DisplayFramebufferScale.x;
	float sy = ImGui::GetIO().DisplayFramebufferScale.y;
	return vec2f(sx*imgui.x, H-1.0f-sy*imgui.y);
    }

    vec2f Overlay::GL_to_imgui(const vec2f& GL) const {
	float H = float(rendering_context_->get_height());
	float sx = 1.0f/ImGui::GetIO().DisplayFramebufferScale.x;
	float sy = 1.0f/ImGui::GetIO().DisplayFramebufferScale.y;
	return vec2f(sx*GL.x, sy*(H-1.0f-GL.y));
    }

}
