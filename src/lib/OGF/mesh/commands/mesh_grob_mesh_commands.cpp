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
 * As an exception to the GPL, Graphite can be linked with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#include <OGF/mesh/commands/mesh_grob_mesh_commands.h>
#include <geogram/mesh/mesh_topology.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_repair.h>

namespace OGF {
    
    MeshGrobMeshCommands::MeshGrobMeshCommands() { 
    }
        
    MeshGrobMeshCommands::~MeshGrobMeshCommands() { 
    }        

    void MeshGrobMeshCommands::display_statistics() {
        mesh_grob()->show_stats("Mesh");
	Logger::out("Mesh") << "bbox min = "
			    << mesh_grob()->bbox().x_min()
			    << ' '
			    << mesh_grob()->bbox().y_min()
			    << ' '	    
			    << mesh_grob()->bbox().z_min()
			    << std::endl;
	Logger::out("Mesh") << "bbox max = "
			    << mesh_grob()->bbox().x_max()
			    << ' '
			    << mesh_grob()->bbox().y_max()
			    << ' '	    
			    << mesh_grob()->bbox().z_max()
			    << std::endl;
	Logger::out("Mesh") << "bbox dim = "
			    <<  mesh_grob()->bbox().x_max() -
	                        mesh_grob()->bbox().x_min()
			    << ' '
			    << mesh_grob()->bbox().y_max() -
	                       mesh_grob()->bbox().y_min()
			    << ' '	    
			    << mesh_grob()->bbox().z_max() -
	                       mesh_grob()->bbox().z_min()	    
			    << std::endl;
    }

    void MeshGrobMeshCommands::display_topology() {
        Logger::out("MeshTopology/surface")
            << "Nb components = "
            << mesh_nb_connected_components(*mesh_grob())
            << std::endl;
        int nb_borders = mesh_nb_borders(*mesh_grob());
        if(nb_borders == -1) {
            Logger::out("MeshTopology/surface")
                << "Surface has non-manifold borders (with bowtie vertex)"
                << std::endl;
        } else {
            Logger::out("MeshTopology/surface")
                << "Nb borders = "
                << mesh_nb_borders(*mesh_grob())
                << std::endl;
        }
        Logger::out("MeshTopology/surface")
            << "Xi = "
            << mesh_Xi(*mesh_grob())
            << std::endl;
    }

    void MeshGrobMeshCommands::copy(
        const std::string& name,
        bool edges,
        bool facets,
        bool cells,
        bool attributes,
        bool remove_isolated_vertices
    ) {
        GEO::MeshElementsFlags what = GEO::MESH_VERTICES;
        if(edges) {
            what = GEO::MeshElementsFlags(what | GEO::MESH_EDGES);
        }
        if(facets) {
            what = GEO::MeshElementsFlags(what | GEO::MESH_FACETS);
        }
        if(cells) {
            what = GEO::MeshElementsFlags(what | GEO::MESH_CELLS);
        }
        MeshGrob* copy = MeshGrob::find_or_create(scene_graph(), name);
        copy->copy(*mesh_grob(), attributes, what);
        if(what == GEO::MESH_VERTICES && remove_isolated_vertices) {
            Logger::warn("copy")
                << "If you copy just the vertices "
                << " and then remove the isolated ones..."
                << std::endl;
            Logger::warn("copy")
                << "... there will be nothing left (keeping them)"
                << std::endl;
        } else {
            if(remove_isolated_vertices) {
                copy->vertices.remove_isolated();
            }
        }
        copy->update();
    }
    
    void MeshGrobMeshCommands::remove_mesh_elements(
        bool vertices, bool edges, bool facets, bool cells,
        bool remove_isolated_vertices
    ) {
        if(vertices) {
            mesh_grob()->clear();
        } else {
            if(facets) {
                mesh_grob()->facets.clear();
            }
            if(edges) {
                mesh_grob()->edges.clear();
            }
            if(cells) {
                mesh_grob()->cells.clear();
            }
            if(remove_isolated_vertices) {
                mesh_grob()->vertices.remove_isolated();
            }
        }
        mesh_grob()->update();
    }

    void MeshGrobMeshCommands::remove_isolated_vertices() {
        mesh_grob()->vertices.remove_isolated();
        mesh_grob()->update();
    }


    void MeshGrobMeshCommands::normalize_mesh(
        double Cx, double Cy, double Cz, double radius
    ) {
        GEO::Mesh& M = *mesh_grob();
        double xyz_min[3];
        double xyz_max[3];
        GEO::get_bbox(M, xyz_min, xyz_max);
        double C[3];
        C[0] = 0.5*(xyz_min[0]+xyz_max[0]);
        C[1] = 0.5*(xyz_min[1]+xyz_max[1]);
        C[2] = 0.5*(xyz_min[2]+xyz_max[2]);        
        double R = 0.0;
        for(index_t v: M.vertices) {
            M.vertices.point_ptr(v)[0] -= C[0];
            M.vertices.point_ptr(v)[1] -= C[1];
            M.vertices.point_ptr(v)[2] -= C[2];
            double cur_R = 0.0;
            cur_R += GEO::geo_sqr(M.vertices.point_ptr(v)[0]);
            cur_R += GEO::geo_sqr(M.vertices.point_ptr(v)[1]);
            cur_R += GEO::geo_sqr(M.vertices.point_ptr(v)[2]);
            cur_R = ::sqrt(cur_R);
            R = std::max(R,cur_R);
        }
        for(index_t v: M.vertices) {
            for(coord_index_t c=0; c<3; ++c) {
                M.vertices.point_ptr(v)[c] *= (radius/R);
            }
            M.vertices.point_ptr(v)[0] += Cx;
            M.vertices.point_ptr(v)[1] += Cy;
            M.vertices.point_ptr(v)[2] += Cz;            
        }
        mesh_grob()->update();
    }

    void MeshGrobMeshCommands::normalize_mesh_box(
	double xmin,
	double ymin,
	double zmin,
	double xmax,
	double ymax,
	double zmax,
	bool uniform
    ) {
	double mesh_xyz_min[3];
	double mesh_xyz_max[3];
	get_bbox(*mesh_grob(), mesh_xyz_min, mesh_xyz_max);
	double scale[3];
	scale[0] = (xmax - xmin) / (mesh_xyz_max[0] - mesh_xyz_min[0]);
	scale[1] = (ymax - ymin) / (mesh_xyz_max[1] - mesh_xyz_min[1]);
	scale[2] = (zmax - zmin) / (mesh_xyz_max[2] - mesh_xyz_min[2]);
	if(uniform) {
	    double min_scale = std::min(std::min(scale[0], scale[1]),scale[2]);
	    scale[0] = min_scale;
	    scale[1] = min_scale;
	    scale[2] = min_scale;
	}
	for(index_t v: mesh_grob()->vertices) {
	    double* p = mesh_grob()->vertices.point_ptr(v);
	    p[0] = xmin + scale[0] * (p[0] - mesh_xyz_min[0]);
	    p[1] = ymin + scale[1] * (p[1] - mesh_xyz_min[1]);
	    p[2] = zmin + scale[2] * (p[2] - mesh_xyz_min[2]);		
	}
	mesh_grob()->update();
    }

    void MeshGrobMeshCommands::append(
        const MeshGrobName& other
    ) {
        MeshGrob* M = MeshGrob::find(scene_graph(), other);
        if(M == mesh_grob()) {
            Logger::err("MeshGrob") << "Cannot append mesh to itself"
                                    << std::endl;
        }
        index_t dim = std::max(
            M->vertices.dimension(),
            mesh_grob()->vertices.dimension()
        );
        mesh_grob()->vertices.set_dimension(dim);
        index_t new_v = mesh_grob()->vertices.create_vertices(M->vertices.nb());
        for(index_t v: M->vertices) {
            for(index_t c=0; c<M->vertices.dimension(); ++c) {
                mesh_grob()->vertices.point_ptr(new_v + v)[c] =
                    M->vertices.point_ptr(v)[c];
            }
        }
        for(index_t f: M->facets) {
            index_t N = M->facets.nb_vertices(f);
            index_t new_f = mesh_grob()->facets.create_polygon(N);
            for(index_t lv=0; lv<N; ++lv) {
                index_t v = M->facets.vertex(f,lv);
                mesh_grob()->facets.set_vertex(new_f, lv, new_v+v);
            }
        }

        mesh_repair(*mesh_grob());

        // TODO: cells
        
        mesh_grob()->update();
    }
    

}

