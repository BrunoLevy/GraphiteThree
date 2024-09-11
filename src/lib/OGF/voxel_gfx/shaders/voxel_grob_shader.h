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


#ifndef H_OGF_VOXEL_SHADERS_VOXEL_GROB_SHADER_H
#define H_OGF_VOXEL_SHADERS_VOXEL_GROB_SHADER_H

#include <OGF/voxel_gfx/common/common.h>
#include <OGF/voxel/grob/voxel_grob.h>
#include <OGF/scene_graph_gfx/shaders/shader.h>
#include <OGF/scene_graph/types/properties.h>

/**
 * \file OGF/voxel_gfx/shaders/voxel_grob_shader.h
 * \brief Classes for drawing and picking VoxelGrob.
 */

namespace GEO {
    class Image;
}

namespace OGF {

    //________________________________________________________

    class RenderingContext;
    class Builder;
    class Texture;

    /**
     * \brief Base class for drawing and picking VoxelGrob.
     */
    gom_attribute(abstract, "true")
    gom_class VOXEL_GFX_API VoxelGrobShader : public Shader {
    public:
        /**
         * \brief VoxelGrobShader constructor.
         * \param[in] grob a pointer to the VoxelGrob this shader is attached to
         */
        VoxelGrobShader(VoxelGrob* grob);

        /**
         * \brief VoxelGrobShader destructor.
         */
        ~VoxelGrobShader() override;

        /**
         * \copydoc Shader::draw()
         */
        void draw() override;

        /**
         * \copydoc Shader::pick_object()
         */
        void pick_object(index_t object_id) override;

        /**
         * \copydoc Shader::blink()
         */
        void blink() override;


    gom_properties:
        std::string get_displayable_attributes() const {
            std::string result;
            if(voxel_grob() != nullptr) {
                result = voxel_grob()->get_displayable_attributes();
            }
            return result;
        }

    protected:
        /**
         * \brief Gets the VoxelGrob.
         * \return a pointer to the VoxelGrob this shader is attached to
         */
        VoxelGrob* voxel_grob() const {
            return static_cast<VoxelGrob*>(grob());
        }
    };

    //________________________________________________________

    /**
     * \brief The default implementation of VoxelGrobShader
     */
    gom_class VOXEL_GFX_API PlainVoxelGrobShader : public VoxelGrobShader {
    public:
        /**
         * \brief PlainVoxelGrobShader constructor.
         * \param[in] grob a pointer to the VoxelGrob this shader is attached to
         */
        PlainVoxelGrobShader(VoxelGrob* grob);

        /**
         * \brief PlainVoxelGrobShader destructor.
         */
        ~PlainVoxelGrobShader() override;

        /**
         * \copydoc VoxelGrobShader::draw()
         */
        void draw() override;

        /**
         * \copydoc VoxelGrobShader::pick_object()
         */
        void pick_object(index_t object_id) override;

        /**
         * \copydoc VoxelGrobShader::blink()
         */
        void blink() override;

    gom_properties:


    gom_slots:

        /**
         * \brief Sets the displayed attribute range automatically.
         */
        void autorange();

    gom_properties:

        /**
         * \brief Sets the minimum of the displayed range for
         *  attribute values.
         * \param[in] value the minimum attribute value
         */
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
	gom_attribute(handler, "combo_box")
	gom_attribute(values, "$displayable_attributes")
        void set_attribute(const std::string& value);

        /**
         * \brief Gets the name of the displayed attribute.
         * \return the name of the displayed attribute
         */
        const std::string& get_attribute() const {
            return attribute_name_;
        }

        /**
         * \brief Sets the colormap used to display attributes.
         * \param[in] value the name of the colormap
         */
        void set_colormap(const ColormapStyle& value) {
            colormap_style_ = value;
            colormap_texture_.reset();
            update();
        }

        const ColormapStyle& get_colormap() const {
            return colormap_style_;
        }

        /**
         * \brief Sets whether lighting should be used.
         * \param[in] value true if lighting is enabled, false
         *  otherwise
         */
        void set_lighting(bool value) {
            lighting_ = value;
            update();
        }

        /**
         * \brief Gets whether lighting is used.
         * \retval true if lighting is used
         * \retval false otherwise
         */
        bool get_lighting() const {
            return lighting_;
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
         * \brief Sets the style used to draw the mesh in
         *  the facets and in the cells.
         * \param[in] value a const reference to the style that
         *  should be used to draw the mesh in the facets and
         *  in the cells.
         */
        void set_box_style(const EdgeStyle& value) {
            box_style_ = value;
            update();
        }

        /**
         * \brief Gets the style used to draw the mesh in
         *  the facets and in the cells.
         * \return a const reference to the style that
         *  should be used to draw the mesh in the facets and
         *  in the cells.
         */
        const EdgeStyle& get_box_style() const {
            return box_style_;
        }

    protected:

        /**
         * \brief Draws the bounding box in wireframe.
         */
        void draw_wireframe_box();

        /**
         * \brief Draws the volume as a textured hexahedron.
         * \param[in] attribute_min the value mapped to texture coordinate 0.0
         * \param[in] attribute_max the value mapped to texture coordinate 1.0
         * \param[in] repeat the number of times the colormap is repeated.
         */
        void draw_volume(
            Texture* colormap_texture,
            double attribute_min,
            double attribute_max,
            index_t repeat
        );

        /**
         * \brief Creates the colormap texture and the attribute
         *  texture if they do not exist.
         */
        void create_textures_if_needed();

    protected:
        std::string       attribute_name_;
        double            attribute_min_;
        double            attribute_max_;
        ColormapStyle     colormap_style_;
        Texture_var       colormap_texture_;
        Texture_var       attribute_texture_;
        bool              lighting_;
        bool              clipping_;
        bool              picking_;
        EdgeStyle         box_style_;
    };

    //________________________________________________________
}
#endif
