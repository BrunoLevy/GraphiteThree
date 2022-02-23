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

#include <OGF/skin/transforms/ray_picker.h>
#include <OGF/renderer/context/rendering_context.h>

namespace OGF {

    std::ostream& operator<<(std::ostream& out, const RayPick& ev) {
        return out << ev.p_ndc << " " << ev.button ;
    }
    
    std::istream& operator>>(std::istream& in, RayPick& ev) {
        return in >> ev.p_ndc >> ev.button;
    }
    
    
    RayPicker::RayPicker() {
    }

    RayPicker::~RayPicker() {
    }
    
    void RayPicker::grab(
        RenderingContext* rendering_context, const vec2& p_ndc, int button
    ) {
        ray_grab(
            rendering_context,
            point_to_ray_pick(rendering_context, p_ndc, button)
        ) ;
    }

    void RayPicker::drag(
        RenderingContext* rendering_context, const vec2& p_ndc, int button
    ) {
        ray_drag(
            rendering_context,
            point_to_ray_pick(rendering_context, p_ndc, button)
        ) ;
    }
    
    void RayPicker::release(
        RenderingContext* rendering_context, const vec2& p_ndc, int button
    ) {
        ray_release(
            rendering_context,
            point_to_ray_pick(rendering_context, p_ndc, button)
        ) ;
    }

    RayPick RayPicker::point_to_ray_pick(
        RenderingContext* rendering_context, const vec2& p_ndc, int button
    ) {
        // For now, just a placeholder for more efficient picking mechanisms
        // (see comments in RayPick class).
        geo_argused(rendering_context);
        return RayPick(p_ndc, button);
    }

}
