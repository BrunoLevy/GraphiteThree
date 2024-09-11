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
 * As an exception to the GPL, Graphite can be linked with
 *  the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#include <OGF/mesh_gfx/shaders/param_mesh_grob_shader.h>

namespace OGF {

    ParamMeshGrobShader::ParamMeshGrobShader(
	MeshGrob* grob
    ) : MeshGrobShader(grob) {

        surface_style_.visible = true;
        surface_style_.color = Color(1.0,1.0,1.0,1.0);

	mesh_style_.visible = true;
        mesh_style_.color   = Color(0.0,0.0,0.0,1.0);
        mesh_style_.width   = 1;
    }

    ParamMeshGrobShader::~ParamMeshGrobShader() {
    }

    void ParamMeshGrobShader::draw() {
        MeshGrobShader::draw();

	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(
	    mesh_grob()->facet_corners.attributes(), "tex_coord"
	);
	if(tex_coord.is_bound() && tex_coord.dimension() != 2) {
	    tex_coord.unbind();
	}
	if(!tex_coord.is_bound()) {
	    return;
	}

	// Setup GLUP transform in such a way that parameter space
	// is displayed in the bounding box of the object, so that
	// the parameter space is displayed where the object was,
	// and the "home" button works as expected.

	Box3d B = mesh_grob()->bbox();
	double s = B.radius();
	glupMatrixMode(GLUP_MODELVIEW_MATRIX);
	glupPushMatrix();

	glupTranslated(B.x_min(), B.y_min(), B.z_min() + s);
	glupScaled(s,s,s);

	glupDisable(GLUP_LIGHTING);
	if(surface_style_.visible) {
	    glupSetColor4dv(
		GLUP_FRONT_AND_BACK_COLOR,surface_style_.color.data()
	    );

	    if(mesh_style_.visible) {
		glupEnable(GLUP_DRAW_MESH);
		glupSetColor4dv(GLUP_MESH_COLOR,mesh_style_.color.data());
		glupSetMeshWidth(GLUPint(mesh_style_.width));
	    } else {
		glupDisable(GLUP_DRAW_MESH);
	    }
	    glupBegin(GLUP_TRIANGLES);
	    for(index_t f: mesh_grob()->facets) {
		if(mesh_grob()->facets.nb_vertices(f) == 3) {
		    index_t c = mesh_grob()->facets.corners_begin(f);
		    glupVertex2dv(&tex_coord[2*c]);
		    glupVertex2dv(&tex_coord[2*(c+1)]);
		    glupVertex2dv(&tex_coord[2*(c+2)]);
		}
	    }
	    glupEnd();
	    glupBegin(GLUP_QUADS);
	    for(index_t f: mesh_grob()->facets) {
		if(mesh_grob()->facets.nb_vertices(f) == 4) {
		    index_t c = mesh_grob()->facets.corners_begin(f);
		    glupVertex2dv(&tex_coord[2*c]);
		    glupVertex2dv(&tex_coord[2*(c+1)]);
		    glupVertex2dv(&tex_coord[2*(c+2)]);
		    glupVertex2dv(&tex_coord[2*(c+3)]);
		}
	    }
	    glupEnd();
	}
	glupEnable(GLUP_LIGHTING);

	glupPopMatrix();
	tex_coord.unbind();
    }
}
