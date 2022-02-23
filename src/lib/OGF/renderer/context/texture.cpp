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
 
#include <OGF/renderer/context/texture.h>


namespace OGF {

//_________________________________________________________

    Texture::Texture() {
        dimension_ = 0;
        sizes_[0] = 0;
        sizes_[1] = 0;
        sizes_[2] = 0;
        filtering_ = GL_LINEAR;
        wrapping_ = GL_CLAMP_TO_EDGE;
        target_ = GLUP_TEXTURE_2D_TARGET;
        type_ = GLUP_TEXTURE_2D;
        unit_ = 0;
        id_ = 0;
	has_mipmaps_ = false;
    }
    
    Texture::~Texture() {
        glDeleteTextures(1, &id_);    
        id_ = 0 ;
    }

    index_t Texture::dimension() const {
        return dimension_;
    }

    void Texture::create_from_image(
        const Image* image,
        GLint filtering,
        GLint wrapping
    ) {
        create_from_data(
            image->base_mem(),
            image->color_encoding(), image->component_encoding(),
            image->width(), image->height(), image->depth(),
            filtering, wrapping
        );
    }

    bool Texture::get_GL_formats(
        Image::ColorEncoding color_encoding,
        Image::ComponentEncoding component_encoding,
        GLint& internal_format,
        GLenum& format,
        GLenum& type
    ) {
        internal_format = GLint(Image::nb_components(color_encoding));
        format = GLenum(-1);
        type = GLenum(-1);

        switch(color_encoding) {
        case Image::GRAY:
            format = GL_RED;
            switch(component_encoding) {
            case Image::BYTE:
                internal_format = GL_R;
                break;
            case Image::INT16:
                internal_format = GL_R16UI;
                break;
            case Image::INT32:
                internal_format = GL_R32UI;
                break;
            case Image::FLOAT32:
            case Image::FLOAT64:
                internal_format = GL_R32F;
                break;
            }
            break;
        case Image::RGB:
            format = GL_RGB;
            internal_format = GL_RGB;
            break;            
        case Image::BGR:
            format = GL_BGR;
            internal_format = GL_RGB;            
            break;            
        case Image::RGBA:
            format = GL_RGBA;
            internal_format = GL_RGBA;                        
            break;            
        case Image::INDEXED:
        case Image::YUV:
            return false;
        }
        
        switch(component_encoding) {
        case Image::BYTE:
            type = GL_UNSIGNED_BYTE;
            break;
        case Image::INT16:
            type = GL_UNSIGNED_SHORT;
            break;
        case Image::INT32:
            type = GL_UNSIGNED_INT;
            break;
        case Image::FLOAT32:
            type = GL_FLOAT;
            break;
        case Image::FLOAT64:
            type = GL_DOUBLE;
            break;
        }

        return true;
    }
    
    void Texture::create_from_data_1d(
        Memory::pointer ptr,
        Image::ColorEncoding color_encoding,
        Image::ComponentEncoding component_encoding,
        index_t width
    ) {
        sizes_[0] = width;
        sizes_[1] = 1;
        sizes_[2] = 1;

        GLint internal_format;
        GLenum format;
        GLenum type;
        
        if(
            !get_GL_formats(
                color_encoding, component_encoding,
                internal_format, format, type
           )
        ) {
            Logger::err("OpenGL") << "Could not create texture from image data"
                                  << std::endl;
            Logger::err("OpenGL") << "Unsupported image format"
                                  << std::endl;
            return;
        }

        glTexImage1D(
            target_, 0, internal_format, GLsizei(width), 0, format, type, ptr
        );
    }

    void Texture::create_from_data_2d(
        Memory::pointer ptr,
        Image::ColorEncoding color_encoding,
        Image::ComponentEncoding component_encoding,
        index_t width, index_t height
    ) {
        sizes_[0] = width;
        sizes_[1] = height;
        sizes_[2] = 1;

        GLint internal_format;
        GLenum format;
        GLenum type;

        if(
            !get_GL_formats(
                color_encoding, component_encoding,
                internal_format, format, type
           )
        ) {
            Logger::err("OpenGL") << "Could not create texture from image data"
                                  << std::endl;
            Logger::err("OpenGL") << "Unsupported image format"
                                  << std::endl;
            return;
        }

        glTexImage2D(
            target_, 0, internal_format,
            GLsizei(width), GLsizei(height), 0, format, type, ptr
        );
    }

    void Texture::create_from_data_3d(
        Memory::pointer ptr,
        Image::ColorEncoding color_encoding,
        Image::ComponentEncoding component_encoding,
        index_t width, index_t height, index_t depth
    ) {

        sizes_[0] = width;
        sizes_[1] = height;
        sizes_[2] = depth;

        GLint internal_format;
        GLenum format;
        GLenum type;
        
        if(
            !get_GL_formats(
                color_encoding, component_encoding,
                internal_format, format, type
           )
        ) {
            Logger::err("OpenGL") << "Could not create texture from image data"
                                  << std::endl;
            Logger::err("OpenGL") << "Unsupported image format"
                                  << std::endl;
            return;
        }

        glTexImage3D(
            target_, 0, internal_format,
            GLsizei(width), GLsizei(height), GLsizei(depth),
            0, format, type, ptr
        );
    }
    
    void Texture::create_from_data(
        Memory::pointer ptr,
        Image::ColorEncoding color_encoding,
        Image::ComponentEncoding component_encoding,
        index_t width, index_t height, index_t depth,
        GLint filtering,
        GLint wrapping
    ) {
        dimension_ = 1;
        target_ = GLUP_TEXTURE_1D_TARGET;
        unit_ = GLUP_TEXTURE_1D_UNIT;
        type_ = GLUP_TEXTURE_1D;
        
        if(height > 1) {
            dimension_ = 2;
            target_ = GLUP_TEXTURE_2D_TARGET;
            unit_ = GLUP_TEXTURE_2D_UNIT;
            type_ = GLUP_TEXTURE_2D;
        }
        
        if(depth > 1) {
            dimension_ = 3;
            target_ = GLUP_TEXTURE_3D_TARGET;
            unit_ = GLUP_TEXTURE_3D_UNIT;
            type_ = GLUP_TEXTURE_3D;
        }

        if(id_ == 0) {
            glGenTextures(1, &id_);            
        }

        bind();
        
        switch(dimension_) {
            // Note: we use 2d textures even for 1d colormaps,
            // because 1d texturing is not supported by all OpenGL
            // versions (for instance, it is not supported by OpenGL ES/2)
        case 1:
        case 2:
            create_from_data_2d(
                ptr, color_encoding, component_encoding, width, height
            );
            break;
        case 3:
            create_from_data_3d(
                ptr, color_encoding, component_encoding, width, height, depth
            );
            break;
        default:
            geo_assert_not_reached;
        }

	// Always pre-generate mipmaps for 1D and 2D textures.
	if(dimension_ != 3) {
	    glGenerateMipmap(target_);
	    has_mipmaps_ = true;
	}
	
	set_filtering(filtering);
	set_wrapping(wrapping);
        
        unbind();
    }

    void Texture::set_filtering(GLint filtering) {
	filtering_ = filtering;
	if((filtering_ != GL_NEAREST) && !has_mipmaps_) {
            glGenerateMipmap(target_);
	    has_mipmaps_ = true;
	}
    }

    
    void Texture::bind() {
        glActiveTexture(GLenum(GL_TEXTURE0 + unit_));
        
        //  Make sure there is only one texture bound
        // to the same unit.
        glBindTexture(GL_TEXTURE_1D,0);
        glBindTexture(GL_TEXTURE_2D,0);
        glBindTexture(GL_TEXTURE_3D,0);
        glBindTexture(target_, id_);

        
        if(filtering_ == GL_NEAREST) {
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(
                target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
            );
            glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        glTexParameteri(target_, GL_TEXTURE_WRAP_S, wrapping_);
        if(dimension() >= 2) {
            glTexParameteri(target_, GL_TEXTURE_WRAP_T, wrapping_);
        }
        if(dimension() >= 3) {
            glTexParameteri(target_, GL_TEXTURE_WRAP_R, wrapping_);
        }
        glupTextureType(type_);
        glupEnable(GLUP_TEXTURING);
        glupTextureMode(GLUP_TEXTURE_REPLACE);
    }

    void Texture::unbind() {
        glActiveTexture(GLenum(GL_TEXTURE0+unit_));        
        glBindTexture(target_, 0);
        glupDisable(GLUP_TEXTURING);
        glActiveTexture(GL_TEXTURE0);
    }
//_________________________________________________________

}

