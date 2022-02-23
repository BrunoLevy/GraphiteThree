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
 
#ifndef H_SKIN_IMGUI_CONSOLE_H
#define H_SKIN_IMGUI_CONSOLE_H

#include <OGF/skin_imgui/common/common.h>
#include <geogram_gfx/gui/console.h>

namespace OGF {

    class Application;
    
    /**
     * \brief A console for Graphite.
     */
    class SKIN_IMGUI_API Console : public GEO::Console {
      public:
	/**
	 * \brief Console constructor.
	 * \param[in] application a pointer to the Application.
	 */
	Console(Application* application);

	/**
	 * \brief Gets the Application.
	 * \return a pointer to the Application.
	 */
	Application* application() {
	    return application_;
	}

	/**
	 * \copydoc GEO::Console::draw()
	 */
        void draw(bool* visible=nullptr, bool with_window=true) override;
	
      protected:
	/**
	 * \copydoc Console::exec_command()
	 */
	bool exec_command(const char* command) override;

	/**
	 * \copydoc Console::update()
	 */
	void update() override;
	
	/**
	 * \copydoc Console::notify_error()
	 */
	void notify_error(const std::string& err) override;

      private:
	Application* application_;
    };
    
}

#endif
