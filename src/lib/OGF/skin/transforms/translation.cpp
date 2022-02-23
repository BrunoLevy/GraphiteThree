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
 

#include <OGF/skin/transforms/translation.h>

namespace OGF {

//_________________________________________________________
    
    
    Translation::Translation() {
    }

    Translation::~Translation() {
    }
    
    void Translation::grab(const vec2& value) {
        last_position_ = vec2(value.x, 1.0-value.y);
    }

    void Translation::drag(const vec2& value) {
        vec2 new_pos = vec2(value.x, 1.0-value.y);
	vec2 delta = last_position_ - new_pos;
        value_changed(vec3(delta.x, delta.y, 0.0));
        last_position_ = new_pos ;
    }

    void Translation::release(const vec2& value) {
        vec2 new_pos = vec2(value.x, 1.0-value.y);
	vec2 delta = last_position_ - new_pos;
        value_changed(vec3(delta.x, delta.y, 0.0));	
        last_position_ = new_pos ;
    }

//_________________________________________________________

}

