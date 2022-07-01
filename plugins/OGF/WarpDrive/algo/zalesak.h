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
 

#ifndef OGF_WARPDRIVE_ALGO_ZALESAK
#define OGF_WARPDRIVE_ALGO_ZALESAK

#include <OGF/WarpDrive/common/common.h>
#include <OGF/WarpDrive/algo/velocity_field.h>

namespace OGF {

    /**
     * \brief Implements the VelocityField for Zalezak test.  \details
     * see Enright & al. 2002 - a hybrid particle level set method
     * for improved interface capturing. This implementation by David Lopez
     * (June 2010).
     */

    class WarpDrive_API ZalesakVelocityField : public VelocityField {
      public:
	ZalesakVelocityField();
	~ZalesakVelocityField() override;
	void get_velocity(
	    double current_time, const vec3& vertex, vec3 &velocity
	) const override;
    };
}
#endif

