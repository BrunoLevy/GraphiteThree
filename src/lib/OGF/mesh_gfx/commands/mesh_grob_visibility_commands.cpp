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
 * As an exception to the GPL, Graphite can be linked
 *  with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */


#include <OGF/mesh_gfx/commands/mesh_grob_visibility_commands.h>
#include <OGF/mesh_gfx/shaders/mesh_grob_shader.h>
#include <OGF/scene_graph_gfx/tools/scene_graph_tools_manager.h>
#include <OGF/renderer/context/rendering_context.h>

#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>

namespace {
    using namespace OGF;

    /**
     * \brief Creates a random rotation matrix from three parameters.
     * \details Maps three values (x[0], x[1], x[2]) in the range [0,1]
     *  into a 3x3 rotation matrix, M.  Uniformly distributed random variables
     *  x0, x1, and x2 create uniformly distributed random rotation matrices.
     *  To create small uniformly distributed "perturbations", supply
     *  samples in the following ranges.
     *      x[0] in [ 0, d ]
     *      x[1] in [ 0, 1 ]
     *      x[2] in [ 0, d ]
     *  Author: Jim Arvo, 1991
     * \param[in] x array of three parameters.
     * \param[out] M the resulting 9 coefficients of the rotation matrix.
     */
    void rand_rotation( double x[], double M[]) {
	double theta = x[0] * 2.0 * M_PI; // Rotation about the pole (Z).
	double phi   = x[1] * 2.0 * M_PI; // For direction of pole deflection.
	double z     = x[2] * 2.0;        // For magnitude of pole deflection.

	// Compute a vector V used for distributing points over the sphere
	// via the reflection I - V Transpose(V).  This formulation of V
	// will guarantee that if x[1] and x[2] are uniformly distributed,
	// the reflected points will be uniform on the sphere.  Note that V
	// has length sqrt(2) to eliminate the 2 in the Householder matrix.

	double r  = sqrt( z );
	double Vx = sin( phi ) * r;
	double Vy = cos( phi ) * r;
	double Vz = sqrt( 2.0 - z );

	// Compute the row vector S = Transpose(V) * R, where R is a simple
	// rotation by theta about the z-axis.  No need to compute Sz since
	// it's just Vz.

	double st = sin( theta );
	double ct = cos( theta );
	double Sx = Vx * ct - Vy * st;
	double Sy = Vx * st + Vy * ct;

	// Construct the rotation matrix  ( V Transpose(V) - I ) R, which
	// is equivalent to V S - R.

	M[0] = Vx * Sx - ct;
	M[1] = Vx * Sy - st;
	M[2] = Vx * Vz;

	M[3] = Vy * Sx + st;
	M[4] = Vy * Sy - ct;
	M[5] = Vy * Vz;

	M[6] = Vz * Sx;
	M[7] = Vz * Sy;
	M[8] = 1.0 - z;   // This equals Vz * Vz - 1.0
    }

    void rand_rotation(mat4& M) {
	double x[3];
	x[0] = Numeric::random_float64();
	x[1] = Numeric::random_float64();
	x[2] = Numeric::random_float64();
	double MM[9];
	rand_rotation(x,MM);
	M.load_identity();
	M(0,0) = MM[0]; M(0,1) = MM[1]; M(0,2) = MM[2];
	M(1,0) = MM[3]; M(1,1) = MM[4]; M(1,2) = MM[5];
	M(2,0) = MM[6]; M(2,1) = MM[7]; M(2,2) = MM[8];
    }
}

namespace OGF {

    MeshGrobVisibilityCommands::MeshGrobVisibilityCommands() {
    }

    MeshGrobVisibilityCommands::~MeshGrobVisibilityCommands() {
    }


    void MeshGrobVisibilityCommands::compute_facets_visibility(
        index_t nb_views, bool dual_sided
    ) {
        if(!mesh_grob()->facets.are_simplices()) {
            mesh_repair(*mesh_grob(), MeshRepairMode(MESH_REPAIR_TRIANGULATE));
            mesh_grob()->update();
        }

        Box3d B = mesh_grob()->bbox();
        vec3 c = B.center();

        SceneGraphToolsManager* mgr = SceneGraphToolsManager::instance();
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob()->get_shader()
        );

        if(shd == nullptr) {
            Logger::err("Attributes") << "Cannot find shader."
                                      << std::endl;
            return;
        }

        // We need to hide everything that may interfere with
        // facets visibility.
        shd->hide_borders();
        shd->hide_vertices();

        Image image(
            Image::RGBA,
            Image::BYTE,
            mgr->rendering_context()->get_width(),
            mgr->rendering_context()->get_height()
        );


        std::vector<Numeric::int64> histo(mesh_grob()->facets.nb());

	index_t nb_sides = (dual_sided ? 2 : 1);

        for(index_t i=0; i<nb_views; ++i) {
	    for(index_t side=0; side<nb_sides; ++side) {
                mgr->rendering_context()->begin_picking(vec2(0.0, 0.0));
                mgr->rendering_context()->begin_frame();

		if(dual_sided) {
		    glEnable(GL_CULL_FACE);
		}


                glupMultMatrix(mgr->get_focus());
                glupMultMatrix(mesh_grob()->get_obj_to_world_transform()) ;

                mat4 M;
                M.load_identity();
		rand_rotation(M);
		M(3,3) = 2.0; // shrink it a little bit (*0.5) to make it fit in
                              // the screen even when it is rotated.

                glupTranslated(c.x, c.y, c.z);
                glupMultMatrixd(M.data());
                glupTranslated(-c.x, -c.y, -c.z);

		if(dual_sided) {
		    glCullFace((side == 0) ? GL_BACK : GL_FRONT);
		}
		shd->pick(MESH_FACETS);
		if(dual_sided) {
		    glCullFace((side == 0) ? GL_FRONT : GL_BACK);
		    // Cannot use index_t(-1), reserved for "no picking"
		    shd->pick_object(index_t(-2));
		    glDisable(GL_CULL_FACE);
		}

                mgr->rendering_context()->snapshot(&image);
                mgr->rendering_context()->end_frame();
                mgr->rendering_context()->end_picking();

                for(index_t y=0; y<image.height(); ++y) {
                    for(index_t x=0; x<image.width(); ++x) {
                        Memory::byte* pixel = image.pixel_base(x,y);
                        Numeric::uint64 facet =
                            Numeric::uint64(pixel[0])         |
                            (Numeric::uint64(pixel[1]) << 8)  |
                            (Numeric::uint64(pixel[2]) << 16) |
                            (Numeric::uint64(pixel[3]) << 24) ;
                        if(facet < histo.size()) {
                            if(side == 0) {
                                histo[facet] = histo[facet]+Numeric::int64(1);
                            } else {
                                histo[facet] = histo[facet]-Numeric::int64(1);
                            }
                        }
                    }
                }
            }
        }

        Attribute<double> visibility(
            mesh_grob()->facets.attributes(), "visibility"
        );
        for(index_t f=0; f<mesh_grob()->facets.nb(); ++f) {
            visibility[f] =
                double(histo[f]) / (
                    double(nb_views) *
                    Geom::mesh_facet_area(*mesh_grob(), f, 3)
                );
        }
        double vismax = 0.0;
        for(index_t f=0; f<mesh_grob()->facets.nb(); ++f) {
            vismax = std::max(vismax, ::fabs(visibility[f]));
        }
        for(index_t f=0; f<mesh_grob()->facets.nb(); ++f) {
            visibility[f] /= vismax;
        }

	show_attribute("facets.visibility");
    }

}
