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
#include <geogram_gfx/third_party/ImGuiColorTextEdit_geogram/TextEditor.h>

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
		      target->impl()->GetCurrentCursorPosition();

		std::vector<std::string>& matches = target->completions();
		if(
		    mouse_coord.line == cursor_coord.line &&
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
		      target->impl()->GetCurrentCursorPosition();
		std::string L = target->impl()->GetLine(coords.line);

		static const char*
		    completer_word_break_characters = " .(){},+-*/=";

		if(L == "") {
		    return;
		}

		const char* buf = &L[0];

		// Locate beginning of current word
		const char* word_end = buf + coords.column;
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
	return impl_->GetSelection();
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
	bool first_one = !impl_->HasMarkers();
	if(get_language() == "lua" && impl()->HasSelection()) {
	    line += index_t(impl()->GetSelectionStart().line);
	}
	impl_->AddMarker(
	    line-1, 0xFF0000FF, 0xFF0000FF, error_message, error_message
	);
	// Move cursor position to first error so that users sees it !
	if(first_one) {
	    impl_->SetCursorPosition(::TextEditor::Coordinates(int(line-1),0));
	}
    }

    void TextEditor::clear_error_markers() {
	impl_->ClearMarkers();
    }

    void TextEditor::show_find_and_replace_dialog() {
	impl_->ShowFindAndReplaceDialog();
    }

    void TextEditor::set_language(const std::string& language) {
	language_ = language;
	if(language_ == "lua") {
	    impl_->SetLanguage(::TextEditor::Language::Lua());
	} else if(language == "glsl") {
	    impl_->SetLanguage(::TextEditor::Language::Glsl());
	} else if(language == "c") {
	    impl_->SetLanguage(::TextEditor::Language::C());
	} else if(language == "c++") {
	    impl_->SetLanguage(::TextEditor::Language::Cpp());
	} else if(language == "py" || language == "Python") {
	    language_ = "py";
	    impl_->SetLanguage(::TextEditor::Language::Python());
	} else {
	    Logger::err("TextEditor") << language << ": unknown language"
				      << std::endl;
	}
    }

    const std::string& TextEditor::get_language() const {
	return language_;
    }
}
