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

#ifndef OGF_WARPDRIVE_ALGO_TIME_INTEGRATOR
#define OGF_WARPDRIVE_ALGO_TIME_INTEGRATOR

#include <OGF/WarpDrive/common/common.h>
#include <geogram/basic/geometry.h>

namespace OGF {

    enum time_integrator_t {SIMPLE=0, RUNGE_KUTTA_2=1, RUNGE_KUTTA_4=2};

    class VelocityField;

    /**
     * \brief Computes a velocity with the specified algorithm and
     *  the specified velocity driver.
     * \details Implements some general methods to compute
     *  velocities (Simple, RK2, RK4). By David Lopez, June 2010
     */
    void WarpDrive_API time_integrator(
	double current_time, double delta_t,
	const vec3& vertex, vec3 &veloc,
	time_integrator_t algo, const VelocityField* m_veloc
    );

}

#endif
