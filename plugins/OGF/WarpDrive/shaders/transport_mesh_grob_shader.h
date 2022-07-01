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
 

#ifndef H_OGF_WARPDRIVE_SHADERS_TRANSPORT_MESH_GROB_SHADER_H
#define H_OGF_WARPDRIVE_SHADERS_TRANSPORT_MESH_GROB_SHADER_H

#include <OGF/WarpDrive/common/common.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>

#define wheel_int int

namespace OGF {

    gom_class WarpDrive_API TransportMeshGrobShader : 
        public MeshGrobShader {
    public:
        TransportMeshGrobShader(MeshGrob* grob);
        ~TransportMeshGrobShader() override;
        void draw() override;

    gom_properties:

	gom_attribute(widget_class,"OGF::Wheel")
        wheel_int get_time() const { 
            return time_; 
        }

	gom_attribute(widget_class,"OGF::Wheel")	
        void set_time(const wheel_int& x) { 
            time_=x; 
            interp_ = double(int(time_)+25.0)/50.0;
            ogf_clamp(interp_, 0.0, 1.0);
            update(); 
        }

	bool get_lighting() const {
	    return lighting_;
	}

	void set_lighting(bool x) {
	    lighting_ = x;
	    update();
	}

        const SurfaceStyle& get_surface_style() const {
            return surface_style_;
        }

        void set_surface_style(const SurfaceStyle& value) { 
            surface_style_ = value;
            update(); 
        }

        bool get_origin() const { 
            return origin_;
        }
        
        void set_origin(bool x) {
            origin_ = x;
	    update();
        }


        bool get_potential() const {
            return potential_;
        }

        void set_potential(bool x) {
            potential_=x; update();
        }

        bool get_conjugate() const {
            return conjugate_;
        }

        void set_conjugate(bool x) {
            conjugate_=x; update();
        }

        bool get_two_d() const {
            return two_d_;
        }

        void set_two_d(bool x) {
            two_d_=x; update();
        }


        const EdgeStyle& get_trajectories() const {
            return trajectories_;
        }

        void set_trajectories(const EdgeStyle& x) {
            trajectories_ = x;
	    update();
        }

        index_t get_skip() const {
	    return skip_ ;
	}

        void set_skip(index_t x) {
	    skip_ = x;
	    update();
	}

    protected:

        vec3 get_point_potential(index_t v) {
            vec3 result(mesh_grob()->vertices.point_ptr(v)) ;
            result.z = interp_*phi_[v];
            if(conjugate_) {
                // TO BE FIXED
                result.z = 
                    0.5*(result.x*result.x+result.y*result.y) - result.z;
            }
            return result;
        }

        vec3 get_point_interp(index_t v) {
	    vec3 p(mesh_grob()->vertices.point_ptr(v));	    
            vec3 result = interp_*morph_[v] + (1.0 - interp_)*p;
            if(two_d_) { 
                result.z = 0.0; 
            }
            return result;
        }

        vec3 get_point(index_t v) {
            return phi_.is_bound() ? 
                get_point_potential(v) : 
                get_point_interp(v);
        }

    protected:
        virtual void draw_surface();
        void draw_trajectories();

    private:
        Attribute<vec3> morph_;
        Attribute<double> phi_;
        wheel_int time_;
        double interp_;
        bool potential_;
        bool conjugate_;
        bool origin_;
        EdgeStyle trajectories_;
        index_t skip_;
        bool two_d_;
	bool lighting_;
	SurfaceStyle surface_style_;
    } ;

}
#endif

