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
 *
 */


#include <OGF/WarpDrive/shaders/transport_mesh_grob_shader.h>

namespace OGF {

    TransportMeshGrobShader::TransportMeshGrobShader(
        MeshGrob* grob
    ) : MeshGrobShader(grob) {
        interp_ = 0.0;
        time_ = -25;
        potential_=false;
        conjugate_=false;
        origin_=false;
        skip_ = 100;

        trajectories_.visible = false;
        trajectories_.color   = Color(0.0,0.0,0.5,1.0);
        trajectories_.width   = 1;

        two_d_=false;
	lighting_=true;
        surface_style_.visible = true;
        surface_style_.color = Color(0.5,0.5,0.5,1.0);
    }

    TransportMeshGrobShader::~TransportMeshGrobShader() {
    }

    void TransportMeshGrobShader::draw() {
        morph_.bind_if_is_defined(mesh_grob()->vertices.attributes(), "geom2");
	draw_surface();
        if(origin_) {
            double bkp = interp_;
            interp_=0;
	    draw_surface();
            interp_=bkp;
        }

	phi_.bind_if_is_defined(
	    mesh_grob()->vertices.attributes(),"potential"
	);
	if(potential_) {
	    draw_surface();
	}
        if(trajectories_.visible) {
	    draw_trajectories();
        }

	if(morph_.is_bound()) {
	    morph_.unbind();
	}
	if(phi_.is_bound()) {
	    phi_.unbind();
	}
    }

    void TransportMeshGrobShader::draw_trajectories() {
        if(!morph_.is_bound() || !phi_.is_bound()) {
            return;
        }
	glupSetColor3dv(GLUP_FRONT_COLOR, trajectories_.color.data());
	glupSetMeshWidth(GLUPint(trajectories_.width));
	glupBegin(GLUP_LINES);
	for(index_t v=0; v<mesh_grob()->vertices.nb(); ++v) {
            if(!(v%skip_)) {
                vec3 p1(mesh_grob()->vertices.point_ptr(v));
                if(two_d_) {
                    p1.z = 0.0 ;
                }
                vec3 p2 = get_point_potential(v);
                vec3 p3 = get_point_interp(v);
		glupVertex(p1);
		glupVertex(p2);
		glupVertex(p2);
		glupVertex(p3);
            }
        }
	glupEnd();
    }

    void TransportMeshGrobShader::draw_surface() {
	if(!morph_.is_bound()) {
	    return;
	}
	if(lighting_) {
	    glupEnable(GLUP_LIGHTING);
	} else {
	    glupDisable(GLUP_LIGHTING);
	}
	glupSetColor3dv(
	    GLUP_FRONT_AND_BACK_COLOR, surface_style_.color.data()
	);
	glupBegin(GLUP_TRIANGLES);
	for(index_t t=0; t<mesh_grob()->facets.nb(); ++t) {
	    index_t v1 = mesh_grob()->facets.vertex(t,0);
	    index_t v2 = mesh_grob()->facets.vertex(t,1);
	    index_t v3 = mesh_grob()->facets.vertex(t,2);
	    glupVertex(get_point(v1));
	    glupVertex(get_point(v2));
	    glupVertex(get_point(v3));
	}
	glupEnd();
    }
}
