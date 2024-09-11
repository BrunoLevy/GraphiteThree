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


#include <OGF/mesh/commands/mesh_grob_attributes_commands.h>

#include <geogram/image/image.h>
#include <geogram/image/image_library.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/delaunay/LFS.h>
#include <geogram/points/kd_tree.h>
#include <geogram/basic/stopwatch.h>

#include <stack>


namespace OGF {


    MeshGrobAttributesCommands::MeshGrobAttributesCommands() {
    }

    MeshGrobAttributesCommands::~MeshGrobAttributesCommands() {
    }

    void MeshGrobAttributesCommands::create_attribute(
        const std::string& name,
        const std::string& where,
        const std::string& type,
        index_t dimension
    ) {
        MeshElementsFlags where_id = Mesh::name_to_subelements_type(where);
        if(where_id == MESH_NONE) {
            Logger::err("Attributes")
                << where << ": invalid attribute localization"
                << std::endl;
            return;
        }

        MeshSubElementsStore& elts =
            mesh_grob()->get_subelements_by_type(where_id);

        if(elts.attributes().is_defined(name)) {
            Logger::err("Attributes")
                << name << ": already bound attribute"
                << std::endl;
            return;
        }

        if(type == "int32") {
            Attribute<Numeric::int32> attr;
            attr.create_vector_attribute(elts.attributes(),name,dimension);
        } else if(type == "uint32") {
            Attribute<Numeric::uint32> attr;
            attr.create_vector_attribute(elts.attributes(),name,dimension);
        } else if(type == "float64") {
            Attribute<double> attr;
            attr.create_vector_attribute(elts.attributes(),name,dimension);
        } else if(type == "bool") {
            Attribute<bool> attr;
            attr.create_vector_attribute(elts.attributes(),name,dimension);
        } else {
            Logger::err("Attributes")
                << type << ": invalid attribute type"
                << std::endl;
            return;
        }

        show_attribute(where + "." + name);
    }

    void MeshGrobAttributesCommands::delete_attribute(
        const std::string& name
    ) {
        std::string mesh_element_name;
        std::string attribute_name;
        if(!
           String::split_string(name, '.', mesh_element_name, attribute_name)
        ) {
            Logger::err("Mesh") << attribute_name
                                << " :malformed attribute name"
                                << std::endl;
            Logger::err("Mesh")
                << "expected element.name (for instance vertices.distance)"
                << std::endl;
            return;
        }

        MeshElementsFlags subelement =
            Mesh::name_to_subelements_type(mesh_element_name);

        if(subelement == MESH_NONE) {
            Logger::err("Mesh") << mesh_element_name << " :no such mesh element"
                                << std::endl;
            return;
        }

        AttributesManager& mgr =
            mesh_grob()->get_subelements_by_type(subelement).attributes();

        if(!mgr.is_defined(attribute_name)) {
            Logger::err("Mesh") << name << " :no such attribute"
                                << std::endl;
            return;
        }

        if(name == "vertices.point" || name == "vertices.point_fp32") {
            Logger::err("Mesh") << "Cannot delete mesh geometry"
                                << std::endl;
            return;
        }

        mgr.delete_attribute_store(attribute_name);
        mesh_grob()->update();
    }

    /*************************************************************************/

    void MeshGrobAttributesCommands::compute_sub_elements_id(
        MeshElementsFlags what,
        const std::string& attribute_name
    ) {
        MeshSubElementsStore& store =
            mesh_grob()->get_subelements_by_type(what);
        Attribute<index_t> attribute(store.attributes(), attribute_name);
        for(index_t i=0; i<store.nb(); ++i) {
            attribute[i] = i;
        }
	show_attribute(Mesh::subelements_type_to_name(what)+"."+attribute_name);
        mesh_grob()->update();
    }

    void MeshGrobAttributesCommands::compute_vertices_id(
        const std::string& attribute
    ) {
        compute_sub_elements_id(MESH_VERTICES, attribute);
    }

    void MeshGrobAttributesCommands::compute_edges_id(
        const std::string& attribute
    ) {
        compute_sub_elements_id(MESH_EDGES, attribute);
    }

    void MeshGrobAttributesCommands::compute_facets_id(
        const std::string& attribute
    ) {
        compute_sub_elements_id(MESH_FACETS, attribute);
    }

    void MeshGrobAttributesCommands::compute_chart_id(
        const std::string& attribute
    ) {
        Attribute<index_t> chart(
            mesh_grob()->facets.attributes(), attribute
        );
        for(index_t f: mesh_grob()->facets) {
            chart[f] = index_t(-1);
        }
        std::stack<index_t> S;
        index_t cur_chart = 0;
        for(index_t f: mesh_grob()->facets) {
            if(chart[f] == index_t(-1)) {
                chart[f] = cur_chart;
                S.push(f);
                while(!S.empty()) {
                    index_t g = S.top();
                    S.pop();
                    for(
                        index_t le=0;
                        le<mesh_grob()->facets.nb_vertices(g); ++le
                    ) {
                        index_t h = mesh_grob()->facets.adjacent(g,le);
                        if(h != index_t(-1) && chart[h] == index_t(-1)) {
                            chart[h] = cur_chart;
                            S.push(h);
                        }
                    }
                }
                ++cur_chart;
            }
        }
	show_charts(attribute);
    }


    void MeshGrobAttributesCommands::compute_cells_id(
        const std::string& attribute) {
        compute_sub_elements_id(MESH_CELLS, attribute);
    }


    void MeshGrobAttributesCommands::compute_distance_to_surface(
        const MeshGrobName& surface_name,
        const std::string& attribute_name
    ) {
        MeshGrob* surface = MeshGrob::find(scene_graph(), surface_name);
        if(surface == nullptr) {
            Logger::err("MeshGrob") << surface << ": no such MeshGrob"
                                    << std::endl;
            return;
        }
        //   We need to lock the graphics because the AABB will change
        // the order of the surface facets.
        surface->lock_graphics();
        MeshFacetsAABB AABB(*surface);
        Attribute<double> attribute(
            mesh_grob()->vertices.attributes(), attribute_name
        );

	parallel_for(
	    0, mesh_grob()->vertices.nb(),
	    [&attribute, &AABB, this](index_t v) {
		attribute[v] = ::sqrt(
		    AABB.squared_distance(
			vec3(mesh_grob()->vertices.point_ptr(v))
			)
		    );
	    }
	);
        surface->unlock_graphics();
	show_attribute("vertices."+attribute_name);
        mesh_grob()->update();
    }


    void MeshGrobAttributesCommands::compute_local_feature_size(
        const MeshGrobName& surface_name,
        const std::string& attribute_name
    ) {
        MeshGrob* surface = MeshGrob::find(scene_graph(), surface_name);
        if(surface == nullptr) {
            Logger::err("MeshGrob") << surface << ": no such MeshGrob"
                                    << std::endl;
            return;
        }

        LocalFeatureSize LFS(
            surface->vertices.nb(), surface->vertices.point_ptr(0)
        );

        Attribute<double> lfs(
            mesh_grob()->vertices.attributes(), attribute_name
        );

	parallel_for(
	    0, mesh_grob()->vertices.nb(),
	    [&lfs, &LFS, this](index_t v) {
		lfs[v] = ::sqrt(
		    LFS.squared_lfs(mesh_grob()->vertices.point_ptr(v))
		);
	    }
	);
	show_attribute("vertices."+attribute_name);
        mesh_grob()->update();
    }


    namespace {
	vec2 interpolate_tex_coord(
	    Mesh& M, index_t f, const vec3& p,
	    Attribute<double>& tex_coord
	) {
	    geo_assert(M.facets.nb_vertices(f) == 3);

	    // If facet is a triangle, interpolate normals
	    // using barycentric coords in triangle.
	    index_t c0 = M.facets.corners_begin(f);
	    index_t v0 = M.facet_corners.vertex(c0);
	    vec3 p0(M.vertices.point_ptr(v0));
	    index_t c1 = c0+1;
	    index_t v1 = M.facet_corners.vertex(c1);
	    vec3 p1(M.vertices.point_ptr(v1));
	    index_t c2 = c1+1;
	    index_t v2 = M.facet_corners.vertex(c2);
	    vec3 p2(M.vertices.point_ptr(v2));
	    double A =  Geom::triangle_area(p0,  p1, p2);
	    double l0 = Geom::triangle_area(p,  p1, p2)/A;
	    double l1 = Geom::triangle_area(p0, p,  p2)/A;
	    double l2 = Geom::triangle_area(p0, p1, p )/A;
	    return vec2(
		l0 * tex_coord[2*c0  ] +
		l1 * tex_coord[2*c1  ] +
		l2 * tex_coord[2*c2  ] ,
		l0 * tex_coord[2*c0+1] +
		l1 * tex_coord[2*c1+1] +
		l2 * tex_coord[2*c2+1]
	    );
	}
    }


    void MeshGrobAttributesCommands::copy_texture_colors(
	const MeshGrobName& surface_name,
	const ImageFileName& texture_filename,
	bool copy_tex_coords
    ) {
	Image_var texture = ImageLibrary::instance()->load_image(
	    texture_filename
	);
	if(texture.is_null()) {
	    Logger::err("Mesh") << texture << ": could not load"
				<< std::endl;
	    return;
	}

	MeshGrob* surface = MeshGrob::find(scene_graph(), surface_name);
	if(surface == nullptr) {
	    Logger::err("Mesh") << surface_name << " no such surface"
				<< std::endl;
	    return;
	}
	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(
	    surface->facet_corners.attributes(), "tex_coord"
	);
	if(!tex_coord.is_bound()) {
	    Logger::err("Mesh") << "Mesh does not have texture coordinates"
				<< std::endl;
	    return;
	}

	Attribute<double> color;
	color.bind_if_is_defined(mesh_grob()->vertices.attributes(), "color");
	if(!color.is_bound()) {
	    color.create_vector_attribute(
		mesh_grob()->vertices.attributes(), "color", 3
	    );
	}

	Attribute<double> to_tex_coord;
	if(copy_tex_coords) {
	    to_tex_coord.bind_if_is_defined(
		mesh_grob()->vertices.attributes(), "tex_coord"
	    );
	    if(!to_tex_coord.is_bound()) {
		to_tex_coord.create_vector_attribute(
		    mesh_grob()->vertices.attributes(), "tex_coord", 2
		);
	    }
	}

	MeshFacetsAABB AABB(*surface);

	parallel_for(
	    0, mesh_grob()->vertices.nb(),
	    [
		surface, &AABB, &color, &tex_coord, &texture, &to_tex_coord,
		copy_tex_coords, this
	    ](index_t v) {
		vec3 p(mesh_grob()->vertices.point_ptr(v));
		vec3 q;
		double sqdist;
		index_t f = AABB.nearest_facet(p,q,sqdist);
		vec2 uv = interpolate_tex_coord(*surface,f,q,tex_coord);
		geo_clamp(uv.x, 0.0, 1.0);
		geo_clamp(uv.y, 0.0, 1.0);
		Memory::byte* rgb =
		    texture->pixel_base(
			index_t(uv.x * double(texture->width()-1)),
			index_t(uv.y * double(texture->height()-1))
		    );
		if(to_tex_coord.is_bound()) {
		    to_tex_coord[2*v]   = uv.x;
		    to_tex_coord[2*v+1] = uv.y;
		}
		color[3*v  ] = double(rgb[0]/255.0);
		color[3*v+1] = double(rgb[1]/255.0);
		color[3*v+2] = double(rgb[2]/255.0);
	    }
	);

	surface->update();
	show_colors();
	mesh_grob()->update();
    }

/************************************************************************/

    void MeshGrobAttributesCommands::compute_ambient_occlusion(
	const std::string& attribute, index_t nb_rays_per_vertex,
	index_t nb_smoothing_iter
    ) {
	Attribute<double> AO(mesh_grob()->vertices.attributes(), attribute);
	MeshFacetsAABB AABB(*mesh_grob());

	parallel_for(
	    0, mesh_grob()->vertices.nb(),
	    [this,&AABB,&AO,nb_rays_per_vertex](index_t v) {
		double ao = 0.0;
		vec3 p(mesh_grob()->vertices.point_ptr(v));
		for(index_t i=0; i<nb_rays_per_vertex; ++i) {
		    // https://math.stackexchange.com/questions/1585975/
		    //   how-to-generate-random-points-on-a-sphere
		    double u1 = Numeric::random_float64();
		    double u2 = Numeric::random_float64();
		    double theta = 2.0 * M_PI * u2;
		    double phi = acos(2.0 * u1 - 1.0) - M_PI / 2.0;
		    vec3 d(
			cos(theta)*cos(phi),
			sin(theta)*cos(phi),
			sin(phi)
		    );
		    if(!AABB.ray_intersection(Ray(p + 1e-3*d, d))) {
			ao += 1.0;
		    }
		}
		ao /= double(nb_rays_per_vertex);
		AO[v] = ao;
	    }
	);

	vector<double> next_val;
	vector<index_t> degree;
	for(index_t i=0; i<nb_smoothing_iter; ++i) {
	    next_val.assign(mesh_grob()->vertices.nb(),0.0);
	    degree.assign(mesh_grob()->vertices.nb(),1);
	    for(index_t v: mesh_grob()->vertices) {
		next_val[v] = AO[v];
	    }
	    for(index_t f: mesh_grob()->facets) {
		index_t d = mesh_grob()->facets.nb_vertices(f);
		for(index_t lv=0; lv < d; ++lv) {
		    index_t v1 = mesh_grob()->facets.vertex(f,lv);
		    index_t v2 = mesh_grob()->facets.vertex(f,(lv + 1) % d);
		    degree[v1]++;
		    degree[v2]++;
		    next_val[v1] += AO[v2];
		    next_val[v2] += AO[v1];
		}
	    }
	    for(index_t v: mesh_grob()->vertices) {
		AO[v] = next_val[v] / double(degree[v]);
	    }
	}
	show_attribute("vertices."+attribute);
	mesh_grob()->update();
    }


    void MeshGrobAttributesCommands::compute_vertices_normals(
        const std::string& attribute
    ) {
        Attribute<double> N;
        N.bind_if_is_defined(mesh_grob()->vertices.attributes(), attribute);
        if(N.is_bound()) {
            if(N.dimension() != 3) {
                Logger::err("Attributes")
                    << attribute << " already exists with wrong dim "
                    << N.dimension()
                    << std::endl;
                return;
            }
        } else {
            N.create_vector_attribute(
                mesh_grob()->vertices.attributes(), attribute, 3
            );
        }

        N.zero();

        for(index_t f: mesh_grob()->facets) {
            vec3 fN = Geom::mesh_facet_normal(*mesh_grob(), f);
            for(index_t lv=0; lv<mesh_grob()->facets.nb_vertices(f); ++lv) {
                index_t v = mesh_grob()->facets.vertex(f,lv);
                N[3*v]   += fN.x;
                N[3*v+1] += fN.y;
                N[3*v+2] += fN.z;
            }
        }

        for(index_t v: mesh_grob()->vertices) {
            vec3 vN(N[3*v],N[3*v+1],N[3*v+2]);
            vN = normalize(vN);
            N[3*v]   = vN.x;
            N[3*v+1] = vN.y;
            N[3*v+2] = vN.z;
        }

        mesh_grob()->update();
    }

}
