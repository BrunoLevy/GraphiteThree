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
 
#include <OGF/scene_graph_gfx/symbols/checker_sphere.h>
#include <OGF/renderer/context/rendering_context.h>

namespace OGF {

//_________________________________________________________
    
    CheckerSphere::CheckerSphere() : 
        nb_segments_(40),
        checker_size_(5),
        wireframe_(false),
        mesh_color_(1,1,1,1),
        color1_(1,1,1,1),
        color2_(1,0,0,1),
        lighting_(false)
    {
        rendering_context_ = nullptr ;
    }

    CheckerSphere::~CheckerSphere() {
    }
    
    void CheckerSphere::draw(RenderingContext* rendering_ctxt) {

        if(rendering_ctxt != nullptr) {
            rendering_context_ = rendering_ctxt ;
        }
        
        if(rendering_context_ == nullptr || !rendering_context_->initialized()) {
            return ;
        }

        if(lighting_) {
            glupEnable(GLUP_LIGHTING);
        } else {
            glupDisable(GLUP_LIGHTING);
        }

        const index_t nu = nb_segments_ ;
        const index_t nv = nb_segments_ ;
        const double radius = 1.0 ;

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        if(wireframe_) {
            glupSetColor3dv(GLUP_FRONT_AND_BACK_COLOR, mesh_color_.data());
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
            glupEnable(GLUP_VERTEX_COLORS);
            glupEnable(GLUP_VERTEX_NORMALS);            	    
        }

        glupBegin(GLUP_QUADS);
        
        for(index_t u=0; u<nu; u++) {
            double theta1 = double(u)   * 2.0 * M_PI / double(nu) ;
            double theta2 = double(u+1) * 2.0 * M_PI / double(nu) ;
            for(index_t v=0; v<nv; v++) {
                double phi1 = double(v  ) * M_PI / double(nv) - M_PI / 2.0 ;
                double phi2 = double(v+1) * M_PI / double(nv) - M_PI / 2.0 ;
            
                double x11 = radius * cos(theta1) * cos(phi1) ;
                double y11 = radius * sin(theta1) * cos(phi1) ;
                double z11 = radius * sin(phi1) ;
            
                double x12 = radius * cos(theta1) * cos(phi2) ;
                double y12 = radius * sin(theta1) * cos(phi2) ;
                double z12 = radius * sin(phi2) ;
            
                double x21 = radius * cos(theta2) * cos(phi1) ;
                double y21 = radius * sin(theta2) * cos(phi1) ;
                double z21 = radius * sin(phi1) ;
            
                double x22 = radius * cos(theta2) * cos(phi2) ;
                double y22 = radius * sin(theta2) * cos(phi2) ;
                double z22 = radius * sin(phi2) ;
            
                index_t toggle =
                    ((u / checker_size_) ^ (v / checker_size_)) & 1 ;
            
                if(!wireframe_) {
                    if(toggle != 0) {
                        glupColor3dv(color1_.data()) ;
                    } else {
                        glupColor3dv(color2_.data()) ;
                    }
                }

		glupNormal3d(x11,y11,z11);
                glupVertex3d(x11,y11,z11);
                glupNormal3d(x21,y21,z21);		
                glupVertex3d(x21,y21,z21);
                glupNormal3d(x22,y22,z22);		
                glupVertex3d(x22,y22,z22);
                glupNormal3d(x12,y12,z12);		
                glupVertex3d(x12,y12,z12);
            }
        }

        glupEnd();
	glupDisable(GLUP_VERTEX_NORMALS);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    }

//_________________________________________________________

}

