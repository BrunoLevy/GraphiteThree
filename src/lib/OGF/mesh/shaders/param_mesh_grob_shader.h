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
 

#ifndef H_OGF_MESH_SHADERS_PARAM_MESH_GROB_SHADER_H
#define H_OGF_MESH_SHADERS_PARAM_MESH_GROB_SHADER_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>

/**
 * \file OGF/mesh/shaders/param_mesh_grob_shader.h
 * \brief Classes for drawing the parameter space of a MeshGrob.
 */

namespace OGF {
    /**
     * \brief A MeshGrobShader to display parameter space.
     */
    gom_class MESH_API ParamMeshGrobShader : public MeshGrobShader {
    public:
        /**
         * \brief PlainMeshGrobShader constructor.
         * \param[in] grob a pointer to the MeshGrob this shader is attached to
         */
        ParamMeshGrobShader(MeshGrob* grob);

        /**
         * \brief PlainMeshGrobShader destructor.
         */
         ~ParamMeshGrobShader() override;

	/**
	 * \copydoc MeshGrobShader::draw()
	 */
	void draw() override;
	
      gom_properties:
	        
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

      private:
        SurfaceStyle surface_style_;
        EdgeStyle    mesh_style_;
    };
    
}

#endif
