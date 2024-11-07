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
 * As an exception to the GPL, Graphite can be linked with the
 *  following (non-GPL) libraries: Qt, SuperLU, WildMagic and CGAL
 */

#ifndef H_OGF_MESH_INTERFACES_MESH_GROB_EDITOR_INTERFACE_H
#define H_OGF_MESH_INTERFACES_MESH_GROB_EDITOR_INTERFACE_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/grob/mesh_grob.h>
#include <OGF/scene_graph/commands/commands.h>
#include <geogram/basic/attributes.h>

namespace OGF {

    namespace NL {
	class Vector;
    }

    /**
     * \brief A wrapper to script low-level editing operations
     *  on a MeshGrob.
     */
    gom_class MESH_API MeshGrobEditor : public Interface {
      public:
	/**
	 * \brief MeshGrobEditor constructor.
	 */
	MeshGrobEditor();

	/**
	 * \brief MeshGrobEditor destrutor.
	 */
	~MeshGrobEditor() override;

	/**
	 * \brief Gets the wrapped MeshGrob.
	 * \return a pointer to the MeshGrob or nullptr.
	 */
	MeshGrob* mesh_grob() const {
	    return dynamic_cast<MeshGrob*>(grob());
	}

      gom_properties:
	/**
	 * \brief Sets the dimension of the vertices.
	 * \param dim one of 2,3
	 */
	void set_dimension(index_t dim);

	/**
	 * \brief Gets the dimension
	 * \return the dimension of the vertices, one of 2,3
	 */
	index_t get_dimension() const;

	/**
	 * \brief Gets the number of vertices.
	 * \return the number of vertices.
	 */
	index_t get_nb_vertices() const;

	/**
	 * \brief Gets the number of edges.
	 * \return the number of edges explicitly stored
	 *  (facets and cell edges do not count).
	 */
	index_t get_nb_edges() const;

	/**
	 * \brief Gets the number of facets.
	 * \return the number of facets.
	 */
	index_t get_nb_facets() const;

	/**
	 * \brief Gets the number of cells.
	 * \return the number of cells.
	 */
	index_t get_nb_cells() const;

      gom_slots:

	/**
	 * \brief Gets a wrapper around an attribute.
	 * \param[in] attribute_name the name of the attribute, preceded by
	 *  the elements (e.g., "vertices.attr_name", "edges.attr_name" etc...).
	 * \param[in] quiet if true, do not display any error message if the
	 *  attribute does not exist.
	 * \return a pointer to the NL::Vector or nullptr if
	 *  there is no such attribute.
	 */
	NL::Vector* find_attribute(
	    const std::string& attribute_name, bool quiet=false
	);

	/**
	 * \brief Creates an attribute.
	 * \param[in] attribute_name the name of the attribute, preceded by
	 *  the elements (e.g., "vertices.attr_name", "edges.attr_name" etc...).
	 * \param[in] dimension number of elements per item. Default is 1.
	 * \param[in] type optional meta type for the attribute elements.
	 *  Default is double precision number.
	 * \return a wrapper around the newly created attribute.
	 */
	NL::Vector* create_attribute(
	    const std::string& attribute_name,
	    index_t dimension=1, MetaType* type=nullptr
	);

	/**
	 * \brief Creates an attribute.
	 * \param[in] attribute_name the name of the attribute, preceded by
	 *  the elements (e.g., "vertices.attr_name", "edges.attr_name" etc...).
	 * \param[in] dimension number of elements per item. Default is 1.
	 * \param[in] type optional meta type for the attribute elements.
	 *  Default is double precision number.
	 * \return a wrapper around the attribute.
	 */
	NL::Vector* find_or_create_attribute(
	    const std::string& attribute_name,
	    index_t dimension=1, MetaType* type=nullptr
	);

	/**
	 * \brief Tests whether the mesh has an attribute.
	 * \param[in] attribute_name the name of the attribute, preceded by
	 *  the elements (e.g., "vertices.attr_name", "edges.attr_name" etc...).
	 * \retval true if the mesh has an attribute of the specified name.
	 * \retval false otherwise.
	 */
	bool has_attribute(const std::string& attribute_name) const;


        /**
         * \brief Gets the points
	 * \return a pointer to a new NL::Vector referring to
	 *  the array of points
         */
        NL::Vector* get_points() {
            return find_attribute("vertices.point");
        }

	/**
	 * \brief If the surface mesh is triangulated,
	 *   gets the array of triangles.
	 * \return a pointer to a new NL::Vector referring to
	 *  the array of triangles or nullptr if the surface is
	 *  not triangulated.
	 */
	NL::Vector* get_triangles() const;

	/**
	 * \brief If the surface mesh is triangulated, gets
	 *   the array of triangles.
	 * \return a pointer to a new NL::Vector referring to
	 *  the array of adjacent facets of each triangle or
	 *  nullptr if the surface is not triangulated.
	 */
	NL::Vector* get_triangle_adjacents() const;

	/**
	 * \brief If the volume mesh is tetrahedralized,
	 *  gets the array of tetrahedra.
	 * \return a pointer to a new NL::Vector referring to
	 *  the array of tetrahedra or nullptr if the volume mesh
	 *  is not tetrahedralized.
	 */
	NL::Vector* get_tetrahedra() const;

	/**
	 * \brief If the volume mesh is tetrahedralized,
	 *  gets the array of tetrahedra adjacencies.
	 * \return a pointer to a new NL::Vector referring to the array
	 *  of tetrahedra adjacencies or nullptr if the volume mesh is
	 *  not tetrahedralized.
	 */
	NL::Vector* get_tetrahedra_adjacents() const;

	/**
	 * \brief clears this mesh.
	 */
	void clear();

	/**
	 * \brief Creates a new vertex.
	 * \details If this is a 2D mesh, \p z is ignored.
	 * \param[in] x , y , z the coordinates of the vertex.
	 * \return the index of the newly created vertex.
	 */
	index_t create_vertex(double x=0.0, double y=0.0, double z=0.0);

	/**
	 * \brief Sets the coordinates of an existing vertex.
	 * \details If this is a 2D mesh, \p z is ignored.
	 * \param[in] v the index of the vertex, in 0..nb_vertices()-1
	 * \param[in] x , y , z the coordinates of the vertex.
	 */
	void set_vertex(index_t v, double x=0.0, double y=0.0, double z=0.0);

	/**
	 * \brief Creates multiple vertices.
	 * \param[in] nb the number of vertices to create.
	 * \return the index of the first created vertex.
	 */
	index_t create_vertices(index_t nb);


	/**
	 * \brief Creates a facet.
	 * \details The vertices of the facet are left uninitialized.
	 * \param[in] nb_vertices number of vertices in facet.
	 * \return the index of the newly created vertex.
	 */
	index_t create_facet(index_t nb_vertices);

	/**
	 * \brief Creates a triangle.
	 * \param[in] v1 , v2 , v3 the indices of the vertices,
	 *  in 0..nb_vertices()-1
	 * \return the index of the newly created triangle.
	 */
	index_t create_triangle(index_t v1, index_t v2, index_t v3);

	/**
	 * \brief Creates a quad.
	 * \param[in] v1 , v2 , v3 , v4 the indices of the vertices,
	 *  in 0..nb_vertices()-1
	 * \return the index of the newly created quad.
	 */
	index_t create_quad(index_t v1, index_t v2, index_t v3, index_t v4);

	/**
	 * \brief Creates an edge.
	 * \param[in] v1 , v2 the two extremities of the edge, in
	 *  0..nb_vertices()-1
	 * \return the index of the newly created edge.
	 */
	index_t create_edge(index_t v1, index_t v2);


	/**
	 * \brief Creates triangles.
	 * \details The facet vertices are uninitialized.
	 * \param[in] nb_triangles the number of triangles to create.
	 * \return the index of the first newly created triangle.
	 */
	index_t create_triangles(index_t nb_triangles);

	/**
	 * \brief Creates a chunk of quadrilateral facets.
	 * \details The facet vertices are uninitialized.
	 * \param[in] nb_quads the number of quads to create.
	 * \return the index of the first newly created triangle.
	 */
	index_t create_quads(index_t nb_quads);

	/**
	 * \brief Creates a chunk of facets.
	 * \details The facet vertices are uninitialized.
	 * \param[in] nb_facets the number of facets to create.
	 * \param[in] nb_vertices_per_facet number of vertices in
	 *   each facet.
	 * \return the index of the first newly created triangle.
	 */
	index_t create_facets(
	    index_t nb_facets, index_t nb_vertices_per_facet
	);

	/**
	 * \brief Computes facet adjacencies.
	 */
	void connect_facets();


	/**
	 * \brief Gets the number of vertices in a facet.
	 * \param[in] f the index of the facet.
	 * \return the number of vertices in \p f.
	 */
	index_t facet_nb_vertices(index_t f) const;

	/**
	 * \brief Gets a vertex in a facet.
	 * \param[in] f the facet index.
	 * \param[in] lv the local index of the vertex in the facet.
	 * \return the global index of the vertex.
	 */
	index_t facet_vertex(index_t f, index_t lv) const;

	/**
	 * \brief Sets a vertex of the facet.
	 * \param[in] f the facet index.
	 * \param[in] lv the local index of the vertex in the facet.
	 * \param[in] v the global index of the vertex.
	 */
	void set_facet_vertex(index_t f, index_t lv, index_t v);

	/**
	 * \brief Deletes a set of vertices.
	 * \param[in] to_delete a vector of type unsigned int or index_t.
	 * \details to_delete needs to have the same size as the number of
	 *  vertices in the mesh. A non-zero entry in to_delete means that
	 *  the associated vertex should be deleted.
	 */
	void delete_vertices(NL::Vector* to_delete);

	/**
	 * \brief Deletes a set of edges.
	 * \param[in] to_delete a vector of type unsigned int or index_t.
	 * \param[in] delete_isolated_vertices if set, all vertices that
	 *  have no mesh element incident to them are deleted.
	 * \details to_delete needs to have the same size as the number of
	 *  edges in the mesh. A non-zero entry in to_delete means that
	 *  the associated edge should be deleted.
	 */
	void delete_edges(
	    NL::Vector* to_delete, bool delete_isolated_vertices=true
	);

	/**
	 * \brief Deletes a set of facets.
	 * \param[in] delete_isolated_vertices if set, all vertices that
	 *  have no mesh element incident to them are deleted.
	 * \param[in] to_delete a vector of type unsigned int or index_t.
	 * \details to_delete needs to have the same size as the number of
	 *  facets in the mesh. A non-zero entry in to_delete means that
	 *  the associated facet should be deleted.
	 */
	void delete_facets(
	    NL::Vector* to_delete, bool delete_isolated_vertices=true
	);

	/**
	 * \brief Deletes a set of cells.
	 * \param[in] to_delete a vector of type unsigned int or index_t.
	 * \param[in] delete_isolated_vertices if set, all vertices that
	 *  have no mesh element incident to them are deleted.
	 * \details to_delete needs to have the same size as the number of
	 *  cells in the mesh. A non-zero entry in to_delete means that
	 *  the associated cell should be deleted.
	 */
	void delete_cells(
	    NL::Vector* to_delete, bool delete_isolated_vertices=true
	);

      protected:

	/**
	 * \brief Checks whether MeshGrob is valid.
	 * \details Displays an error message if not.
	 * \retval true if MeshGrob is not nullptr.
	 * \retval false otherwise.
	 */
	bool check_mesh_grob() const;

	/**
	 * \brief Checks whether a vertex index is valid.
	 * \details Displays an error message if not.
	 * \retval true if vertex index is valid.
	 * \retval false otherwise.
	 */
	bool check_vertex_index(index_t v) const;

	/**
	 * \brief Checks whether a facet index is valid.
	 * \details Displays an error message if not.
	 * \retval true if facet index is valid.
	 * \retval false otherwise.
	 */
	bool check_facet_index(index_t f) const;

	/**
	 * \brief Redisplays the wrapped MeshGrob.
	 */
	void update();
    };
}

#endif
