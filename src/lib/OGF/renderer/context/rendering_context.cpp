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

#include <OGF/renderer/context/rendering_context.h>
#include <OGF/basic/math/geometry.h>

#include <geogram/image/image.h>
#include <geogram/image/image_library.h>
#include <geogram_gfx/basic/common.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/file_system.h>

#include <string.h>

namespace {
    using namespace OGF;

    /**
     * \brief Converts a C string (returned by OpenGL)
     *  into a std::string.
     * \param[in] str the C null-terminated string.
     * \return a std::string with the same content as \p str.
     */
    inline std::string convert_string(const GLubyte* str) {
	return (str == nullptr) ? "nil" : std::string((const char*)str);
    }
}

namespace OGF {

/******************************************************************/


    RenderingContext* RenderingContext::current_ = nullptr;
    index_t RenderingContext::nb_render_locks_ = 0;
    index_t RenderingContext::nb_picking_locks_ = 0;

    RenderingContext::RenderingContext(GLUPcontext glup_context) {
        initialized_ = false;
        width_ = 0;
        height_ = 0;
        double_buffer_ = false;

        inverse_viewing_matrix_dirty_ = false;
        viewing_matrix_.load_identity();
        inverse_viewing_matrix_.load_identity();

        lighting_ = true;
        lighting_matrix_.load_identity();
        clipping_ = false;
        clipping_matrix_.load_identity();
        clipping_equation_=vec4(0.0,0.0,0.0,0.0);
        clipping_viewer_=false;
        clipping_mode_=GLUP_CLIP_STRADDLING_CELLS;

	transparent_ = CmdLine::get_arg_bool("gfx:transparent");

	if(transparent_) {
	    background_color_ = Color(0.0, 0.0, 0.0, 0.0);
	    background_color_2_ = Color(0.0, 0.0, 0.0, 0.0);
	} else {
	    background_color_ = Color(0.0, 0.0, 0.0, 1.0);
	    background_color_2_ = Color(0.0, 0.0, 0.0, 1.0);
	}

        perspective_ = true;
        stereo_ = false;
        stereo_odd_frame_ = false;
        stereo_eye_dist_ = 0.0;
        head_tilt_ = 0.0;
        head_position_ = vec3(0.0, 0.0, 0.0);

        picking_mode_ = false;
        picked_id_ = index_t(-1);
        picked_background_ = false;

        get_view_parameters();

        frame_buffer_id_init_ = false;
        frame_buffer_id_ = 0;

        last_frame_was_picking_ = false;

        glup_context_ = glup_context;
	owns_glup_context_ = (glup_context == nullptr);
	use_core_profile_ =
	    (CmdLine::get_arg("gfx:GL_profile") != "compatibility");
	use_ES_profile_ =
	    (CmdLine::get_arg("gfx:GL_profile") == "ES");
    }

    RenderingContext::~RenderingContext() {
        if(glup_context_ != nullptr && owns_glup_context_) {
            glupDeleteContext(glup_context_);
            glup_context_ = nullptr;
        }
    }

    bool RenderingContext::is_currently_rendering() {
        return (nb_render_locks_ > 0);
    }

    bool RenderingContext::is_currently_picking() {
        return (nb_picking_locks_ > 0);
    }

    void RenderingContext::set_double_buffer(bool b) {
        double_buffer_ = b;
    }

    const Color& RenderingContext::background_color() const {
        return background_color_;
    }

    const Color& RenderingContext::background_color_2() const {
        return background_color_2_;
    }

    void RenderingContext::set_background_color(
        const Color& c
    ) {
	if(!transparent_) {
	    background_color_ = c;
	}
    }

    void RenderingContext::set_background_color_2(
        const Color& c
    ) {
	if(!transparent_) {
	    background_color_2_ = c;
	}
    }

    void RenderingContext::set_background_image(Image* image) {
        background_texture_.reset();
        if(image != nullptr) {
            background_texture_ = new Texture;
            background_texture_->create_from_image(image);
        }
    }

    void RenderingContext::update_background_image_from_data(
        Memory::pointer ptr,
        Image::ColorEncoding color_encoding,
        Image::ComponentEncoding component_encoding,
        index_t width, index_t height
    ) {
        if(ptr == nullptr) {
            background_texture_.reset();
            return;
        }
        if(background_texture_ == nullptr) {
            background_texture_ = new Texture;
        }
        background_texture_->create_from_data(
            ptr, color_encoding, component_encoding, width, height
        );
    }

    void RenderingContext::make_current() {
        current_ = this;
        if(glup_context_ == nullptr) {
            glup_context_ = glupCreateContext();
        }
        glupMakeCurrent(glup_context_);
    }

    void RenderingContext::done_current() {
        current_ = nullptr;
    }

    void RenderingContext::swap_buffers() {
        // Base class implementation does nothing.
    }

    std::string RenderingContext::gl_vendor() const {
        const GLubyte* str = glGetString(GL_VENDOR);
        check_gl();
        return convert_string(str);
    }

    std::string RenderingContext::gl_renderer() const {
        const GLubyte* str = glGetString(GL_RENDERER);
        check_gl();
        return convert_string(str);
    }

    std::string RenderingContext::gl_version() const {
        const GLubyte* str = glGetString(GL_VERSION);
        check_gl();
        return convert_string(str);
    }

    std::string RenderingContext::gl_extensions() const {
	std::string result;
	GLint n = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for(GLint i=0; i<n; ++i) {
	    const char* extension = (const char*)glGetStringi(
		GL_EXTENSIONS, GLuint(i)
	    );
	    result += extension;
	    result += ";";
	}
	return result;
    }

    std::string RenderingContext::get_gpu_information() const {
        return gl_renderer() + ":" + gl_version() + ":" + gl_vendor();
    }

    std::string RenderingContext::get_gpu_extensions() const {
        return gl_extensions();
    }

    void RenderingContext::set_full_screen_effect(FullScreenEffectImpl* fse) {
        full_screen_effect_ = fse;
    }

    void RenderingContext::load_viewing_matrix(
        const mat4& m
    ) {
        viewing_matrix_ = m;
        inverse_viewing_matrix_dirty_ = true;
    }

    void RenderingContext::mul_viewing_matrix(const mat4& m) {
        viewing_matrix_ = m * viewing_matrix_;
        inverse_viewing_matrix_dirty_ = true;
    }

    const mat4& RenderingContext::viewing_matrix() const {
        return viewing_matrix_;
    }

    const mat4& RenderingContext::inverse_viewing_matrix() const {
        if(inverse_viewing_matrix_dirty_) {
            inverse_viewing_matrix_ = viewing_matrix_.inverse();
            inverse_viewing_matrix_dirty_ = false;
        }
        return inverse_viewing_matrix_;
    }

    const GLdouble* RenderingContext::convert_matrix(
        const mat4& m
    ) {
        static double result[16];
        index_t k = 0;
        for(index_t i=0; i<4; i++) {
            for(index_t j=0; j<4; j++) {
                result[k] = m(i,j);
                k++;
            }
        }
        return result;
    }

    void RenderingContext::clear() {
        glClear((GLbitfield)(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        check_gl();
    }

    void RenderingContext::load_projection_matrix(const mat4& m) {
        glupMatrixMode(GLUP_PROJECTION_MATRIX);
        glupLoadMatrixd(convert_matrix(m));
        glupMatrixMode(GLUP_MODELVIEW_MATRIX);
        check_gl();
    }

    mat4 RenderingContext::projection_matrix() const {
        static double d[16];
        glupGetMatrixdv(GLUP_PROJECTION_MATRIX, d);
        mat4 result;
        index_t k = 0;
        for(index_t i=0; i<4; i++) {
            for(index_t j=0; j<4; j++) {
                result(i,j) = d[k];
                k++;
            }
        }
        return result;
    }

    void RenderingContext::set_lighting_matrix(const mat4& m) {
        lighting_matrix_ = m;
    }

    const mat4& RenderingContext::lighting_matrix() const {
        return lighting_matrix_;
    }

    bool RenderingContext::get_clipping() const {
        return clipping_;
    }

    void RenderingContext::set_clipping(bool x) {
        clipping_ = x;
    }

    mat4 RenderingContext::clipping_matrix() const {
        return clipping_matrix_;
    }

    void RenderingContext::set_clipping_matrix(const mat4& m) {
        clipping_matrix_ = m;
    }

    void RenderingContext::update_clipping() {
        if(clipping_) {
            glupPushMatrix();
            if(clipping_viewer_) {
                glupLoadIdentity();
            }
            mat4 M = clipping_matrix_;
            glupMultMatrixd(&M(0,0));
            glupEnable(GLUP_CLIPPING);
            glupClipMode(GLUP_CLIP_WHOLE_CELLS);
            glupClipPlane(clipping_equation_.data());
            glupPopMatrix();
        } else {
            // Safer to set equation with zero, since
            // some shaders do not test whether clipping
            // is activated.
            double eqn[4] = {0.0, 0.0, 0.0, 0.0};
            glupClipPlane(eqn);
            glupDisable(GLUP_CLIPPING);
        }
        glupClipMode(clipping_mode_);
    }

    void RenderingContext::snapshot(
        Image* image, bool do_make_current,
        index_t x0, index_t y0, index_t width, index_t height
    ) {

	if(do_make_current) {
	    make_current();
	}

        if(width == 0) {
            width = get_width();
        }

        if(height == 0) {
            height = get_height();
        }

        if(image->base_mem() == nullptr) {
            image->initialize(
                Image::RGB, Image::BYTE, width, height
            );
        }

        width  = std::min(image->width(),  get_width()-x0);
        height = std::min(image->height(), get_height()-y0);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ROW_LENGTH, int(image->width()));

        if(image->component_encoding() != Image::BYTE) {
            Logger::err("RenderingContext")
                << "snapshot(): wrong image component encoding"
                << std::endl;
            return;
        }

        switch (image->color_encoding()) {
        case Image::RGB:
            glReadPixels(
                GLint(x0), GLint(y0), GLsizei(width), GLsizei(height),
                GL_RGB, GL_UNSIGNED_BYTE, image->base_mem()
            );
            break;

        case Image::BGR:
            glReadPixels(
                GLint(x0), GLint(y0), GLsizei(width), GLsizei(height),
                GL_BGR, GL_UNSIGNED_BYTE, image->base_mem()
            );
            break;

        case Image::RGBA:
            glReadPixels(
                GLint(x0), GLint(y0), GLsizei(width), GLsizei(height),
                GL_RGBA, GL_UNSIGNED_BYTE, image->base_mem()
            );
            break;

        case Image::GRAY:
        case Image::INDEXED:
        case Image::YUV:
	    Logger::err("RenderingContext")
                << "snapshot(): wrong image color encoding"
                << std::endl;
	    break;
        }
        // Restore default
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);

	if(do_make_current) {
	    done_current();
	}
    }

/******************************************************************************/

    static bool is_same_color(const Color& c1, const Color& c2) {
        return (
            c1.r() == c2.r() &&
            c1.g() == c2.g() &&
            c1.b() == c2.b()
        );
    }


    void RenderingContext::draw_background(){
        glupDisable(GLUP_LIGHTING);

        if(picking_mode_) {
            glClearColor(1.0, 1.0, 1.0, 1.0);
            glClear((GLbitfield)(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
            return;
        }

        if(background_texture() == nullptr) {
            if(is_same_color(background_color(), background_color_2())) {
                glClearColor(
                    float(background_color().r()),
                    float(background_color().g()),
                    float(background_color().b()),
                    float(background_color().a())
                );
                clear();
            } else {
                glDisable(GL_DEPTH_TEST);
                glClear((GLbitfield)(GL_DEPTH_BUFFER_BIT));

                glupMatrixMode(GLUP_PROJECTION_MATRIX);
                glupLoadIdentity();
                glupMatrixMode(GLUP_MODELVIEW_MATRIX);
                glupLoadIdentity();
                float z = 1.0f;
                glupEnable(GLUP_VERTEX_COLORS);
                glupBegin(GLUP_QUADS);
                glupColor3dv(background_color().data());
                glupVertex3f(-1.0f,-1.0f,z);
                glupVertex3f( 1.0f,-1.0f,z);
                glupColor3dv(background_color_2().data());
                glupVertex3f(  1.0f, 1.0f,z);
                glupVertex3f( -1.0f, 1.0f,z);
                glupEnd();
                glupDisable(GLUP_VERTEX_COLORS);

                glEnable(GL_DEPTH_TEST);
                // TODO: this triggers a FPE with some drivers,
                // to be investigated. Maybe it's no longer the
                // case with new management of ModelView and Project.
            }
        } else {
            glClearColor(
                float(background_color().r()),
                float(background_color().g()),
                float(background_color().b()),
                float(background_color().a())
            );
            glDisable(GL_DEPTH_TEST);
            glClear((GLbitfield)(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

            glupMatrixMode(GLUP_PROJECTION_MATRIX);
            glupLoadIdentity();
            glupMatrixMode(GLUP_MODELVIEW_MATRIX);
            glupLoadIdentity();

            float z = 1.0f;


            float h = 1.0;
            float w = 1.0;

            if(width_ > height_) {
               h = float(height_) / float(width_);
            } else {
               w = float(width_) / float(height_);
            }

            glupTextureMode(GLUP_TEXTURE_REPLACE);
            background_texture()->bind();
            glupBegin(GLUP_QUADS);
            glupTexCoord2f(0.0f,0.0f);
            glupVertex3f(-w,-h,z);
            glupTexCoord2f(1.0f,0.0f);
            glupVertex3f( w,-h,z);
            glupTexCoord2f(1.0f,1.0f);
            glupVertex3f(  w, h,z);
            glupTexCoord2f(0.0f,1.0f);
            glupVertex3f( -w, h,z);
            glupEnd();

            glEnable(GL_DEPTH_TEST);
            background_texture_->unbind();
        }
        check_gl();
    }

    void RenderingContext::resize(index_t w, index_t h) {

	//   This may change the frame buffer, thus we will
	// query it again next time it is needed.
	frame_buffer_id_init_ = false;

        int w_width = int(w);
        int w_height = int(h);

        set_width(w);
        set_height(h);

        if(w_width > w_height) {
            viewport_x_ = 0;
            viewport_y_ = -(w_width - w_height) / 2;
            viewport_width_ = w_width;
            viewport_height_ = w_width;
        } else {
            viewport_x_ = -(w_height - w_width) / 2;
            viewport_y_ = 0;
            viewport_width_ = w_height;
            viewport_height_ = w_height;
        }
    }

    vec2 RenderingContext::screen_to_ndc(index_t x_in, index_t y_in) const {
        int x = int(x_in);
        int y = int(y_in);

        double x_ndc =
            double(x -  viewport_x_) * 2.0 /
            double( viewport_width_) - 1.0;

        double y_ndc =
            double(y -  viewport_y_) * 2.0 /
            double( viewport_height_) - 1.0;

        return vec2(x_ndc, y_ndc);
    }

    void RenderingContext::ndc_to_screen(
        const vec2& ndc, index_t& x, index_t& y
    ) const {
        double x_ndc = ndc.x;
        double y_ndc = ndc.y;

        double x_screen = (x_ndc + 1.0) * double(viewport_width_);
        x_screen /= 2.0;
        x_screen += viewport_x_;

        double y_screen = (y_ndc + 1.0) * double(viewport_height_);
        y_screen /= 2.0;
        y_screen += viewport_y_;

        ogf_clamp(x_screen, 0.0, double(get_width()-1));
        ogf_clamp(y_screen, 0.0, double(get_height()-1));

        x = index_t(x_screen);
        y = index_t(y_screen);
    }


    void RenderingContext::begin_picking(const vec2& ndc) {
        geo_assert(!picking_mode_);

        //   The first frame that is in picking mode increments
        // nb_picking_locks_.
        if(!last_frame_was_picking_) {
            ++nb_picking_locks_;
        }

        picking_mode_ = true;
        picked_ndc_ = ndc;
        picked_id_ = index_t(-1);
    }

    void RenderingContext::end_picking() {
        picking_mode_ = false;
        last_frame_was_picking_ = true;
        // nb_picking_locks_ is not decremented here,
        // since the frame buffer still contains a
        // picking image.
        // nb_picking_locks_ is decremented in end_frame()
        // if last frame was picking.
    }

    void RenderingContext::setup_viewport() {
        glViewport(
            viewport_x_, viewport_y_, viewport_width_, viewport_height_
        );
        glDisable(GL_SCISSOR_TEST);
    }

    void RenderingContext::setup_projection_ortho(
        double zNear, double zFar
    ) {
        glupMatrixMode(GLUP_PROJECTION_MATRIX);
        glupLoadIdentity();
        double x = 1.0;
        double y = 1.0;
        glupOrtho(-x, x, -y, y, zNear, zFar);
        glupMatrixMode(GLUP_MODELVIEW_MATRIX);
        check_gl();
    }

    static double headtracking_ratio = 0.0;

    void RenderingContext::setup_projection_perspective(
        double zScreen, double zNear, double zFar, double eye_offset
    ) {
        // field of view of the larger dimension in degrees
        double camera_aperture = 9.0;

        glupMatrixMode(GLUP_PROJECTION_MATRIX);
        glupLoadIdentity();

        const double DTR=0.0174532925; // degrees to radians

        // half the width of the screen from the central point of view
        double view_half_max_size = zScreen * tan((camera_aperture/2) * DTR);

        double top =   view_half_max_size;
        double right = view_half_max_size;

        // headtracking_ratio = view_half_max_size / (x_screen_size/2.0);
        headtracking_ratio = 3.5/900.0;
        // TODO why ? this value does not make sense, but it works well...

        double c_tilt = cos(head_tilt_);
        double s_tilt = sin(head_tilt_);

        // shift of the view from the current point of view
        double eye_shift = eye_offset * zNear / zScreen;
        double x_eye_shift =  eye_shift*c_tilt;
        double y_eye_shift = -eye_shift*s_tilt;

        // TODO the screen/viewport size and ratio
        // need to be taken into account when computing head shift
        double x_head_shift =
            head_position_.x * headtracking_ratio * zNear / zScreen;
        double y_head_shift =
            head_position_.y * headtracking_ratio * zNear / zScreen;


        glupFrustum(
            - right - x_eye_shift - x_head_shift,
              right - x_eye_shift - x_head_shift,
            - top   - y_eye_shift - y_head_shift,
              top   - y_eye_shift - y_head_shift,
            zNear, zFar
        );
        glupTranslatef(
            float(-eye_offset*c_tilt), float(eye_offset*s_tilt), 0.0f
        );
        glupMatrixMode(GLUP_MODELVIEW_MATRIX);
        check_gl();
    }


    void RenderingContext::setup_modelview(double zScreen) {

        glupEnable(GLUP_LIGHTING);
        glupMatrixMode(GLUP_MODELVIEW_MATRIX);
        glupLoadIdentity();

        GLfloat light_position[4];
        vec3 light = transform_vector(
            vec3(1.0, 1.0, 4.0), lighting_matrix_
        );

        light_position[0] = float(light.x);
        light_position[1] = float(light.y);
        light_position[2] = float(light.z);
        light_position[3] = 0.0f;

        glupLightVector3fv(light_position);

        double x_translation = head_position_.x*headtracking_ratio;
        double y_translation = head_position_.y*headtracking_ratio;
        double z_translation = -zScreen;
        glupTranslatef(
            float(-x_translation),
            float(-y_translation),
            float(z_translation)
        );

        // apply the rotation/pan/zoom matrix
        glupMultMatrixd(convert_matrix(viewing_matrix()));

        check_gl();
    }

    void RenderingContext::setup_lighting() {
        glEnable(GL_DEPTH_TEST);
        if(!lighting_) {
            glupDisable(GLUP_LIGHTING);
        }
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset (1.,1.);
        check_gl();
    }


    void RenderingContext::begin_frame() {
        ++nb_render_locks_;

        make_current();
        setup_viewport();

        if(!frame_buffer_id_init_) {
            GLint id;
            frame_buffer_id_init_ = true;
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &id);
            frame_buffer_id_ = GLuint(id);
        }

        glClear((GLbitfield)(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        if(
            !picking_mode_ &&
            !full_screen_effect_.is_null() &&
            full_screen_effect_->OK()
        ) {
            full_screen_effect_->pre_render(get_width(), get_height());
        }

        // half of the distance between the eyes, if in stereo mode
        double eye_offset = stereo_eye_dist_;

        //TODO try to avoid very intense perspective effects when zooming
        double scaling = viewing_matrix()(3,3);

        double zNear = 1.0;        // near clipping plane
        double zFar = 8.0;         // far clipping plane
        double zScreen = 3.5;      // screen projection plane

        if(double_buffer_ && perspective_ && stereo_) {
            if(stereo_odd_frame_) {
                glDrawBuffer(GL_BACK_RIGHT);
                glClear(GL_COLOR_BUFFER_BIT);
                draw_background();
                setup_projection_perspective(
                    zScreen, zNear, zFar, eye_offset
                );
                setup_modelview(zScreen);
                setup_lighting();
            } else {
                glDrawBuffer(GL_BACK_LEFT);
                glClear(GL_COLOR_BUFFER_BIT);
                draw_background();
                setup_projection_perspective(
                    zScreen, zNear, zFar, -eye_offset
                );
                setup_modelview(zScreen);
                setup_lighting();
            }
            stereo_odd_frame_ = !stereo_odd_frame_;
        } else {
            draw_background();
            if(perspective_){
                setup_projection_perspective(zScreen, zNear, zFar);
                glupMatrixMode(GLUP_MODELVIEW_MATRIX);
                glupLoadIdentity();
                setup_modelview(zScreen);
                update_clipping();
            } else {
                setup_projection_ortho(zNear/scaling, zFar/scaling);
                glupMatrixMode(GLUP_MODELVIEW_MATRIX);
                glupLoadIdentity();
                setup_modelview(zScreen/scaling);
                update_clipping();
            }
            setup_lighting();
        }
        check_gl();
    }

    vec3 RenderingContext::unproject(const vec2& p_ndc_in, double depth) const {
        // TODO: why do we need to change Y orientation here ?
        vec2 p_ndc(p_ndc_in.x, -p_ndc_in.y);

        index_t xs,ys;
        ndc_to_screen(p_ndc, xs, ys);
        double x(xs), y(ys);

        //  Get the modelview, project and viewport transforms from
        // OpenGL
        GLdouble modelview[16];
        GLdouble project[16];
        GLint viewport[4];

        glupGetMatrixdv(GLUP_MODELVIEW_MATRIX, modelview);
        glupGetMatrixdv(GLUP_PROJECTION_MATRIX, project);
        glGetIntegerv(GL_VIEWPORT, viewport);

        vec3 result;
        // Use them to back-transform the picked point in 3D space

        glupUnProject(
            x, y, depth,
            modelview, project, viewport,
            &result.x,
            &result.y,
            &result.z
        );

        return result;
    }

    void RenderingContext::end_frame() {

        glupDisable(GLUP_CLIPPING);

        {
            if(!picking_mode_) {
                bool opaque = (
                    (background_color().a() == 1.0) &&
                    (background_color_2().a() == 1.0)
                );

                if(opaque) {
                    // The following bloc sets the alpha channel to 1,
                    // this is required if using a composite manager
                    // under Linux, and maybe under Windows as well.
                    // (else windows under Graphite's window can be seen through
                    // the objects).
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
                    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                    glClear((GLbitfield)(GL_COLOR_BUFFER_BIT));
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                }

                if(
                    !picking_mode_ &&
                    !full_screen_effect_.is_null() &&
                    full_screen_effect_->OK()
                ) {
                    full_screen_effect_->post_render();
                }
                if(double_buffer_) {
                    swap_buffers();
                } else {
                    glFlush();
                }
            }
        }

        check_gl();

        //   In picking mode, get the picked object by decoding
        // the color (R,G,B,A contain the bytes of a 32bit integer),
        // and transform the picked point into world space.
        if(picking_mode_) {
            get_picked_point();
        }

        check_gl();

        if(!picking_mode_ && last_frame_was_picking_) {
            last_frame_was_picking_ = false;
            --nb_picking_locks_;
        }

        --nb_render_locks_;
    }

//___________________________________________________________________________

    void RenderingContext::get_picked_point() {
        index_t x,y;
        ndc_to_screen(picked_ndc_, x, y);

        if(x >= get_width() || y >= get_height()) {
            picked_id_ = index_t(-1);
        }  else {
            // TODO: why do we need to change orientation here ?
            y = get_height()-1-y;

            //   This flushes all the rendering commands before reading the
            // pixels. This is probably unneeded, since glReadPixels() is
            // supposed to do that, but it seems to be dependent on the
            // actual implementation of OpenGL. This could be done also
            // with glFLush(); glFinish();
            //   Since it is not harmful, I keep it there...

            done_current();
	    glFlush();
	    glFinish();
            make_current();

            // Read the color of the pixel under the mouse pointer
            Memory::byte pixel[4];
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_ROW_LENGTH, 1);
            glReadPixels(
                GLint(x),GLint(y),1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel
            );

            //  Read the z coordinate of the picked point
            // in the depth buffer.
            float z;
            glReadPixels(
                GLint(x), GLint(y),1,1,GL_DEPTH_COMPONENT,GL_FLOAT, &z
            );
            picked_depth_ = double(z);

            // Restore default value
            glPixelStorei(GL_PACK_ROW_LENGTH, 0);

            // Transform the picked point into world space.
            picked_point_ = unproject(picked_ndc_, picked_depth_);

            //  If depth buffer coordinate is 1, then we did hit the
            // background.
            picked_background_ = (picked_depth_ == 1.0);

            // Decode the picked id from the pixel's color
            picked_id_ =
                index_t(pixel[0])        |
                (index_t(pixel[1]) << 8)  |
                (index_t(pixel[2]) << 16) |
                (index_t(pixel[3]) << 24);
        }
    }

//___________________________________________________________________________

    void RenderingContext::get_view_parameters() {
        perspective_ = false;
        stereo_ = false;
        if(Environment::instance()->has_value("gfx:perspective")) {
            std::string perspective_str =
                Environment::instance()->get_value("gfx:perspective");
            if(!String::from_string(perspective_str, perspective_)) {
                Logger::err("Renderer")
                    << "Invalid boolean \'perspective\' env. variable"
                    << " (got " << perspective_str << ")"
                    << std::endl;
            }
        }
        if(perspective_ && Environment::instance()->has_value("stereo")) {
            std::string stereo_str =
                Environment::instance()->get_value("stereo");

            if(!String::from_string(stereo_str, stereo_)) {
                Logger::err("Renderer")
                    << "Invalid boolean \'stereo\' env. variable"
                    << " (got " << stereo_str << ")"
                    << std::endl;
            }
            stereo_eye_dist_ = 0.1;
            if(Environment::instance()->has_value("stereo_eye_dist")){
                std::string stereo_eye_dist_str =
                    Environment::instance()->get_value(
                        "stereo_eye_dist"
                    );
                if(
                    !String::from_string(
                        stereo_eye_dist_str, stereo_eye_dist_
                    )
                ) {
                    Logger::err("Renderer")
                        << "Invalid \'stereo_eye_dist\' env. variable"
                        << " (got " << stereo_str << ")"
                        << std::endl;
                    stereo_eye_dist_ = 0.0;
                } else {
                    stereo_eye_dist_ /= 100.0;
                }
            }
        }
    }

    void RenderingContext::check_gl() const {
        GLenum error_code = glGetError();
        bool has_opengl_errors = false;
        while(error_code != GL_NO_ERROR) {
            has_opengl_errors = true;

            std::string error_string;
            switch(error_code) {
            case GL_INVALID_ENUM:
                error_string = "invalid enum";
                break;
            case GL_INVALID_VALUE:
                error_string = "invalid value";
                break;
            case GL_INVALID_OPERATION:
                error_string = "invalid operation";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error_string = "invalid framebuffer operation";
                break;
            case GL_OUT_OF_MEMORY:
                error_string = "out of memory";
                break;
            default:
                error_string = "code " + String::to_string(error_code);
                break;
            }

            Logger::err("RenderingContext")
                << "There were OpenGL errors: "
                << error_string
                << std::endl;
            error_code = glGetError();
        }
        if(has_opengl_errors) {
            if(Environment::instance()->has_value("opengl_exceptions")) {
                std::string val = Environment::instance()->get_value(
                    "opengl_exceptions"
                );
                bool report_as_exception=false;
                String::from_string(val, report_as_exception);
                if(report_as_exception) {
                    ogf_assert_not_reached;
                }
            }
        }
    }

/**************************************************************************/

}
