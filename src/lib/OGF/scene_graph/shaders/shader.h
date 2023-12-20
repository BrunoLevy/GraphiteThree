/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
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
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 */
 
#ifndef H_OGF_SCENE_GRAPH_TYPES_SHADER_H
#define H_OGF_SCENE_GRAPH_TYPES_SHADER_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/types/properties.h>
#include <OGF/gom/types/node.h>
#include <OGF/renderer/context/texture.h>
#include <geogram/image/image.h>

/**
 * \file OGF/scene_graph/shaders/shader.h
 * \brief The base class for all object shaders.
 */

namespace OGF {

//_________________________________________________________

    class Grob;
    class Node;

    /**
     * \brief Base class for Grob shader.
     * \details A Grob shader is responsible for drawing the object
     *  onto the screen and picking elements of the object.
     */
    gom_attribute(abstract,"true")
    gom_class SCENE_GRAPH_API Shader : public Node {
    public:

        /**
         * \brief Shader constructor.
         * \param[in] grob a pointer to the Grob this Shader is responsible of.
         */
        Shader(Grob* grob);

        /**
         * \brief Shader destructor.
         */
         ~Shader() override;

        /**
         * \brief Draws the Grob.
         * \details The Grob is the one that was passed to the constructor
         *  of this Shader. 
         */
        virtual void draw();

        /**
         * \brief Draws the Grob in picking mode.
         * \details In picking mode, drawing is replaced by filling the pixel
         *  values with 32 bits ids, while ZBuffer is still activated. Thus,
         *  after drawing the whole scene, one can find for a given pixel
         *  which object is visible by reading the 32 bit id encoded in the
         *  pixel color.
         * \param[in] object_id all the pixels covered by the object will be
         *  assigned this value
         */
        virtual void pick_object(index_t object_id) = 0;

        /**
         * \brief Draws the current object several times, while chaning the
         *  value of one graphic attribute (e.g. mesh on/off), to draw attentin
         *  towards it.
         * \details This function is used if object highlighting is activated.
         */
        virtual void blink();

        /** 
         * \brief Redraws the scene.
         * \details This function is called whenever the object changes or
         * its shader attributes change.
         */
        virtual void update();

    gom_properties:

        /**
         * \brief Sets multi shader mode.
         * \param[in] value true if multi shader mode should be activated, 
         *  false otherwise
         * \details In multi shader mode, several shaders can be
         *  active for the same object, i.e. all the shaders that have
         *  multi mode activated are drawn. For instance, this makes it 
         *  possible to display an object with a scalar attribute painted 
         *  on it and several vector attributes superimposed.
         */
        void set_multi(bool value);
        
        /**
         * \brief Tests whether multi shader mode is active.
         * \details In multi shader mode, several shaders can be
         *  active for the same object, i.e. all the shaders that have
         *  multi mode activated are drawn. For instance, this makes it 
         *  possible to display an object with a scalar attribute painted 
         *  on it and several vector attributes superimposed.
         * \retval true if multi shader mode is active
         * \retval false otherwise
         */
        bool get_multi() const;

	/**
	 * \brief Gets the grob.
	 * \return a pointer to the Grob associated with this Shader.
	 */
	Grob* get_grob() const {
	    return grob();
	}

    public:
        GLdouble* latest_modelview() {
            return modelview_;
        }
        
        GLdouble* latest_project() {
            return project_;
        }
        
        GLint* latest_viewport() {
            return viewport_;
        }

        /**
         * \brief Tests whether dark mode is set
         * \retval true if gui colors are rather dark
         * \retval false otherwise
         */
        bool dark_mode() const;
        
    protected:

        /**
         * \brief Creates a texture from an image file.
         * \param[in] file_name the image file name
         * \param[in] filtering the OpenGL filtering mode
         * \param[in] wrapping the wrapping mode for texture
         *  coordinates
         * \return a pointer to the created texture. Ownership is transfered to 
         *  caller (that can store it in a Texture_var)
         */
        Texture* create_texture_from_file(
            const std::string& file_name,
            GLint filtering = GL_LINEAR, GLint wrapping = GL_CLAMP_TO_EDGE
        );

        /**
         * \brief Creates a texture from a colormap by name
         * \param[in] name name of the colormap
         * \return a pointer to the created texture. Ownership is transfered to 
         *  caller (that can store it in a Texture_var)
         */
        Texture* create_texture_from_colormap_name(
            const std::string& name,
            GLint filtering = GL_LINEAR, GLint wrapping = GL_CLAMP_TO_EDGE
        );

        /**
         * \brief Gets the Grob.
         * \return a pointer to the Grob this Shader is associated with
         */
        Grob* grob() const {
            return grob_;
        }


    private:
        Grob* grob_;
        bool multi_;

        // Viewing parameters, queried when this object is drawn.
        // Useful for picking or for drawing overlays.
        GLdouble modelview_[16];
        GLdouble project_[16];
        GLint viewport_[4];
        
        friend class ShaderManager;
       
    protected:
        /**
         * \brief If true, then each shader update does not 
         *  trigger a grob update.
         * \details Can be set by derived classes constructors.
         */
        bool no_grob_update_;
    };

    /**
     * \brief An automatic reference-counted pointer to a Shader.
     */
    typedef SmartPointer<Shader> Shader_var;

//_________________________________________________________

}
#endif

