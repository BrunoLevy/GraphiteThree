
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
 

#ifndef H__OGF_WARPDRIVE_SHADERS_COSMO_MESH_GROB_SHADER__H
#define H__OGF_WARPDRIVE_SHADERS_COSMO_MESH_GROB_SHADER__H

#include <OGF/WarpDrive/common/common.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>

namespace OGF {

    /**
     * \brief A shader to display cosmological simulations
     * \details Displays a pointset as a density field by
     *  accumulating point splats in software. Multithreaded 
     *  to keep interactivity reasonable. Lags a bit starting
     *  from a few tenth million points.
     */
    gom_class WarpDrive_API CosmoMeshGrobShader : public MeshGrobShader {
    public:
        CosmoMeshGrobShader(OGF::MeshGrob* grob);
        ~CosmoMeshGrobShader() override;

        void draw() override;

    gom_properties:

        void set_splat_size(index_t size);

        index_t get_splat_size() const {
            return point_size_;
        }
        
	gom_attribute(handler, "slider_int")
	gom_attribute(min, "0")
	gom_attribute(max, "100")
        void set_w(index_t point_weight) {
            point_weight_ = double(point_weight);
            update();
        }

        index_t get_w() const {
            return index_t(point_weight_);
        }

	gom_attribute(handler, "slider_int")
	gom_attribute(min, "0")
	gom_attribute(max, "300")
        void set_log(index_t x) {
            log_ = double(x);
            update();
        }

        index_t get_log() const {
            return index_t(log_);
        }

        void set_colormap(const ColormapStyle& x) {
            colormap_style_ = x;
            colormap_image_.reset();
            update();
        }

        const ColormapStyle& get_colormap() const {
            return colormap_style_;
        }

        void set_lock_slab(bool x) {
            lock_z_ = x;
        }

        bool get_lock_slab() const {
            return lock_z_;
        }
        
	gom_attribute(handler, "slider_int")
	gom_attribute(min, "0")
	gom_attribute(max, "100")
        void set_minz(double minz) {
            double d = maxz_ - minz_;
            minz_ = double(minz)/100.0;    
            maxz_ = std::max(maxz_, minz_);
            if(lock_z_) {
                maxz_ = minz_ + d;
                if(maxz_ > 1.0) {
                    minz_ -= (maxz_ - 1.0);
                    maxz_ = 1.0;
                }
            }
            update();
        }

        index_t get_minz() const {
            return index_t(minz_*100.0);
        }


	gom_attribute(handler, "slider_int")
	gom_attribute(min, "0")
	gom_attribute(max, "100")
        void set_maxz(double maxz) {
            double d = maxz_ - minz_;
            maxz_ = double(maxz)/100.0;
            minz_ = std::min(minz_, maxz_);
            if(lock_z_) {
                minz_ = maxz_ - d;
                if(minz_ < 0.0) {
                    maxz_ -= minz_;
                    minz_ = 0.0;
                }
            }
            update();
        }

        index_t get_maxz() const {
            return index_t(maxz_*100.0);
        }

	gom_attribute(handler, "slider_int")
	gom_attribute(min, "0")
	gom_attribute(max, "100")
        void set_miny(double miny) {
            miny_ = double(miny)/100.0;
            maxy_ = std::max(maxy_, miny_);
            update();
        }

        index_t get_miny() const {
            return index_t(miny_*100.0);
        }

	gom_attribute(handler, "slider_int")
	gom_attribute(min, "0")
	gom_attribute(max, "100")
        void set_maxy(double maxy) {
            maxy_ = double(maxy)/100.0;
            miny_ = std::min(miny_, maxy_);
            update();
        }

        index_t get_maxy() const {
            return index_t(maxy_*100.0);
        }
        
	gom_attribute(handler, "slider_int")
	gom_attribute(min, "0")
	gom_attribute(max, "100")
        void set_minx(double minx) {
            minx_ = double(minx)/100.0;
            maxx_ = std::max(maxx_, minx_);
            update();
        }

        index_t get_minx() const {
            return index_t(minx_*100.0);
        }

	gom_attribute(handler, "slider_int")
	gom_attribute(min, "0")
	gom_attribute(max, "100")
        void set_maxx(double maxx) {
            maxx_ = double(maxx)/100.0;
            minx_ = std::min(minx_, maxx_);
            update();
        }

        index_t get_maxx() const {
            return index_t(maxx_*100.0);
        }

    protected:
        /**
	 * \brief Creates the image the first time, then resizes it 
	 *  if the size of the window changed.
	 */
	void create_or_resize_image_if_needed();
	
	/**
	 * \brief Gets the viewing parameters from the current GLUP state.
	 * \details Gets the viewport, modelview and projection matrices.
	 */
	void update_viewing_parameters();

	/**
	 * \brief Draws the generated image on the screen.
	 */
	void draw_image();
        
        /**
         * \brief Draws the points to the image
         */
        void draw_points();

        /**
         * \brief Accumulates a point in the floating point image
         * \param[in] u , v the integer coordinates of the points,
         *  in 0 .. width-1, 1 .. height-1
         * \param[in] val the floating-point value to be accumulated
         */
        void splat(int u, int v, float val) {
            if(
                u < 0 || u >= int(intensity_image_->width()) ||
                v < 0 || v >= int(intensity_image_->height())
            ) {
                return;
            }
            *intensity_image_->pixel_base_float32_ptr(index_t(u),index_t(v))
                += val;
        }

        /**
         * \brief Accumulates a point in the floating point image, and
         *  does sub-pixel interpolation
         * \param[in] u , v the floating-point coordinates of the points,
         *  in 0 .. width-1, 1 .. height-1
         * \param[in] val the floating-point value to be accumulated
         */
        void splat(double u, double v, float val) {
            int iu = int(u);
            int iv = int(v);
            if(colormap_style_.smooth) {
                float uu = float(u - floor(u));
                float uv = float(v - floor(v));
                float lu = 1.0f - uu;
                float lv = 1.0f - uv;
                splat(iu,iv,lu*lv*val);
                splat(iu+1,iv,uu*lv*val);
                splat(iu,iv+1,lu*uv*val);
                splat(iu+1,iv+1,uu*uv*val);
            } else {
                splat(iu,iv,val);
            }
        }

        
    private:
        index_t point_size_;
        vector<float> point_weights_;
        double point_weight_;
        double log_;
        ColormapStyle colormap_style_;
        Image_var colormap_image_;
        bool lock_z_;
        double minz_;
        double maxz_;
        double miny_;
        double maxy_;
        double minx_;
        double maxx_;
        GLUPdouble modelview_[16];
        GLUPdouble project_[16];
        GLUPint viewport_[4];
        Image_var intensity_image_;
        Image_var image_;        
        GLuint texture_;
    };
}

#endif

