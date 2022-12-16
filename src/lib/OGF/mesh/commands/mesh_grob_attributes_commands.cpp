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
#include <OGF/scene_graph/types/scene_graph_tools_manager.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>

#include <geogram/image/image.h>
#include <geogram/image/image_library.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/delaunay/LFS.h>
#include <geogram/points/kd_tree.h>
#include <geogram/points/colocate.h>
#include <geogram/basic/stopwatch.h>

namespace {
    using namespace GEO;
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

    /***********************************************************************/

    void MeshGrobAttributesCommands::hide_selection() {
        hide_attribute();
    }

    void MeshGrobAttributesCommands::select_all() {
        MeshElementsFlags where = visible_selection();
        if(visible_selection() == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        Attribute<bool> selection(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );
        for(index_t i=0; i<selection.size(); ++i) {
            selection[i] = true;
        }
        mesh_grob()->update();
    }

    void MeshGrobAttributesCommands::select_none() {
        MeshElementsFlags where = visible_selection();
        if(visible_selection() == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        Attribute<bool> selection(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );
        for(index_t i=0; i<selection.size(); ++i) {
            selection[i] = false;
        }
        mesh_grob()->update();
    }

    void MeshGrobAttributesCommands::invert_selection() {
        MeshElementsFlags where = visible_selection();
        if(visible_selection() == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        Attribute<bool> selection(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );
        for(index_t i=0; i<selection.size(); ++i) {
            selection[i] = !selection[i];
        }
        mesh_grob()->update();
    }

    void MeshGrobAttributesCommands::remove_selected_elements(
        bool remove_isolated
    ) {
        MeshElementsFlags where = visible_selection();
        if(visible_selection() == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        
        Attribute<bool> selection(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );

        // Particular case: vertices. Delete all edges, facets, cells 
        // incident to a vertex to delete.
        if(where == MESH_VERTICES) {
            {
                vector<index_t> delete_e(mesh_grob()->edges.nb(),0);
                for(index_t e: mesh_grob()->edges) {
                    if(
                        selection[mesh_grob()->edges.vertex(e,0)] ||
                        selection[mesh_grob()->edges.vertex(e,0)]
                    ) {
                        delete_e[e] = 1;
                    }
                }
                mesh_grob()->edges.delete_elements(delete_e, remove_isolated);
            }

            {
                vector<index_t> delete_f(mesh_grob()->facets.nb(),0);
                for(index_t f: mesh_grob()->facets) {
                    for(index_t lv=0;
                        lv<mesh_grob()->facets.nb_vertices(f); ++lv
                    ) {
                        if(selection[mesh_grob()->facets.vertex(f,lv)]) {
                            delete_f[f] = 1;
                            break;
                        }
                    }
                }
                mesh_grob()->facets.delete_elements(delete_f, remove_isolated);
            }

            {
                vector<index_t> delete_c(mesh_grob()->cells.nb(),0);
                for(index_t c: mesh_grob()->cells) {
                    for(index_t lv=0;
                        lv<mesh_grob()->cells.nb_vertices(c); ++lv
                    ) {
                        if(selection[mesh_grob()->cells.vertex(c,lv)]) {
                            delete_c[c] = 1;
                            break;
                        }
                    }
                }
                mesh_grob()->cells.delete_elements(delete_c, remove_isolated);
            }
        }

        vector<index_t> remove_element(selection.size(), 0);
        for(index_t i=0; i<selection.size(); ++i) {
            remove_element[i] = index_t(selection[i]);
        }

        MeshElements& elts = dynamic_cast<MeshElements&>(
            mesh_grob()->get_subelements_by_type(where)
        );
        elts.delete_elements(remove_element, remove_isolated);
        mesh_grob()->update();
    }

    
    void MeshGrobAttributesCommands::show_vertices_selection() {
        Attribute<bool> sel(mesh_grob()->vertices.attributes(),"selection");
        show_attribute("vertices.selection");
    }

    void MeshGrobAttributesCommands::select_vertices_on_surface_border() {
        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );
        for(index_t f: mesh_grob()->facets) {
            for(index_t c: mesh_grob()->facets.corners(f)) {
                if(mesh_grob()->facet_corners.adjacent_facet(c) == NO_FACET) {
                    v_selection[mesh_grob()->facet_corners.vertex(c)] = true;
                }
            }
        }
        show_vertices_selection();
        mesh_grob()->update();        
    }

    void MeshGrobAttributesCommands::unselect_vertices_on_surface_border() {
        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );
        for(index_t f: mesh_grob()->facets) {
            for(index_t c: mesh_grob()->facets.corners(f)) {
                if(mesh_grob()->facet_corners.adjacent_facet(c) == NO_FACET) {
                    v_selection[mesh_grob()->facet_corners.vertex(c)] = false;
                }
            }
        }
        show_vertices_selection();        
        mesh_grob()->update();        
    }

    void MeshGrobAttributesCommands::select_duplicated_vertices(
	double tolerance
    ) {
	vector<index_t> old2new(mesh_grob()->vertices.nb());
	index_t nb_distinct;
	if(tolerance == 0.0) {
	    nb_distinct = Geom::colocate_by_lexico_sort(
		mesh_grob()->vertices.point_ptr(0),
		coord_index_t(mesh_grob()->vertices.dimension()),
		mesh_grob()->vertices.nb(),
		old2new,
		mesh_grob()->vertices.dimension()
	    );
	} else {
	    nb_distinct = Geom::colocate(
		mesh_grob()->vertices.point_ptr(0),
		coord_index_t(mesh_grob()->vertices.dimension()),
		mesh_grob()->vertices.nb(),
		old2new,
		tolerance
	    );
	}

	Logger::out("Colocate") << mesh_grob()->vertices.nb() - nb_distinct
				<< " colocated vertices"
				<< std::endl;
	
	vector<index_t> new_count(mesh_grob()->vertices.nb(),0);
	for(index_t v: mesh_grob()->vertices) {
	    ++new_count[old2new[v]];
	}
        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );
	for(index_t v: mesh_grob()->vertices) {
	    if(new_count[old2new[v]] > 1) {
		v_selection[v] = true;
	    }
	}
        show_vertices_selection();        
	mesh_grob()->update();
    }

    void MeshGrobAttributesCommands::show_facets_selection() {
        Attribute<bool> sel(mesh_grob()->facets.attributes(),"selection");
        show_attribute("facets.selection");
    }

    void MeshGrobAttributesCommands::show_cells_selection() {
        Attribute<bool> sel(mesh_grob()->cells.attributes(),"selection");
        show_attribute("cells.selection");
    }
    
    MeshElementsFlags MeshGrobAttributesCommands::visible_selection() const {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob()->get_shader()
        );
        if(shd == nullptr) {
            return MESH_NONE;
        }

        {
            std::string painting;
            shd->get_property("painting",painting);
            if(painting != "ATTRIBUTE") {
                return MESH_NONE;
            }
        }
        
        std::string full_attribute_name;
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;
        shd->get_property("attribute",full_attribute_name);
        if(!Mesh::parse_attribute_name(
               full_attribute_name,where,attribute_name,component)
          ) {
            return MESH_NONE;
        }
        return where;
    }
    
    /*****************************************************************************/    
    
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
        const std::string& attribute) {
        compute_sub_elements_id(MESH_FACETS, attribute);        
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


    void MeshGrobAttributesCommands::compute_facets_visibility(
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

    
}

