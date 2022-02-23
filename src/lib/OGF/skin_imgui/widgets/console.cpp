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

#include <OGF/skin_imgui/widgets/console.h>
#include <OGF/skin_imgui/types/application.h>
#include <OGF/gom/interpreter/interpreter.h>

namespace {
    using namespace OGF;

    /**
     * \brief The autocompletion callback that queries the interpreter
     *  for autocompletion when the tab key is pushed.
     */
    void console_completion_callback(
	GEO::Console* console,
	const std::string& line, index_t startw, index_t endw,
	const std::string& cmpword, std::vector<std::string>& matches
    ) {
	// Makes sure that geogram console (GEO::) is a
	// graphite console (OGF::).	
	OGF::Console* ogf_console = dynamic_cast<OGF::Console*>(console);
	geo_assert(ogf_console != nullptr);
	Interpreter* interpreter = ogf_console->application()->interpreter();
	if(interpreter != nullptr) {
	    interpreter->automatic_completion(
		line, startw, endw, cmpword, matches
	    );
	}
    }

    /**
     * \brief The history callback that queries the interpreter
     *  for history when up and down arrows are pressed.
     */
    void console_history_callback(
	GEO::Console* console,
	index_t index,
	std::string& command_line
    ) {
	// Makes sure that geogram console (GEO::) is a
	// graphite console (OGF::).
	OGF::Console* ogf_console = dynamic_cast<OGF::Console*>(console);
	geo_assert(ogf_console != nullptr);
	Interpreter* interpreter = ogf_console->application()->interpreter();
	if(interpreter != nullptr) {
	    ogf_console->set_history_size(
		index_t(interpreter->history_size())
	    );
	    command_line = interpreter->history_line(index);
	}
    }
}

namespace OGF {

    Console::Console(Application* app) :
	GEO::Console(nullptr),
	application_(app)
    {
	set_completion_callback(console_completion_callback);
	set_history_callback(console_history_callback);
    }

    bool Console::exec_command(const char* command) {
	++max_history_index_;
	history_index_ = max_history_index_+1;
	Application::instance()->exec_command(command);
	return true;
    }

    void Console::draw(bool* visible, bool with_window) {
	::GEO::Console::draw(visible, with_window);
    }

    // TODO: remove once common application framework is there
    void Console::update() {
	Application::instance()->draw();
    }

    void Console::notify_error(const std::string& err) {
	Application::instance()->notify_error(err);
    }
    
}
