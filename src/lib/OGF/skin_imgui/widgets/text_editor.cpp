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

#include <OGF/skin_imgui/widgets/text_editor.h>
#include <OGF/skin_imgui/types/application.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <geogram_gfx/third_party/ImGuiColorTextEdit/TextEditor.h>

namespace {
    using namespace OGF;
    
    void text_editor_callback(TextEditorAction action, void* client_data) {
	OGF::TextEditor* target = static_cast<OGF::TextEditor*>(client_data);
	switch(action) {
	    case TEXT_EDITOR_SAVE:
		target->save_request();
		break;
	    case TEXT_EDITOR_RUN:
		target->run_request();		
		break;
	    case TEXT_EDITOR_FIND:
		target->find_request();		
		break;
	    case TEXT_EDITOR_STOP:
		target->stop_request();
		break;
	    case TEXT_EDITOR_TOOLTIP: {
		// If <tab> was pushed and if there are some completions
		// and if the mouse is on the same line as the cursor,
		// display the completions in the tooltip.
		::TextEditor::Coordinates mouse_coord =
			target->impl()->GetMousePosition();
		    
		::TextEditor::Coordinates cursor_coord =
		      target->impl()->GetCursorPosition();
		    
		std::vector<std::string>& matches = target->completions();
		if(
		    mouse_coord.mLine == cursor_coord.mLine &&
		    matches.size() != 0
		) {
		    ImGui::BeginTooltip();
		    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

		    if(matches[0] == "Completion: No match") {
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]); 
			ImGui::PushStyleColor(
			    0, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)
			);
			ImGui::Text("%s",matches[0].c_str());			
			ImGui::PopStyleColor();
			ImGui::PopFont();
		    } else {
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
			ImGui::Text("Completions");
			ImGui::Separator();
			ImGui::PopFont();
			ImGui::BeginGroup();
			for(index_t i=0; i<index_t(matches.size()); ++i) {
			    if(i % 15 == 0 && i != 0) {
				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::Text("  ");
				ImGui::SameLine();
				ImGui::BeginGroup();
			    }
			    ImGui::Text("%s",matches[i].c_str());
			}
			ImGui::EndGroup();
		    }
		    ImGui::PopFont();
		    ImGui::EndTooltip();
		} else {
		    // Else display the 'online help' for the current context.
		    target->tooltip_request(target->impl()->GetWordContext());
		}
	    } break;
	    case TEXT_EDITOR_TEXT_CHANGED:
		target->completions().clear();
		break;
	    case TEXT_EDITOR_COMPLETION:
		
		if(target->get_language() == "GLSL") {
		    return;
		}
		
		::TextEditor::Coordinates coords =
		      target->impl()->GetCursorPosition();
		std::string L = target->impl()->GetLine(coords.mLine);

		static const char*
		    completer_word_break_characters = " .(){},+-*/=";
		
		if(L == "") {
		    return;
		}

		const char* buf = &L[0];

		// Locate beginning of current word
		const char* word_end = buf + coords.mColumn;
		const char* word_start = word_end;
		while (word_start > buf) {
		    const char c = word_start[-1];
		    if(strchr(completer_word_break_characters,c) != nullptr) {
			break;
		    }
		    word_start--;
		}  
		index_t startw = index_t(word_start - buf);
		index_t endw   = index_t(word_end - buf);
		std::string cmpword(word_start, size_t(word_end - word_start));
		std::vector<std::string>& matches = target->completions();

		Interpreter* interp = Interpreter::instance_by_file_extension(
		    target->get_language() // TODO: language <-> file extension
		);
		if(interp != nullptr) {
		    interp->automatic_completion(
			L, startw, endw, cmpword, matches
		    );
		}

		if(matches.size() == 0) {
		    matches.push_back("Completion: No match");
		    return;
		} else if(matches.size() == 1) {
		    target->impl()->InsertText(
			matches[0].c_str() + cmpword.length()
		    );
		} else {
		    // Several matches, find longest common prefix
		    std::string longest_prefix;
		    size_t cur_char = 0;
		    bool finished = false;
		    while(!finished) {
			char c = '\0';
			for(size_t i=0; i<matches.size(); ++i) {
			    if(
				cur_char >= matches[i].length() ||
				(i != 0 && matches[i][cur_char] != c)
			    ) {
				finished = true;
				break;
			    }
			    c = matches[i][cur_char];
			}
			if(!finished) {
			    longest_prefix.push_back(c);
			}
			++cur_char;
		    }
		    // Replace edited text with longest prefix
		    if(longest_prefix.length() != 0) {
			target->impl()->InsertText(
			    longest_prefix.c_str() + cmpword.length()
			);
		    }
		}
		if(matches.size() == 1) {
		    matches.clear();
		}
		break;
	}
    }
}

namespace OGF {

    TextEditor::TextEditor(Interpreter* interpreter) :
	interpreter_(interpreter)
    {
	impl_ = new ::TextEditor();
	impl_->set_callback(text_editor_callback, this);
	set_language("lua");
	clear();
    }

    TextEditor::~TextEditor() {
	delete impl_;
	impl_ = nullptr;
    }

    std::string TextEditor::get_text() const {
	return impl_->GetText();
    }

    void TextEditor::set_text(const std::string& value) {
	impl_->SetText(value);
    }

    std::string TextEditor::get_selection() const {
	return impl_->GetSelectedText();
    }
    
    void TextEditor::clear() {
	impl_->SetText("\n");
	impl_->SetCursorPosition(
	    ::TextEditor::Coordinates(0,0)
	);
    }

    void TextEditor::draw(const std::string& title) {
	if(
	    String::string_starts_with(
		Application::instance()->get_style(),
		"Light"
	    )
	) {
	    impl_->SetPalette(::TextEditor::GetLightPalette());	    
	} else {
	    impl_->SetPalette(::TextEditor::GetDarkPalette());	    	    
	}
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);	
	impl_->Render(title.c_str());
	ImGui::PopFont();
    }
    

    void TextEditor::load(const std::string& filename) {
	impl_->SetText("");
	std::ifstream in(filename.c_str());
	if(!in) {
	    Logger::err("TextEditor")
		<< "Could not open " << filename << std::endl;
	}
	while(in) {
	    std::string line;
	    std::getline(in,line);
	    impl_->InsertText(line);
	    impl_->InsertText("\n");	    
	}
	impl_->SetCursorPosition(
	    ::TextEditor::Coordinates(0,0)
	);
    }

    void TextEditor::save(const std::string& filename) {
	std::ofstream out(filename.c_str());
	if(!out) {
	    Logger::err("TextEditor")
		<< "Could not open " << filename << std::endl;
	}
	out << impl_->GetText();
    }

    void TextEditor::add_error_marker(
	index_t line, const std::string& error_message
    ) {
	if(get_language() == "lua" && impl()->HasSelection()) {
	    line += index_t(impl()->GetSelectionStart().mLine);
	}
	
	std::map<int, std::string> error_markers = impl_->GetErrorMarkers();
	bool first_one = (error_markers.size() == 0);
	error_markers[int(line)] = error_message;
	impl_->SetErrorMarkers(error_markers);
	// Move cursor position to first error so that users sees it !
	if(first_one) {
	    impl_->SetCursorPosition(::TextEditor::Coordinates(int(line-1),0));
	}
    }
    
    void TextEditor::clear_error_markers() {
	std::map<int, std::string> error_markers;
	impl_->SetErrorMarkers(error_markers);
    }

    void TextEditor::add_breakpoint(index_t line) {
	::TextEditor::Breakpoints bp = impl_->GetBreakpoints();
	bp.insert(int(line));
	impl_->SetBreakpoints(bp);	
    }
    
    void TextEditor::clear_breakpoints() {
	::TextEditor::Breakpoints bp;
	impl_->SetBreakpoints(bp);
    }

    
    
    void TextEditor::set_language(const std::string& language) {
	language_ = language;
	if(language_ == "lua") {
	    impl_->SetLanguageDefinition(
		::TextEditor::LanguageDefinition::Lua()
	    );
	} else if(language == "glsl") {
	    impl_->SetLanguageDefinition(
		::TextEditor::LanguageDefinition::GLSL()
	    );
	} else if(language == "c") {
	    impl_->SetLanguageDefinition(
		::TextEditor::LanguageDefinition::C()
	    );
	} else if(language == "c++") {
	    impl_->SetLanguageDefinition(
		::TextEditor::LanguageDefinition::CPlusPlus()
	    );
	} else if(language == "py" || language == "Python") {
	    language_ = "py";
	    // TODO: implement Python language definition
	    impl_->SetLanguageDefinition(
		::TextEditor::LanguageDefinition::Lua()
	    );
	} else {
	    Logger::err("TextEditor") << language << ": unknown language"
				      << std::endl;
	}
    }
    
    const std::string& TextEditor::get_language() const {
	return language_;
    }

    void TextEditor::find(const std::string& s) {
	if(s == "") {
	    clear_breakpoints();
	    return;
	}
	int first = -1;
	int where_in_first = -1;
	int cursor_line = impl_->GetCursorPosition().mLine;
	int cursor_column = impl_->GetCursorPosition().mColumn;	
	int found_line = -1;
	int where_in_line = -1;
	::TextEditor::Breakpoints bp;	
	for(int i=0; i<impl_->GetTotalLines(); ++i) {
	    std::string cur_line = impl_->GetText(
		::TextEditor::Coordinates(i,0),
		::TextEditor::Coordinates(i+1,0)
	    );
	    size_t from = (i == cursor_line) ? size_t(cursor_column) : 0;
	    size_t where_in_this_line = cur_line.find(s,from);
	    if(where_in_this_line != std::string::npos) {
		if(first == -1) {
		    first = i;
		    where_in_first = int(where_in_this_line);
		}
		if(i >= cursor_line && found_line == -1) {
		    found_line = i;
		    where_in_line = int(where_in_this_line);
		}
	    }
	    if(
		where_in_this_line != std::string::npos ||
		cur_line.find(s) != std::string::npos
	    ) {
		bp.insert(i+1);
	    }
	}
	impl_->SetBreakpoints(bp);
	if(found_line == -1 && first != -1) {
	    found_line = first;
	    where_in_line = where_in_first;
	}

	if(found_line != -1) {
	    std::string line = impl_->GetText(
		::TextEditor::Coordinates(found_line,0),
		::TextEditor::Coordinates(found_line+1,0)
	    );
	    impl_->SetCursorPosition(
		::TextEditor::Coordinates(found_line,int(where_in_line))
	    );
	    impl_->SetSelection(
		::TextEditor::Coordinates(found_line,int(where_in_line)),
		::TextEditor::Coordinates(
		    found_line,int(where_in_line) + int(s.length())
		)
	    );
	}
	
    }

    void TextEditor::cursor_forward() {
	int cursor_line = impl_->GetCursorPosition().mLine;
	int cursor_column = impl_->GetCursorPosition().mColumn;	
	impl_->SetCursorPosition(
	    ::TextEditor::Coordinates(cursor_line,cursor_column+1)
	);	
    }
    
}
