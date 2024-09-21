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

#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <geogram/basic/algorithm.h>

namespace {
    using namespace OGF;

    /**
     * \brief Trims the beginning of a string if it matches a specified string
     * \param[in,out] s the string to be trimmed
     * \param[in] head the candidate head to be removed from
     *  the beginning of \p s
     * \retval true if the input string \p starts with \p head (and \p head
     *   is removed from \p s)
     * \retval false otherwise (and \p s remains unchanged)
     */
    bool trim_string_head(std::string& s, const std::string& head) {
        if(String::string_starts_with(s,head)) {
            s = s.substr(head.length(), s.length()-head.length());
            return true;
        }
        return false;
    }

    /**
     * \brief Trims the end of a string if it matches a specified string
     * \param[in,out] s the string to be trimmed
     * \param[in] tail the candidate head to be removed from the end of \p s
     * \retval true if the input string \p ends with \p tail (and \p tail
     *   is removed from \p s)
     * \retval false otherwise (and \p s remains unchanged)
     */
    bool trim_string_tail(std::string& s, const std::string& tail) {
        if(String::string_ends_with(s,tail)) {
            s = s.substr(0, s.length()-tail.length());
            return true;
        }
        return false;
    }

}

namespace OGF {

    SceneGraphLibrary* SceneGraphLibrary::instance_ = nullptr;

    SceneGraphLibrary::SceneGraphLibrary() {
        scene_graph_ = nullptr;
        scene_graph_shader_manager_ = nullptr;
        scene_graph_tools_manager_ = nullptr;
	owns_scene_graph_ = false;
    }

    SceneGraphLibrary::~SceneGraphLibrary() {
	if(owns_scene_graph_ && scene_graph_ != nullptr) {
	    scene_graph_->unref();
	    scene_graph_ = nullptr;
	}
    }

    void SceneGraphLibrary::initialize() {
        ogf_assert(instance_ == nullptr);
        instance_ = new SceneGraphLibrary();
        Environment::instance()->add_environment(instance_);
    }

    void SceneGraphLibrary::terminate() {
        ogf_assert(instance_ != nullptr);
        instance_ = nullptr;
        // Do not delete instance, pointer ownership was
        // transfered to Environment::instance().
    }

    SceneGraphLibrary* SceneGraphLibrary::instance() {
        ogf_assert(instance_ != nullptr);
        return instance_;
    }

    void SceneGraphLibrary::register_grob_type(
        const std::string& grob_class_name, bool abstract
    ) {
        ogf_assert(grob_infos_.find(grob_class_name) == grob_infos_.end());
        grob_infos_[grob_class_name] = GrobInfo(abstract);
        Environment::notify_observers("grob_types");
    }

    void SceneGraphLibrary::register_grob_read_file_extension(
        const std::string& grob_class_name, const std::string& extension
    ) {
        auto it = grob_infos_.find(grob_class_name);
        ogf_assert(it != grob_infos_.end());
        it->second.read_file_extensions.push_back(extension);
        Environment::notify_observers("grob_read_extensions");
        Environment::notify_observers(grob_class_name + "_read_extensions");
    }

    void SceneGraphLibrary::register_grob_write_file_extension(
        const std::string& grob_class_name, const std::string& extension
    ) {
        auto it = grob_infos_.find(grob_class_name);
        ogf_assert(it != grob_infos_.end());
        it->second.write_file_extensions.push_back(extension);
        Environment::notify_observers("grob_write_extensions");
        Environment::notify_observers(grob_class_name + "_write_extensions");
    }

    void SceneGraphLibrary::register_grob_shader(
        const std::string& grob_class_name,
        const std::string& shader_class_name,
        const std::string& shader_user_name_in
    ) {
        auto it = grob_infos_.find(grob_class_name);
        ogf_assert(it != grob_infos_.end());
        it->second.shaders.push_back(shader_class_name);

        std::string shader_user_name = shader_user_name_in;
        if(shader_user_name == "") {
            shader_user_name = shader_class_name;
            trim_string_head(shader_user_name,"OGF::");
            std::string tail = grob_class_name + "Shader";
            trim_string_head(tail, "OGF::");
            trim_string_tail(shader_user_name,tail);
        }

        it->second.shaders_user_names.push_back(
            shader_user_name == "" ? shader_class_name : shader_user_name
        );

	MetaType* shader_type =
            Meta::instance()->resolve_meta_type(shader_class_name);
	shader_type->create_custom_attribute("grob_class_name", grob_class_name);

        Environment::notify_observers(grob_class_name + "_shaders");
    }

    void SceneGraphLibrary::register_grob_tool(
        const std::string& grob_class_name, const std::string& tool_class_name
    ) {
        auto it = grob_infos_.find(grob_class_name);
        ogf_assert(it != grob_infos_.end());
        it->second.tools.push_back(tool_class_name);
        Environment::notify_observers(grob_class_name + "_tools");
	MetaType* tool_type =
            Meta::instance()->resolve_meta_type(tool_class_name);
	tool_type->create_custom_attribute("grob_class_name", grob_class_name);
    }

    void SceneGraphLibrary::register_grob_interface(
        const std::string& grob_class_name,
        const std::string& interface_class_name
    ) {
	auto it = grob_infos_.find(grob_class_name);
	ogf_assert(it != grob_infos_.end());
	it->second.interfaces.push_back(interface_class_name);
        Environment::notify_observers(grob_class_name + "_interfaces");
	MetaType* iface_type =
            Meta::instance()->resolve_meta_type(interface_class_name);
	iface_type->create_custom_attribute("grob_class_name", grob_class_name);
    }

    void SceneGraphLibrary::register_grob_commands(
        const std::string& grob_class_name,
        const std::string& commands_class_name
    ) {
	register_grob_interface(grob_class_name, commands_class_name);
	{
            auto it = grob_infos_.find(grob_class_name);
            ogf_assert(it != grob_infos_.end());
            it->second.commands.push_back(commands_class_name);
        }
        Environment::notify_observers(grob_class_name + "_commands");
    }

    void SceneGraphLibrary::register_full_screen_effect(
        const std::string& full_screen_effect_class_name,
        const std::string& user_name_in
    ) {
        full_screen_effects_.push_back(full_screen_effect_class_name);
        std::string user_name = user_name_in;
        if(user_name == "") {
            user_name = full_screen_effect_class_name;
            trim_string_head(user_name, "OGF::");
        }
        full_screen_effects_user_names_.push_back(user_name);
        Environment::notify_observers("full_screen_effects");
    }

    std::string SceneGraphLibrary::file_extension_to_grob(
        const std::string& extension
    ) const {
        std::string result;
        for(auto& it : grob_infos_) {
            if(it.second.has_read_extension(extension)) {
                if(result.length() == 0) {
                    result = it.first;
                } else {
                    result = result + ";" + it.first;
                }
            }
        }
        return result;
    }


    bool SceneGraphLibrary::set_local_value(
        const std::string& name, const std::string& value
    ) {
        ogf_argused(name);
        ogf_argused(value);
        return false;
    }

    bool SceneGraphLibrary::get_local_value(
        const std::string& name, std::string& value
    ) const {

        value = "";

        if(name == "grob_types") {
            for(auto& it : grob_infos_) {
		// Skip abstract grob classes.
                if(it.second.abstract) {
                    continue;
                }
                if(value.length() != 0) {
                    value += ";";
                }
                value += it.first;
            }
            return true;
        } else if(name == "grob_read_extensions") {
            for(auto& it : grob_infos_) {
                if(value.length() != 0) {
                    value += ";";
                }
                value += it.second.read_file_extensions_string();
            }
            return true;
        } else if(name == "grob_write_extensions") {
            for(auto& it : grob_infos_) {
                if(value.length() != 0) {
                    value += ";";
                }
                value += it.second.write_file_extensions_string();
            }
            return true;
        } else if(name == "full_screen_effects") {
            for(unsigned int i=0; i<full_screen_effects_.size(); i++) {
                if(i != 0) {
                    value += ";";
                }
                value += full_screen_effects_user_names_[i];
            }
            return true;
        } else if(name == "grob_instances") {
            value = "";
            for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
                Grob* grob = scene_graph()->ith_child(i);
                if(value != "") {
                    value += ";";
                }
                value += grob->name();
            }
            return true;
        } else {
            size_t sep = name.find('_',0);
            if(sep != std::string::npos) {
                std::string grob_class_name = name.substr(0, sep);
                std::string grob_var =
		    name.substr(sep + 1, name.length() - sep);

		// Recursively get attached tools/shaders/commands
		// from base class.
		if(grob_var != "instances") {
		    MetaClass* mclass = dynamic_cast<MetaClass*>(
			Meta::instance()->resolve_meta_type(grob_class_name)
		    );
		    if(mclass != nullptr) {
			MetaClass* super_class = mclass->super_class();
			if(super_class->name() != "OGF::Object") {
			    get_local_value(
				super_class->name() + "_" + grob_var,
				value
			    );
			    if(value != "") {
				value += ";";
			    }
			}
		    }
		}

                auto it = grob_infos_.find(grob_class_name);

                if(it == grob_infos_.end()) {
                    return false;
                }

                if(grob_var == "read_extensions") {
                    value += it->second.read_file_extensions_string();
                    return true;
                } else if(grob_var == "write_extensions") {
                    value += it->second.write_file_extensions_string();
                    return true;
                } else if(grob_var == "shaders") {
		    if(value == "") {
			value = it->second.shaders_user_names_string();
		    } else {
			// Remove duplicated shader names (when derived
			// class "overloads" a shader.
			// Note: see also classname resolution mechanism
			// in shader_user_to_classname()
			value += it->second.shaders_user_names_string();
			std::vector<std::string> shaders;
			String::split_string(value, ';', shaders);
			sort_unique(shaders);
			value = String::join_strings(shaders, ';');
		    }
                    return true;
                } else if(grob_var == "tools") {
                    value += it->second.tools_string();
                    return true;
                } else if(grob_var == "interfaces") {
                    value += it->second.interfaces_string();
                    return true;
                } else if(grob_var == "commands") {
                    value += it->second.commands_string();
                    return true;
                } else if(grob_var == "instances") {
                    value = "";
                    for(index_t i=0; i<scene_graph()->get_nb_children(); ++i) {
                        Grob* grob = scene_graph()->ith_child(i);
                        if(grob->meta_class()->name() == grob_class_name) {
                            if(value != "") {
                                value += ";";
                            }
                            value += grob->name();
                        }
                    }
                    return true;
                } else {
                    return false;
                }
            }
        }

        return false;
    }


    /*************************************************************************/

    std::string
    SceneGraphLibrary::GrobInfo::read_file_extensions_string() const {
        std::string result;
        for(unsigned int i=0; i<read_file_extensions.size(); i++) {
            if(i != 0) {
                result += ";";
            }
            result += "*." + read_file_extensions[i];
        }
        return result;
    }

    std::string
    SceneGraphLibrary::GrobInfo::write_file_extensions_string() const {
        std::string result;
        for(unsigned int i=0; i<write_file_extensions.size(); i++) {
            if(i != 0) {
                result += ";";
            }
            result += "*." + write_file_extensions[i];
        }
        return result;
    }

    std::string SceneGraphLibrary::GrobInfo::shaders_user_names_string() const {
        std::string result;
        for(unsigned int i=0; i<shaders.size(); i++) {
            if(i != 0) {
                result += ";";
            }
            result += shaders_user_names[i];
        }
        return result;
    }

    std::string SceneGraphLibrary::GrobInfo::tools_string() const {
        std::string result;
        for(unsigned int i=0; i<tools.size(); i++) {
            if(i != 0) {
                result += ";";
            }
            result += tools[i];
        }
        return result;
    }

    std::string SceneGraphLibrary::GrobInfo::commands_string() const {
        std::string result;
        for(unsigned int i=0; i<commands.size(); i++) {
            if(i != 0) {
                result += ";";
            }
            result += commands[i];
        }
        return result;
    }

    std::string SceneGraphLibrary::GrobInfo::interfaces_string() const {
        std::string result;
        for(unsigned int i=0; i<interfaces.size(); i++) {
            if(i != 0) {
                result += ";";
            }
            result += interfaces[i];
        }
        return result;
    }

    bool SceneGraphLibrary::GrobInfo::has_read_extension(
        const std::string& ext
    ) const {
        for(unsigned int i=0; i<read_file_extensions.size(); i++) {
            if(ext == read_file_extensions[i]) {
                return true;
            }
        }
        return false;
    }

    std::string SceneGraphLibrary::default_grob_read_extension(
        const std::string& grob_class_name
    ) const {
        auto it = grob_infos_.find(grob_class_name);
        ogf_assert(it != grob_infos_.end());
        if(it->second.read_file_extensions.size() != 0) {
            return it->second.read_file_extensions[0];
        }
        return "";
    }

    std::string SceneGraphLibrary::default_grob_write_extension(
        const std::string& grob_class_name
    ) const {
        auto it = grob_infos_.find(grob_class_name);
        ogf_assert(it != grob_infos_.end());
        if(it->second.write_file_extensions.size() != 0) {
            return it->second.write_file_extensions[0];
        }
        return "";
    }

    /*************************************************************************/

    const std::string& SceneGraphLibrary::shader_user_to_classname(
        const std::string& grob_class_name,
        const std::string& shader_user_name
    ) const {
        auto it = grob_infos_.find(grob_class_name);
        ogf_assert(it != grob_infos_.end());
        const GrobInfo& info = it->second;
        for(index_t i=0; i<info.shaders.size(); ++i) {
            if(info.shaders_user_names[i] == shader_user_name) {
                return info.shaders[i];
            }
        }
	// If not found, recurse to superclass.
	MetaClass* mclass = dynamic_cast<MetaClass*>(
	    Meta::instance()->resolve_meta_type(grob_class_name)
	);
	geo_assert(mclass != nullptr);
	MetaClass* super_class = mclass->super_class();
	if(super_class->name() != "OGF::Object") {
	    return shader_user_to_classname(
		super_class->name(), shader_user_name
	    );
	}
        ogf_assert_not_reached;
    }

    const std::string& SceneGraphLibrary::shader_classname_to_user(
        const std::string& grob_class_name,
        const std::string& shader_class_name
    ) const {
        auto it = grob_infos_.find(grob_class_name);
        ogf_assert(it != grob_infos_.end());
        const GrobInfo& info = it->second;
        for(index_t i=0; i<info.shaders.size(); ++i) {
            if(info.shaders[i] == shader_class_name) {
                return info.shaders_user_names[i];
            }
        }
        ogf_assert_not_reached;
    }

    const std::string& SceneGraphLibrary::full_screen_effect_classname_to_user(
        const std::string& full_screen_effect_classname
    ) const {
        for(index_t i=0; i<full_screen_effects_.size(); ++i) {
            if(full_screen_effects_[i] == full_screen_effect_classname) {
                return full_screen_effects_user_names_[i];
            }
        }
        ogf_assert_not_reached;
    }

    std::string SceneGraphLibrary::full_screen_effect_user_to_classname(
        const std::string& full_screen_effect_user_name
    ) const {
        for(index_t i=0; i<full_screen_effects_.size(); ++i) {
            if(full_screen_effects_user_names_[i] ==
               full_screen_effect_user_name) {
                return full_screen_effects_[i];
            }
        }
	Logger::warn("Effects") << full_screen_effect_user_name
				<< ": no such effect"
				<< std::endl;
	return "OGF::PlainFullScreenEffect";
    }

    void SceneGraphLibrary::scene_graph_values_changed_notify_environment() {
        Environment::notify_observers("grob_instances");
        for(auto& it : grob_infos_) {
            Environment::notify_observers(it.first+"_instances");
        }
    }

    void SceneGraphLibrary::set_scene_graph(
	SceneGraph* scene_graph, bool transfer_ownership
    ) {
	ogf_assert(scene_graph_ == nullptr);
	scene_graph_ = scene_graph;
	owns_scene_graph_ = transfer_ownership;
	if(owns_scene_graph_) {
	    scene_graph_->ref();
	}
    }


}
