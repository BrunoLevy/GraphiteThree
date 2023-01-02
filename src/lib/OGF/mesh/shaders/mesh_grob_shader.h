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
 * As an exception to the GPL, Graphite can be linked with 
 *  the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_MESH_SHADERS_MESH_GROB_SHADER_H
#define H_OGF_MESH_SHADERS_MESH_GROB_SHADER_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/grob/mesh_grob.h>
#include <OGF/scene_graph/shaders/shader.h>
#include <OGF/scene_graph/types/properties.h>

#include <geogram_gfx/mesh/mesh_gfx.h>


/**
 * \file OGF/mesh/shaders/mesh_grob_shader.h
 * \brief Classes for drawing and picking MeshGrob.
 */

namespace GEO {
    class Image;
}


namespace OGF {

    /**********************************************************/
    
    class Builder;
    class Texture;

    enum CullingMode {NO_CULL, CULL_FRONT, CULL_BACK};

    enum PaintingMode {SOLID_COLOR, ATTRIBUTE, COLOR, TEXTURE};
    
    /**
     * \brief Base class for drawing and picking MeshGrob.
     */
    gom_attribute(abstract, "true") 
    gom_class MESH_API MeshGrobShader : public Shader {
    public:
        /**
         * \brief MeshGrobShader constructor.
         * \param[in] grob a pointer to the MeshGrob this shader is attached to
         */
        MeshGrobShader(MeshGrob* grob);

        /**
         * \brief MeshGrobShader destructor.
         */
        ~MeshGrobShader() override;

        /**
         * \copydoc Shader::draw()
         */
        void draw() override;

        /**
         * \brief Picks an element of a mesh.
	 * \param[in] what the type of mesh element
	 *  to be picked.
	 * \details Base class implementation does nothing.
         */
        virtual void pick(MeshElementsFlags what);

        /**
         * \copydoc Shader::pick_object()
	 * \details Base class implementation does nothing.
         */
        void pick_object(index_t object_id) override;

        /**
         * \copydoc Shader::blink()
         */
        void blink() override;

        /**
         * \brief Makes vertices visible and triggers a drawing event.
         */
        virtual void show_vertices();

        /**
         * \brief Makes vertices invisible and triggers a drawing event.
         */
        virtual void hide_vertices();

        /**
         * \brief Makes vertices selection visible and triggers a drawing event.
         */
        virtual void show_vertices_selection();

        /**
         * \brief Makes vertices selection invisible and 
         *  triggers a drawing event.
         */
        virtual void hide_vertices_selection();
        
        /**
         * \brief Makes the mesh visible and triggers a drawing event.
         */
        virtual void show_mesh();

        /**
         * \brief Makes the mesh invisible and triggers a drawing event.
         */
        virtual void hide_mesh();

        /**
         * \brief Makes surface borders visible and triggers a drawing event.
         */
        virtual void show_borders();

        /**
         * \brief Makes surface borders invisible and triggers a drawing event.
         */
        virtual void hide_borders();

    gom_properties:

        /**
         * \brief Tests whether the MeshGrob attached to this shader has edges.
         * \details This is used by the GUI that makes some widgets visible only
         *  if the mesh has the corresponding elements.
         * \retval true if the mesh has edges
         * \retval false otherwise
         */
        bool get_has_edges() const {
            return
                mesh_grob() != nullptr &&
                mesh_grob()->edges.nb() != 0;
        }

        /**
         * \brief Tests whether the MeshGrob attached to this shader has facets.
         * \details This is used by the GUI that makes some widgets visible only
         *  if the mesh has the corresponding elements.
         * \retval true if the mesh has facets
         * \retval false otherwise
         */
        bool get_has_facets() const {
            return            
                mesh_grob() != nullptr &&
                mesh_grob()->facets.nb() != 0;
        }

        /**
         * \brief Tests whether the MeshGrob attached to this shader has cells.
         * \details This is used by the GUI that makes some widgets visible only
         *  if the mesh has the corresponding elements.
         * \retval true if the mesh has cells
         * \retval false otherwise
         */
        bool get_has_cells() const {
            return            
                mesh_grob() != nullptr &&
                mesh_grob()->cells.nb() != 0;
        }

        /**
         * \brief Tests whether the MeshGrob attached to this shader is 
         *  a volumetric hybrid mesh.
         * \details This is used by the GUI that makes some widgets visible only
         *  if the mesh has the corresponding elements.
         * \retval true if the mesh is a volumetric hybrid mesh
         * \retval false otherwise
         */
        bool get_is_hybrid() const {
            return
                mesh_grob() != nullptr &&
                mesh_grob()->cells.nb() != 0 &&
                !mesh_grob()->cells.are_simplices();
        }

        /**
         * \brief Tests whether the MeshGrob attached to this shader stores
         *  a time evolution.
         * \details Time evolution can be stored as 6d coordinates 
         *  (coordinates 4,5,6 correspond to location at final time). 
         *  This is used by the GUI that makes some widgets visible only
         *  if the mesh has the corresponding elements.
         * \retval true if the mesh stores a time evolution
         * \retval false otherwise
         */
        bool get_has_time() const {
            return            
                mesh_grob() != nullptr &&
                mesh_grob()->vertices.dimension() >= 6;
        }

        bool get_has_vertices_selection() const {
            return mesh_grob() != nullptr &&
                mesh_grob()->vertices.attributes().is_defined("selection");
        }


        bool get_has_weird_cells() const {
            return mesh_grob() != nullptr &&
                mesh_grob()->cells.attributes().is_defined("weird");
        }

        std::string get_scalar_attributes() const {
            std::string result;
            if(mesh_grob() != nullptr) {
                result = mesh_grob()->get_scalar_attributes();
            }
            return result;
        }

	std::string get_vector_attributes() const {
	    std::string result;
            if(mesh_grob() != nullptr) {
                result = mesh_grob()->get_vector_attributes(3);
            }
            return result;
	}
	
    protected:
        /**
         * \brief Gets the MeshGrob.
         * \return a pointer to the MeshGrob this shader is attached to
         */
        MeshGrob* mesh_grob() const {
            return static_cast<MeshGrob*>(grob());
        }
    };

    /****************************************************************/

    /**
     * \brief The default implementation of MeshGrobShader
     */
    gom_class MESH_API PlainMeshGrobShader : public MeshGrobShader {
    public:
        /**
         * \brief PlainMeshGrobShader constructor.
         * \param[in] grob a pointer to the MeshGrob this shader is attached to
         */
        PlainMeshGrobShader(MeshGrob* grob);

        /**
         * \brief PlainMeshGrobShader destructor.
         */
         ~PlainMeshGrobShader() override;

        /**
         * \copydoc MeshGrobShader::draw()
         */
	void draw() override;

        /**
         * \copydoc MeshGrobShader::pick()
         */
	void pick(MeshElementsFlags what) override;

        /**
         * \copydoc MeshGrobShader::pick_object()
         */
        void pick_object(index_t object_id) override;

        /**
         * \copydoc MeshGrobShader::blink()
         */
        void blink() override;

        /**
         * \copydoc MeshGrobShader::show_vertices()
         */
        void show_vertices() override;

        /**
         * \copydoc MeshGrobShader::hide_vertices()
         */
        void hide_vertices() override;

        /**
         * \copydoc MeshGrobShader::show_vertices_selection()
         */
	void show_vertices_selection() override;

        /**
         * \copydoc MeshGrobShader::hide_vertices_selection()
         */
        void hide_vertices_selection() override;
        
        /**
         * \copydoc MeshGrobShader::show_mesh()
         */
         void show_mesh() override;

        /**
         * \copydoc MeshGrobShader::hide_mesh()
         */
	 void hide_mesh() override;

        /**
         * \copydoc MeshGrobShader::show_borders()
         */
	 void show_borders() override;

        /**
         * \copydoc MeshGrobShader::hide_borders()
         */
	 void hide_borders() override;

    gom_properties:

        /**
         * \brief Tests whether attributes should be displayed.
         * \retval true if attributes should be displayed
         * \retval false otherwise
         */
        bool get_attributes() const {
            return (painting_mode_ == ATTRIBUTE);
        }

    gom_slots:

        /**
         * \brief Sets the displayed attribute range automatically.
         */
        gom_attribute(visible_if, "attributes")        
        void autorange();


	/**
	 * \brief Sets the time with a floating point
	 *  parameter.
	 * \param[in] x the time, as a floating point
	 *  number betwee 0.0 and 1.0.
	 */
	void set_time_smooth(double x) {
	    time_ = x;
	    update();
	}

	/**
	 * \brief Gets the time as floating point
	 *  parameter.
	 * \return the time, as a floating point
	 *  number betwee 0.0 and 1.0.
	 */
	double get_time_smooth() const {
	    return time_;
	}
	
    gom_properties:

	/**
	 * \brief Sets painting mode.
	 * \param[in] value one of SOLID_COLOR, ATTRIBUTE, TEXTURE.
	 */
	void set_painting(PaintingMode value);

	/**
	 * \brief Gets painting mode.
	 * \return one of SOLID_COLOR, ATTRIBUTE, TEXTURE.
	 */
	PaintingMode get_painting() const {
	    return painting_mode_;
	}
	
        /**
         * \brief Sets the minimum of the displayed range for
         *  attribute values.
         * \param[in] value the minimum attribute value
         */
        gom_attribute(visible_if, "attributes")                            
        void set_attribute_min(double value) {
            attribute_min_ = value;
            update();
        }

        /**
         * \brief Gets the minimum of the displayed range for
         *  attribute values.
         * \return the minimum attribute value
         */
        double get_attribute_min() const {
            return attribute_min_;
        }

        /**
         * \brief Sets the maximum of the displayed range for
         *  attribute values.
         * \param[in] value the maximum attribute value
         */
        gom_attribute(visible_if, "attributes")         
        void set_attribute_max(double value) {
            attribute_max_ = value;
            update();
        }

        /**
         * \brief Gets the maximum of the displayed range for
         *  attribute values.
         * \return the maximum attribute value
         */
        double get_attribute_max() const {
            return attribute_max_;
        }

        
        /**
         * \brief Sets the name fo the attribute to be displayed.
         * \param[in] value the name of the attribute to be displayed
         */
        gom_attribute(visible_if, "attributes")
	gom_attribute(handler, "combo_box")
	gom_attribute(values, "$scalar_attributes")
        void set_attribute(const std::string& value);

        /**
         * \brief Gets the name of the displayed attribute.
         * \return the name of the displayed attribute
         */
        const std::string& get_attribute() const {
            return attribute_;
        }

        /**
         * \brief Sets the colormap used to display attributes.
         * \param[in] value the name of the colormap
         */
        gom_attribute(visible_if, "attributes")
        void set_colormap(const ColormapStyle& value) {
            colormap_style_ = value;
            colormap_texture_.reset();
            update();
        }

        const ColormapStyle& get_colormap() const {
            return colormap_style_;
        }

        /**
         * \brief Tests whether texturing is active.
         * \retval true if textures should be displayed
         * \retval false otherwise
         */
        bool get_texturing() const {
            return (painting_mode_ == TEXTURE);
        }

        /**
         * \brief Tests whether coloring is active.
         * \retval true if colors should be displayed
         * \retval false otherwise
         */
        bool get_coloring() const {
            return (painting_mode_ == COLOR);
        }

	
	/**
	 * \brief Sets the image used for texturing.
	 * \param[in] value the name of the file with the
	 *  image used for texturing.
	 */
        gom_attribute(visible_if, "texturing")                    	
	void set_tex_image(const ImageFileName& value) {
	    texture_filename_ = value;
	    texture_.reset();
	    update();
	}

	/**
	 * \brief Gets the image used for texturing.
	 * \return The name of the file with the image
	 *  used for texturing.
	 */
	const ImageFileName& get_tex_image() const {
	    return texture_filename_;
	}
	
        /**
         * \brief Sets the name fo the attribute to be displayed.
         * \param[in] value the name of the attribute to be displayed
         */
        gom_attribute(visible_if, "texturing")
	gom_attribute(handler, "combo_box")
	gom_attribute(values, "$vector_attributes")
        void set_tex_coords(const std::string& value);

        /**
         * \brief Gets the name of the displayed attribute.
         * \return the name of the displayed attribute
         */
        const std::string& get_tex_coords() const {
            return tex_coord_attribute_;
        }


        /**
         * \brief Sets the name fo the color attribute to be displayed.
         * \param[in] value the name of the color attribute to be displayed
         */
        gom_attribute(visible_if, "coloring")
	gom_attribute(handler, "combo_box")
	gom_attribute(values, "$vector_attributes")
	void set_colors(const std::string& value) {
	    // It is an alias of tex_coord, because we use
	    // 3D texturing internally to display colors
	    // (it is stupid, but makes code simpler for now.
	    set_tex_coords(value);
	}

        /**
         * \brief Gets the name of the displayed attribute.
         * \return the name of the displayed attribute
         */
        const std::string& get_colors() const {
	    // It is an alias of tex_coord, because we use
	    // 3D texturing internally to display colors
	    // (it is stupid, but makes code simpler for now.
	    return get_tex_coords();
        }
	
	/**
	 * \brief Sets the texture repeat factor.
	 * \param[in] value the number of times the texture
	 *  should be repeated in the unit texture space.
	 */
        gom_attribute(visible_if, "texturing")                    
	void set_tex_repeat(index_t value) {
	    tex_coord_repeat_ = value;
	    update();
	}

	/**
	 * \brief Gets the texture repeat factor.
	 * \return The number of times the texture is repeated
	 *  in the unit texture space.
	 */
	index_t get_tex_repeat() const {
	    return tex_coord_repeat_;
	}



	/**
	 * \brief Sets normal mapping mode.
	 * \param[in] value true if normal mapping mode should
	 *  be used, false otherwise.
	 */
	gom_attribute(visible_if, "texturing")	
	void set_normal_map(bool value) {
	    tex_normal_mapping_ = value;
	    update();
	}

	/**
	 * \brief Gets normal mapping mode.
	 * \retval true if normal mapping is active.
	 * \retval false otherwise.
	 */
	bool get_normal_map() const {
	    return tex_normal_mapping_;
	}
	
        /**
         * \brief Sets whether lighting should be used.
         * \param[in] value true if lighting is enabled, false
         *  otherwise
         */
        void set_lighting(bool value) {
            gfx_.set_lighting(value);
            update();
        }

        /**
         * \brief Gets whether lighting is used.
         * \retval true if lighting is used
         * \retval false otherwise
         */
        bool get_lighting() const {
            return gfx_.get_lighting();
        }


        /**
         * \brief Sets whether clipping should
         *  be active.
         * \param[in] value true if clipping should
         *  be used, false otherwise.
         */
        void set_clipping(bool value) {
            clipping_ = value;
            update();
        }

        /**
         * \brief Gets whether clipping should be
         *  used.
         * \retval true if clipping is used
         * \retval false otherwise
         */
        bool get_clipping() const {
            return clipping_;
        }

        
        /**
         * \brief Sets surface drawing style.
         * \param[in] value a const reference to the SurfaceStyle
         */
        gom_attribute(visible_if, "has_facets")
        void set_surface_style(const SurfaceStyle& value) { 
            surface_style_ = value;
            update(); 
        }

        /**
         * \brief Gets the current surface drawing style.
         * \return a const reference to the current surface drawing style.
         */
        const SurfaceStyle& get_surface_style() const {
            return surface_style_;
        }

        /**
         * \brief Sets facet filtering
         * \param[in] value true if facet filtering is activated
         */
        gom_attribute(visible_if, "has_facets")
        void set_facets_filter(bool value) { 
            facets_filter_ = value;
            gfx_.set_filter(
                MESH_FACETS, value ? "filter" : ""
            );
            update(); 
        }

        bool get_facets_filter() const {
            return facets_filter_;
        }
        
        /**
         * \brief Sets culling mode;
         * \param[in] value one of NO_CULL, CULL_FRONT, CULL_BACK
         */
        gom_attribute(visible_if, "has_facets")
        void set_culling_mode(const CullingMode value) { 
	    culling_mode_ = value;
            update(); 
        }

        /**
         * \brief Gets the current culling mode.
         * \return the culling mode.
         */
        CullingMode get_culling_mode() const {
            return culling_mode_;
        }

	/**
	 * \brief Sets the current surface specular factor.
	 * \param[in] x specular factor, use 0 for Lambert shading, 10 for
	 *  highly specular shading.
	 */
        gom_attribute(visible_if, "has_facets or has_cells")	
	void set_specular(index_t x) {
	    specular_ = x;
	    update();
	}

	/**
	 * \brief Gets the current surface specular factor.
	 * \return the specular factor.
	 */
	index_t get_specular() const {
	    return specular_;
	}

        /**
         * \brief Sets whether different colors should be used for 
         *  inside/outside.
         * \details For drawing the inside, the complementary color of the one
         *  declared in the surface style is used (each color component is 
         *  replaced with 1.0 minus the value of the component). If color is 
         *  average gray, it is replaced with light gray outside / dark gray 
         *  inside so that one can see a difference.
         * \param[in] value true if different colors should be used, 
         *  false otherwise
         */
        gom_attribute(visible_if, "has_facets")
        void set_two_sided(bool value) {
            two_sided_ = value;
            update();
        }

        /**
         * \brief Tests whether different colors are used for inside/outside.
         * \retval true if different colors are used
         * \retval false otherwise
         */
        bool get_two_sided() const {
            return two_sided_;
        }

        /**
         * \brief Sets the style of the volumetric cells.
         * \param[in] value a const reference to the style used to
         *  draw the volumetric cells
         */
        gom_attribute(visible_if, "has_cells")        
        void set_volume_style(const SurfaceStyle& value) { 
            volume_style_ = value;
            update(); 
        }

        /**
         * \brief Gets the style used to draw the volumetric
         *  cells.
         * \return a const reference to the style used to 
         *  draw the volumetric cells
         */
        const SurfaceStyle& get_volume_style() const {
            return volume_style_;
        }


        /**
         * \brief Sets cells filtering
         * \param[in] value true if facet filtering is activated
         */
        gom_attribute(visible_if, "has_cells")
        void set_cells_filter(bool value) {
            gfx_.set_filter(
                MESH_CELLS, value ? "filter" : ""
            );
            cells_filter_ = value;
            update(); 
        }

        bool get_cells_filter() const {
            return cells_filter_;
        }

        
        /**
         * \brief Sets whether volumetric cells should be colored
         *  by type.
         * \param[in] value true if volumetric cells should be colored
         *  by type, false otherwise.
         */
        gom_attribute(visible_if, "is_hybrid")
        void set_colored_cells(bool value) {
            colored_cells_ = value;
            update();
        }

        /**
         * \brief Tests whether volumetric cells are colored by type.
         * \retval true if volumetric cells are colored by type
         * \retval false otherwise
         */
        bool get_colored_cells() const {
            return colored_cells_;
        }


        /**
         * \brief Sets whether tetrahedra should be displayed.
         * \param[in] value true if tetrahedra should be displayed,
         *  false otherwise
         */
        gom_attribute(visible_if, "is_hybrid")
        void set_tets(bool value) {
            tets_ = value;
            update();
        }

        /**
         * \brief Tests whether tetrahedra are displayed
         * \retval true if tetrahedra are displayed
         * \retval false otherwise
         */
        bool get_tets() const {
            return tets_;
        }

        /**
         * \brief Sets whether hexahedra should be displayed.
         * \param[in] value true if hexahedra should be displayed,
         *  false otherwise
         */
        gom_attribute(visible_if, "is_hybrid")
        void set_hexes(bool value) {
            hexes_ = value;
            update();
        }

        /**
         * \brief Tests whether hexahedra are displayed
         * \retval true if hexahedra are displayed
         * \retval false otherwise
         */
        bool get_hexes() const {
            return hexes_;
        }


        /**
         * \brief Sets whether prisms should be displayed.
         * \param[in] value true if prisms should be displayed,
         *  false otherwise
         */
        gom_attribute(visible_if, "is_hybrid")
        void set_prisms(bool value) {
            prisms_ = value;
            update();
        }

        /**
         * \brief Tests whether prisms are displayed
         * \retval true if prisms are displayed
         * \retval false otherwise
         */
        bool get_prisms() const {
            return prisms_;
        }


        /**
         * \brief Sets whether pyramids should be displayed.
         * \param[in] value true if pyramids should be displayed,
         *  false otherwise
         */
        gom_attribute(visible_if, "is_hybrid")
        void set_pyramids(bool value) {
            pyramids_ = value;
            update();
        }

        /**
         * \brief Tests whether pyramids are displayed
         * \retval true if pyramids are displayed
         * \retval false otherwise
         */
        bool get_pyramids() const {
            return pyramids_;
        }


        /**
         * \brief Sets whether connectors should be displayed.
         * \param[in] value true if connectors should be displayed,
         *  false otherwise
         */
        gom_attribute(visible_if, "is_hybrid")
        void set_connectors(bool value) {
            connectors_ = value;
            update();
        }

        /**
         * \brief Tests whether connectors are displayed
         * \retval true if connectors are displayed
         * \retval false otherwise
         */
        bool get_connectors() const {
            return connectors_;
        }
        
        /**
         * \brief Sets the style used to draw the edges.
         * \param[in] value a const reference to the style used
         *  to draw the edges
         */
        gom_attribute(visible_if, "has_edges")
        void set_edges_style(const EdgeStyle& value) { 
            edges_style_ = value;
            update(); 
        }

        /**
         * \brief Gets the style used to draw the edges.
         * \return a const reference to the style used to draw
         *  the edges
         */
        const EdgeStyle& get_edges_style() const {
            return edges_style_;
        }


        /**
         * \brief Sets the style used to draw the mesh in
         *  the facets and in the cells.
         * \param[in] value a const reference to the style that
         *  should be used to draw the mesh in the facets and
         *  in the cells.
         */
        gom_attribute(visible_if, "has_facets or has_cells")
        void set_mesh_style(const EdgeStyle& value) { 
            mesh_style_ = value;
            update(); 
        }

        /**
         * \brief Gets the style used to draw the mesh in
         *  the facets and in the cells.
         * \return a const reference to the style that
         *  should be used to draw the mesh in the facets and
         *  in the cells.
         */
        const EdgeStyle& get_mesh_style() const {
            return mesh_style_;
        }


        /**
         * \brief Sets the style used to draw the border
         *  of the surface.
         * \param[in] value a const reference to the style that
         *  should be used to draw the border of the surface
         */
        gom_attribute(visible_if, "has_facets")
        void set_border_style(const EdgeStyle& value) { 
            border_style_ = value;
            update(); 
        }

        /**
         * \brief Gets the style used to draw the border
         *  of the surface.
         * \return a const reference to the style that
         *  should be used to draw the border of the surface
         */
        const EdgeStyle& get_border_style() const {
            return border_style_;
        }

        /**
         * \brief Sets the style used to draw the vertices.
         * \param[in] value a const reference to the style that
         *  should be used to draw the vertices
         */
        void set_vertices_style(const PointStyle& value) {
            vertices_style_ = value;
            update();
        }

        /**
         * \brief Gets the style used to draw the vertices.
         * \return a const reference to the style that
         *  should be used to draw the vertices
         */
        const PointStyle& get_vertices_style() const {
            return vertices_style_;
        }


        /**
         * \brief Sets vertices filtering
         * \param[in] value true if vertices filtering is activated
         */
        void set_vertices_filter(bool value) {
            gfx_.set_filter(
                MESH_VERTICES, value ? "filter" : ""
            );
            vertices_filter_ = value;
            update(); 
        }

        bool get_vertices_filter() const {
            return vertices_filter_;
        }
        
	/**
	 * \brief Sets the transparency of the vertices 
	 *  (use with dark background).
	 * \param[in] value the transparency, 0.0 for opaque,
	 *  1.0 for invisible.
	 */
	void set_vertices_transparency(double value) {
	    vertices_transparency_ = value;
	    update();
	}

	/**
	 * \brief Gets the transparency of the vertices.
	 * \return the transparency coefficient, 0.0 for opaque,
	 *  1.0 for invisible.
	 */
	double get_vertices_transparency() const {
	    return vertices_transparency_;
	}

        /**
         * \brief Sets the vertices selection style.
         * \param[in] value a const reference to the vertices
         *  selection style.
         */
        gom_attribute(visible_if, "has_vertices_selection")
        void set_vert_select_style(const PointStyle& value) {
            vertices_selection_style_ = value;
            update();
        }

        /**
         * \brief Gets the vertices selection style.
         * \return a const reference to the vertices selection style.
         */
        const PointStyle& get_vert_select_style() const {
            return vertices_selection_style_;
        }

        /**
         * \brief Sets the shrinking coefficient of the 
         *  volumetric cells.
         * \param[in] value the shrinking coefficient, between 
         *  0 (no shrinking) and 10 (completely shrunk).
         */
        gom_attribute(visible_if, "has_cells")
        void set_shrink(index_t value) {
            gfx_.set_shrink(double(value)/10.0);
            update();
        }

        /**
         * \brief Gets the shrinking coefficient of the 
         *  volumetric cells.
         * \return the shrinking coefficient, between 
         *  0 (no shrinking) and 10 (completely shrunk).
         */
        index_t get_shrink() const {
            return index_t(gfx_.get_shrink()*10.0);
        }


        /**
         * \brief Sets whether time interpolation should be
         *  used.
         * \details If time interpolation is used, then 6d coordinates
         *  of vertices are interpreted as initial location (coordinates
         *  1,2,3) and final location (coordinates 4,5,6).
         * \param[in] value true if time interpolation should be
         *  used, false otherwise.
         * \see set_time(), get_time()
         */
        gom_attribute(visible_if, "has_time")
        void set_animate(bool value) {
            animate_ = value;
            update();
        }

        /**
         * \brief Tests whether time interpolation should be
         *  used.
         * \details If time interpolation is used, then 6d coordinates
         *  of vertices are interpreted as initial location (coordinates
         *  1,2,3) and final location (coordinates 4,5,6).
         * \retval true if time interpolation should be used
         * \retval false otherwise
         * \see set_time(), get_time()
         */
        bool get_animate() const {
            return animate_;
        }

        /**
         * \brief Sets the time parameter used for time interpolation.
         * \param[in] value the time parameter, in 0 (initial time) .. 20 
         *  (final time).
         */
        gom_attribute(visible_if, "has_time and animate")
	gom_attribute(handler, "slider_int")
	gom_attribute(min, "0")
	gom_attribute(max, "20")
        void set_time(index_t value) {
            time_ = double(value) / 20.0;
            update();
        }

        /**
         * \brief Gets the time parameter used for time interpolation.
         * \return the time parameter, in 0 (initial time) .. 20 (final time).
         */
        index_t get_time() const {
            return index_t(time_ * 20.0);
        }


        gom_attribute(visible_if, "has_cells")
        void set_slivers(double value) {
            slivers_ = value;
            update();
        }

        double get_slivers() const {
            return slivers_;
        }

        gom_attribute(visible_if, "has_weird_cells")
        void set_weird_cells(bool x) {
            weird_cells_ = x;
            update();
        }

        bool get_weird_cells() const {
            return weird_cells_;
        }

	bool get_glsl_source_visible() const {
	    return false;
	}

        gom_attribute(visible_if, "glsl_source_visible")        	
	void set_glsl_source(const std::string& value) {
	    glsl_source_ = value;
	    update_glsl_program();
	}

	const std::string& get_glsl_source() const {
	    return glsl_source_;
	}
	
    protected:
        void draw_slivers();
        void draw_weird_cells();
	void draw_surface_with_glsl_shader();
	void update_glsl_program();
	
    protected:
        GEO::MeshGfx gfx_;

	PaintingMode      painting_mode_;
	
        std::string       attribute_;
        MeshElementsFlags attribute_subelements_;
        std::string       attribute_name_;
        double            attribute_min_;
        double            attribute_max_;
        ColormapStyle     colormap_style_;
        Texture_var       colormap_texture_;

	MeshElementsFlags tex_coord_subelements_;
	std::string       tex_coord_attribute_;
	std::string       tex_coord_name_;
	index_t           tex_coord_repeat_;
	bool              tex_normal_mapping_;
	Texture_var       texture_;
	ImageFileName     texture_filename_;
	
        SurfaceStyle surface_style_;
        bool         facets_filter_;
	CullingMode  culling_mode_;
	
	index_t      specular_;
        bool         two_sided_;
        SurfaceStyle volume_style_;
        bool         cells_filter_;
        bool         colored_cells_;
        bool         tets_;
        bool         hexes_;
        bool         prisms_;
        bool         pyramids_;
        bool         connectors_;
        index_t      shrink_;
        EdgeStyle    edges_style_;
        EdgeStyle    mesh_style_;
        EdgeStyle    border_style_;
        PointStyle   vertices_style_;
        bool         vertices_filter_;
	double       vertices_transparency_;
        PointStyle   vertices_selection_style_;
        bool         animate_;
        double       time_;
        bool         picking_;
        double       slivers_;
        bool         weird_cells_;
        bool         clipping_;

	bool         glsl_program_changed_;
	double       glsl_start_time_;
	index_t      glsl_frame_;
	std::string  glsl_source_;
	GLuint       glsl_program_;
    };

    /**********************************************************/

    /**
     * \brief Exploded view, moves regions apart.
     */
    gom_class MESH_API ExplodedViewMeshGrobShader : public PlainMeshGrobShader {
    public:
        /**
         * \brief PlainMeshGrobShader constructor.
         * \param[in] grob a pointer to the MeshGrob this shader is attached to
         */
         ExplodedViewMeshGrobShader(MeshGrob* grob);

        /**
         * \brief PlainMeshGrobShader destructor.
         */
         ~ExplodedViewMeshGrobShader() override;

    gom_properties:

         gom_attribute(handler, "combo_box")
         gom_attribute(values, "$scalar_attributes")
         void set_region(const std::string& value) {
             region_ = value;
             dirty_ = true;
             update();
         }

         const std::string& get_region() const {
             return region_;
         }
         
         void set_amount(index_t value) {
             amount_ = value;
             update();
         }

         index_t get_amount() const {
             return amount_;
         }
         
    public:
         void draw() override;

    protected:
         bool dirty_;
         std::string region_;         
         index_t amount_;
         int rgn_min_;
         int rgn_max_;
         vec3 bary_;
         vector<vec3> region_bary_;
    };

    /**********************************************************/
         
}

#endif

