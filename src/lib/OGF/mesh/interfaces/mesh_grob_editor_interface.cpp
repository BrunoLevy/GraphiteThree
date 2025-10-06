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

#include <OGF/mesh/interfaces/mesh_grob_editor_interface.h>
#include <OGF/scene_graph/NL/vector.h>
#include <OGF/gom/reflection/meta_type.h>
#include <OGF/gom/reflection/meta.h>

namespace OGF {

    /*************************************************************/

    MeshGrobEditor::MeshGrobEditor() {
    }

    MeshGrobEditor::~MeshGrobEditor() {
    }

    void MeshGrobEditor::clear() {
	if(mesh_grob() == nullptr) {
	    Logger::err("MeshEditor") << "No MeshGrob" << std::endl;
	}
	mesh_grob()->clear();
    }

    index_t MeshGrobEditor::get_dimension() const {
	if(mesh_grob() == nullptr) {
	    return 0;
	}
	return mesh_grob()->vertices.dimension();
    }

    void MeshGrobEditor::set_dimension(index_t dim) {
	if(!check_mesh_grob()) {
	    return;
	}
	if(dim < 2 || dim > 3) {
	    Logger::err("MeshEditor") << dim << ": invalid dim" << std::endl;
	}
	mesh_grob()->vertices.set_dimension(dim);
    }

    index_t MeshGrobEditor::get_nb_vertices() const {
	if(!check_mesh_grob()) {
	    return 0;
	}
	return mesh_grob()->vertices.nb();
    }

    index_t MeshGrobEditor::get_nb_edges() const {
	if(!check_mesh_grob()) {
	    return 0;
	}
	return mesh_grob()->edges.nb();
    }

    index_t MeshGrobEditor::get_nb_facets() const {
	if(!check_mesh_grob()) {
	    return 0;
	}
	return mesh_grob()->facets.nb();
    }

    index_t MeshGrobEditor::get_nb_cells() const {
	if(!check_mesh_grob()) {
	    return 0;
	}
	return mesh_grob()->cells.nb();
    }

    bool MeshGrobEditor::get_facets_are_simplices() const {
	if(!check_mesh_grob()) {
	    return false;
	}
	return mesh_grob()->facets.are_simplices();
    }

    bool MeshGrobEditor::get_cells_are_simplices() const {
	if(!check_mesh_grob()) {
	    return false;
	}
	return mesh_grob()->cells.are_simplices();
    }


    index_t MeshGrobEditor::create_vertex(const vec3& V) {
	if(!check_mesh_grob()) {
	    return 0;
	}
	index_t result = mesh_grob()->vertices.create_vertex(V.data());
	update();
	return result;
    }

    void MeshGrobEditor::set_vertex(index_t v, const vec3& V) {
	if(
	    !check_mesh_grob() ||
	    !check_vertex_index(v)
	) {
	    return;
	}
	double* p = mesh_grob()->vertices.point_ptr(v);
	p[0] = V.x;
	p[1] = V.y;
	if(mesh_grob()->vertices.dimension() >= 3) {
	    p[2] = V.z;
	}
	update();
    }

    index_t MeshGrobEditor::create_vertices(index_t nb) {
	if(!check_mesh_grob()) {
	    return 0;
	}
	index_t result = mesh_grob()->vertices.create_vertices(nb);
	update();
	return result;
    }

    index_t MeshGrobEditor::create_facet(index_t nb_vertices) {
        index_t result = mesh_grob()->facets.create_facets(1,nb_vertices);
	for(index_t lv=0; lv<nb_vertices; ++lv) {
	    mesh_grob()->facets.set_vertex(result,lv,0);
	}
	update();
	return result;
    }

    index_t MeshGrobEditor::create_triangle(
       index_t v1, index_t v2, index_t v3
    ) {
	if(
	    !check_mesh_grob() ||
	    !check_vertex_index(v1) ||
	    !check_vertex_index(v2) ||
	    !check_vertex_index(v3)
	) {
	    return 0;
	}
	index_t result = mesh_grob()->facets.create_triangle(v1,v2,v3);
	update();
	return result;
    }

    index_t MeshGrobEditor::create_facets(
	index_t nb_facets, index_t nb_vertices_per_facet
    ) {
	if(!check_mesh_grob()) {
	    return 0;
	}
	index_t result = mesh_grob()->facets.create_facets(
	    nb_facets, nb_vertices_per_facet
	);
	for(index_t f=result; f<result+nb_facets; ++f) {
	    for(index_t le=0; le<nb_vertices_per_facet; ++le) {
		mesh_grob()->facets.set_vertex(f,le,0);
	    }
	}
	update();
	return result;
    }

    index_t MeshGrobEditor::create_triangles(index_t nb_triangles) {
	return create_facets(nb_triangles, 3);
    }

    index_t MeshGrobEditor::create_quads(index_t nb_quads) {
	return create_facets(nb_quads, 4);
    }

    index_t MeshGrobEditor::create_quad(
	index_t v1, index_t v2, index_t v3, index_t v4
    ) {
	if(
	    !check_mesh_grob() ||
	    !check_vertex_index(v1) ||
	    !check_vertex_index(v2) ||
	    !check_vertex_index(v3) ||
	    !check_vertex_index(v4)
	) {
	    return 0;
	}
	index_t result = mesh_grob()->facets.create_quad(v1,v2,v3,v4);
	update();
	return result;
    }

    index_t MeshGrobEditor::create_edge(index_t v1, index_t v2) {
	if(
	    !check_mesh_grob() ||
	    !check_vertex_index(v1) ||
	    !check_vertex_index(v2)
	) {
	    return 0;
	}
	index_t result = mesh_grob()->edges.create_edge(v1,v2);
	update();
	return result;
    }


    index_t MeshGrobEditor::facet_nb_vertices(index_t f) const {
      if(!check_mesh_grob() || !check_facet_index(f)) {
	return 0;
      }
      return mesh_grob()->facets.nb_vertices(f);
    }

    void MeshGrobEditor::set_facet_vertex(index_t f, index_t lv, index_t v) {
	if(!check_mesh_grob() || !check_facet_index(f) ||
	   !check_vertex_index(v)
	) {
	    return;
	}
	if(lv >= mesh_grob()->facets.nb_vertices(f)) {
	    Logger::err("MeshEditor")
		<< "Invalid local vertex index" << std::endl;
	    return;
	}
	mesh_grob()->facets.set_vertex(f,lv,v);
    }

    index_t MeshGrobEditor::facet_vertex(index_t f, index_t lv) const {
	if(!check_mesh_grob() || !check_facet_index(f)) {
	    return index_t(-1);
	}
	if(lv >= mesh_grob()->facets.nb_vertices(f)) {
	    Logger::err("MeshEditor") << "Invalid local vertex index"
				      << std::endl;
	    return index_t(-1);
	}
	return mesh_grob()->facets.vertex(f,lv);
    }

    void MeshGrobEditor::connect_facets() {
	if(!check_mesh_grob()) {
	    return;
	}
	mesh_grob()->facets.connect();
	update();
    }

    /*******************************************************************/

    void MeshGrobEditor::update() {
	if(mesh_grob() != nullptr) {
	    mesh_grob()->update();
	}
    }

    bool MeshGrobEditor::check_mesh_grob() const {
	if(mesh_grob() == nullptr) {
	    Logger::err("MeshGrobEditor") << "No MeshGrob" << std::endl;
	    return false;
	}
	return true;
    }

    bool MeshGrobEditor::check_vertex_index(index_t v) const {
	if(v >= mesh_grob()->vertices.nb()) {
	    Logger::err("MeshGrobEditor") << v << ": invalid vertex index"
					  << std::endl;
	    return false;
	}
	return true;
    }

    bool MeshGrobEditor::check_facet_index(index_t f) const {
	if(f >= mesh_grob()->facets.nb()) {
	    Logger::err("MeshGrobEditor") << f << ": invalid facet index"
					  << std::endl;
	    return false;
	}
	return true;
    }

    /*******************************************************************/

    NL::Vector* MeshGrobEditor::find_attribute(
	const std::string& full_attribute_name, bool quiet
    ) {
	std::string subelements_name;
	std::string attribute_name;
        String::split_string(
            full_attribute_name, '.',
            subelements_name,
            attribute_name
        );
	MeshElementsFlags elt =
	    mesh_grob()->name_to_subelements_type(subelements_name);
	if(elt == MESH_NONE) {
	    if(!quiet) {
		Logger::err("MeshGrobEditor")
		    << subelements_name << " : no such mesh element"
		    << std::endl;
	    }
	    return nullptr;
	}

	AttributesManager& attrmgr =
	    mesh_grob()->get_subelements_by_type(elt).attributes();

	AttributeStore* attrstore =
	    attrmgr.find_attribute_store(attribute_name);

	if(attrstore == nullptr) {
	    if(!quiet) {
		Logger::err("MeshGrobEditor")
		    << full_attribute_name << " : no such attribute"
		    << std::endl;
	    }
	    return nullptr;
	}
	return new NL::Vector(mesh_grob(), attrstore);
    }

    NL::Vector* MeshGrobEditor::create_attribute(
	const std::string& full_attribute_name,
	index_t dimension, MetaType* type
    ) {
	if(type == nullptr) {
	    type = ogf_meta<double>::type();
	}
	std::string subelements_name;
	std::string attribute_name;
        String::split_string(
            full_attribute_name, '.',
            subelements_name,
            attribute_name
        );
	MeshElementsFlags elt =
	    mesh_grob()->name_to_subelements_type(subelements_name);
	if(elt == MESH_NONE) {
	    Logger::err("MeshGrobEditor")
		<< subelements_name << " : no such mesh element"
		<< std::endl;
	    return nullptr;
	}
	AttributesManager& attrmgr =
	    mesh_grob()->get_subelements_by_type(elt).attributes();
	if(attrmgr.is_defined(attribute_name)) {
	    Logger::err("MeshGrobEditor")
		<< full_attribute_name << "already defined." << std::endl;
	    return nullptr;
	}
	AttributeStore* store =
	    AttributeStore::create_attribute_store_by_element_type_name(
		type->name(),dimension
	    );
	if(store == nullptr) {
	    Logger::err("Attribute")
		<< "Could not create an attribute of type "
		<< type->name()
		<< std::endl;
	    return nullptr;
	}
	attrmgr.bind_attribute_store(attribute_name,store);
	return find_attribute(full_attribute_name);
    }

    NL::Vector* MeshGrobEditor::find_or_create_attribute(
	const std::string& attribute_name, index_t dimension, MetaType* type
    ) {
	NL::Vector* result = find_attribute(attribute_name, true);

	if(result == nullptr) {
	    return create_attribute(attribute_name, dimension, type);
	}

	if(type != nullptr && result->get_element_meta_type() != type) {
	    Logger::err("MeshEditor")
		<< "Attribute " << attribute_name
		<< " already exist with different element type"
		<< std::endl;
	    // This decrements the refcount that reaches 0
	    // thus deallocates the object.
	    result->unref();
	    return nullptr;
	}

	if(result->get_dimension() != dimension) {
	    Logger::err("MeshEditor")
		<< "Attribute " << attribute_name
		<< " already exist with different dimension"
		<< std::endl;
	    // This decrements the refcount that reaches 0
	    // thus deallocates the object.
	    result->unref();
	    return nullptr;
	}

	return result;
    }

    bool MeshGrobEditor::has_attribute(
	const std::string& full_attribute_name
    ) const {
	std::string subelements_name;
	std::string attribute_name;
        String::split_string(
            full_attribute_name, '.',
            subelements_name,
            attribute_name
        );
	MeshElementsFlags elt =
	    mesh_grob()->name_to_subelements_type(subelements_name);
	if(elt == MESH_NONE) {
	    Logger::err("MeshGrobEditor")
		<< subelements_name << " : no such mesh element"
		<< std::endl;
	    return false;
	}
	AttributesManager& attrmgr =
	    mesh_grob()->get_subelements_by_type(elt).attributes();
	return attrmgr.is_defined(attribute_name);
    }


    NL::Vector* MeshGrobEditor::get_facet_pointers() const {
	// if facets are triangles, then facet pointers are implicit,
	// so we need to explicitize them
	if(mesh_grob()->facets.are_simplices()) {
            NL::Vector* result = new NL::Vector(
                mesh_grob()->facets.nb()+1, 1, ogf_meta<index_t>::type()
            );
	    for(index_t f=0; f<mesh_grob()->facets.nb()+1; ++f) {
		result->data_index_t()[f] = 3*f;
	    }
	    return result;
	}

        // return pointers to internal storage
	return new NL::Vector(
	    mesh_grob(),
	    mesh_grob()->facets.corners_begin_ptr(0),
	    mesh_grob()->facets.nb()+1,
	    1,
	    ogf_meta<index_t>::type(),
	    true
	);
    }

    NL::Vector* MeshGrobEditor::get_facet_vertices() const {
        // return pointers to internal storage
	return new NL::Vector(
	    mesh_grob(),
	    mesh_grob()->facet_corners.vertex_index_ptr(0),
	    mesh_grob()->facet_corners.nb(),
	    1,
	    ogf_meta<index_t>::type(),
	    true
	);
    }

    NL::Vector* MeshGrobEditor::get_triangles() const {

        // triangulate on-the-fly
        if(!mesh_grob()->facets.are_simplices()) {
            index_t nb_t = 0;
            for(index_t f: mesh_grob()->facets) {
                nb_t += (mesh_grob()->facets.nb_vertices(f) - 2);
            }

            NL::Vector* result = new NL::Vector(
                nb_t, 3, ogf_meta<index_t>::type()
            );

            index_t t = 0;
            for(index_t f: mesh_grob()->facets) {
                index_t v1 = mesh_grob()->facets.vertex(f,0);
                for(
                    index_t lv=1; lv+1<mesh_grob()->facets.nb_vertices(f);
                    ++lv
                ) {
                  index_t v2 = mesh_grob()->facets.vertex(f,lv);
                  index_t v3 = mesh_grob()->facets.vertex(f,lv+1);

                  geo_assert(result->data_index_t() != nullptr);
                  geo_assert(3*t+2 < result->nb_elements());

                  result->data_index_t()[3*t  ] = v1;
                  result->data_index_t()[3*t+1] = v2;
                  result->data_index_t()[3*t+2] = v3;
                  ++t;
                }
            }
            return result;
	}

        // return pointers to internal storage
	return new NL::Vector(
	    mesh_grob(),
	    mesh_grob()->facet_corners.vertex_index_ptr(0),
	    mesh_grob()->facets.nb(),
	    3,
	    ogf_meta<index_t>::type(),
	    true
	);

    }

    NL::Vector* MeshGrobEditor::get_triangle_adjacents() const {
	if(!mesh_grob()->facets.are_simplices()) {
	    Logger::err("MeshGrobEditor")
            << "get_triangles(): facets are not triangulated"
            << std::endl;
	    return nullptr;
	}
	return new NL::Vector(
	    mesh_grob(),
	    mesh_grob()->facet_corners.adjacent_facet_ptr(0),
	    mesh_grob()->facets.nb(),
	    3,
	    ogf_meta<index_t>::type(),
	    true
	);
    }

    NL::Vector* MeshGrobEditor::get_tetrahedra() const {
	if(!mesh_grob()->cells.are_simplices()) {
	    Logger::err("MeshGrobEditor")
		<< "get_tetrahedra(): volume is not tetrahedralized"
		<< std::endl;
	    return nullptr;
	}
	return new NL::Vector(
	    mesh_grob(),
	    mesh_grob()->cell_corners.vertex_index_ptr(0),
	    mesh_grob()->cells.nb(),
	    4,
	    ogf_meta<index_t>::type(),
	    true
	);
    }

    NL::Vector* MeshGrobEditor::get_tetrahedra_adjacents() const {
	if(!mesh_grob()->cells.are_simplices()) {
	    Logger::err("MeshGrobEditor")
		<< "get_tetrahedra_adjacent(): volume is not tetrahedralized"
		<< std::endl;
	    return nullptr;
	}
	return new NL::Vector(
	    mesh_grob(),
	    mesh_grob()->cell_facets.adjacent_cell_ptr(0),
	    mesh_grob()->cells.nb(),
	    4,
	    ogf_meta<index_t>::type(),
	    true
	);
    }


    NL::Vector* MeshGrobEditor::get_cell_vertices() const {
        // return pointers to internal storage
	return new NL::Vector(
	    mesh_grob(),
	    mesh_grob()->cell_corners.vertex_index_ptr(0),
	    mesh_grob()->cell_corners.nb(),
	    1,
	    ogf_meta<index_t>::type(),
	    true
	);
    }

    NL::Vector* MeshGrobEditor::get_cell_pointers() const {
	// if cells are tetrahedra, then cell pointers are implicit,
	// so we need to explicitize them
	if(mesh_grob()->cells.are_simplices()) {
            NL::Vector* result = new NL::Vector(
                mesh_grob()->cells.nb()+1, 1, ogf_meta<index_t>::type()
            );
	    for(index_t c=0; c<mesh_grob()->cells.nb()+1; ++c) {
		result->data_index_t()[c] = 4*c;
	    }
	    return result;
	}

        // return pointers to internal storage
	return new NL::Vector(
	    mesh_grob(),
	    mesh_grob()->cells.cell_ptr_ptr(0),
	    mesh_grob()->cells.nb()+1,
	    1,
	    ogf_meta<index_t>::type(),
	    true
	);
    }

    NL::Vector* MeshGrobEditor::get_cell_types() const {
	NL::Vector* result = new NL::Vector(
	    mesh_grob()->cells.nb(), 1, ogf_meta<index_t>::type()
	);
	for(index_t c=0; c<mesh_grob()->cells.nb()+1; ++c) {
	    result->data_index_t()[c] = index_t(mesh_grob()->cells.type(c));
	}
	return result;
    }

    void MeshGrobEditor::delete_vertices(NL::Vector* to_delete) {
	MetaType* type = to_delete->get_element_meta_type();
	if(
	    type != ogf_meta<unsigned int>::type() &&
	    type != ogf_meta<index_t>::type()
	) {
	    Logger::err("Mesh") << "delete_vertices(): invalid type"
				<< std::endl;
	    return;
	}
	if(to_delete->dimension() != 1) {
	    Logger::err("Mesh") << "delete_vertices(): invalid dim"
				<< std::endl;
	    return;
	}
	if(to_delete->size() != mesh_grob()->vertices.nb()) {
	    Logger::err("Mesh") << "delete_vertices(): invalid size"
				<< std::endl;
	    return;
	}
	vector<index_t> to_delete_as_vector(to_delete->size());
	for(index_t i=0; i<to_delete->size(); ++i) {
	    to_delete_as_vector[i] = to_delete->data_index_t()[i];
	}
	mesh_grob()->vertices.delete_elements(to_delete_as_vector);
	update();
    }

    void MeshGrobEditor::delete_edges(
	NL::Vector* to_delete, bool delete_isolated_vertices
    ) {
	MetaType* type = to_delete->get_element_meta_type();
	if(
	    type != ogf_meta<unsigned int>::type() &&
	    type != ogf_meta<index_t>::type()
	) {
	    Logger::err("Mesh") << "delete_vertices(): invalid type"
				<< std::endl;
	    return;
	}
	if(to_delete->dimension() != 1) {
	    Logger::err("Mesh") << "delete_vertices(): invalid dim"
				<< std::endl;
	    return;
	}
	if(to_delete->size() != mesh_grob()->edges.nb()) {
	    Logger::err("Mesh") << "delete_vertices(): invalid size"
				<< std::endl;
	    return;
	}
	vector<index_t> to_delete_as_vector(to_delete->size());
	for(index_t i=0; i<to_delete->size(); ++i) {
	    to_delete_as_vector[i] = to_delete->data_index_t()[i];
	}
	mesh_grob()->edges.delete_elements(
	    to_delete_as_vector,delete_isolated_vertices
	);
	update();
    }

    void MeshGrobEditor::delete_facets(
	NL::Vector* to_delete, bool delete_isolated_vertices
    ) {
	MetaType* type = to_delete->get_element_meta_type();
	if(
	    type != ogf_meta<unsigned int>::type() &&
	    type != ogf_meta<index_t>::type()
	) {
	    Logger::err("Mesh") << "delete_vertices(): invalid type"
				<< std::endl;
	    return;
	}
	if(to_delete->dimension() != 1) {
	    Logger::err("Mesh") << "delete_vertices(): invalid dim"
				<< std::endl;
	    return;
	}
	if(to_delete->size() != mesh_grob()->facets.nb()) {
	    Logger::err("Mesh") << "delete_vertices(): invalid size"
				<< std::endl;
	    return;
	}
	vector<index_t> to_delete_as_vector(to_delete->size());
	for(index_t i=0; i<to_delete->size(); ++i) {
	    to_delete_as_vector[i] = to_delete->data_index_t()[i];
	}
	mesh_grob()->facets.delete_elements(
	    to_delete_as_vector,delete_isolated_vertices
	);
	update();
    }

    void MeshGrobEditor::delete_cells(
	NL::Vector* to_delete, bool delete_isolated_vertices
    ) {
	MetaType* type = to_delete->get_element_meta_type();
	if(
	    type != ogf_meta<unsigned int>::type() &&
	    type != ogf_meta<index_t>::type()
	) {
	    Logger::err("Mesh") << "delete_vertices(): invalid type"
				<< std::endl;
	    return;
	}
	if(to_delete->dimension() != 1) {
	    Logger::err("Mesh") << "delete_vertices(): invalid dim"
				<< std::endl;
	    return;
	}
	if(to_delete->size() != mesh_grob()->cells.nb()) {
	    Logger::err("Mesh") << "delete_vertices(): invalid size"
				<< std::endl;
	    return;
	}
	vector<index_t> to_delete_as_vector(to_delete->size());
	for(index_t i=0; i<to_delete->size(); ++i) {
	    to_delete_as_vector[i] = to_delete->data_index_t()[i];
	}
	mesh_grob()->cells.delete_elements(
	    to_delete_as_vector,delete_isolated_vertices
	);
	update();
    }

}
