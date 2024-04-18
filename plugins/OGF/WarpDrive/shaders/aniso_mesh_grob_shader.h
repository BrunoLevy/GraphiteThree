
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
 

#ifndef OGF_WARPDRIVE_SHADERS_ANISO_MESH_GROB_SHADER
#define OGF_WARPDRIVE_SHADERS_ANISO_MESH_GROB_SHADER

#include <OGF/WarpDrive/common/common.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>

namespace OGF {

    gom_class WarpDrive_API AnisoMeshGrobShader : public MeshGrobShader {
    public:
        AnisoMeshGrobShader(OGF::MeshGrob* grob);
        ~AnisoMeshGrobShader() override;

        void draw() override;

    gom_properties:

        const Color& get_color() const {
            return color_;
        }
        
        void set_color(const Color& x) {
            color_ = x;
            update();
        }

        double get_scaling() const {
            return scaling_;
        }

        void set_scaling(double x) {
            scaling_ = x;
            update();
        }

        bool get_ellipsoids() const {
            return ellipsoids_;
        }

        void set_ellipsoids(bool x) {
            ellipsoids_ = x;
            update();
        }
        
        bool get_points() const {
            return points_;
        }

        void set_points(bool x) {
            points_ = x;
            update();
        }
        
        bool get_V0() const {
            return V0_;
        }

        void set_V0(bool x) {
            V0_ = x;
            update();
        }


        bool get_V1() const {
            return V1_;
        }

        void set_V1(bool x) {
            V1_ = x;
            update();
        }

        bool get_V2() const {
            return V2_;
        }

        void set_V2(bool x) {
            V2_ = x;
            update();
        }

    protected:
        void draw_crosses();
        void draw_ellipsoids();
        
    private:
        Color color_;
        double scaling_;
        bool points_;
        bool ellipsoids_;
        bool V0_;
        bool V1_;
        bool V2_;
        GLuint program_;
    };
}

#endif

