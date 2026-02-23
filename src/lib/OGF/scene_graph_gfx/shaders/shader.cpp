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


#include <OGF/scene_graph_gfx/shaders/shader.h>
#include <OGF/scene_graph_gfx/shaders/scene_graph_shader_manager.h>
#include <OGF/scene_graph/grob/grob.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/gom/types/node.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_struct.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/basic/os/file_manager.h>

#include <geogram/image/image.h>
#include <geogram/image/image_library.h>
#include <geogram/basic/command_line.h>

namespace OGF {

//_________________________________________________________

    Shader::Shader(
        Grob* grob
    ) :
        grob_(grob),
        no_grob_update_(false),
	transparency_(TRANSP_OPAQUE) {
    }

    Shader::~Shader() {
    }

    void Shader::draw() {
        glupGetMatrixdv(GLUP_MODELVIEW_MATRIX, modelview_);
        glupGetMatrixdv(GLUP_PROJECTION_MATRIX, project_);
        glGetIntegerv(GL_VIEWPORT, viewport_);
    }

    void Shader::update() {
        if(no_grob_update_) {
           grob_->scene_graph()->update();
        } else {
           // For the moment, redraw everything
           grob_->update();
        }
    }

    void Shader::blink() {
    }

    bool Shader::dark_mode() const {
        std::string gui_mode = CmdLine::get_arg("gui:style");
        return gui_mode == "Dark";
    }

    bool Shader::set_property(const std::string& name, const Any& value) {
	Interpreter* interpreter =
	    (grob_ == nullptr) ? nullptr : grob_->interpreter();
	return set_property_and_record_to_history(name, value, interpreter);
    }


    Texture* Shader::create_texture_from_file(
        const std::string& file_name, GLint filtering, GLint wrapping
    ) {
        if(file_name.length() == 0) {
            return nullptr ;
        }
        Image_var image = ImageLibrary::instance()->load_image(file_name) ;
        if(image.is_null()) {
            return nullptr ;
        }
        Texture* result = new Texture;
        result->create_from_image(image, filtering, wrapping);
        return result;
    }


    Texture* Shader::create_texture_from_colormap_name(
        const std::string& name, GLint filtering, GLint wrapping
    ) {

       std::string filename = "icons/colormaps/" + name + ".xpm" ;
       FileManager::instance()->find_file(filename) ;

        Image_var image = ImageLibrary::instance()->load_image(filename) ;
        if(image == nullptr) {
            return nullptr ;
        }

        // Create a 1D image with the first row of the image.
        Image_var image1D = new Image(
            image->color_encoding(),
            image->component_encoding(),
            image->width()
        );

        Memory::copy(
            image1D->base_mem(), image->base_mem(),
            image->width() * image->bytes_per_pixel()
        );

	{
	    geo_assert(image1D->bytes_per_pixel() == 4);
	    image1D->base_mem()[4*0+3] = 0;
	    image1D->base_mem()[4*(image->width()-1)+3] = 0;
	}


        Texture* result = new Texture;
        result->create_from_image(image1D, filtering, wrapping);
        return result;
    }

//_________________________________________________________

}
