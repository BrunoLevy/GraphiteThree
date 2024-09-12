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


#ifndef H_OGF_WARPDRIVE_SHADERS_VORONOI_MESH_GROB_SHADER_H
#define H_OGF_WARPDRIVE_SHADERS_VORONOI_MESH_GROB_SHADER_H

#include <OGF/WarpDrive/common/common.h>
#include <OGF/mesh_gfx/shaders/mesh_grob_shader.h>

namespace OGF {

    gom_class WarpDrive_API VoronoiMeshGrobShader : public MeshGrobShader {

    public:
        VoronoiMeshGrobShader(MeshGrob* grob);
        ~VoronoiMeshGrobShader() override;
        void draw() override;

    gom_properties:

        const PointStyle& get_vertices_style() const {
            return vertices_style_;
        }

        void set_vertices_style(const PointStyle& value) {
            vertices_style_ = value;
            update();
        }


	bool get_lighting() const {
	    return lighting_;
	}

	void set_lighting(bool x) {
	    lighting_ = x;
	    update();
	}

	double get_radius() const {
	    return radius_;
	}

	void set_radius(double x) {
	    radius_ = x;
	    update();
	}

	index_t get_precision() const {
	    return precision_;
	}

	void set_precision(index_t x) {
	    precision_ = x;
	    update();
	}

	void set_square(bool x) {
	    square_ = x;
	    update();
	}

	bool get_square() const {
	    return square_;
	}

	void set_shift_point(index_t x) {
	    shift_point_ = x;
	    update();
	}

	index_t get_shift_point() const {
	    return shift_point_;
	}

	void set_shift_amount(int x) {
	    shift_amount_ = x;
	    update();
	}

	int get_shift_amount() const {
	    return shift_amount_;
	}

    private:
	PointStyle vertices_style_;
	index_t precision_;
	double radius_;
	bool lighting_;
        bool square_;
        index_t shift_point_;
        int shift_amount_;
    };

}
#endif
