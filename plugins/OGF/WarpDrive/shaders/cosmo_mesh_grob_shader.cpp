
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
#include <OGF/basic/os/file_manager.h>
#include <geogram/image/image_library.h>

namespace OGF {

    CosmoMeshGrobShader::CosmoMeshGrobShader(
        OGF::MeshGrob* grob
    ) : MeshGrobShader(grob) {
        point_size_ = 0;
        point_weight_ = 50.0;
        log_  = 0.0;
        minx_ = 0.0;
        miny_ = 0.0;
        minz_ = 0.0;
        maxx_ = 1.0;
        maxy_ = 1.0;
        maxz_ = 1.0;
        lock_z_ = false;
        texture_ = 0;
        colormap_style_.colormap_name = "inferno";
    }
        
    CosmoMeshGrobShader::~CosmoMeshGrobShader() {
        if(texture_ != 0) {
	    glDeleteTextures(1, &texture_);
	    texture_ = 0;
	}
    }        

    void CosmoMeshGrobShader::draw() {
        create_or_resize_image_if_needed();
	get_viewing_parameters();
	draw_points();
	draw_image();
        restore_viewing_parameters();
    }

    void CosmoMeshGrobShader::draw_points() {

        // Get colormap image 
        if(colormap_image_.is_null()) {
            std::string filename =
                "icons/colormaps/" +
                std::string(colormap_style_.colormap_name) + ".xpm" ;
            FileManager::instance()->find_file(filename) ;
            colormap_image_ = ImageLibrary::instance()->load_image(filename) ;
        }

        // Clear floating point image
        Memory::clear(
            intensity_image_->base_mem(),
            sizeof(float) *
            intensity_image_->width()*intensity_image_->height()
        );

        // Scale point weight according to number of points in mesh
        float pw = float(
            20.0 *point_weight_ / (
                pow(double(mesh_grob()->vertices.nb()), 0.666)
            )
        );

        // Scale point weight according to window size and zooming factor
        pw *= float(geo_sqr(double(viewport_[3]/1000.0)/double(modelview_[15])));

        
        // Splat the points into the floating-point image
        parallel_for(
            0, mesh_grob()->vertices.nb(),
            [&](index_t v) {
                const double* p = mesh_grob()->vertices.point_ptr(v);

                // Discard points outside of selection window
                if(
                    p[0] < minx_ || p[0] > maxx_ ||
                    p[1] < miny_ || p[1] > maxy_ ||
                    p[2] < minz_ || p[2] > maxz_
                ) {
                    return;
                }

                // Project points onto image, using modelview, projection
                // and viewport transform read from GLUP
                double X,Y,Z;
                glupProject(
                    p[0], p[1], p[2],
                    modelview_, project_, viewport_,
                    &X, &Y, &Z
                );

                // Splat point onto floating point image
                if(point_size_ == 0) {
                    splat(X,Y,pw);
                } else {
                    auto it = point_weights_.begin();
                    int D = int(point_size_);
                    for(int dx = -D; dx <= D; ++dx) {
                        for(int dy = -D; dy <= D; ++dy) {
                            splat(X+double(dx), Y+double(dy), (*it++)*pw);
                        }
                    }
                }
            }
        );

        // Map the floating-point image to colors (could be done by GPU
        // in a shader, but well, it is easier to do that here)
        parallel_for(
            0, image_->height(),
            [&](index_t y) {
                FOR(x, image_->width()) {
                    float g = *intensity_image_->pixel_base_float32_ptr(x,y);
                    geo_clamp(g, 0.0f, 1.0f);
                    if(log_ != 0.0 && g != 0.0f) {
                        g = logf(g * float(log_)+1.0f) / logf(float(log_+1.0));
                    }
                    if(colormap_style_.flip) {
                        g = 1.0f - g;
                    }
                    index_t G = index_t(g*float(colormap_image_->width()-1));
                    Memory::byte* p = image_->pixel_base(x,y);
                    Numeric::uint8* rgb = colormap_image_->pixel_base(G,0);
                    p[0] = rgb[0];
                    p[1] = rgb[1];
                    p[2] = rgb[2];
                    p[3] = 255;
                }
            }
        );
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
            intensity_image_ = new Image(Image::GRAY, Image::FLOAT32, w, h);
	    image_ = new Image(Image::RGBA, Image::BYTE, w, h);
	}
    }

    void CosmoMeshGrobShader::get_viewing_parameters() {
	glGetIntegerv(GL_VIEWPORT, viewport_);
	glupGetMatrixdv(GLUP_MODELVIEW_MATRIX, modelview_);
	glupGetMatrixdv(GLUP_PROJECTION_MATRIX, project_);
    }

    void CosmoMeshGrobShader::restore_viewing_parameters() {
        glViewport(viewport_[0], viewport_[1], viewport_[2], viewport_[3]);
        glupMatrixMode(GLUP_PROJECTION_MATRIX);
        glupLoadMatrixd(project_);
        glupMatrixMode(GLUP_MODELVIEW_MATRIX);
        glupLoadMatrixd(modelview_);
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
        glEnable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
    }

    /**
     * \brief Smoothstep from 0 to R as a function of r
     * \return R if r = 0, 0 if r >= R, and a smooth
     *  interpolation inbetween.
     */
    static inline float smoothstep(float r, float R) {
        if(r > R) {
            return 0.0;
        }
        float x = r/R;
        return R*(1.0f-x*x*(3.0f-2.0f*x));
    }
    
    void CosmoMeshGrobShader::set_splat_size(index_t size) {
        // Computes splatting weights.
        // Used splat is a little radial smoothstep function
        // (cubic interpolation)
        point_weights_.resize(0);
        point_size_ = size;
        if(size > 0) {
            float R = float(point_size_);
            for(int dx = -int(point_size_); dx <= int(point_size_); ++dx) {
                for(int dy = -int(point_size_); dy <= int(point_size_); ++dy) {
                    float r = ::sqrtf(float(dx*dx)+float(dy*dy));
                    point_weights_.push_back(smoothstep(r,R));
                }
            }
            float S = 0;
            for(float w: point_weights_) {
                S += w;
            }
            for(float& w: point_weights_) {
                w /= S;
            }
        }
        update();
    }
    
}
