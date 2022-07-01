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
 

#include <OGF/WarpDrive/shaders/voronoi_mesh_grob_shader.h>

namespace {
    static const float CC1 = 0.35f;
    static const float CC2 = 0.5f;
    static const float CC3 = 1.0f;

    static float color_table[12][3] = {
        {CC3, CC2, CC2},
        {CC2, CC3, CC2},
        {CC2, CC2, CC3},
        {CC2, CC3, CC3},
        {CC3, CC2, CC3},
        {CC3, CC3, CC2},

        {CC1, CC2, CC2},
        {CC2, CC1, CC2},
        {CC2, CC2, CC1},
        {CC2, CC1, CC1},
        {CC1, CC2, CC1},
        {CC1, CC1, CC2}
    };
}

namespace OGF {

    static index_t random_color_index_ = 0;

    static void gl_random_color() {
	glupColor3fv(color_table[random_color_index_]);
        random_color_index_ = (random_color_index_ + 1) % 12;
    }
    
    static void gl_randomize_colors(index_t index=0) {
        random_color_index_ = (index % 12);
    }

    VoronoiMeshGrobShader::VoronoiMeshGrobShader(
        MeshGrob* grob
    ) : MeshGrobShader(grob) {
        vertices_style_.visible = true;
        vertices_style_.color   = Color(0.0,0.0,0.5,1.0);
        vertices_style_.size    = 2;
	radius_ = 0.0;
	precision_ = 100;
	lighting_ = false;
        square_ = false;
        shift_point_ = 0;
        shift_amount_ = 0; 
    }
        
    VoronoiMeshGrobShader::~VoronoiMeshGrobShader() { 
    }        

    void VoronoiMeshGrobShader::draw() {
        MeshGrobShader::draw();
	
	if(lighting_) {
	    glupEnable(GLUP_LIGHTING);
	} else {
	    glupDisable(GLUP_LIGHTING);
	}

	if(vertices_style_.visible) {
	    glupSetColor3dv(GLUP_FRONT_COLOR, vertices_style_.color.data());
	    glupSetPointSize(float(vertices_style_.size) * 5.0f);
	    glupBegin(GLUP_POINTS);
	    for(index_t v=0; v<mesh_grob()->vertices.nb(); ++v) {
		glupVertex3dv(mesh_grob()->vertices.point_ptr(v));
	    }
	    glupEnd();
	}
	
	if(radius_ == 0.0) {
	    return;
	}
	glupEnable(GLUP_VERTEX_COLORS);
	glupDisable(GLUP_DRAW_MESH);
	double d = 2.0 * M_PI / double(precision_);
	gl_randomize_colors();

        if(square_) {
	    glupBegin(GLUP_QUADS);
	    for(index_t v=0; v<mesh_grob()->vertices.nb(); ++v) {
		vec3 o(mesh_grob()->vertices.point_ptr(v));
		if((v+1) == shift_point_) {
		    o.z += double(shift_amount_)/10.0 * radius_;
		}
                gl_random_color();
                double alpha = 0;
                for(unsigned int j=0; j<precision_; j++) {
                    double r1 = radius_ * double(j)   / double(precision_-1);
                    double r2 = radius_ * double(j+1) / double(precision_-1);
                    for(unsigned int i=0; i<precision_; i++) {
                        double s1 = sin(alpha);     double c1 = cos(alpha);
                        double s2 = sin(alpha+d);   double c2 = cos(alpha+d);
                        vec3 v1(r1 * c1, r1 * s1, -r1*r1);
                        vec3 v2(r1 * c2, r1 * s2, -r1*r1);
                        vec3 w1(r2 * c1, r2 * s1, -r2*r2);
                        vec3 w2(r2 * c2, r2 * s2, -r2*r2);
                        alpha += d;
                        glupVertex(o + v1);
                        glupVertex(o + v2);
                        glupVertex(o + w2);
                        glupVertex(o + w1);
                    }
                }
            }
	    glupEnd();
        } else {
	    glupBegin(GLUP_TRIANGLES);
	    for(index_t v=0; v<mesh_grob()->vertices.nb(); ++v) {
		vec3 o(mesh_grob()->vertices.point_ptr(v));
                gl_random_color();
                double alpha = 0;
                for(unsigned int i=0; i<precision_; i++) {
                    double s1 = sin(alpha);     double c1 = cos(alpha);
                    double s2 = sin(alpha+d);   double c2 = cos(alpha+d);
                    vec3 v1(radius_ * c1, radius_ * s1, -radius_);
                    vec3 v2(radius_ * c2, radius_ * s2, -radius_);
                    alpha += d;
                    glupVertex(o);
                    glupVertex(o + v1);
                    glupVertex(o + v2);
                }
            }
	    glupEnd();
        }
        glupEnable(GLUP_LIGHTING);
	glupDisable(GLUP_VERTEX_COLORS);
    }
}

