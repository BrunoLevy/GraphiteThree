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

#include <OGF/skin_imgui/types/application.h>
#include <OGF/skin_imgui/types/icon_repository.h>
#include <OGF/skin_imgui/types/rendering_context.h>
#include <OGF/skin_imgui/widgets/console.h>
#include <OGF/skin/types/preferences.h>
#include <OGF/scene_graph/commands/commands.h>
#include <OGF/gom/lua/lua_interpreter.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/basic/math/geometry.h>
#include <OGF/basic/os/file_manager.h>

#include <geogram_gfx/gui/application.h>
#include <geogram_gfx/gui/status_bar.h>
#include <geogram_gfx/lua/lua_imgui.h>

#include <geogram/image/image_library.h>
#include <geogram/basic/command_line.h>

// ApplicationImpl has no out-of-line virtual function. It is
// not a problem since it is only visible from this translation
// unit, but clang will complain.
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

namespace OGF {

    /**
     * \brief A GEO::Application that redirects its calls to
     *  an OGF::Application.
     */
    class ApplicationImpl : public GEO::Application {
    public:
	/**
	 * \brief ApplicationImpl constructor
	 * \param[in] app a pointer to the companion OGF::Application.
	 */
	ApplicationImpl(OGF::Application* app) :
	    GEO::Application("Graphite ///"),
	    application_(app) {
	    console_ = new Console(app);
	    status_bar_ = new StatusBar;
	}

	/**
	 * \brief ApplicationImpl destructor.
	 */
	~ApplicationImpl() override {
	    GEO::Application::GL_terminate();
	}
	
	/**
	 * \brief Gets the console.
	 * \return a pointer to the console.
	 */
	OGF::Console* console() {
	    return console_;
	}

	/**
	 * \brief Gets the status bar.
	 * \return a pointer to the status bar.
	 */
	OGF::StatusBar* status_bar() {
	    return status_bar_;
	}

	/**
	 * \copydoc GEO::Application::set_style()
	 */
	void set_style(const std::string& style_name) override {
	    GEO::Application::set_style(style_name);
	    if(style_name == "Light" || style_name == "LightGray") {
		application_->get_render_area()->set_background_color_1(
		    Color(1.0, 1.0, 1.0, 1.0)
		);
		application_->get_render_area()->set_background_color_2(
		    Color(1.0, 1.0, 1.0, 1.0)
		);
	    } else {
		application_->get_render_area()->set_background_color_1(
		    Color(0.0, 0.0, 0.0, 1.0)
		);
		application_->get_render_area()->set_background_color_2(
		    Color(0.0, 0.0, 0.0, 1.0)
		);
	    }
	}
	
    protected:
	
	/**
	 * \copydoc GEO::Application::needs_to_redraw()
	 */
	bool needs_to_redraw() const override {
	    return (GEO::Application::needs_to_redraw() ||
		    application_->get_render_area()->needs_to_redraw()) ;
	}

	/**
	 * \copydoc GEO::Application::one_frame()
	 */
	void one_frame() override {
	    if(application_->is_stopping()) {
		return;
	    }
	    GEO::Application::one_frame();
	}

	/**
	 * \copydoc GEO::Application::post_draw()
	 */
	void post_draw() override {
	    application_->flush_command_queue();
	    GEO::Application::post_draw();
	}

	/**
	 * \copydoc GEO::Application::draw_gui()
	 */
	void draw_gui() override {
	    application_->redraw_request();
            Overlay& ovl = application_->get_render_area()
                                       ->get_rendering_context()
                                       ->overlay();
            ovl.playback();
	}

	/**
	 * \copydoc GEO::Application::draw_graphics()
	 */
	void draw_graphics() override {
	    if(
		get_width() != application_->get_render_area()->get_width()
		||
		get_height() != application_->get_render_area()->get_height()
	    ) {
		application_->get_render_area()->resize(
		    get_width(),
		    get_height(),
		    get_frame_buffer_width(),
		    get_frame_buffer_height()
		);
	    }
	    application_->get_render_area()->draw();
	}

	/**
	 * \copydoc GEO::Application::resize()
	 */
	void resize(index_t w, index_t h, index_t fb_w, index_t fb_h) override {
	    GEO::Application::resize(w,h,fb_w,fb_h);
	    application_->get_render_area()->resize(w,h,fb_w,fb_h);
	}

	/**
	 * \copydoc GEO::Application::mouse_button_callback()
	 */
	void mouse_button_callback(
	    int button, int action, int mods, int source
	) override {
	    geo_argused(source);
	    application_->get_render_area()->mouse_button_callback(
		button, action, mods
	    );
	}

	/**
	 * \copydoc GEO::Application::scroll_callback()
	 */
	void scroll_callback(double xoffset, double yoffset) override {
	    application_->get_render_area()->scroll_callback(xoffset, yoffset);
	}

	/**
	 * \copydoc GEO::Application::cursor_pos_callback()
	 */
	void cursor_pos_callback(double xf, double yf, int source) override {
	    geo_argused(source);
	    application_->get_render_area()->cursor_pos_callback(xf, yf);
	}

	/**
	 * \copydoc GEO::Application::drop_callback()
	 */
	void drop_callback(int nb, const char** p) override {
	    application_->get_render_area()->drop_callback(nb, p);
	}

	/**
	 * \copydoc GEO::Application::char_callback()
	 */
	void char_callback(unsigned int c) override {
	    application_->get_render_area()->char_callback(c);
	}

	/**
	 * \copydoc GEO::Application::key_callback()
	 */
	void key_callback(
	    int key, int scancode, int action, int mods
	) override {
	    application_->get_render_area()->key_callback(
		key, scancode, action, mods
	    );
	}

	/**
	 * \copydoc GEO::Application::ImGui_initialize()
	 */
	void ImGui_initialize() override {
	    GEO::Application::ImGui_initialize();
	    Logger::instance()->register_client(console_);
	    Progress::set_client(status_bar_);
	}

	/**
	 * \copydoc GEO::Application::ImGui_terminate()
	 */
	void ImGui_terminate() override {
	    Logger::instance()->unregister_client(console_);
	    Progress::set_client(nullptr);
	    GEO::Application::ImGui_terminate();	    
	}

	/**
	 * \copydoc GEO::Application::create_window()
	 */
	void create_window() override {
	    GEO::Application::create_window();
	    std::string icon_file_name = "icons/logos/small-graphite-logo.xpm";
	    if(!FileManager::instance()->find_file(icon_file_name)) {
		Logger::warn("Graphite") << "Could not find Graphite icon"
					 << std::endl;
		return;
	    }
	    Image_var icon_image = ImageLibrary::instance()->load_image(
		icon_file_name
	    );
	    if(icon_image.is_null()) {
		Logger::warn("Graphite") << "Could not find Graphite icon"
					 << std::endl;
		return;
	    }
	    set_window_icon(icon_image);
	}

	void GL_terminate() override {
	    // GL_terminate() is called right at the end of the main event
	    // loop. We prefer to deallocate GL stuff later, after all
	    // graphic objects have been deallocated, in the destructor.
	}
	
    private:
	OGF::Application* application_;
	SmartPointer<OGF::Console> console_;
	SmartPointer<OGF::StatusBar> status_bar_;
    };

    Application* Application::instance_;
    
    Application::Application(Interpreter* interpreter) :
	ApplicationBase(interpreter)
    {
        ogf_assert(instance_ == nullptr);
	picked_grob_ = nullptr;
	impl_ = new ApplicationImpl(this);	
        icon_repository_ = new IconRepository; 
        instance_ = this;
	if(CmdLine::arg_is_declared("gui:font_size")) {
	    set_font_size(CmdLine::get_arg_uint("gui:font_size"));
	}
	
	// TODO: get size from preferences so that we do not
	// need to resize render area after (would be cleaner)
	render_area_ = new RenderArea(
	    get_width(), get_height(),
	    get_frame_buffer_width(), get_frame_buffer_height()
	);	
	camera_ = new Camera(this);
	LuaInterpreter* lua_interp = dynamic_cast<LuaInterpreter*>(
	    interpreter
	);
	if(lua_interp != nullptr) {
	    lua_State* lua_state = lua_interp->lua_state();
	    init_lua_imgui(lua_state);
	}
	declare_preference_variable("gui:state","","ImGui windows state");
    }

    Application::~Application() {
#ifdef GEO_DEBUG
	Logger::out("Skin") << "Application::~Application()" << std::endl;
#endif	
        ogf_assert(instance_ == this);
	delete icon_repository_;
	icon_repository_ = nullptr;
	// Make sure RenderArea is deleted before
	// window / GL context
	render_area_.reset();
	delete impl_;
	impl_ = nullptr;
        instance_ = nullptr;
    }

    void Application::lock_updates() {
	impl_->lock_updates();
    }
    
    void Application::unlock_updates() {
	impl_->unlock_updates();
    }

    const std::string& Application::get_style() const {
	return impl_->get_style();
    }

    index_t Application::get_width() const {
	return impl_->get_width();
    }

    index_t Application::get_height() const {
	return impl_->get_height();
    }

    index_t Application::get_frame_buffer_width() const {
	return impl_->get_frame_buffer_width();
    }

    index_t Application::get_frame_buffer_height() const {
	return impl_->get_frame_buffer_height();
    }
    
    void Application::set_accept_drops(bool value) {
	impl_->set_accept_drops(value);
    }

    bool Application::get_accept_drops() const {
	return impl_->get_accept_drops();
    }
    
    void Application::start() {
        ApplicationBase::start();
        if(!is_stopping()) {
	    Logger::out("GOM") << "Starting Graphite ..." << std::endl;
	    impl_->start();
        }
	// At this point, we are after the main event loop.
	// The app and its render area are deleted after the Lua interpreter,
	// for this reason it is necessary to make sure they do not keep any
	// reference to functions in the Lua interpreter (through signal
	// connections)
	disconnect();
	render_area_->disconnect();
    }
    
    void Application::stop() {
	if(Commands::command_is_running()) {
	    Progress::cancel();
	    Logger::err("GOM") << "Cannot exit now, command is running"
			       << std::endl;
	    return;
	}
	set_stopping_flag();
        ApplicationBase::stop();
        Logger::out("GOM") << "Exiting Graphite" << std::endl;
	impl_->stop();
    }

    void Application::set_style(const std::string& style_name) {
	impl_->set_style(style_name);
        ApplicationBase::set_style(style_name);
    }
    
    void Application::set_font_size(index_t value) {
        ApplicationBase::set_font_size(value);
	impl_->set_font_size(value);
    }

    double Application::scaling() const {
        return impl_->scaling();
    }

    void Application::draw() {
	update();
	impl_->draw();
    }

    void Application::update() {
	ApplicationBase::update();
	impl_->update();
    }

    void Application::draw_dock_space() {
	impl_->draw_dock_space();
    }
	
    bool Application::draw_console(bool visible) {
	if(impl_->console() == nullptr) {
	    return false;
	}
	// second argument false : console is not windowed
	// (window is created by Lua script).
	impl_->console()->draw(&visible,false);
	return visible;
    }

    bool Application::status_bar_is_active() const {
	return
	    (impl_->status_bar() != nullptr) &&
	     impl_->status_bar()->active();
    }
    
    void Application::draw_status_bar() {
	if(status_bar_is_active()) {
	    impl_->status_bar()->draw();
	}
    }

    index_t Application::resolve_icon(
	const std::string& name, bool mipmap
    ) const {
	union {
	    ImTextureID im_texture_id;
	    index_t gl_texture_id;
	};
	im_texture_id = icon_repository()->resolve_icon(name, mipmap);
	return gl_texture_id;
    }

    void Application::exec_command(
	const std::string& command, bool add_to_history
    ) {
	queued_commands_.push_back(QueuedCommand(command, add_to_history));
    }

    void Application::exec_command_now(
	const std::string& command, bool add_to_history
    ) {
	interpreter()->execute(
	    command, add_to_history, false
	);
    }

    void Application::flush_command_queue() {
	// Execute queued commands.
	// Note: need to consume them, else
	//   they are executed multiple times
	//   due to re-entrant calls in one_frame()
	//   when updating progress logger.
	while(queued_commands_.size() != 0) {
	    QueuedCommand cmd = queued_commands_.front();
	    queued_commands_.erase(queued_commands_.begin());
	    exec_command_now(cmd.command, cmd.add_to_history);
	}
    }

    void Application::save_preferences() {
	std::string state(ImGui::SaveIniSettingsToMemory());
	for(size_t i=0; i<state.length(); ++i) {
	    if(state[i] == '\n') {
		state[i] = '\t';
	    }
	}
	CmdLine::set_arg("gui:state",state);
	ApplicationBase::save_preferences();
    }

    void Application::load_preferences(const std::string& filename) {
	ApplicationBase::load_preferences(filename);
	impl_->restart_gui();
    }

    void* Application::impl_window() const {
	return impl_->impl_window();
    }
    
    void Application::set_gui_state(std::string x) {
	impl_->set_gui_state(x);
    }

    std::string Application::get_gui_state() const {
	return impl_->get_gui_state();
    }
    
    void Application::set_full_screen_mode(
	index_t w, index_t h, index_t Hz, index_t monitor
    ) {
	impl_->set_full_screen_mode(w,h,Hz,monitor);
    }

    void Application::set_windowed_mode(index_t w, index_t h) {
	impl_->set_windowed_mode(w,h);
    }

    void Application::list_video_modes() {
	impl_->list_video_modes();
    }

    void Application::iconify() {
	impl_->iconify();
    }
    
    void Application::restore() {
	impl_->restore();
    }

    bool Application::get_full_screen() const {
	return impl_->get_full_screen();
    }
    
    void Application::set_full_screen(bool x) {
	impl_->set_full_screen(x);
    }
    
}
