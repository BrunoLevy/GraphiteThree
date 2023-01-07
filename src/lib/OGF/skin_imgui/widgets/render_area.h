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
 
#ifndef H_SKIN_IMGUI_RENDER_AREA_H
#define H_SKIN_IMGUI_RENDER_AREA_H

#include <OGF/skin_imgui/common/common.h>
#include <OGF/skin/types/events.h>
#include <OGF/skin/transforms/ray_picker.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/gom/types/node.h>

#include <geogram/image/color.h>

struct GLUPContext;

namespace OGF {

    /**
     * \brief Implementation of RenderArea using ImGui.
     * \note Client code should not use this class directly. It
     *  should be only used through the GOM abstraction layer.
     */
    gom_class SKIN_IMGUI_API RenderArea : public Object {
    public:

        /**
         * \brief RenderArea constructor.
         * \param[in] w , h window width and height in pixels.
	 * \param[in] fb_w , fb_h frame buffer width and height in pixels.
         */
        RenderArea(index_t w, index_t h, index_t fb_w, index_t fb_h);

        /**
         * \brief RenderArea destructor.
         */
        ~RenderArea() override;

	/**
	 * \brief Resizes this RenderArea.
	 * \param[in] w , h the new window width and height in pixels.
	 * \param[in] fb_w , fb_h the new frame buffer width and height
	 *   in pixels. May be different from w and h on retina displays.
	 * \details Called whenenver the size of the window does not
	 *  match the current size.
	 */
	void resize(index_t w, index_t h, index_t fb_w, index_t fb_h);

	/**
	 * \brief Tests whether the window needs to be redrawn.
	 * \retval true if the window needs to be redrawn.
	 * \retval false if the window is up to date.
	 */
	bool needs_to_redraw() const {
	    return dirty_;
	}
	
	
    gom_slots:

	/**
	 * \brief Updates the memorized frame if need be,
	 *  then draws it.
	 */
	void draw();
	
	/**
	 * \brief Invalidates the memorized frame so that it
	 *  will be updated on the next call to draw().
	 */
	void update();

        /**
         * \brief Re-generates the memorized frame.
	 */
	void update_memorized_frame();
	
        /**
         * \brief Draws the latest memorized frame.
	 */
	void draw_memorized_frame();

	/**
	 * \brief Saves a snapshot of this rendering area
	 *  to an image file.
	 * \param[in] filename a string with the name of 
	 *  the image file.
	 * \param[in] make_current if true, makes the associated
	 *  rendering context current, else use the current 
	 *  rendering context.
	 */
	void snapshot(
	    const std::string& filename, bool make_current = true
	);

	/**
	 * \brief Call this function if perspective 
	 *  mode is changed.
	 */
	void update_view_parameters();

        /**
         * \brief Updates the background image from raw image data.
	 * \details Image data is in Cairo format (ARGB). Note that 
	 *  it is different from the convention in Graphite (RGBA). 
	 *  Note also that the Y axis is flipped as compared to the
	 *  rest of Graphite.
         * \param[in] ptr a pointer to image data
         * \param[in] color_encoding the color encoding of the data
         * \param[in] component_encoding the data type used for 
         *  the color components
         * \param[in] width image width
         * \param[in] height image height 
         */
        void update_background_image_from_data(
            Memory::pointer ptr,
            Image::ColorEncoding color_encoding,
            Image::ComponentEncoding component_encoding,
            index_t width, index_t height
        );
	
    gom_properties:
	/**
	 * \brief Gets the width of the window.
	 * \return the width of the window.
	 */
	index_t get_width() const {
	    return width_;
	}

	/**
	 * \brief Gets the height of the window.
	 * \return the height of the window.
	 */
	index_t get_height() const {
	    return height_;
	}

	/**
	 * \brief Gets the width of the window.
	 * \return the width of the frame buffer in pixels.
	 */
	index_t get_frame_buffer_width() const {
	    return frame_buffer_width_;
	}

	/**
	 * \brief Gets the height of the window.
	 * \return the height of the frame buffer in pixels.
	 */
	index_t get_frame_buffer_height() const {
	    return frame_buffer_height_;
	}
	
        /**
         * \brief Sets the projection matrix.
         * \param[in] value the projection matrix, as a 4x4
         *  homogeneous coordinates matrix.
         */
        void set_projection_matrix(const mat4& value);

        /**
         * \brief Gets the projection matrix.
         * \return a const reference to the 4x4 homogeneous
         *  coordinates projection matrix.
         */
        const mat4& get_projection_matrix() const;

        /**
         * \brief Sets the viewing matrix.
         * \param[in] value the viewing matrix, as a 4x4
         *  homogeneous coordinates matrix.
         */
        void set_viewing_matrix(const mat4& value);

        /**
         * \brief Gets the viewing matrix.
         * \return a const reference to the 4x4 homogeneous
         *  coordinates viewing matrix.
         */
        const mat4& get_viewing_matrix() const;

        /**
         * \brief Sets the primary background color.
         * \param[in] value the background color
         */
        void set_background_color_1(const Color& value);

        /**
         * \brief Gets the background color.
         * \return the background color
         */
        const Color& get_background_color_1() const;

        /**
         * \brief Sets the secondary background color.
         * \details If different from the (primary) background
         *  color, then a color-ramp between both is generated.
         * \param[in] value the secondary background color
         */
        void set_background_color_2(const Color& value);

        /**
         * \brief Gets the secondary background color.
         * \details If different from the (primary) background
         *  color, then a color-ramp between both is generated.
         */
        const Color& get_background_color_2() const;

        /**
         * \brief Sets the background image.
         * \details If a background image is specified, then it
         *  is used to fill the background (instead of the background
         *  color).
         * \param[in] value the path to a file that contains the image
         *  or the empty string if no background image should be used
         */
        void set_background_image(const std::string& value);

        /**
         * \brief Gets the background image.
         * \return the path to a file that contains the image, or the
         *  empty string if no background image is used.
         */
        const std::string& get_background_image() const;

        /**
         * \brief Sets the lighting matrix.
         * \details The light source is transformed by the lighting
         *  matrix. This transform occurs in normalized device coordinates.
         * \param[in] value a const reference to the 4x4 homongeneous 
         *  coordinates lighting matrix to be used
         */
        void set_lighting_matrix(const mat4& value);

        /**
         * \brief Gets the lighting matrix.
         * \return a const reference to the 4x4 homogeneous coordinates
         *  lighting matrix
         */
        const mat4& get_lighting_matrix() const;

        /**
         * \brief Gets the rendering context.
         * \return a pointer to the RenderingContext
         */
        RenderingContext* get_rendering_context() const;

        /**
         * \brief Gets the GPU information.
         * \return a string with GPU name, vendor, version
         */
        std::string get_gpu_information() const;

        /**
         * \brief Gets GPU extensions.
         * \return a string with the list of supported OpenGL extensions
         */
        std::string get_gpu_extensions() const;

        /**
         * \brief Gets the current clipping configuration
         * \return a ';'-separated string with the following fields:
         *  - active (one of "true","false")
         *  - axis (one of "x","y","z","d")
         *  - volume-mode (one of "std.","cell","strad.","slice")
         *  - shift (an integer in [-250, 250])
         *  - rotation (the 16 coefficients of a rotation matrix)
         */
        const std::string& get_clipping_config() const {
            return clipping_config_;
        }

        /**
         * \brief Sets the current clipping configuration
         * \param[in] clipping_config a ';'-separated string with 
         *  the following fields:
         *  - active (one of "true","false")
         *  - axis (one of "x","y","z","d")
         *  - volume-mode (one of "std.","cell","strad.","slice")
         *  - shift (an integer in [-250, 250])
         *  - rotation (the 16 coefficients of a rotation matrix)
         */
        void set_clipping_config(const std::string& clipping_config) {
            clipping_config_ = clipping_config;
            update_clipping_config();
        }

    gom_signals:
	/**
	 * \brief This signal is triggered when the contents 
	 *  shoud be redisplayed.
	 */
	void redraw_request(RenderingContext* rendering_context=nullptr);

        /**
         * \brief A signal that is triggered each time the render area
         *  is resized.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] width the new width
         * \param[in] height the new height
         */
        void resize_request(
            RenderingContext* rendering_context, int width, int height
        );

        /**
         * \brief A signal that is triggered each time a mouse button
         *  is pushed.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] point_ndc the picked point, in normalized device
         *  coordinates
         * \param[in] point_wc the picked point, in world coordinates
         * \param[in] button the pushed button (see MouseButton)
         * \param[in] control true if the control key is pushed
         * \param[in] shift true if the shift key is pushed
         */
        void mouse_down(
            RenderingContext* rendering_context,
            const vec2& point_ndc, const vec2& point_wc, 
            int button, bool control, bool shift
        );

        /**
         * \brief A signal that is triggered when the mouse is dragged with
         *  a pushed button.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] point_ndc the picked point, in normalized device
         *  coordinates
         * \param[in] point_wc the picked point, in world coordinates
         * \param[in] delta_ndc the displacement vector in normalized device
         *  coordinates
         * \param[in] delta_x_ndc the x component of the displacement vector
         *  in normalized device coordinates
         * \param[in] delta_y_ndc the y component of the displacement vector
         *  in normalized device coordinates
         * \param[in] delta_wc the displacement vector in world coordinates
         * \param[in] button the pushed button (see MouseButton)
         * \param[in] control true if the control key is pushed
         * \param[in] shift true if the shift key is pushed
         */
        void mouse_move(
            RenderingContext* rendering_context,
            const vec2& point_ndc, const vec2& point_wc, 
            const vec2& delta_ndc, double delta_x_ndc, double delta_y_ndc,
            const vec2& delta_wc,
            int button, bool control, bool shift
        );

        /**
         * \brief A signal that is triggered each time a mouse button
         *  is released.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] point_ndc the picked point, in normalized device
         *  coordinates
         * \param[in] point_wc the picked point, in world coordinates
         * \param[in] button the pushed button (see MouseButton)
         * \param[in] control true if the control key is pushed
         * \param[in] shift true if the shift key is pushed
         */
        void mouse_up(
            RenderingContext* rendering_context,
            const vec2& point_ndc, const vec2& point_wc, 
            int button, bool control, bool shift
        );

        /**
         * \brief A signal that is triggered each time a key is pushed.
         * \param[in] value a string with the pressed key, or "shift", 
         *  "control", "alt", "left", "right", "up", "down" for special
         *  keys.
         */
        void key_down(const std::string& value);

        /**
         * \brief A signal that is triggered each time a key is released.
         * \param[in] value a string with the pressed key, or "shift", 
         *  "control", "alt", "left", "right", "up", "down" for special
         *  keys.
         */
        void key_up(const std::string& value);

        /**
         * \brief A signal that is triggered each time a mouse button
         *  is pressed.
         * \param[in] value the button (see MouseButton)
         */
        void button_down(int value);

        /**
         * \brief A signal that is triggered each time a mouse button
         *  is released.
         * \param[in] value the button (see MouseButton)
         */
        void button_up(int value);

        /**
         * \brief A signal that is triggered whenever a file is drag-dropped
         *  into this RenderArea.
         * \param[in] value the name of the file that was dropped
         * \see set_accept_drops(), get_accept_drops()
         */
        void dropped_file(const std::string& value);

    public:
	void mouse_button_callback(int button, int action, int mods);
	void scroll_callback(double xoffset, double yoffset);
	void cursor_pos_callback(double xf, double yf);
	void drop_callback(int nb, const char** p);
	void char_callback(unsigned int c);
	void key_callback(int key, int scancode, int action, int mods);

    protected:

	/**
	 * \brief Initializes OpenGL and GLUP objects.
	 */
	void GL_initialize();

	/**
	 * \brief Deallocates OpenGL and GLUP objects.
	 */
	void GL_terminate();
	void update_clipping_config();

    private:
	RenderingContext_var rendering_context_;
	index_t width_;
	index_t height_;
	index_t frame_buffer_width_;
	index_t frame_buffer_height_;
	std::string clipping_config_;

	index_t button_down_;
	bool control_is_down_;
	bool shift_is_down_;
	vec2 last_point_dc_;
	vec2 last_point_ndc_;
	vec2 last_point_wc_;
	Color background_color_;
	Color background_color_2_;
	mat4 viewing_matrix_;
	mat4 lighting_matrix_;
	std::string background_image_;
	GLUPContext* glup_context_;
	bool dirty_;
    };

    typedef SmartPointer<RenderArea> RenderArea_var;

}

#endif

