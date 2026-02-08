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


#ifndef H_OGF_RENDERER_CONTEXT_RENDERING_CONTEXT_H
#define H_OGF_RENDERER_CONTEXT_RENDERING_CONTEXT_H

#include <OGF/renderer/common/common.h>
#include <OGF/renderer/context/texture.h>
#include <OGF/renderer/context/overlay.h>
#include <geogram/image/image.h>
#include <geogram_gfx/full_screen_effects/full_screen_effect.h>
#include <geogram_gfx/basic/GL.h>

/**
 * \file OGF/renderer/context/rendering_context.h
 * \brief Helper class for OpenGL context management
 */

namespace OGF {

    class Colormap;
    class RenderArea;

/*****************************************************************/

   /**
    * \brief Helper class for OpenGL context management.
    * \details RenderingContext manages the viewing matrices
    *  and light sources. It also provides functionalities
    *  for picking.
    */

    class RENDERER_API RenderingContext : public Counted {
    public:

        /**
         * \brief RenderingContext constructor.
	 * \param[in] glup_context the GLUP context to be used
	 *  for rendering, or 0. If 0, then RenderingContext will
	 *  create and manage its own GLUP context.
         */
        RenderingContext(GLUPcontext glup_context=nullptr);

        /**
         * \brief RenderingContext destructor.
         */
        ~RenderingContext() override;

        /**
         * \brief Gets the current RenderingContext.
         * \return a pointer to the current RenderingContext
         */
        static RenderingContext* current() {
            return current_;
        }


        /**
         * \brief Tests whether a rendering operation is
         *  occuring.
         * \details This concerns all the rendering contexts
         *  (thus a static method).
         * \retval true if a rendering operation is currently
         *  occuring
         * \retval false otherwise
         */
        static bool is_currently_rendering();


        /**
         * \brief Tests whether a picking operation is
         *  occuring.
         * \details This concerns all the rendering contexts
         *  (this a static method).
         * \retval true if the content of one of the framebuffers
         *  is a picking image
         * \retval false otherwise
         */
        static bool is_currently_picking();


        /**
         * \brief Gets the width.
         * \return the width of this RenderingContext in pixels.
         */
        index_t get_width() const {
            return width_;
        }

        /**
         * \brief Gets the height.
         * \return the height of this RenderingContext in pixels.
         */
        index_t get_height() const {
            return height_;
        }

        /**
         * \brief Tests whether this RenderingContext is initialized.
         * \details A RenderingContext is initialized once the first
         *  drawing event was processed.
         * \retval true if this RenderingContext is initialized
         * \retval false otherwise
         */
        bool initialized() const {
            return initialized_;
        }

        /**
         * \brief Makes this RenderingContext the current one.
         * \details OpenGL uses global variables. Since several
         * 3D graphic windows may be
         * opened at the same time, the corresponding renderer
         * has to be 'made current' before issuing 3D drawing
         * commands to it. Note that begin_frame() automatically
         * calls make_current() and clear().
         */
        virtual void make_current();

        /**
         * \brief This function should be called when client
         *  code is done with rendering to the current context.
         * \details Some implementations of make_current() acquire
         *  a lock, that is released by done_current()
         */
        virtual void done_current();

        /**
         * \brief Meant to make the rendered frame visible.
         * \details Base class implementation does nothing.
         *  In most cases, this operation is already done
         *  by the toolkit (e.g. Qt), therefore this function
         *  does not need to be overloaded in most cases.
         */
        virtual void swap_buffers();

        /**
         * \brief Clears this RenderingContext.
         * \details This clears both the color buffer and the frame buffer.
         */
        void clear();

        /**
         * \brief Sets the current viewing matrix
         * \details This just changes the stored viewing matrix,
         *  nothing is sent to OpenGL. The viewing matrix is sent to
         *  OpenGL when begin_frame() is called.
         * \param[in] m a const reference to the viewing matrix
         */
        void load_viewing_matrix(const mat4& m);

        /**
         * \brief Multiplies the current viewing matrix by another one.
         * \details This just changes the stored viewing matrix,
         *  nothing is sent to OpenGL. The viewing matrix is sent to
         *  OpenGL when begin_frame() is called.
         *  This replaces the viewing matrix with m times the
         *  viewing matrix (mutliplies m on the left).
         * \param[in] m a const reference to the viewing matrix
         */
        void mul_viewing_matrix(const mat4& m);

        /**
         * \brief Gets the current viewing matrix.
         * \return a const reference to the current viewing matrix.
         */
        const mat4& viewing_matrix() const;

        /**
         * \brief Gets the inverse of the current viewing matrix.
         * \return a const reference to the inverse of the current
         *  viewing matrix.
         */
        const mat4& inverse_viewing_matrix() const;

        /**
         * \brief Sets the projection matrix and sends it to OpenGL.
         * \param[in] m a const reference to the projection matrix.
         */
        void load_projection_matrix(const mat4& m);

        /**
         * \brief Gets the projection matrix.
         * \return The projection matrix
         */
        mat4 projection_matrix() const;

        /**
         * \brief Begins a new frame.
         * \details This clears the color and depth buffer,
         *  and setups the viewing matrix. This function
         *  calls make_current() before issuing any OpenGL
         *  call.
         */
        virtual void begin_frame();

        /**
         * \brief Terminates a frame.
         */
        virtual void end_frame();


        /**
         * \brief Gets the primary background color.
         * \details The primary background color is used to clear
         *  the color buffer.
         * \return a const reference to the primary background color
         */
        const Color& background_color() const;

        /**
         * \brief Sets the primary background color.
         * \param[in] c a const reference to the primary background color
         */
        void set_background_color(const Color& c);


        /**
         * \brief Gets the secondary background color.
         * \details If the secondary background color is different from
         *  the primary background color, then a vertical colorramp is
         *  generated in the background.
         * \return a const reference to the secondary background color
         */
        const Color& background_color_2() const;

        /**
         * \brief Sets the secondary background color.
         * \details If the secondary background color is different from
         *  the primary background color, then a vertical colorramp is
         *  generated in the background.
         * \param[in] c a const reference to the secondary background color
         */
        void set_background_color_2(const Color& c);

        /**
         * \brief Sets the background image.
         * \details If a background image is defined, then it is
         *  used to fill the background. This generates an OpenGL
         *  texture from the image.
         * \param[in] image a pointer to the image
         */
        void set_background_image(Image* image);

        /**
         * \brief Updates the background image from raw data pointer.
         * \details If a background texture is already present, then
         *  it updates it with the data, else it creates a new
         *  background texture.
         * \param[in] ptr a pointer to image data
         * \param[in] color_encoding the color encoding of the data
         * \param[in] component_encoding the data type used for the
         *  color components
         * \param[in] width image width
         * \param[in] height image height
         */
        void update_background_image_from_data(
            Memory::pointer ptr,
            Image::ColorEncoding color_encoding,
            Image::ComponentEncoding component_encoding,
            index_t width, index_t height
        );

        /**
         * \brief Sets the lighting matrix.
         * \details The lighting matrix transforms the light sources. Nothing
         *  is sent to OpenGL, this is taken into account by begin_frame().
         * \param[in] m a const reference to the lighting matrix.
         */
        void set_lighting_matrix(const mat4& m);

        /**
         * \brief Gets the lighting matrix.
         * \return a const reference to the lighting matrix.
         */
        const mat4& lighting_matrix() const;

        /**
         * \brief Gets the clipping matrix.
         * \details The clipping matrix transforms the clipping plane.
         * \return a const reference to the clipping matrix
         */
        mat4 clipping_matrix() const;

        /**
         * \brief Sets the lighting matrix.
         * \details The clipping matrix transforms the clipping plane. Nothing
         *  is sent to OpenGL, this is taken into account by begin_frame().
         * \param[in] m a const reference to the clipping matrix.
         */
        void set_clipping_matrix(const mat4& m);

        /**
         * \brief Activates or deactivates clipping.
         * \param[in] x true if clipping should be used, false otherwise
         * \see set_clipping_matrix(), clipping_matrix()
         */
        void set_clipping(bool x);

        /**
         * \brief Tests whether clipping is active.
         * \retval true if clipping is active
         * \retval false otherwise
         */
        bool get_clipping() const;

        /**
         * \brief Sets the clipping equation.
         * \param[in] x a const reference to the clipping equation,
         *  stored in a vec4.
         */
        void set_clipping_equation(const vec4& x) {
            clipping_equation_ = x;
        }

        /**
         * \brief Gets the clipping equation.
         * \return a const reference to the clipping equation as
         *  a vec4
         */
        const vec4& get_clipping_translation() const {
            return clipping_equation_;
        }

        /**
         * \brief Sets whether clipping translation and rotation
         *  are applied in object bounding box or in viewer coordinates.
         * \param[in] x true if clipping plane is fixed in viewer space, false
         *  if clipping is defined in object space
         */
        void set_clipping_viewer(bool x) {
            clipping_viewer_ = x;
        }

        /**
         * \brief Tests whether clipping translation and rotation
         *  are applied in object bounding box or in viewer coordinates.
         * \retval true if clipping plane is fixed in viewer space
         * \retval false if clipping is defined in object space
         */
        bool get_clipping_viewer() const {
            return clipping_viewer_;
        }

        /**
         * \brief Gets the current clipping mode.
         * \return one of GLUP_CLIP_STANDARD,
         *  GLUP_CLIP_WHOLE_CELLS, GLUP_CLIP_STRADDLING_CELLS,
         *  GLUP_CLIP_SLICE_CELLS
         */
        GLUPclipMode get_clipping_mode() const {
            return clipping_mode_;
        }

        /**
         * \brief Sets the current clipping mode.
         * \param[in] mode one of GLUP_CLIP_STANDARD,
         *  GLUP_CLIP_WHOLE_CELLS, GLUP_CLIP_STRADDLING_CELLS,
         *  GLUP_CLIP_SLICE_CELLS
         */
        void set_clipping_mode(GLUPclipMode mode) {
            clipping_mode_ = mode;
        }

        /**
         * \brief Tests whether this RenderingContext uses double buffering.
         * \retval true if double buffering is used
         * \retvavl false otherwise
         */
        bool double_buffer() const {
            return double_buffer_;
        }

        /**
         * \brief Tests whether this RenderingContext uses stereo rendering.
         * \retval true if stereo rendering is used
         * \retval false otherwise
         */
        bool stereo() const {
            return stereo_;
        }

        /**
         * \brief Specifies whether double buffering should be used.
         * \param[in] b true if double buffering should be used, false otherwise
         */
        virtual void set_double_buffer(bool b);

        /**
         * Copies the color buffer onto the specified image.
         * \param[out] image The image, it should have the same size as
	 *  this RenderingContext and should be in RGB mode.
	 * \param[in] make_current if true, makes this rendering context
	 *  the current rendering context before reading the pixels.
         * \param[in] x0 , y0 , width , height optional image bounds. If let
         *  unspecified, the entire picking buffer is copied.
         */
        virtual void snapshot(
            Image* image, bool make_current=true,
            index_t x0=0, index_t y0=0,
            index_t width=0, index_t height=0
        );

        /**
         * \brief Gets the OpenGL vendor.
         * \return the OpenGL vendor as a string.
         */
        std::string gl_vendor() const;

        /**
         * \brief Gets the OpenGL renderer.
         * \return the OpenGL renderer as a string.
         */
        std::string gl_renderer() const;

        /**
         * \brief Gets the OpenGL version.
         * \return the OpenGL version as a string.
         */
        std::string gl_version() const;

        /**
         * \brief Gets the OpenGL extension.
         * \return a string with the list of the supported OpenGL
         *  extensions
         */
        std::string gl_extensions() const;

        /**
         * \brief Gets the information on the GPU.
         * \return a string with the OpenGL renderer, version and
         *  vendor
         */
        std::string get_gpu_information() const;

        /**
         * \brief Gets the GPU extensions
         * \return same as gl_extensions()
         */
        std::string get_gpu_extensions() const;

        /**
         * \brief Sets the full screen effect
         * \details Full screen effects are shaders that operate
         *  on the content of the color buffer and depth buffer.
         * \param[in] fse a pointer to the FullScreenEffectImpl
         *  that implements the full screen effect or nil if no
         *  full screen effect should be used.
         */
        void set_full_screen_effect(FullScreenEffectImpl* fse);

        /**
         * \brief Gets the current full screen effect.
         * \return a pointer to the implementation of the current
         *  full screen effect, or nil if no full screen effect
         *  is used.
         */
        FullScreenEffectImpl* get_full_screen_effect() const {
            return full_screen_effect_;
        }

        /**
         * \brief Must be called whenever the rendering context
         *  is resized.
         * \details Updates the viewport transform.
         * \param[in] w the new width
         * \param[in] h the new height
         */
        virtual void resize(index_t w, index_t h);


	/**
	 * \brief Sets the center
	 * \details This corresponds to the pixel where cordinates (0.0,0.0)
	 *   will be projected on the screen
	 * \param[in] x , y the center in pixel coordinates
	 */
	void set_center(int x, int y);

	int get_center_x() const {
	    return center_x_;
	}

	int get_center_y() const {
	    return center_y_;
	}

        /**
         * \brief Transforms screen coordinates to normalized
         *  device coordinates (viewport transform).
         * \param[in] x , y the integer coordinates of a point, in
         *  screen space.
         * \return the coordinates of the point in normalized device
         *  coordinates (both between -1.0 and 1.0).
         */
        vec2 screen_to_ndc(index_t x, index_t y) const;

        /**
         * \brief Transforms normalized device coordinates to
         *  screen coordinates (inverse viewport transform).
         * \parma[in] ndc the normalized device coordinates of the
         *  point (x and y between -1.0 and 1.0).
         * \param[out] x , y the integer coordinates of the point, in
         *  screen space
         */
        void ndc_to_screen(const vec2& ndc, index_t& x, index_t& y) const;

        /**
         * \brief Enters picking mode.
         * \details This function should be called before begin_frame()
         * \param[in] ndc ndc the normalized device coordinates of the
         *  picked point (x and y between -1.0 and 1.0).
         */
        virtual void begin_picking(const vec2& ndc);

        /**
         * \brief Exits picking mode.
         * \details This function should be called after end_frame().
         */
        virtual void end_picking();

        /**
         * \brief Gets the picked id that was encoded in the pixel color
         *  under the mouse pointer.
         * \details Some specialized shaders replace the color with
         *  (color-coded) identifiers.
         * \return the picked id
         */
        index_t picked_id() const {
            return picked_id_;
        }

        /**
         * \brief Gets the depth of the picked point in screen
         *   coordinates.
         * \return the depth of the picked point
         */
        double picked_depth() const {
            return picked_depth_;
        }

        /**
         * \brief Gets the picked point in world coordinates.
         * \return a const reference to the picked point
         */
        const vec3& picked_point() const {
            return picked_point_;
        }

        /**
         * \brief Tests whether the background was picked.
         * \details This tests whether the picked depth is equal to 1.0
         * \retval true if the background was picked (i.e. no object was picked)
         * \retval false otherwise (i.e. an object was picked)
         */
        bool picked_background() const {
            return picked_background_;
        }

        /**
         * \brief Back-transforms a point given by its normalized
         *  device coordinates and depth.
         * \return the world-coordinates of the point.
         */
        vec3 unproject(const vec2& p_ndc, double depth) const;

        /**
         * \brief Tests whether there was any OpenGL error.
         * \details Displays error messages in the console as obtained by
         *   glGetError()
         */
        void check_gl() const;

        /**
         * \brief Gets the default frame buffer id.
         * \details Some versions of Qt use a FrameBufferObject instead of
         *  a full OpenGL context in rendering widgets.
         * \return the id of the default frame buffer object used with this
         *  context, or 0 if no frame buffer object is used.
         */
        GLuint frame_buffer_id() const {
            return frame_buffer_id_;
        }

	/**
	 * \brief Tests whether the image is a picking image.
	 * \details Graphite picking uses images made of picking IDs
	 *  (instead of colors). This function tests whether the last
	 *  rendering operation generated such images. In this case, the
	 *  image should not be displayed to the user.
	 * \retval true if the contained image has picking ids.
	 * \retval false otherwise.
	 */
	bool contains_picking_image() const {
	    return last_frame_was_picking_;
	}

        /**
         * \brief Gets the Overlay
         * \details The Overlay has a couple of basic drawing primitives for
         *  displaying graphics over the rendering window.
         */
        Overlay& overlay() {
            return overlay_;
        }

    protected:

        /**
         * \brief Draws the background.
         * \details If a background texture was specified, uses the texture,
         *  else generates a colorramp between the primary background color
         *  and the secondary background color.
         * \see set_background_color(), set_background_color_2(),
         *  set_background_image(), update_background_image_from_data()
         */
        virtual void draw_background();

        /**
         * \brief Setups the viewport transform.
         * \details Sends the stored viewport parameters to OpenGL.
         */
        void setup_viewport();

        /**
         * \brief Setups the OpenGL projection matrix in orthographic mode.
         * \param[in] zNear depth coordinate of the near plane
         * \param[in] zFar depth coordinate of the far plane
         */
        void setup_projection_ortho(double zNear, double zFar);

        /**
         * \brief Setups the OpenGL projection matrix in orthographic mode.
         * \param[in] zScreen screen projection plane, used
         *  to compute the frustrum angle
         * \param[in] zNear depth coordinate of the near plane
         * \param[in] zFar depth coordinate of the far plane
         * \param[in] eye_offset distance between the eyes, used in
         *  stereo rendering
         */
        void setup_projection_perspective(
            double zScreen, double zNear, double zFar,
            double eye_offset=0.0
        );

        /**
         * \brief Setups the OpenGL model view transform from the viewing
         *  parameters.
         * \param[in] zScreen screen projection plane
         */
        void setup_modelview(double zScreen);

        /**
         * \brief Setups OpenGL lighting parameters.
         */
        void setup_lighting();


        /**
         * \brief Create a clipping plane perpendicular to the Z axis,
         *  transformed by the current clipping matrix and the modelview
         *  matrix.
         */
        void update_clipping();

        /**
         * \brief Converts (transposes) a Graphite matrix for
         *  OpenGL use.
         * \param[in] M a const reference to the matrix to be
         *  converted
         * \return a pointer to a static buffer with the
         *  converted (i.e. transposed) matrix
         */
        static const double* convert_matrix(const mat4& M);

        /**
         * \brief Gets the background texture.
         * \return a pointer to the background texture if it is defined,
         *  nil otherwise
         */
        Texture* background_texture() const {
            return background_texture_;
        }

        /**
         * \brief Indicates whether this RenderingContext is
         *  initialized.
         * \details This function is called by the RenderArea widget,
         *  after the first repaint event, once the RenderingContext
         *  is ready for rendering.
         * \param[in] x true if this rendering context is initialized,
         *  false otherwise
         */
        void set_initialized(bool x = true) {
            initialized_ = x;
        }

        /**
         * \brief Sets the width
         * \details This function is just used to indicate to this
         *  RenderingContext what is its current width
         *  (it does not resize the RenderingContext).
         * \param[in] w the width
         */
        void set_width(index_t w) {
            width_ = w;
        }

        /**
         * \brief Sets the height
         * \details This function is just used to indicate to this
         *  RenderingContext what is its current height
         *  (it does not resize the RenderingContext).
         * \param[in] h the height
         */
        void set_height(index_t h) {
            height_ = h;
        }

        /**
         * \brief Queries the Graphite environment for global
         *  rendering parameters, e.g. stereo & perspective,
         *  and updates member variables if this RenderingContext accordingly.
         */
        void get_view_parameters();


        /**
         * \brief In picking mode, this function gets all the information about
         *  the picked point, by reading the depth buffer and the color buffer
         *  with color-coded id.
         */
        void get_picked_point();


    protected:

        static RenderingContext* current_;
        bool initialized_;

        index_t width_;
        index_t height_;
        bool double_buffer_;

        int viewport_x_;
        int viewport_y_;
        int viewport_width_;
        int viewport_height_;
	int center_x_;
	int center_y_;

        mat4 viewing_matrix_;
        mutable mat4 inverse_viewing_matrix_;
        mutable bool inverse_viewing_matrix_dirty_;

        bool lighting_;
        mat4 lighting_matrix_;

        bool clipping_;
        mat4 clipping_matrix_;
        vec4 clipping_equation_;
        bool clipping_viewer_;
        GLUPclipMode clipping_mode_;

        Color background_color_;
        Color background_color_2_;
        Texture_var background_texture_;
        FullScreenEffectImpl_var full_screen_effect_;

        bool perspective_;
        bool stereo_;
        bool stereo_odd_frame_;
        double stereo_eye_dist_;
        vec3 head_position_;
        double head_tilt_;

        bool picking_mode_;
        vec2 picked_ndc_;
        double picked_depth_;
        index_t picked_id_;
        vec3 picked_point_;
        bool picked_background_;
        bool last_frame_was_picking_;

        /**
         * \brief The identifier of the default frame buffer
         *  associated with this RenderingContext.
         * \details QOpenGLWidget in Qt5.4 does not use
         * a real OpenGL context, it uses a FrameBuffer
         * object. Therefore, when we want to unbind a
         * FrameBuffer object, we need to restore the
         * binding to the one used by Qt instead.
         */
        GLuint frame_buffer_id_;
        bool frame_buffer_id_init_;

        static bool geogram_gfx_initialized_;

        /**
         * \brief The number of RenderingContext instances
         *  that are currently rendering something, i.e.
         *  between begin_frame() and end_frame().
         */
        static index_t nb_render_locks_;

        /**
         * \brief The number of RenderingContext instances
         *  that are currently picking something, or that
         *  contain a picking image in their color buffer.
         */
        static index_t nb_picking_locks_;

        /**
         * \brief The GLUP context.
         * \details The GLUP API provides immediate-mode
         *  OpenGL-like primitives and other ones, such
         *  as tetrahedra, hexahedra etc...
         */
        GLUPcontext glup_context_;
	bool owns_glup_context_;

        bool use_core_profile_;
        bool use_ES_profile_;

	bool transparent_;

        Overlay overlay_;

	friend class RenderArea;
    };

    typedef SmartPointer<RenderingContext> RenderingContext_var;

/******************************************************************/

}

#endif
