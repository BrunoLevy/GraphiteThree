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
 */

#include <OGF/WarpDrive/algo/velocity_field.h>
#include <OGF/WarpDrive/algo/enright.h>
#include <OGF/WarpDrive/algo/zalesak.h>
#include <OGF/WarpDrive/algo/curlnoise.h>

namespace OGF {
    
    VelocityField::VelocityField() {
    }
    
    VelocityField::~VelocityField() {
    }
    
    void VelocityField::get_velocity(
	double current_time, const vec3& vertex, vec3 &veloc
    ) const {
	geo_argused(current_time);
	geo_argused(vertex);
	veloc = vec3(1,0,0) ;
    }

    VelocityField* VelocityField::create(velocity_field_t type) {
	VelocityField* result = nullptr;
	switch(type) {
	    case ENRIGHT:
		result = new EnrightVelocityField();
		break;
	    case ZALESAK:
		result = new ZalesakVelocityField();
		break;
	    case CURLNOISE:
		result = new CurlNoiseVelocityField();
		break;
	}
	return result;
    }
}


