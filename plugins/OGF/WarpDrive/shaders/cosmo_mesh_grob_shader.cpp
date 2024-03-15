
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
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
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: OGF
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
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */
 

#include <OGF/WarpDrive/shaders/cosmo_mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/gom/interpreter/interpreter.h>

namespace OGF {

    CosmoMeshGrobShader::CosmoMeshGrobShader(
        OGF::MeshGrob* grob
    ) : MeshGrobShader(grob) {
        point_size_ = 0.5;
        minx_ = 0.0;
        miny_ = 0.0;
        minz_ = 0.0;
        maxx_ = 1.0;
        maxy_ = 1.0;
        maxz_ = 1.0;
        texture_ = 0;
    }
        
    CosmoMeshGrobShader::~CosmoMeshGrobShader() {
        if(texture_ != 0) {
	    glDeleteTextures(1, &texture_);
	    texture_ = 0;
	}
    }        

    void CosmoMeshGrobShader::draw() {
        create_or_resize_image_if_needed();
	update_viewing_parameters();
	draw_points();
	draw_image();
    }

    void CosmoMeshGrobShader::draw_points() {
        FOR(y, image_->height()) {
            FOR(x, image_->width()) {
                Memory::byte* p = image_->pixel_base(x,y);
                p[0] = 0;
                p[1] = 0;
                p[2] = 0;
                p[3] = 255;
            }
        }
        for(index_t v: mesh_grob()->vertices) {
            const double* p = mesh_grob()->vertices.point_ptr(v);

            if(
                p[0] < minx_ || p[0] > maxx_ ||
                p[1] < miny_ || p[1] > maxy_ ||
                p[2] < minz_ || p[2] > maxz_
            ) {
                continue;
            }
               
            
            double X,Y,Z;
            glupProject(
                p[0], p[1], p[2],
                modelview_,
                project_,
                viewport_,
                &X, &Y, &Z
            );
            if(
                X >= 0.0 && X < double(image_->width()-1) &&
                Y >= 0.0 && Y < double(image_->width()-1)
            ) {
                Memory::byte* p = image_->pixel_base(index_t(X),index_t(Y));
                ++p[0];
                ++p[1];
                ++p[2];
            }
        }
    }

    void CosmoMeshGrobShader::create_or_resize_image_if_needed() {

	// Get window size
	Object* main=Interpreter::instance_by_language("Lua")->
	    resolve_object("main");
	index_t w,h;
	{
	    Any tmp;
	    main->get_property("width",tmp);
	    tmp.get_value(w);
	    main->get_property("height",tmp);	
	    tmp.get_value(h);
	}

	if(image_.is_null() ||
	   image_->width() != w ||
	   image_->height() != h
	) {
	    image_ = new Image(Image::RGBA, Image::BYTE, w, h);
	}
    }

    void CosmoMeshGrobShader::update_viewing_parameters() {
	glGetIntegerv(GL_VIEWPORT, viewport_);
	glupGetMatrixdv(GLUP_MODELVIEW_MATRIX, modelview_);
	glupGetMatrixdv(GLUP_PROJECTION_MATRIX, project_);
    }

    void CosmoMeshGrobShader::draw_image() {
	if(texture_ == 0) {
	    glGenTextures(1, &texture_);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glTexImage2D(
	    GL_TEXTURE_2D, 0, GL_RGBA,
	    GLsizei(image_->width()), GLsizei(image_->height()),
	    0, GL_RGBA, GL_UNSIGNED_BYTE,
	    image_->base_mem()
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glViewport(0, 0, GLsizei(image_->width()), GLsizei(image_->height()));
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	draw_unit_textured_quad();
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
    }
    
}
