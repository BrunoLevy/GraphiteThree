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

#include <OGF/skin_imgui/widgets/render_area.h>
#include <OGF/skin_imgui/types/rendering_context.h>
#include <OGF/skin_imgui/types/application.h>
#include <OGF/basic/math/geometry.h>

#include <geogram/image/image_library.h>

// Too many documentation warnings in glfw
// (glfw uses tags that clang does not understand).
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif

#ifdef GEO_USE_SYSTEM_GLFW3
#include <GLFW/glfw3.h>
#else
#include <third_party/glfw/include/GLFW/glfw3.h>
#endif

namespace OGF {

    RenderArea::RenderArea(index_t w, index_t h, index_t fb_w, index_t fb_h) {
	width_ = w;
	height_ = h;
	frame_buffer_width_ = fb_w;
	frame_buffer_height_ = fb_h;

	background_color_ = Color(1.0,1.0,1.0,1.0);
	background_color_2_ = Color(1.0,1.0,1.0,1.0);
	viewing_matrix_.load_identity();
	lighting_matrix_.load_identity();
	last_point_dc_ = vec2(0.0, 0.0);
	last_point_ndc_ = vec2(0.0, 0.0);
	last_point_wc_ = vec2(0.0, 0.0);
	
        // active;axis;volume_mode;shift;rotation;flip        
        clipping_config_ =
            "false;z;strad.;0;1 0 0 0  0 1 0 0  0 0 1 0  0 0 0 1;false";

	button_down_ = 0;
	control_is_down_ = false;
	shift_is_down_ = false;

	dirty_ = true;
    }
    
    RenderArea::~RenderArea() {
    }
    
    void RenderArea::set_projection_matrix(const mat4& value) {
	if(rendering_context_ != nullptr) {	
	    rendering_context_->load_projection_matrix(value);
	}
	update();
    }

    const mat4& RenderArea::get_projection_matrix() const {
	static mat4 result;
	result = rendering_context_->projection_matrix();
	return result;
    }

    void RenderArea::set_viewing_matrix(const mat4& value) {
	viewing_matrix_ = value;
	if(rendering_context_ != nullptr) {
	    rendering_context_->load_viewing_matrix(value);
	}
	update();
    }

    const mat4& RenderArea::get_viewing_matrix() const {
	return viewing_matrix_;
    }

    void RenderArea::set_background_color_1(const Color& value) {
	background_color_ = value;
	if(rendering_context_ != nullptr) {
	    rendering_context_->set_background_color(value);
	}
	update();
    }

    const Color& RenderArea::get_background_color_1() const {
	return background_color_;
    }

    void RenderArea::set_background_color_2(const Color& value) {
	background_color_2_ = value;
	if(rendering_context_ != nullptr) {
	    rendering_context_->set_background_color_2(value);
	}
	update();
    }

    const Color& RenderArea::get_background_color_2() const {
	return background_color_2_;
    }

    void RenderArea::set_background_image(const std::string& value) {
	background_image_ = value;
	if(rendering_context_ != nullptr) {
	    if(value.length() > 0) {
		Image_var image = 
		    ImageLibrary::instance()->load_image(value);
		rendering_context_->set_background_image(image);
	    } else {
		rendering_context_->set_background_image(nullptr);
	    }
	}
	update();
    }

    const std::string& RenderArea::get_background_image() const {
	return background_image_;
    }

    void RenderArea::set_lighting_matrix(const mat4& value) {
	lighting_matrix_ = value;
	if(rendering_context_ != nullptr) {
	    rendering_context_->set_lighting_matrix(value);
	}
	update();
    }

    const mat4& RenderArea::get_lighting_matrix() const {
	if(rendering_context_ != nullptr) {
	    return rendering_context_->lighting_matrix();
	}
	return lighting_matrix_;
    }

    RenderingContext* RenderArea::get_rendering_context() const {
	return rendering_context_;
    }

    std::string RenderArea::get_gpu_information() const {
        RenderingContext* rc = rendering_context_;
        if(rc == nullptr) {
            return "Unknown GPU (uninitialized rendering context)";
        }
        return rc->get_gpu_information();
    }

    std::string RenderArea::get_gpu_extensions() const {
        RenderingContext* rc = rendering_context_;
        if(rc == nullptr) {
            return "Unknown GPU (uninitialized rendering context)";
        }
        return rc->get_gpu_extensions();
    }

    void RenderArea::update_clipping_config() {
        std::vector<std::string> words;
        String::split_string(clipping_config_, ';', words);
        if(words.size() != 6) {
            Logger::warn("Skin_imgui")
                << "update_clipping_config(): Wrong clipping config: "
                << clipping_config_ << std::endl;
            return;
        }
        if(rendering_context_ == nullptr) {
            Logger::warn("Skin_imgui")
                << "update_clipping_config(): no rendering context"
                << std::endl;
            return;
        }
        
        rendering_context_->set_clipping(String::to_bool(words[0]));

        bool viewer = false;
        vec4 eqn(0.0, 0.0, 0.0, 0.0);

        if(words[1] == "x") {
            eqn[0] = 1.0;
        } else if(words[1] == "y") {
            eqn[1] = 1.0;
        } else if(words[1] == "z") {
            eqn[2] = 1.0;
        } else if(words[1] == "d") {
            eqn[2] = 1.0;
            viewer = true;
        } else {
            Logger::warn("Skin_imgui")
                << "update_clipping_config(): invalid clipping axis:"
                << words[2]
                << std::endl;
        }

        double shift = double(String::to_int(words[3]) + 500)/1000.0;
        eqn[3] = 1.0 - (shift * 2.0); // in [-1,1] rather than [0,1] !!
        
        if(!String::to_bool(words[5])) {
            eqn = -1.0 * eqn;
        }
        
        rendering_context_->set_clipping_equation(eqn);
        rendering_context_->set_clipping_viewer(viewer);

        GLUPclipMode mode = GLUP_CLIP_STANDARD;
        if(words[2] == "std.") {
            mode = GLUP_CLIP_STANDARD;
        } else if(words[2] == "cell") {
            mode = GLUP_CLIP_WHOLE_CELLS;
        } else if(words[2] == "strad.") {
            mode = GLUP_CLIP_STRADDLING_CELLS;
        } else if(words[2] == "slice") {
            mode = GLUP_CLIP_SLICE_CELLS;            
        } else {
            Logger::warn("Skin_imgui")
                << "update_clipping_config(): invalid clipping mode:"
                << words[2]
                << std::endl;
        }
        rendering_context_->set_clipping_mode(mode);

        mat4 rotation;
        if(!String::from_string(words[4],rotation)) {
            Logger::warn("Skin_imgui")
                << "update_clipping_config(): invalid clipping rotation:"
                << words[3]
                << std::endl;
        } else {
            rendering_context_->set_clipping_matrix(rotation);
        }
	update();
    }

    void RenderArea::draw() {
	// The contents of the off-screen buffer should be generated if:
	//   - it is marked as dirty_ (update() was called before) or
	//   - the off-screen buffer contains a picking image (i.e., with
	//     object IDs rather than colors). 
	if(dirty_ || rendering_context_->contains_picking_image()) {
	    dirty_ = false;	    
	    update_memorized_frame();
	}
	draw_memorized_frame();
    }
    
    void RenderArea::update() {
	dirty_ = true;
    }

    void RenderArea::update_memorized_frame() {
	if(rendering_context_ == nullptr) {
	    GL_initialize();
	}
	rendering_context_->begin_frame();
	redraw_request(rendering_context_);
	rendering_context_->end_frame();
    }

    void RenderArea::draw_memorized_frame() {
	SkinImGUIRenderingContext* ctxt =
	    dynamic_cast<SkinImGUIRenderingContext*>(
		rendering_context_.get()
	    );
	if(ctxt != nullptr) {
	    ctxt->draw_last_frame();
	}
    }
    
    void RenderArea::resize(
	index_t w, index_t h, index_t fb_w, index_t fb_h
    ) {
	width_ = w;
	height_ = h;
	frame_buffer_width_ = fb_w;
	frame_buffer_height_ = fb_h;
	if(rendering_context_ != nullptr) {
	    rendering_context_->resize(fb_w, fb_h);
	    resize_request(rendering_context_, int(fb_w), int(fb_h));
	}
	update();
    }

    void RenderArea::GL_initialize() {
#ifndef __EMSCRIPTEN__    
	if(!gladLoadGL()) {
	    printf("GLAD: could not load OpenGL\n");
	    exit(-1);
	}
#endif

	if(glupCurrentContext() == nullptr) {
	    glupMakeCurrent(glupCreateContext());
	}

	if(glupCurrentContext() == nullptr) {
	    Logger::err("Skin_imgui") << "Could not create GLUP context"
				      << std::endl;
	    exit(-1);
	}

	rendering_context_ = new SkinImGUIRenderingContext(
	    glupCurrentContext()
	);
	
	rendering_context_->resize(
	    get_frame_buffer_width(), get_frame_buffer_height()
	);
	rendering_context_->set_background_color(background_color_);
	rendering_context_->set_background_color_2(background_color_2_);
	rendering_context_->load_viewing_matrix(viewing_matrix_);
	rendering_context_->set_lighting_matrix(lighting_matrix_);
	update_clipping_config();
	
	update();	
    }

    void RenderArea::GL_terminate() {
	std::cerr << "RENDER_AREA::GL_terminate()" << std::endl;
	glupDeleteContext(glupCurrentContext());
	glupMakeCurrent(nullptr);
    }
    
    void RenderArea::mouse_button_callback(
	int button, int action, int mods
    ) {

        if(button > 6) {
            return;
        }

        // Maps GLFW button id to RenderArea symbolic constant.
        const MouseButton button_mapping[7] = {
            MOUSE_BUTTON_LEFT,
            MOUSE_BUTTON_RIGHT,
            MOUSE_BUTTON_MIDDLE,
            MOUSE_BUTTON_WHEEL_UP,
            MOUSE_BUTTON_WHEEL_DOWN,
            MOUSE_BUTTON_AUX1,
            MOUSE_BUTTON_AUX2
        };

	last_point_ndc_ = rendering_context_->screen_to_ndc(
	    index_t(last_point_dc_.x), index_t(last_point_dc_.y)
        );
	    
	last_point_wc_ = transform_point(
	    last_point_ndc_,
	    rendering_context_->inverse_viewing_matrix()
        );
	
	bool control = ((mods & GLFW_MOD_CONTROL) != 0);
	bool shift = ((mods & GLFW_MOD_SHIFT) != 0);
	switch(action) {
	    case GLFW_PRESS: {
		control_is_down_ = control;
		shift_is_down_ = shift;
                button_down_ = button_mapping[button];

		if(button_down_ != 0) {
		    mouse_down(
			rendering_context_, last_point_ndc_,last_point_wc_,
			int(button_down_), control_is_down_, shift_is_down_
		    );
		}
		
	    } break;
	    case GLFW_RELEASE: {
		mouse_up(
		    rendering_context_, last_point_ndc_,last_point_wc_,
		    int(button_down_), control_is_down_, shift_is_down_
		);
		control_is_down_ = false;
		shift_is_down_ = false;
		button_down_ = 0;
	    } break;
	}
    }

    void RenderArea::cursor_pos_callback(
	double xf, double yf	
    ) {
	// For retina displays: x and y are in 'window pixels',
	// and GLUP project / unproject expect 'framebuffer pixels'.
	double sx = double(get_frame_buffer_width()) / double(get_width());
	double sy = double(get_frame_buffer_height()) / double(get_height());

	xf *= sx;
	yf *= sy;

	last_point_dc_.x = xf;
	last_point_dc_.y = yf;
	
	if(button_down_ != 0) {
	    vec2 prev_point_ndc = last_point_ndc_;
	    vec2 prev_point_wc = last_point_wc_;

	    last_point_ndc_ = rendering_context_->screen_to_ndc(
		index_t(last_point_dc_.x), index_t(last_point_dc_.y)
	    );
	    
	    last_point_wc_ = transform_point(
		last_point_ndc_, rendering_context_->inverse_viewing_matrix()
	    );

	    vec2 delta_ndc = last_point_ndc_ - prev_point_ndc;
	    vec2 delta_wc  = last_point_wc_ - prev_point_wc;

	    mouse_move(
		rendering_context_,
		last_point_ndc_, last_point_wc_,
		delta_ndc, delta_ndc.x, delta_ndc.y,
		delta_wc,
		int(button_down_), control_is_down_, shift_is_down_
	    );
	}
    }
    
    void RenderArea::scroll_callback(
	 double xoffset, double yoffset
    ) {
	geo_argused(xoffset);

	// Mouse wheel is inversed on Apple as compared to
	// other OSes.
#ifdef GEO_OS_APPLE
	yoffset = -yoffset;
#endif

	// Synthetize move/press/release mouse events
	// with center button pressed.
        
	vec2 last_point_dc_bak = last_point_dc_;
	GLFWwindow* w = (GLFWwindow*)Application::instance()->impl_window();
	bool ctrl = (glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
	bool shift = (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

        // Note: this is a GLFW button code (not RenderArea::Button that uses
        // a different mapping).
        const int button = (yoffset > 0) ? 3 : 4;

	int mods = (ctrl  ? GLFW_MOD_CONTROL : 0) |
	           (shift ? GLFW_MOD_SHIFT   : 0) ;
	
        // Wheel emulates move/press/move/release event
	cursor_pos_callback(last_point_dc_.x, last_point_dc_.y);
	mouse_button_callback(button, GLFW_PRESS, mods);
	cursor_pos_callback(last_point_dc_.x,last_point_dc_.y-double(yoffset));
	mouse_button_callback(button, GLFW_RELEASE, mods);
	cursor_pos_callback(last_point_dc_bak.x, last_point_dc_bak.y);
    }
    
    void RenderArea::drop_callback(int nb, const char** p) {
	for(int i=0; i<nb; ++i) {
	    dropped_file(p[i]);
	}
    }
    
    void RenderArea::char_callback(unsigned int c) {
	char str[2];
	str[0] = char(c);
	str[1] = '\0';
	key_down(std::string(str));
	key_up(std::string(str));
    }
    
    void RenderArea::key_callback(
	int key, int scancode, int action, int mods
    ) {
	geo_argused(scancode);
	geo_argused(mods);
	const char* keystr = nullptr;
	switch(key) {
	    case GLFW_KEY_LEFT_CONTROL:
	    case GLFW_KEY_RIGHT_CONTROL:		
	      keystr = "control";
	      break;
	    case GLFW_KEY_LEFT_SHIFT:
	    case GLFW_KEY_RIGHT_SHIFT:		
	      keystr = "shift";
	      break;
	    case GLFW_KEY_LEFT_ALT:
	    case GLFW_KEY_RIGHT_ALT:
	      keystr = "alt";
	      break;
	    case GLFW_KEY_LEFT:
	      keystr = "left";
              break;
	    case GLFW_KEY_RIGHT:
	      keystr = "right";
              break;
	    case GLFW_KEY_UP:
	      keystr = "up";
              break;
	    case GLFW_KEY_DOWN:
	      keystr = "down";
              break;
	    case GLFW_KEY_F1:
	      keystr = "F1";
              break;
	    case GLFW_KEY_F2:
	      keystr = "F2";
              break;
	    case GLFW_KEY_F3:
	      keystr = "F3";
              break;
	    case GLFW_KEY_F4:
	      keystr = "F4";
              break;
	    case GLFW_KEY_F5:
	      keystr = "F5";
              break;
	    case GLFW_KEY_F6:
	      keystr = "F6";
              break;
	    case GLFW_KEY_F7:
	      keystr = "F7";
              break;
	    case GLFW_KEY_F8:
	      keystr = "F8";
              break;
	    case GLFW_KEY_F9:
	      keystr = "F9";
              break;
	    case GLFW_KEY_F10:
	      keystr = "F10";
              break;
	    case GLFW_KEY_F11:
	      keystr = "F11";
              break;
	    case GLFW_KEY_F12:
	      keystr = "F12";
              break;
	    case GLFW_KEY_ESCAPE:
	      keystr = "escape";
              break;
	    case GLFW_KEY_PAGE_UP:
	      keystr = "page_up";
	      break;
	    case GLFW_KEY_PAGE_DOWN:
	      keystr = "page_down";
	      break;
	    case GLFW_KEY_HOME:
	      keystr = "home";
	      break;
	    case GLFW_KEY_END:
	      keystr = "end";
	      break;
	}
	if(keystr != nullptr) {
	    if(action == GLFW_PRESS) {
		key_down(std::string(keystr));
	    } else if(action == GLFW_RELEASE) {
		key_up(std::string(keystr));
	    }
	}
    }

    /*************************************************************************/

    void RenderArea::snapshot(const std::string& filename, bool make_current) {
	if(rendering_context_ == nullptr) {
	    Logger::warn("RenderArea")
		<< "Cannot take a snapshot, not initialized yet"
		<< std::endl;
	} else {
            Image image;
            rendering_context_->snapshot(&image, make_current);
            ImageLibrary::instance()->save_image(filename,&image);
	}
    }

    void RenderArea::update_view_parameters() {
	if(rendering_context_ != nullptr) {
	    rendering_context_->get_view_parameters();
	}
	update();
    }

    /**
     * \brief Flips both the Y axis and RGB <-> BGR
     * \details This is required because Cairo has conventions different
     *  from us.
     */
    static void flip_flip(
        Memory::pointer ptr,
        unsigned int width, 
        unsigned int height
    ) {
        ogf_assert(width <= 4096);

        // Flip image
        char temp[4096];
        unsigned int line_size = width*4;
        for(unsigned int i=0; i<height/2; i++) {
            Memory::pointer line1 = ptr + i*line_size;
            Memory::pointer line2 = ptr + (height - 1 - i)*line_size;
            Memory::copy(temp,  line1, line_size);
            Memory::copy(line1, line2, line_size);
            Memory::copy(line2, temp,  line_size);
        }

        // Cairo's ARGB (i.e. BGRA) -> RGBA
        for(unsigned int i=0; i<height; i++) {
            Memory::byte* pixel = ptr+i*line_size;
            for(unsigned int j=0; j<width; j++) {
                Memory::byte a = pixel[3];
                Memory::byte r = pixel[2];
                Memory::byte g = pixel[1];
                Memory::byte b = pixel[0];
                pixel[0] = r;
                pixel[1] = g;
                pixel[2] = b;
                pixel[3] = a;
                pixel += 4;
            }
        }
    }
    
    void RenderArea::update_background_image_from_data(
	Memory::pointer ptr,
	Image::ColorEncoding color_encoding,
	Image::ComponentEncoding component_encoding,
	index_t width, index_t height
    ) {
	if(ptr == nullptr || rendering_context_ == nullptr) {
	    return;
	}
        // Grrr, CAIRO stores in format ARGB and I use RGBA 
        //  (and Y is flipped !!!)
        flip_flip(ptr, width, height);

        rendering_context_->update_background_image_from_data(
            ptr, color_encoding, component_encoding,
            width, height
        );
       
        // Restore image.
        flip_flip(ptr, index_t(width), index_t(height));        
    }

    
}

