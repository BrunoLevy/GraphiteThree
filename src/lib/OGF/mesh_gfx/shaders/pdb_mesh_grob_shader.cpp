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

#include <OGF/mesh_gfx/shaders/pdb_mesh_grob_shader.h>

namespace {

// #define c1 0.35
#define c2 0.5
#define c3 1.0

    const unsigned int nb_colormap_entries = 6;
    double colormap[nb_colormap_entries][3] = {
	{c2, c2, c3},
	{c3, c2, c2},
	{c2, c3, c2},
	{c3, c2, c3},
	{c2, c3, c3},
	{c3, c3, c2}
    };

}

namespace OGF {

    PDBMeshGrobShader::PDBMeshGrobShader(
	MeshGrob* grob
    ) : MeshGrobShader(grob) {
	lighting_ = false;
	atom_size_ = 10;
	atom_colors_ = chain;
    }

    PDBMeshGrobShader::~PDBMeshGrobShader() {
    }

    void PDBMeshGrobShader::draw() {
        MeshGrobShader::draw();

	bool slicing_mode = false;

	if(glupGetClipMode() == GLUP_CLIP_SLICE_CELLS) {
	    glupClipMode(GLUP_CLIP_STANDARD);
	    draw();
	    glupClipMode(GLUP_CLIP_SLICE_CELLS);
	    slicing_mode = true;
	}

	Attribute<char> atom_type;
	Attribute<char> chain_id;
	atom_type.bind_if_is_defined(
	    mesh_grob()->vertices.attributes(), "atom_type"
	);
	chain_id.bind_if_is_defined(
	    mesh_grob()->vertices.attributes(), "chain_id"
	);

	if(atom_colors_ != constant) {
	    glupEnable(GLUP_VERTEX_COLORS);
	} else {
	    glupDisable(GLUP_VERTEX_COLORS);
	    glupSetColor3d(
		GLUP_FRONT_AND_BACK_COLOR,
		0.5, 1.0, 0.0
	    );
	}
	if(lighting_ && !slicing_mode) {
	    glupEnable(GLUP_LIGHTING);
	} else {
	    glupDisable(GLUP_LIGHTING);
	}
	glupBegin(GLUP_SPHERES);
	for(index_t v: mesh_grob()->vertices) {
	    double* xyz = mesh_grob()->vertices.point_ptr(v);
	    double R = 1.0;
	    double r=0.5, g=0.5, b=0.5;
	    if(atom_type.is_bound()) {
		char t = atom_type[v];
		if(t == 'C') {
		    r = g = b = 0.0;
		} else if(t == 'H') {
		    r = g = b = 0.9;
		    R = 0.5;
		} else if(t == 'O') {
		    r = 1.0; g = 0.0; b = 0.0;
		} else if(t == 'N') {
		    r = 0.0; g = 0.0; b = 1.0;
		} else if(t == 'S') {
		    r = 1.0; g = 1.0; b = 0.0;
		} else {
		    r = 1.0; g = 0.0; b = 1.0;
		}
	    }
	    if(atom_colors_ == atom) {
		glupColor3d(r,g,b);
	    } else if(atom_colors_ == chain) {
		index_t i = chain_id.is_bound() ? index_t(chain_id[v]) % nb_colormap_entries : 0;
		double gray = 1.0;
		if(!slicing_mode) {
		    gray = 0.299*r + 0.587*g + 0.114*b;
		    gray = 0.8 + 0.2*gray;
		}
		glupColor3d(
		    gray*colormap[i][0],
		    gray*colormap[i][1],
		    gray*colormap[i][2]
		);
	    }
	    glupVertex4d(
		xyz[0], xyz[1], xyz[2],
		R * double(atom_size_) / 10.0
	    );
	}
	glupEnd();
	glupDisable(GLUP_VERTEX_COLORS);
    }


}
