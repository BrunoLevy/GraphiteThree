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
 */

#include <OGF/WarpDrive/algo/time_integrator.h>
#include <OGF/WarpDrive/algo/velocity_field.h>

namespace {
    using namespace OGF;


    /**
     * \brief Computes a velocity with the 2th order Runge Kuta scheme.
     */
    void compute_Runge_Kutta_2 (
	double current_time, double delta_t, const vec3& vertex,
	vec3 &veloc, const VelocityField* m_veloc
    ) {
	vec3 vertex_tmp, velocity_tmp ;
	m_veloc->get_velocity(current_time, vertex, velocity_tmp);
	vertex_tmp = vertex + velocity_tmp * delta_t ;
	m_veloc->get_velocity(current_time+delta_t/2, vertex_tmp, veloc) ;
    }

    /**
     * \brief Computes a velocity with the 4th order Runge Kuta scheme.
     */
    void compute_Runge_Kutta_4(
	double current_time, double delta_t, const vec3& vertex,
	vec3 &veloc, const VelocityField* m_veloc
    ) {

	vec3  veloc_tmp, k1, k2, k3, k4 ;
	// k1 = dt * f( t, x );
	m_veloc->get_velocity(current_time, vertex, veloc_tmp );
        // getVelocity au temps t pour le pt xi, retour : vitesse v
	k1 = delta_t * veloc_tmp;

	// k2 = dt * f( t + 0.5*dt, x + 0.5*k1 );
	m_veloc->get_velocity(
	    current_time + 0.5*delta_t, vertex + 0.5 * k1, veloc_tmp
	);
	k2 = delta_t * veloc_tmp;

	// k3 = dt * f( t + 0.5*dt, x + 0.5*k2 );
	m_veloc->get_velocity(
	    current_time + 0.5*delta_t, vertex + 0.5 * k2, veloc_tmp
	);
	k3 = delta_t * veloc_tmp;

	// k4 = dt * f( t + dt, x + k3 );
	m_veloc->get_velocity(current_time + delta_t, vertex + k3, veloc_tmp);
	k4 = delta_t * veloc_tmp;
	veloc = (1./6. * ( k1 + k4 ) + 1./3. * ( k2 + k3 ) ) / delta_t ;
    }

    /**
     * \brief Computes the velocity with the method of characteristics
     * (cf Stam99) (i.e. semi-lagrangian advection method).
     * \note not implemented yet.
     */
    void compute_semi_Lagrangian(
	double current_time, double delta_t, const vec3& vertex, vec3 &veloc,
	const VelocityField* m_veloc
    ) {
	geo_argused(current_time);
	geo_argused(delta_t);
	geo_argused(vertex);
	geo_argused(veloc);
	geo_argused(m_veloc);
	geo_assert_not_reached;
    }
}

namespace OGF {

    void time_integrator(
	double current_time, double delta_t, const vec3& vertex, vec3& veloc,
	time_integrator_t algo, const VelocityField* m_veloc
    ) {
	switch(algo) {
	    case RUNGE_KUTTA_2 :
		compute_Runge_Kutta_2(
		    current_time, delta_t, vertex, veloc, m_veloc
		) ;
		break;
	    case RUNGE_KUTTA_4 :
		compute_Runge_Kutta_4(
		    current_time, delta_t, vertex, veloc, m_veloc
		);
		break;
	    case SEMI_LAGRANGIAN:
		compute_semi_Lagrangian(
		    current_time, delta_t, vertex, veloc, m_veloc
		);
		break;
	    case SIMPLE :
		m_veloc->get_velocity(current_time, vertex, veloc);
		break;
	}
    }
}
