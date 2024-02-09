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


#ifndef H_SKIN_APPLICATION_BASE_H
#define H_SKIN_APPLICATION_BASE_H

#include <OGF/skin/common/common.h>
#include <OGF/gom/types/node.h>
#include <geogram/basic/progress.h>
#include <iostream>

/**
 * \file OGF/skin/types/application_base.h
 * \brief The toolkit-independent part of the Application class.
 */

namespace OGF {

    /**
     * \brief Base class for Application.
     * \details Contains all the toolkit-independent
     *  material.
     */
    gom_attribute(abstract, "true")
    gom_class SKIN_API ApplicationBase : public Object {
        
    public:
        /**
         * \brief ApplicationBase constructor.
	 * \param[in] interpreter a pointer to the main Interpreter.
         */
        ApplicationBase(Interpreter* interpreter);

        /**
         * \brief ApplicationBase destructor.
         */
         ~ApplicationBase() override;

	/**
	 * \brief Gets the main Interpreter.
	 * \return a pointer to the main Interpreter.
	 */
	Interpreter* interpreter() {
	    return interpreter_;
	}

        static ApplicationBase* instance() {
            return instance_;
        }

    gom_properties:

        /**
         * \brief Sets a tooltip to be displayed near the cursor
         *  in the rendering area.
         */
        void set_tooltip(const std::string& x) {
            tooltip_ = x;
            // Interpret "\\n" in string.
            for(
                size_t index = tooltip_.find("\\n");
                index != std::string::npos;
                index = tooltip_.find("\\n", index+2)
            ) {
                tooltip_.replace(index, 2, " \n");
            }
        }

        /**
         * \brief Gets the tooltip to be displayed near the cursor
         *  in the rendering area, or an empty string if there is
         *  no tooltip.
         */
        const std::string& get_tooltip() const {
            return tooltip_;
        }
        
    gom_slots:
        /**
         * \brief Starts the application.
         * \details In derived classes, it enters the main event loop
         *  of the widgets toolkit.
         */
        virtual void start();

        /**
         * \brief Stops the application.
         * \details In derived classes, it exits the main event loop
         *  of the widgets toolkit.
         */
        virtual void stop();

        /**
         * \brief Sets the style to be used for widgets.
         * \details Some toolkits can have different styles.
         *  Default implementation does nothing but storing
         *  the style in the preferences. Can be overloaded
         *  in implementations.
         * \param[in] value the name of the widgets style
         */
        virtual void set_style(const std::string& value);

        /**
         * \brief Sets the size of fonts in the application.
         * \details Default implementation does nothing but
         *  storing the font size in the preferences. Can be
         *  overloaded in implementations.
         * \param[in] value the size of fonts
         */
        virtual void set_font_size(index_t value);

	/**
	 * \brief Declares a preference variable.
	 * \param[in] name the name of the variable.
	 * \param[in] value the default value to be used, 
	 *  if the variable does not already exist in the
	 *  environment.
	 * \param[in] help the optional help string.
	 */
	void declare_preference_variable(
	    const std::string& name,
	    const std::string& value,
	    const std::string& help=""
	);
	
	/**
	 * \brief Saves the preferences to home directory / graphite.ini.
	 */
	virtual void save_preferences();


	/**
	 * \brief Loads preference variables from a file.
	 * \param[in] filename the full path to the file.
	 */
	virtual void load_preferences(const std::string& filename);

	/**
	 * \brief Tests whether preferences were loaded.
	 * \details This function returns false if there was
	 *  no user defined graphite.ini preferences file.
	 * \retval true if preferences were loaded.
	 * \retval false otherwise.
	 */
	virtual bool preferences_loaded();
	
        /**
         * \brief Cancels the current job.
         * \details This function is called when the 'stop' button near
         *  the progress bar is pushed.
         * \see GEO::ProgressTask
         */
        virtual void cancel_current_job();

	/**
	 * \brief Redraws the main window.
	 * \details This function is called by commands that animate 
	 *  objects during computation, by the progress bar and by
	 *  console output.
	 */
	virtual void draw();

	/**
	 * \brief Indicates that the main window should be redrawn.
	 */
	virtual void update();

        /**
         * \brief ProgressClient overload, callback called by ProgressTask.
         * \details Called when a ProgressTask starts.
         */
        virtual void begin();
        
        /**
         * \brief ProgressClient overload, callback called by ProgressTask.
         * \details Called when a ProgressTask is updated.
         * \param[in] step current step
         * \param[in] percent current percentage
         */
        virtual void progress(index_t step, index_t percent);

        /**
         * \brief ProgressClient overload, callback called by ProgressTask.
         * \details Called when a ProgressTask is terminated.
         * \param[in] canceled indicates whether the ProgressTask was
         *  interupted by the user.
         */
        virtual void end(bool canceled);

	/**
	 * \brief Finds a file in the OGF_PATH.
	 * \return the found file or the empty string if no
	 *  such file was found.
	 */
	std::string find_file(const std::string& filename) const;


        /**
         * \brief Saves Graphite state to a file.
         */
        virtual void save_state();

        /**
         * \brief Restores latest saved state.
         */
        virtual void restore_state();
        
    gom_signals:
        /**
         * \brief A signal that is triggered after the Graphite
         *  application is started.
         */
        void started();

        /**
         * \brief A signal that is triggered when a 'separation div'
         *   should be displayed in the logger.
         * \details Overloads LoggerClient::div()
         * \param[in] value the name of the 'separation div'
         */
        void div(const std::string& value);

        /**
         * \brief A signal that is triggered when the logger displays
         *  a message.
         * \details Overloads LoggerClient::out()
         * \param[in] value the message to be displayed.
         */
        void out(const std::string& value);

        /**
         * \brief A signal that is triggered when the logger displays
         *  a warning message.
         * \details Overloads LoggerClient::warn()
         * \param[in] value the message to be displayed.
         */
        void warn(const std::string& value);

        /**
         * \brief A signal that is triggered when the logger displays
         *  an error message.
         * \details Overloads LoggerClient::err()
         * \param[in] value the message to be displayed.
         */
        void err(const std::string& value);

        /**
         * \brief A signal that is triggered when the message in the
         *  status bar should be updated.
         * \param[in] value the message to be displayed.
         */
        void status(const std::string& value);

        /**
         * \brief A signal that is triggered when a ProgressTask starts.
         * \param[in] value the name of the ProgressTask
         */
        void notify_progress_begin(const std::string& value);

        /**
         * \brief A signal that is triggered when a ProgressTask is updated.
         * \param[in] value the value of the progress, in percent.
         */
        void notify_progress(index_t value);

        /**
         * \brief A signal that is triggered when a ProgressTask finishes.
         */
        void notify_progress_end();

	/**
	 * \brief Tests whether the application is stopping.
	 * \details When the application is stopping, it processes the last
	 *  events from the queue before exiting the event loop. Some
	 *  event handlers need to test this flag in order to avoid sending
	 *  new events (else the application will be never able to stop).
	 * \retval true if the application is stopping
	 * \retval false otherwise
	 */
	static bool is_stopping() {
	    return stopping_;
	}

	
    protected:

	/**
	 * \brief Indicates that the application is stopping, i.e. processes
	 *  the last events from the event queue.
	 * \details Implementations of the application class are supposed to
	 *  set the stopping flag, then process the last events from the
	 *  event queue, then exit the event loop.
	 */
	static void set_stopping_flag() {
	    stopping_ = true;
	}
	
        /**
         * \brief A LoggerClient that redirects all messages to
         *  an ApplicationBase.
         */
        class ApplicationBaseLoggerClient : public LoggerClient {
        public:
            ApplicationBaseLoggerClient(ApplicationBase* app);

            /**
             * \copydoc LoggerClient::div()
             */
            void div(const std::string& value) override;

            /**
             * \copydoc LoggerClient::out()
             */
            void out(const std::string& value) override;

            /**
             * \copydoc LoggerClient::warn()
             */
            void warn(const std::string& value) override;

            /**
             * \copydoc LoggerClient::err()
             */
            void err(const std::string& value) override;

            /**
             * \copydoc LoggerClient::status()
             */
            void status(const std::string& value) override;

        private:
            ApplicationBase* application_base_;
        };

        /**
         * \brief A ProgressClient that redirects all messages to
         *  an ApplicationBase.
         */
        class ApplicationBaseProgressClient : public ProgressClient {
        public:
            ApplicationBaseProgressClient(ApplicationBase* app);
            
            /**
             * \copydoc ProgressClient::begin()
             */
            void begin() override;
        
            /**
             * \copydoc ProgressClient::progress()
             */
            void progress(index_t step, index_t percent) override;
            
            /**
             * \copydoc ProgressClient::end()
             */
            void end(bool canceled) override;
            
        private:
            ApplicationBase* application_base_;
        };

	Interpreter* interpreter_;
        LoggerClient_var logger_client_;
        ProgressClient_var progress_client_;
        std::string tooltip_;

        static ApplicationBase* instance_;
	static bool stopping_;
    };

}

#endif
