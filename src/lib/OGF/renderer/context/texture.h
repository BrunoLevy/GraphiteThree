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
 
#ifndef H_OGF_RENDERER_CONTEXT_TEXTURE_H
#define H_OGF_RENDERER_CONTEXT_TEXTURE_H

#include <OGF/renderer/common/common.h>
#include <geogram/image/image.h>
#include <geogram_gfx/basic/GL.h>

/**
 * \file OGF/renderer/context/texture.h
 * \brief Helper class for manipulating OpenGL textures.
 */

namespace OGF {

//_________________________________________________________

    /**
     * \brief An OpenGL texture.
     */
    class RENDERER_API Texture : public Counted {
    public:

        /**
         * \brief Texture constructor.
         */
        Texture();

        /**
         * \brief Texture destructor.
         * \details Deletes the associated OpenGL texture
         *  if it was created.
         */
        ~Texture();


        /**
         * \brief Gets the dimension of the texture
         * \retval 1 for a 1d texture
         * \retval 2 for a 2d texture
         * \retval 3 for a 3d texture
         */
        index_t dimension() const;

        /**
         * \brief Gets the width.
         * \return the width of this texture
         */
        index_t width() const {
            return sizes_[0];
        }

        /**
         * \brief Gets the height.
         * \return the height of this texture (or 1 for a 1d texture)
         */
        index_t height() const {
            return sizes_[1];
        }

        /**
         * \brief Gets the height.
         * \return the depth of this texture (or 1 for 1d and 2d textures)
         */
        index_t depth() const {
            return sizes_[2];
        }

        /**
         * \brief Creates a texture from an image.
         * \param[in] image a const pointer to the Image
         * \param[in] filtering the OpenGL filtering mode
         * \param[in] wrapping the wrapping mode for texture
         *  coordinates
         * \note For now, only 2d textures are implemented, and 
         *   not all datatypes/storage work (under work...)
         */
        void create_from_image(
            const Image* image,
            GLint filtering = GL_LINEAR,
            GLint wrapping = GL_CLAMP_TO_EDGE
        );

        /**
         * \brief Creates a texture from raw data.
         * \param[in] ptr a pointer to image data
         * \param[in] color_encoding the color encoding of the data
         * \param[in] component_encoding the data type used for 
         *  the color components
         * \param[in] width image width
         * \param[in] height image height (or 1 for 1d textures)
         * \param[in] depth image depth (or 1 for 1d and 2d textures)
         * \param[in] filtering the OpenGL filtering mode
         * \param[in] wrapping the wrapping mode for texture coordinates
         * \note For now, only 2d textures are implemented, and 
         *   not all datatypes/storage work (under work...)
         */
        void create_from_data(
            Memory::pointer ptr,
            Image::ColorEncoding color_encoding,
            Image::ComponentEncoding component_encoding,
            index_t width, index_t height, index_t depth=1,
            GLint filtering = GL_LINEAR,
            GLint wrapping = GL_CLAMP_TO_EDGE
        );

        /**
         * \brief Binds the texture to its texture target.
         */
        void bind();

        /**
         * \brief Unbinds the texture from its texture target.
         */
        void unbind();

        /**
         * \brief Gets the id of the texture.
         * \return the OpenGL opaque id of the texture
         */
        GLuint id() const {
            return id_;
        }

	/**
	 * \brief Resets the id of this texture.
	 * \details If a texture was created, then the association
	 *  with this Texture is forgotten, and this Texture's 
	 *  destructor no longer destroys the OpenGL texture.
	 */
	void reset_id() {
	    id_ = 0;
	}

	/*
	 * \brief Gets the filtering mode.
	 * \return one of GL_NEAREST, GL_LINEAR
	 */
	GLint get_filtering() const {
	    return filtering_;
	}

	/*
	 * \brief Sets the filtering mode.
	 * \param filtering one of GL_NEAREST, GL_LINEAR
	 */
	void set_filtering(GLint filtering);

	/*
	 * \brief Gets the wrapping mode.
	 * \return one of GL_CLAMP_TO_EDGE, GL_WRAP.
	 */
	GLint get_wrapping() const {
	    return wrapping_;
	}

	/*
	 * \brief Sets the wrapping mode.
	 * \param wrapping one of GL_CLAMP_TO_EDGE, GL_WRAP.
	 */
	void set_wrapping(GLint wrapping) {
	    wrapping_ = wrapping;
	}
	
    protected:

        /**
         * \brief Initializes texture data for a 1d texture.
         * \param[in] ptr pointer to texture data
         * \param[in] color_encoding the color encoding of the data
         * \param[in] component_encoding the data type used for 
         *  the color components
         * \param[in] width the width of the texture
         */  
        void create_from_data_1d(
            Memory::pointer ptr,
            Image::ColorEncoding color_encoding,
            Image::ComponentEncoding component_encoding,
            index_t width
        );

        /**
         * \brief Initializes texture data for a 2d texture.
         * \param[in] ptr pointer to texture data
         * \param[in] color_encoding the color encoding of the data
         * \param[in] component_encoding the data type used for 
         *  the color components
         * \param[in] width the width of the texture
         * \param[in] height the height of the texture
         */  
        void create_from_data_2d(
            Memory::pointer ptr,
            Image::ColorEncoding color_encoding,
            Image::ComponentEncoding component_encoding,
            index_t width, index_t height
        );

        /**
         * \brief Initializes texture data for a 2d texture.
         * \param[in] ptr pointer to texture data
         * \param[in] color_encoding the color encoding of the data
         * \param[in] component_encoding the data type used for 
         *  the color components
         * \param[in] width the width of the texture
         * \param[in] height the height of the texture
         * \param[in] depth the height of the texture
         */  
        void create_from_data_3d(
            Memory::pointer ptr,
            Image::ColorEncoding color_encoding,
            Image::ComponentEncoding component_encoding,
            index_t width, index_t height, index_t depth
        );


        /**
         * \brief Gets the parameters for specifying OpenGL
         *  textures from Image color encoding and component
         *  encoding.
         * \param[in] color_encoding the color encoding of the data
         * \param[in] component_encoding the data type used for 
         *  the color components
         * \param[out] internal_format the internal format used 
         *  by OpenGL texture (parameter of glTexImagenD())
         * \param[out] format the OpenGL format used to pass image
         *  data to the OpenGL texture (parameter of glTexImagenD())
         * \param[out] type the type to specify the image components to
         *  the OpenGL texture (parameter of glTexImagenD())
         */
        static bool get_GL_formats(
            Image::ColorEncoding color_encoding,
            Image::ComponentEncoding component_encoding,
            GLint& internal_format,
            GLenum& format,
            GLenum& type
        );
        
    private:
        index_t dimension_;
        index_t sizes_[3];
        GLenum target_;
        GLUPtextureType type_;
        GLint unit_;
        GLint filtering_;
        GLint wrapping_;
        GLuint id_;
	bool has_mipmaps_;
    } ;

    typedef SmartPointer<Texture> Texture_var ;

//_________________________________________________________

}
#endif

