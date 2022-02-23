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
 
#ifndef H_SKIN_IMGUI_TEXT_EDITOR_H
#define H_SKIN_IMGUI_TEXT_EDITOR_H

#include <OGF/skin_imgui/common/common.h>
#include <OGF/gom/types/node.h>

// Forward declaration for ImGui text editor.
class TextEditor;

namespace OGF {

    /**
     * \brief A TextEditor for writing LUA functions.
     */
    gom_class TextEditor : public Object {
      public:
	/**
	 * \brief TextEditor constructor.
	 */
	TextEditor(Interpreter* interpreter);

	/**
	 * \brief TextEditor destructor.
	 */
	 ~TextEditor() override;

	/**
	 * \brief Gets the implementation.
	 * \return a pointer to the implementation.
	 */
	::TextEditor* impl() {
	    return impl_;
	}

	/**
	 * \brief Gets the Interpreter.
	 * \return a pointer to the Interpreter.
	 */
	Interpreter* interpreter() {
	    return interpreter_;
	}

	/**
	 * \brief Gets the current list of completions.
	 * \return a reference to a vector with the list of
	 *  completions.
	 */
	std::vector<std::string>& completions() {
	    return completions_;
	}
	
      gom_properties:
	/**
	 * \brief Sets the text.
	 * \param[in] value the new text that will replace the
	 *  contents of this editor.
	 */
	void set_text(const std::string& value);

	/**
	 * \brief Gets the text.
	 * \return a copy of the contents of the text buffer.
	 */
	std::string get_text() const;

	/**
	 * \brief Gets the selected text.
	 * \return a string with the selected text.
	 */
	std::string get_selection() const;
	
	/**
	 * \brief Sets the language used for syntax highlighting.
	 * \param[in] language one of "lua","glsl","c","c++"
	 */
	void set_language(const std::string& language);

	/**
	 * \brief Gets the language used for syntax highlighting.
	 */
	const std::string& get_language() const;

      gom_slots:
	/**
	 * \brief Clears the contents of this editor.
	 */
	void clear();

	/**
	 * \brief Draws the editor.
	 * \param[in] title a unique ImGui ID used to 
	 *  identify this editor.
	 */
	void draw(const std::string& title);

	/**
	 * \brief Loads a file in the editor.
	 * \param[in] filename the name of the file.
	 */
	void load(const std::string& filename);

	/**
	 * \brief Saves the contents of the editor into a file.
	 * \param[in] filename the name of the file.
	 */
	void save(const std::string& filename);

	void add_error_marker(index_t line, const std::string& error_message);

	void clear_error_markers();

	void add_breakpoint(index_t line);

	void clear_breakpoints();

	void find(const std::string& s);

	void cursor_forward();
	
      gom_signals:
	/**
	 * \brief Invoked when <F5> is pressed.
	 */
	void run_request();

	/**
	 * \brief Invoked when <F2> is pressed.
	 */
	void save_request();

	/**
	 * \brief Invoked when <Ctrl><F> is pressed.
	 */
	void find_request();

	/**
	 * \brief Invoked when <Ctrl><C> is pressed.
	 */
	void stop_request();

	/**
	 * \brief Invoked when the cursor hovers over a
	 *  word.
	 * \param[in] context a string with the sourcecode
	 *  element to be explained in the tooltip.
	 */
	void tooltip_request(const std::string& context);
	
      private:
	Interpreter* interpreter_;
	::TextEditor* impl_;
	std::string language_;
	std::vector<std::string> completions_;
    };
    
}

#endif
