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
 * As an exception to the GPL, Graphite can be linked with the following 
 *  (non-GPL) libraries:  Qt, SuperLU, WildMagic and CGAL
 */

#include <OGF/voxel/commands/voxel_grob_attributes_commands.h>
#include <OGF/scene_graph/types/scene_graph.h>

#include <geogram/mesh/mesh_AABB.h>
#include <geogram/third_party/PoissonRecon/poisson_geogram.h>

namespace OGF {

    VoxelGrobAttributesCommands::VoxelGrobAttributesCommands() {
    }

    VoxelGrobAttributesCommands::~VoxelGrobAttributesCommands() {
    }
    
    void VoxelGrobAttributesCommands::init_box_from_object(
        const GrobName& object_name,
        index_t nu,
        index_t nv,
        index_t nw
    ) {
        Box3d bbox;

        if(object_name == "") {
            bbox.xyz_min[0] = 0.0;
            bbox.xyz_min[1] = 0.0;
            bbox.xyz_min[2] = 0.0;
            bbox.xyz_max[0] = 1.0;
            bbox.xyz_max[1] = 1.0;
            bbox.xyz_max[2] = 1.0;
        } else {
            Grob* object = Grob::find(scene_graph(), object_name);
            if(object == nullptr) {
                Logger::err("VoxelGrob") << object_name << ": no such object"
                                         << std::endl;
                return;
            }
            bbox = object->bbox();
        }

        vec3 origin(bbox.x_min(), bbox.y_min(), bbox.z_min());
        vec3 U(bbox.x_max() - bbox.x_min(), 0.0, 0.0);
        vec3 V(0.0, bbox.y_max() - bbox.y_min(), 0.0);
        vec3 W(0.0, 0.0, bbox.z_max() - bbox.z_min());

        voxel_grob()->set_box(origin, U, V, W);
        voxel_grob()->resize(nu, nv, nw);
        
        voxel_grob()->update();
    }

    
    void VoxelGrobAttributesCommands::delete_attribute(
        const std::string& attribute_name
    ) {
        if(!voxel_grob()->attributes().is_defined(attribute_name)) {
            Logger::err("VoxelGrob") << attribute_name << " :no such attribute"
                                     << std::endl;
            return;
        }

        voxel_grob()->attributes().delete_attribute_store(attribute_name);
        voxel_grob()->update();
    }

    void VoxelGrobAttributesCommands::normalize_attribute(
        const std::string& attribute_name, float min_val, float max_val
    ) {
        Attribute<float> attribute;
        attribute.bind_if_is_defined(
            voxel_grob()->attributes(), attribute_name
        );
        if(!attribute.is_bound()) {
            Logger::err("VoxelGrob")
                << attribute_name
                << " no such single-precision floating point attribute"
                << std::endl;
            return;
        }
        float min_v = Numeric::max_float32();        
        float max_v = Numeric::min_float32();
        index_t nuvw = voxel_grob()->nu()*voxel_grob()->nv()*voxel_grob()->nw();
        for(index_t i=0; i<nuvw; ++i) {
            min_v = std::min(min_v, attribute[i]);
            max_v = std::max(max_v, attribute[i]);            
        }
        if(min_v != max_v) {
            for(index_t i=0; i<nuvw; ++i) {
                float v = attribute[i];
                attribute[i] =
                    min_val + (v - min_v)*(max_val - min_val)/(max_v - min_v);
            }
        } else {
            for(index_t i=0; i<nuvw; ++i) {
                attribute[i] = min_val;
            }
        }
        voxel_grob()->update();
    }

    
    void VoxelGrobAttributesCommands::compute_distance_to_surface(
        const MeshGrobName& surface_name, const std::string& attribute_name,
        bool signed_dist
    ) {
        MeshGrob* surface = MeshGrob::find(scene_graph(), surface_name);
        if(surface == nullptr) {
            Logger::err("VoxelGrob") << surface << " : no such MeshGrob"
                                     << std::endl;
            return;
        }

        if(signed_dist && (surface->cells.nb() == 0)) {
            Logger::err("VoxelGrob")
                << "Signed distance needs input shape to be tetrahedralized"
                << std::endl;
            return;
        }
        
        Attribute<float> distance(voxel_grob()->attributes(), attribute_name);

            double su = 1.0 / double(voxel_grob()->nu());
            double sv = 1.0 / double(voxel_grob()->nv());
            double sw = 1.0 / double(voxel_grob()->nw());        
        
        {
            MeshFacetsAABB AABB(*surface);
#if defined(_OPENMP) 	    
   #pragma omp parallel for
#endif	    
            for(index_t w=0; w<voxel_grob()->nw(); ++w) {
                for(index_t v=0; v<voxel_grob()->nv(); ++v) {
                    for(index_t u=0; u<voxel_grob()->nu(); ++u) {
                        double uf = su*(double(u) + 0.5);
                        double vf = sv*(double(v) + 0.5);
                        double wf = sw*(double(w) + 0.5);
                        vec3 p = voxel_grob()->origin() +
                            uf * voxel_grob()->U() +
                            vf * voxel_grob()->V() +
                            wf * voxel_grob()->W() ;
                        double d = AABB.squared_distance(p);
                        distance[voxel_grob()->linear_index(u,v,w)] =
                            float(::sqrt(d));
                    }
                }
            }
        }

        if(signed_dist) {
            MeshCellsAABB AABB(*surface);
#ifdef _OPENMP            
#pragma omp parallel for
#endif            
            for(index_t w=0; w<voxel_grob()->nw(); ++w) {
                for(index_t v=0; v<voxel_grob()->nv(); ++v) {
                    for(index_t u=0; u<voxel_grob()->nu(); ++u) {
                        double uf = su*(double(u) + 0.5);
                        double vf = sv*(double(v) + 0.5);
                        double wf = sw*(double(w) + 0.5);
                        vec3 p = voxel_grob()->origin() +
                            uf * voxel_grob()->U() +
                            vf * voxel_grob()->V() +
                            wf * voxel_grob()->W() ;
                        index_t tet = AABB.containing_tet(p);
                        if(tet != MeshCellsAABB::NO_TET) {
                            distance[voxel_grob()->linear_index(u,v,w)]*=-1.0f;
                        }
                    }
                }
            }
        }
        
        voxel_grob()->update();
    }

    void VoxelGrobAttributesCommands::Poisson_reconstruction(
        const MeshGrobName& points_name,
        const std::string& attribute_name,
        index_t depth,
        const NewMeshGrobName& reconstruction_name
    ) {
        MeshGrob* points = MeshGrob::find(scene_graph(), points_name);
        {
            if(points == nullptr) {
                Logger::err("Poisson") << points_name << " no such MeshGrob"
                                       << std::endl;
                return;
            }
            
            Attribute<double> normal;
            normal.bind_if_is_defined(
                points->vertices.attributes(), "normal"
            );
            
            if(!normal.is_bound()) {
                Logger::err("Poisson") << "Missing \'normal\' vertex attribute"
                                       << std::endl;
                return;
            }
            if(normal.dimension() != 3) {
                Logger::err("Poisson")
                    << "Wrong dimension for \'normal\' vertex attribute"
                    << std::endl;
                Logger::err("Poisson")
                    << "Expected 3 and got " << normal.dimension()
                    << std::endl;
                return;
            }
        }

        Mesh* reconstruction = nullptr;
        Mesh tmp;
        if(reconstruction_name != "") {
            reconstruction =
                MeshGrob::find_or_create(scene_graph(), reconstruction_name);
        } else {
            // TODO: it's a bit stupid, we reconstruct the surface even
            // if we do not use it...
            reconstruction = &tmp;
        }
        reconstruction->clear();

        PoissonReconstruction poisson;
        poisson.set_depth(depth);
        poisson.set_keep_voxel(true);
        poisson.reconstruct(points, reconstruction);

        double len = poisson.box_edge_length();
        vec3 origin = poisson.box_origin();
        vec3 U(len, 0.0, 0.0);
        vec3 V(0.0, len, 0.0);
        vec3 W(0.0, 0.0, len);
        voxel_grob()->set_box(origin, U, V, W);
        
        index_t res = poisson.voxel_resolution();
        voxel_grob()->resize(res,res,res);

        
        Attribute<float> attribute(voxel_grob()->attributes(),attribute_name);
        Memory::copy(
            &attribute[0], poisson.voxel_values(), res*res*res*sizeof(float)
        );
        
        voxel_grob()->update();
    }

    void VoxelGrobAttributesCommands::load_attribute(
	const std::string& attribute,
	const FileName& filename
    ) {
	Attribute<float> attr(voxel_grob()->attributes(),attribute);
	FILE* f = fopen(std::string(filename).c_str(),"rb");
	if(!f) {
	    Logger::err("Voxel") << "Could not open file: "
				 << filename << std::endl;
	    return;
	}
	index_t nb_voxels = voxel_grob()->nu() * voxel_grob()->nv() * voxel_grob()->nw();
	float* attr_vals = &attr[0];
	if(fread(attr_vals, sizeof(float), nb_voxels, f) != nb_voxels) {
	    Logger::err("VoxelGrog") << "Error while reading file (file may be truncated)"
				     << std::endl;
	}
	fclose(f);
        voxel_grob()->update();	
    }
    
    
}
