/*
 *  GXML/Graphite: Geometry and Graphics Programming Library + Utilities
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
 

#include <OGF/skin_imgui/types/icon_repository.h>
#include <OGF/basic/os/file_manager.h>
#include <OGF/renderer/context/texture.h>
#include <geogram/image/image_library.h>

#include <iostream>
#include <fstream>
#include <stdlib.h>

namespace {
    using namespace OGF;

    /**
     * \brief Auxiliary function for process_background()
     * \see process_background()
     */
    bool has_color(Image& image, int x, int y) {
	if(
	   x < 0 || y < 0 ||
	   x >= int(image.width()) ||
	   y >= int(image.height())
	) {
	    return false;
	}
	Memory::byte* p = image.pixel_base(index_t(x),index_t(y));
	return (p[3] == 255); 
    }

    /**
     * \brief Auxiliary function for process_background()
     * \see process_background()
     */
    bool in_gutter(Image& image, index_t x, index_t y) {
	if(has_color(image, int(x), int(y))) {
	    return false;
	}
	for(int Y=int(y)-1; Y<int(y); ++Y) {	
	    for(int X=int(x)-1; X<int(x); ++X) {
		if(has_color(image, X, Y)) {
		    return true;
		}
	    }
	}
	return false;
    }

    /**
     * \brief Adds a little subtle shadow to an image.
     * \param[in,out] image the image
     */
    void process_background(Image& image) {
	FOR(y, image.height()) {
	    FOR(x, image.width()) {
		if(in_gutter(image, x, y)) {
		    image.pixel_base(x,y)[3]=155;		    
		}
	    }
	}
	FOR(y, image.height()) {
	    FOR(x, image.width()) {
		Memory::byte* p = image.pixel_base(x,y);
		if(p[3] == 155) {
		    p[0] = p[1] = p[2] = 100;
		}
	    }
	}
    }
}

/****************************************************************************/

namespace OGF {

    IconRepository::IconRepository() {
    }

    IconRepository::~IconRepository() {
        for(auto& it : icons_) {
	    GLuint tex = it.second.gl_texture_id;
	    glDeleteTextures(1,&tex);
        }
    }


    void IconRepository::bind_icon(
        const std::string& icon_name, GLuint gl_texture
    ) {
        if(icons_.find(icon_name) != icons_.end()) {
            Logger::err("IconRepository") 
                << "Icon \'" << icon_name
                << "\' is already bound"
                << std::endl;
            return;
        }
	Icon icon;
	icon.im_texture_id = nullptr;
	icon.gl_texture_id = gl_texture;
        icons_[icon_name] = icon;
    }

    ImTextureID IconRepository::resolve_icon(
        const std::string& icon_name, bool mipmap
    ) const {
        auto it = icons_.find(icon_name);
        if(it != icons_.end()) {
            return it->second.im_texture_id;
        }

	std::string icon_file_name = "icons/" + icon_name + ".png";
	Image_var image;
        if(FileManager::instance()->find_file(icon_file_name, false, "lib/")) {
	    image = ImageLibrary::instance()->load_image(icon_file_name);
	    if(!image.is_null()) {
		// Dammit, my png is flipped w.r.t. xpm (to be fixed)
		image->flip_vertically(); 
	    }
	}
	if(image.is_null()) {
	    icon_file_name = "icons/" + icon_name + ".xpm";
	    if(FileManager::instance()->find_file(
		   icon_file_name, false, "lib/")
	    ) {
		image = ImageLibrary::instance()->load_image(icon_file_name);
	    }
	}
	
        if(image.is_null()) {
	    if(not_found_.find(icon_name) == not_found_.end()) {
		Logger::err("IconRepository") 
		    << "Icon \'" << icon_file_name << "\' :"
		    << "File not found"
		    << std::endl;
	    }
	    not_found_.insert(icon_file_name);
            return resolve_icon("no_icon");
	}	

	process_background(*image);
	
	Texture texture;
	texture.create_from_image(image, mipmap ? GL_LINEAR : GL_NEAREST);
	
        IconRepository* non_const_this = const_cast<IconRepository*>(this);
        ogf_assert(non_const_this != nullptr );
        non_const_this->bind_icon(icon_name, texture.id());
	// Ensure that texture's destructor will not deallocate OpenGL
	// texture. Ownership is transferred to this IconRepository.
	texture.reset_id();
	
        it = icons_.find(icon_name);
        ogf_assert(it != icons_.end());
        return it->second.im_texture_id;
    }
}

