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
 */


#include <OGF/WarpDrive/algo/enright.h>

namespace OGF {

    EnrightVelocityField::EnrightVelocityField() : period_(3.0) {
    }
    
    EnrightVelocityField::~EnrightVelocityField() {
    }
    
    void EnrightVelocityField::get_velocity(
	double current_time, const vec3& vertex, vec3 &velocity
    ) const {
	double x = vertex.x ;
	double y = vertex.y ;
	double z = vertex.z ;
	
	velocity= vec3(
	    2.0 * sin(M_PI*x) * sin(M_PI*x) * sin(2.0*M_PI*y) * sin(2.0*M_PI*z),
	    -sin(2.0*M_PI*x) * sin(M_PI*y)*sin(M_PI*y) * sin(2.0*M_PI*z),
	    -sin(2.0*M_PI*x) * sin(2.0*M_PI*y) * sin(M_PI*z) * sin(M_PI*z)
	);

	// modulate with a period of 3 -> correspond à la longueur de
	// l'animation (pour revenir au point de départ...)
	velocity *= sin(M_PI * current_time * 2 / get_period()) ;    
    }
    
    
}

