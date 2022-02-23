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
 

#ifndef H_OGF_LUAGROB_SHADERS_LUAGROB_GROB_SHADER_H
#define H_OGF_LUAGROB_SHADERS_LUAGROB_GROB_SHADER_H

#include <OGF/luagrob/common/common.h>
#include <OGF/luagrob/grob/lua_grob.h>
#include <OGF/scene_graph/shaders/shader.h>
#include <OGF/scene_graph/types/properties.h>

/**
 * \file OGF/luagrob/shaders/luagrob_grob_shader.h
 * \brief Classes for drawing and picking LuaGrob.
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
     * \brief Base class for drawing and picking LuaGrob.
     */
    gom_attribute(abstract, "true") 
    gom_class LUAGROB_API LuaGrobShader : public Shader {
    public:
        /**
         * \brief LuaGrobShader constructor.
         * \param[in] grob a pointer to the LuaGrob this shader is attached to
         */
        LuaGrobShader(LuaGrob* grob);

        /**
         * \brief LuaGrobShader destructor.
         */
	~LuaGrobShader() override;

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

        
    protected:
        /**
         * \brief Gets the LuaGrob.
         * \return a pointer to the LuaGrob this shader is attached to
         */
        LuaGrob* lua_grob() const {
            return static_cast<LuaGrob*>(grob());
        }
    };

    //________________________________________________________

    /**
     * \brief The default implementation of LuaGrobShader
     */
    gom_class LUAGROB_API PlainLuaGrobShader : public LuaGrobShader {
    public:
        /**
         * \brief PlainLuaGrobShader constructor.
         * \param[in] grob a pointer to the LuaGrob this shader is attached to
         */
        PlainLuaGrobShader(LuaGrob* grob);

        /**
         * \brief PlainLuaGrobShader destructor.
         */
	~PlainLuaGrobShader() override;

        /**
         * \copydoc LuaGrobShader::draw()
         */
        void draw() override;

        /**
         * \copydoc LuaGrobShader::pick_object()
         */
        void pick_object(index_t object_id) override;

        /**
         * \copydoc LuaGrobShader::blink()
         */
        void blink() override;

      gom_properties:
	
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

    protected:
        bool              lighting_;
        bool              clipping_;
        bool              picking_;
        EdgeStyle         box_style_;
    };

    //________________________________________________________    
}
#endif

