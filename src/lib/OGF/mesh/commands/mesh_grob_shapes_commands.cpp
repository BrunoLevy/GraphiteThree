/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
 *
 *  This program is free software{} you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation{} either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY{} without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program{} if not, write to the Free Software
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

#include <OGF/mesh/commands/mesh_grob_shapes_commands.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_subdivision.h>
#include <geogram/mesh/mesh_fill_holes.h>

namespace OGF {

    MeshGrobShapesCommands::MeshGrobShapesCommands() {
    }

    MeshGrobShapesCommands::~MeshGrobShapesCommands() {
    }
    
    void MeshGrobShapesCommands::create_square(
        double x1, double y1, double z1,
        double x2, double y2, double z2,
        double x3, double y3, double z3,
        double x4, double y4, double z4
    ) {
        MeshGrob& M = *mesh_grob();
        if(M.vertices.dimension() < 3) {
            Logger::err("Mesh") << "Dimension smaller than 3"
                                << std::endl;
            return;
        }
        index_t v0 = M.vertices.create_vertices(4);
        index_t v1 = v0+1;
        index_t v2 = v0+2;
        index_t v3 = v0+3;
        Geom::mesh_vertex_ref(M,v0) = vec3(x1,y1,z1);
        Geom::mesh_vertex_ref(M,v1) = vec3(x2,y2,z2);
        Geom::mesh_vertex_ref(M,v2) = vec3(x3,y3,z3);
        Geom::mesh_vertex_ref(M,v3) = vec3(x4,y4,z4);        
        
        index_t f = M.facets.create_polygon(4);
        M.facets.set_vertex(f,0,v0);
        M.facets.set_vertex(f,1,v1);
        M.facets.set_vertex(f,2,v2);
        M.facets.set_vertex(f,3,v3);        
        M.update();
    }

    void MeshGrobShapesCommands::create_cube(
        double x1, double y1, double z1,
        double x2, double y2, double z2
    ) {
        MeshGrob& M = *mesh_grob();
        if(M.vertices.dimension() < 3) {
            Logger::err("Mesh") << "Dimension smaller than 3"
                                << std::endl;
            return;
        }

        index_t v0 = M.vertices.create_vertex(vec3(x1,y1,z1).data());
        index_t v1 = M.vertices.create_vertex(vec3(x1,y1,z2).data());
        index_t v2 = M.vertices.create_vertex(vec3(x1,y2,z1).data());
        index_t v3 = M.vertices.create_vertex(vec3(x1,y2,z2).data());
        index_t v4 = M.vertices.create_vertex(vec3(x2,y1,z1).data());
        index_t v5 = M.vertices.create_vertex(vec3(x2,y1,z2).data());
        index_t v6 = M.vertices.create_vertex(vec3(x2,y2,z1).data());        
        index_t v7 = M.vertices.create_vertex(vec3(x2,y2,z2).data());

        M.facets.create_quad(v3,v7,v6,v2);
        M.facets.create_quad(v0,v1,v3,v2);
        M.facets.create_quad(v1,v5,v7,v3);
        M.facets.create_quad(v5,v4,v6,v7);
        M.facets.create_quad(v0,v4,v5,v1);
        M.facets.create_quad(v2,v6,v4,v0);

        M.facets.connect();
        
        M.update();        
    }
    
    void MeshGrobShapesCommands::create_cylinder(
        const vec3& center,
        const vec3& X_axis,
        const vec3& Y_axis,
        const vec3& Z_axis,
        index_t precision
    ) {
        MeshGrob& M = *mesh_grob();        
        if(M.vertices.dimension() < 3) {
            Logger::err("Mesh") << "Dimension smaller than 3"
                                << std::endl;
            return;
        }

        index_t first_v = M.vertices.create_vertices(2*precision);

        for(index_t i=0; i<precision; i++) {
            double alpha = double(i) * 2.0 * M_PI / double(precision);
            double s = ::sin(alpha);
            double c = ::cos(alpha);
            Geom::mesh_vertex_ref(M,first_v+2*i) =
                (center + c * X_axis + s * Y_axis - Z_axis);
            Geom::mesh_vertex_ref(M,first_v+2*i+1) = 
                (center + c * X_axis + s * Y_axis + Z_axis);
        }

        for(index_t i=0; i<precision; i++) {
            index_t j = (i+1) % precision;
            M.facets.create_quad(
                first_v + 2*j,
                first_v + 2*j+1,
                first_v + 2*i+1,
                first_v + 2*i
            );
        }

        M.facets.connect();
        M.update();
    }

    void MeshGrobShapesCommands::create_icosahedron() {
        MeshGrob& M = *mesh_grob();
        if(M.vertices.dimension() < 3) {
            Logger::err("Mesh") << "Dimension smaller than 3"
                                << std::endl;
            return;
        }

        static double points[] = {
            0,          0.0,       1.175571,
            1.051462,   0.0,       0.5257311,
            0.3249197,  1.0,       0.5257311,
            -0.8506508, 0.618034,  0.5257311,
            -0.8506508, -0.618034, 0.5257311,
            0.3249197,  -1.0,      0.5257311,
            0.8506508,  0.618034,  -0.5257311,
            0.8506508,  -0.618034, -0.5257311,
            -0.3249197,  1.0,      -0.5257311,
            -1.051462,   0.0,      -0.5257311,
            -0.3249197, -1.0,      -0.5257311,
            0.0,         0.0,      -1.175571
        };
        
        static index_t facets[] = {
            0,1,2,
            0,2,3,
            0,3,4,
            0,4,5,
            0,5,1,
            1,5,7,
            1,7,6,
            1,6,2,
            2,6,8,
            2,8,3,
            3,8,9,
            3,9,4,
            4,9,10,
            4,10,5,
            5,10,7,
            6,7,11,
            6,11,8,
            7,10,11,
            8,11,9,
            9,11,10,
        };

        index_t first_v = M.vertices.create_vertices(12);
        for(index_t v=0; v<12; ++v) {
            Geom::mesh_vertex_ref(M,first_v+v) =
                vec3(points[3*v], points[3*v+1], points[3*v+2]) ;
        }

        for(index_t f=0; f<20; ++f) {
            M.facets.create_triangle(
                first_v + facets[3*f],
                first_v + facets[3*f+1],
                first_v + facets[3*f+2]
            );
        }
        
        M.facets.connect();
        M.update();
    }

    void MeshGrobShapesCommands::create_sphere(
	index_t precision,
	double radius,
	const vec3& center
    ) {
        if(!mesh_grob()->facets.are_simplices()) {
           tessellate_facets(*mesh_grob(), 3);
	}
	index_t v_begin = mesh_grob()->vertices.nb();
	index_t facets_begin = mesh_grob()->facets.nb();
	create_icosahedron();
	index_t facets_end = mesh_grob()->facets.nb();
	for(index_t i=0; i<precision; ++i) {
	    mesh_split_triangles(*mesh_grob(), facets_begin, facets_end);
	    facets_end = mesh_grob()->facets.nb();	    
	}
	for(index_t v = v_begin; v < mesh_grob()->vertices.nb(); ++v) {
	    vec3& p = Geom::mesh_vertex_ref(*mesh_grob(), v);
	    p = center + radius * normalize(p);
	}
	mesh_grob()->update();
    }

    void MeshGrobShapesCommands::create_ngon(
        const vec3& center,
        double R,
        index_t nb_edges,
	bool triangulate
    ) {
      index_t first_v = mesh_grob()->vertices.create_vertices(nb_edges);
      FOR(i, nb_edges) {
	 double alpha = 2.0 * M_PI * double(i) / double(nb_edges);
	 double x = center.x + R * ::cos(alpha);
	 double y = center.y + R * ::sin(alpha);	 
	 double* p = mesh_grob()->vertices.point_ptr(i+first_v);
	 p[0] = x;
	 p[1] = y;
	 p[2] = 0.0;		
      }
      if(triangulate) {
	for(index_t i=1; i+1<nb_edges; ++i) {
	  mesh_grob()->facets.create_triangle(first_v,first_v+i,first_v+i+1);
	}
	mesh_grob()->facets.connect();
      } else {
	index_t f = mesh_grob()->facets.create_facets(1,nb_edges);
	FOR(i, nb_edges) {
	   mesh_grob()->facets.set_vertex(f,i,first_v+i);
	}
      }
      mesh_grob()->update();
    }

  
    void MeshGrobShapesCommands::create_from_bounding_box(
	const GrobName& grobname, index_t nb_split
    ) {
	Grob* grob = Grob::find(scene_graph(), grobname);
	if(grob == nullptr) {
	    Logger::out("bbox") << grobname << ": no such object"
				<< std::endl;
	    return;
	}
	Box3d box = grob->bbox();
	index_t facets_begin = mesh_grob()->facets.nb();
	index_t facets_end = index_t(-1);
	create_cube(
	    box.x_min(), box.y_min(), box.z_min(),
	    box.x_max(), box.y_max(), box.z_max()
	);
	for(index_t i=0; i<nb_split; ++i) {
	    mesh_split_quads(*mesh_grob(), facets_begin, facets_end);
	    facets_end = mesh_grob()->facets.nb();	    
	}
	mesh_grob()->update();
    }
    
}

