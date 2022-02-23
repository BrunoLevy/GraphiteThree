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
 
#ifndef H_SKIN_IMGUI_APPLICATION_H
#define H_SKIN_IMGUI_APPLICATION_H

#include <OGF/skin_imgui/common/common.h>
#include <OGF/skin_imgui/types/camera.h>
#include <OGF/skin_imgui/widgets/render_area.h>
#include <OGF/skin/types/application_base.h>

namespace OGF {

    class Builder;
    class IconRepository;
    class ApplicationImpl;
    
    /**
     * \brief Implementation of Skin application using ImGui.
     * \note Client code should not use this class directly. It
     *  should be only used through the GOM abstraction layer.
     */
    gom_class SKIN_IMGUI_API Application : public ApplicationBase {
    public:
        /**
         * \brief Application constructor.
         */
        Application(Interpreter* interpreter);

        /**
         * \brief Application destructor.
         */
        ~Application() override;


	/**
	 * \brief Forbid copy.
	 */
	Application(const Application& rhs) = delete;

	/**
	 * \brief Forbid copy.
	 */
	Application& operator=(const Application& rhs) = delete;
	
        /**
         * \brief Gets the instance.
         * \return a pointer to the unique instance
         */
        static Application* instance() {
            return instance_;
        }
	
        /**
         * \brief Gets the icon repository.
         * \return a pointer to the IconRepository
         */
        IconRepository* icon_repository() {
	    return icon_repository_;
	}

        /**
         * \brief Gets the icon repository.
         * \return a const pointer to the IconRepository
         */
        const IconRepository* icon_repository() const {
	    return icon_repository_;
	}
	
        /**
         * \copydoc ApplicationBase::start()
         */
        void start() override;

        /**
         * \copydoc ApplicationBase::stop()
         */
        void stop() override;

        /**
         * \copydoc ApplicationBase::set_style()
         */
        void set_style(const std::string& value) override;

        /**
         * \copydoc ApplicationBase::set_font_size()
         */
        void set_font_size(index_t value) override;

	/**
	 * \brief Lock updates.
	 * \details If this function is called, updates are ignored. 
	 *  It is useful when a RenderingContext operation is occuring, to
	 *  prevent the Console for triggering a drawing operation.
	 */
	void lock_updates();

	/**
	 * \brief Unlock updates.
	 * \see lock_updates()
	 */
	void unlock_updates();

	/**
	 * \copydoc ApplicationBase::draw()
	 */
	void draw() override;

	/**
	 * \copydoc ApplicationBase::update()
	 */
	void update() override;


	/**
	 * \copydoc ApplicationBase::save_preferences()
	 */
	void save_preferences() override ;

	/**
	 * \copydoc ApplicationBase::load_preferences()
	 */
	void load_preferences(const std::string& filename) override;


	/**
	 * \brief Executes all commands in command queue.
	 */
	void flush_command_queue();

	/**
	 * \brief Gets a pointer to the underlying window.
	 * \return an implementation-specific pointer to the window.
	 * \details Some callbacks in RenderArea need it to query
	 *  modifiers state.
	 */
	void* impl_window() const;
	
    gom_slots:
	/**
	 * \brief Draws a dockspace that fills the current
	 *  window.
	 */
	void draw_dock_space();
	
	/**
	 * \brief Gets the style.
	 * \return a string with the current style;
	 */
	const std::string& get_style() const;
	
	/**
	 * \brief Gets the Camera.
	 * \return a pointer to the Camera.
	 */
	Camera* camera() {
	    return camera_;
	}

	/**
	 * \brief Finds an icon by name.
	 * \param[in] name the name of the icon, without file extension
	 * \param[in] mipmap if true, use mipmapping (filtering)
	 * \details Icons are loaded from XPM files in lib/icons/
	 * \return the OpenGL texture id of the icon, or 0
	 *  if no such icon was found.
	 */
	index_t resolve_icon(const std::string& name, bool mipmap=false) const;

	/**
	 * \brief Executes a command.
	 * \details The command is queued so that it is executed right
	 *  after the current frame (and not nested in the current frame). 
	 *  This allows updating the progress bar.
	 * \param[in] command a string with the LUA command to be executed.
	 * \param[in] add_to_history true if the command should be recorded in
	 *  the history.
	 */
	void exec_command(const std::string& command, bool add_to_history=true);

	/**
	 * \brief Executes a command immediatly.
	 * \details Unlike in exec_command(), the command is not queued and is
	 *  immediatly executed.
	 * \param[in] command a string with the LUA command to be executed.
	 * \param[in] add_to_history true if the command should be recorded in
	 *  the history.
	 */
	void exec_command_now(
	    const std::string& command, bool add_to_history=false
	);

	/**
	 * \brief Gets the global scaling to be applied to all GUI elements.
	 * \return 1.0 if the default font is used, more/less if a larger/
	 *  smaller font is used.
	 */
	double scaling() const;


	double margin() const {
	    return 15.0 * scaling();
	}

	double status_bar_height() const {
	    return 35.0 * scaling();
	}

	
	/**
	 * \brief Draws the console.
	 * \param[in] visible true if the console is visible, false otherwise.
	 * \retval true if the console is still visible.
	 * \retval false otherwise, e.g., when the user clicks on the 
	 *  close button of the console.
	 */
	bool draw_console(bool visible);

	/**
	 * \brief Tests if the status bar should be displayed.
	 * \retval true if the status bar should be displayed, i.e. when there
	 *  is an active ProgressLogger.
	 * \retval false otherwise.
	 */
	bool status_bar_is_active() const;

	/**
	 * \brief Draw the status bar.
	 * \details Ignored if the status bar is not active.
	 */
	void draw_status_bar();

	/**
	 * \brief Sets full-screen mode.
	 * \details All arguments to zero sets default mode.
	 * \param[in] w , h width and height in pixels
	 * \param[in] hz refresh rate in Hz
	 * \param[in] monitor the id of the monitor
	 */
	void set_full_screen_mode(
	    index_t w=0, index_t h=0, index_t hz=0,
	    index_t monitor=0
	);

	/**
	 * \brief Sets windowed mode.
	 * \param[in] w , h width and height in pixels. If
	 *  zero, use current dimensions.
	 */
	void set_windowed_mode(index_t w=0, index_t h=0);

	/**
	 * \brief Lists the video modes that can be used for
	 *  set_full_screen_mode()
	 * \details The video modes are listed in the terminal.
	 */
	void list_video_modes();

	/**
	 * \brief Iconifies this application.
	 */
	void iconify();

	/**
	 * \brief Restores this application.
	 */
	void restore();

	/**
	 * \brief Resets the latest error to the empty string.
	 */
	void reset_latest_error() {
	    latest_error_ = "";
	}

	/**
	 * \brief Notifies than an error occured.
	 * \param[in] message the error message.
	 */
	void notify_error(const std::string& message) {
	    error_occured(message);
	    latest_error_ = message;
	}
	
    gom_properties:

	/**
	 * \brief Gets the RenderArea.
	 * \return a pointer to the RenderArea.
	 */
	RenderArea* get_render_area() const {
	    return render_area_;
	}
	
	void set_gui_state(std::string x);
	std::string get_gui_state() const;
	
	/**
	 * \brief Sets full-screen mode.
	 * \param[in] x true if full-screen mode should be used,
	 *  false if windowed-mode should be used.
	 */
	void set_full_screen(bool x);

	/**
	 * \brief Tests whether this application is in full-screen mode.
	 * \retval true if full-screen mode is used.
	 * \retval false if windowed mode is used.
	 */
	bool get_full_screen() const;
	
	/**
	 * \brief Gets the width of the window.
	 * \return the width of the window in pixels.
	 */
	index_t get_width() const;

	/**
	 * \brief Gets the height of the window.
	 * \return the height of the window in pixels.
	 */
	index_t get_height() const;

	/**
	 * \brief Gets the width of the window.
	 * \return the width of the frame buffer in pixels.
	 */
	index_t get_frame_buffer_width() const;

	/**
	 * \brief Gets the height of the window.
	 * \return the height of the frame buffer in pixels.
	 */
	index_t get_frame_buffer_height() const;
	
        /**
         * \brief Sets whether drag and drop events should be
         *  taken into account.
         * \param[in] value true if drag and drop events should be taken into
         *  account, false otherwise
         */
	void set_accept_drops(bool value);

        /**
         * \brief Tests whether drag and drop events are taken into
         *  account.
         * \retval true if drag and drop events are taken into account
         * \retval false otherwise
         */
	bool get_accept_drops() const;

	void set_picked_grob(Grob* value) {
	    picked_grob_ = value;
	}

	Grob* get_picked_grob() const {
	    return picked_grob_;
	}

	/**
	 * \brief Gets the latest error message.
	 * \return the latest error message.
	 */
	const std::string& get_latest_error() const {
	    return latest_error_;
	}

    gom_signals:
	
	/**
	 * \brief This signal is triggered when the GUI should be redisplayed.
	 */
	void redraw_request();

	/**
	 * \brief This signal is triggered when the console displays an 
	 *  error message.
	 * \param[in] error the error message to be displayed.
	 */
	void error_occured(const std::string& error);

    private:
        static Application* instance_;
	ApplicationImpl* impl_;
        IconRepository* icon_repository_;
	RenderArea_var render_area_;
	Camera_var camera_;
	struct QueuedCommand {
	    QueuedCommand(const std::string& cmd, bool hist) :
	       command(cmd), add_to_history(hist) {
	    }
	    std::string command;
	    bool add_to_history;
	};
	std::vector<QueuedCommand> queued_commands_;
	Grob* picked_grob_;
	std::string latest_error_;
    };

}

#endif

