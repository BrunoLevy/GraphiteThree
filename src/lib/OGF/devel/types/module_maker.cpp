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

#include <OGF/devel/types/module_maker.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/basic/os/file_manager.h>
#include <OGF/basic/modules/module.h>
#include <OGF/basic/modules/modmgr.h>

#include <geogram/basic/file_system.h>
#include <geogram/basic/environment.h>

#include <ctype.h>

namespace {

    /**
     * \brief Converts a character to lowercase
     * \param[in] c the character to be converted
     * \return the lowercase version of character \p c
     *  if it exists, otherwise \p c
     */
    inline char char_tolower(char c) {
        int result = tolower(int(c));
        return result <= 255 ? char(result) : c;
    }

    /**
     * \brief Converts a character to uppercase
     * \param[in] c the character to be converted
     * \return the uppercase version of character \p c
     *  if it exists, otherwise \p c
     */
    inline char char_toupper(char c) {
        int result = toupper(int(c));
        return result <= 255 ? char(result) : c;
    }

    /**
     * \brief Removes OGF:: from OGF::classname
     * \param[in] class_name_in the class name to be stripped
     * \return \p class_name with OGF:: scope removed
     */
    std::string strip_namespaces(const std::string& class_name_in) {
        std::string class_name = class_name_in;
        size_t pos = std::string::npos;
        do {
            pos = class_name.find(':');
            if(pos != std::string::npos) {
                class_name =
                    class_name.substr(pos+1, class_name.length()-pos-1);
            }
        } while(pos != std::string::npos);
        return class_name;
    }
}

namespace OGF {

    ModuleMaker::ModuleMaker() {
    }

    bool ModuleMaker::create_module(
        const std::string& module_name,
        const std::string& author
    ) {
        author_ = author;
        set_module(module_name, false);
        TextUtils::Environment env;
        set_module_name(env);

        if(FileSystem::is_directory(package_path_)) {
            Logger::err("ModuleMaker")
                << "Directory \'" << package_path_ << "\' already exists"
                << std::endl;
            return false;
        }

        if(!FileSystem::create_directory(package_path_)) {
            Logger::err("ModuleMaker") 
                << "Could not create directory \'" << package_path_
                << "\'" << std::endl;
            return false;
        }

        if(!create_file_from_skel(
               "CMakeLists.txt", "devel/CMakeLists.txt.skel", env
            )
        ) {
            return false;
        }

        if(!create_directory("common")) {
            return false;
        }

        if(!create_directory("algo")) {
            return false;
        }

        if(!create_file_from_skel(
               "common/common.h", "devel/common.h.skel", env, true
            )
        ) {
            return false;
        }

        if(!create_file_from_skel(
               "common/" + package_ + "_common.cpp",
               "devel/common.cpp.skel", env, true
            ) 
        ) {
            return false;
        }

        //   Register a module information, so that the other commands
        // can see the new module without having to compile / load it.

        if(ModuleManager::instance()->resolve_module(module_name) == nullptr) {
            Module* module_info = new Module;
            module_info->set_name(module_name);
            module_info->set_vendor(author);
            module_info->set_version("3-1.x");
            module_info->set_info("Module under construction...");
            module_info->set_is_system(false);
            module_info->set_is_dynamic(true);            
            Module::bind_module(module_name, module_info);
        }

	if(FileSystem::is_file(package_path_ + "/../Plugins.txt")) {
	    Logger::out("ModuleMaker") << "Declaring plugin in "
				 << package_path_ + "/../Plugins.txt"
				 << std::endl;
	    std::ofstream out;
	    out.open(
		(package_path_ + "/../Plugins.txt").c_str(),
		std::ios::out | std::ios::app
	    );
	    out << "add_subdirectory(" << module_name << ")" << std::endl;
	} else {
	    Logger::out("ModuleMaker") << "Creating (and declaring plugin in) "
				 << package_path_ + "/../Plugins.txt"
				 << std::endl;
	    std::ofstream out;
	    out.open(
		(package_path_ + "/../Plugins.txt").c_str()
	    );
	    out << "add_subdirectory(" << module_name << ")" << std::endl;
	}

	touch_root_cmake();
	
        return true;
    }

    void ModuleMaker::create_file(
        const std::string& module_name, const std::string& subdirectory,
        const std::string& file_name,
        const std::string& header_skel, const std::string& source_skel
    ) {

        if(!set_module(module_name)) {
            return;
        }

        std::string dir_name = FileSystem::dir_name(file_name);
        std::string base_name = FileSystem::base_name(file_name);
        std::string extension = FileSystem::extension(file_name);

        if(subdirectory.find('/',0) < subdirectory.length()) {
            Logger::err("ModuleMaker")
                << "only one level of subdirectory is supported" << std::endl;
        }

        if(extension.length() != 0) {
            Logger::err("ModuleMaker")
                << "Should not specify any extension" << std::endl;
            return;
        }

        std::string file = subdirectory + "/" + file_name;
        std::string FILE = file_name_to_include_guard(file_name); 

        TextUtils::Environment env;
        set_module_name(env);
        env.set_value("file", file);
        env.set_value("FILE", FILE);

        if(!FileSystem::is_directory(package_path_)) {
            Logger::err("ModuleMaker")
                << "Directory \'" << package_path_ << "\' does not exist"
                << std::endl;
            return;
        }

        if(!create_directory(subdirectory)) {
            return;
        }

        if(!create_file_from_skel(file + ".h", header_skel, env, true)) {
            return;
        }

        if(!create_file_from_skel(file + ".cpp", source_skel, env, true)) {
            return;
        }

	touch_root_cmake();	
    }

    void ModuleMaker::create_class(
        const std::string& module_name, const std::string& subdirectory, 
        const std::string& class_name, 
        const std::string& header_skel, const std::string& source_skel
    ) {
        if(!set_module(module_name)) {
            return;
        }

        if(subdirectory.find('/',0) < subdirectory.length()) {
            Logger::err("ModuleMaker")
                << "only one level of subdirectory is supported" << std::endl;
            return;
        }

        if(!create_directory(subdirectory)) {
            return;
        }

        std::string file_name =
            subdirectory + "/" + class_name_to_file_name(class_name);
        std::string FILE_NAME = file_name_to_include_guard(file_name); 

        TextUtils::Environment env;
        set_module_name(env);
        env.set_value("class_name", strip_namespaces(class_name));
        env.set_value("file", file_name);
        env.set_value("FILE", FILE_NAME);

        if(!create_file_from_skel(
               file_name + ".h", header_skel, env, true
            )
        ) {
            return;
        }

        if(!create_file_from_skel(
               file_name + ".cpp", source_skel, env, true
            )
        ) {
            return;
        }

	touch_root_cmake();	
    }

    bool ModuleMaker::create_gom_class(
        const std::string& module_name, const std::string& subdirectory, 
        const std::string& base_class_name, const std::string& class_name,
        const std::string& header_skel, const std::string& source_skel
    ) {
        TextUtils::Environment env;
        return create_gom_class(
            module_name, subdirectory, base_class_name, class_name,
            header_skel, source_skel, env
        );
    }

    bool ModuleMaker::create_gom_class(
        const std::string& module_name, const std::string& subdirectory, 
        const std::string& base_class_name, const std::string& class_name,
        const std::string& header_skel, const std::string& source_skel,
        TextUtils::Environment& env
    ) {
        if(!set_module(module_name)) {
            return false;
        }

        if(subdirectory.find('/',0) < subdirectory.length()) {
            Logger::err("ModuleMaker")
                << "only one level of subdirectory is supported" << std::endl;
            return false;
        }

        if(Meta::instance()->resolve_meta_type(class_name) != nullptr) {
            Logger::err("ModuleMaker")
                << "MetaType \'" << class_name
                << "\' already exists in GOM" << std::endl;
            return false;
        }

        MetaClass* base_mclass = dynamic_cast<MetaClass*>(
            Meta::instance()->resolve_meta_type(base_class_name)
        );

        if(base_mclass == nullptr) {
            Logger::err("ModuleMaker")
                << base_class_name << ": no such gom_class" << std::endl;
            return false;
        }

        if(!create_directory(subdirectory)) {
            return false;
        }

        std::string base_class_header_file = 
            "OGF/xxx/yyy/" + class_name_to_file_name(base_class_name) + ".h";

        if(base_mclass->has_custom_attribute("file")) {
            base_class_header_file =
                base_mclass->custom_attribute_value("file");
        } {
            Logger::warn("ModuleMaker")
                << "gom_class " << base_class_name 
                << " does not have the \'file\' attribute" << std::endl;
        }

        std::string file_name =
            subdirectory + "/" + class_name_to_file_name(class_name);
        std::string FILE_NAME = file_name_to_include_guard(file_name); 

        set_module_name(env);
        env.set_value("class_name", strip_namespaces(class_name));
        env.set_value("base_class_name", strip_namespaces(base_class_name));
        env.set_value("base_class_header_file", base_class_header_file);
        env.set_value("file", file_name);
        env.set_value("FILE", FILE_NAME);

        if(!create_file_from_skel(
               file_name + ".h", header_skel, env, true
            )
        ) {
            return false;
        }

        if(!create_file_from_skel(
               file_name + ".cpp", source_skel, env, true
            )
        ) {
            return false;
        }
	touch_root_cmake();	
        return true;
    }

    void ModuleMaker::create_commands(
        const std::string& module_name, 
        const std::string& grob_class_name, 
        const std::string& commands_name
    ) {
        
        if(!set_module(module_name)) {
            return;
        }

        MetaClass* grob_mclass = dynamic_cast<MetaClass*>(
            Meta::instance()->resolve_meta_type(grob_class_name)
        );

        if(grob_mclass == nullptr) {
            Logger::err("ModuleMaker")
                << grob_class_name << ": no such grob class" << std::endl;
            return;
        }

        if(!grob_mclass->is_a(
               Meta::instance()->resolve_meta_type("OGF::Grob"))
        ) {
            Logger::err("ModuleMaker")
                << grob_class_name << " does not inherit Grob" << std::endl;
            return;
        }


        if(String::string_starts_with(commands_name, "OGF::")) {
            Logger::err("ModuleMaker")
                << "commands name should be like "
                << "\'Mesh\' for \'SurfaceMeshCommands\'" << std::endl;
            return;
        }
        
        if(String::string_starts_with(commands_name, grob_class_name)) {
            Logger::err("ModuleMaker")
                << "commands name should be like "
                << "\'Mesh\' for \'SurfaceMeshCommands\'" << std::endl;
            return;
        }

        if(String::string_ends_with(commands_name, "Commands")) {
            Logger::err("ModuleMaker")
                << "commands name should be like "
                << "\'Mesh\' for \'SurfaceMeshCommands\'" << std::endl;
            return;
        }

        std::string class_name = grob_class_name + commands_name + "Commands";
        std::string base_class_name = grob_class_name + "Commands";
        create_gom_class(
            module_name, "commands", base_class_name, class_name,
            "devel/commands.h.skel", "devel/commands.cpp.skel"
        );

        insert(
            "common/" + package_ + "_common.cpp", 
            "[includes insertion point]",
            "#include <OGF/" + package_ + "/commands/" +
            class_name_to_file_name(class_name) + ".h>"
        );


        insert(
            "common/" + package_ + "_common.cpp", 
            "[source insertion point]",
            "        ogf_register_grob_commands<" +
            grob_class_name + "," + class_name + ">();"
        );
	touch_root_cmake();	
    }

    void ModuleMaker::create_shader(
        const std::string& module_name,
        const std::string& grob_class_name,
        const std::string& shader_name,        
        const std::string& base_class_name_in
    ) {
        if(!set_module(module_name)) {
            return;
        }
        
        MetaClass* grob_mclass = dynamic_cast<MetaClass*>(
            Meta::instance()->resolve_meta_type(grob_class_name) 
        );
        if(grob_mclass == nullptr) {
            Logger::err("ModuleMaker")
                << grob_class_name << ": no such gom_class" << std::endl;
            return;
        }
        if(!grob_mclass->is_a(
               Meta::instance()->resolve_meta_type("OGF::Grob"))
        ) {
            Logger::err("ModuleMaker")
                << grob_class_name << ": not a Grob class" << std::endl;
            return;
        }

        std::string grob_name = grob_class_name;
        if(String::string_starts_with(grob_name, "OGF::")) {
            grob_name = grob_name.substr(
                5, grob_class_name.length()-5
            );
        }
        
        if(
            String::string_starts_with(shader_name, "OGF::")   ||
            String::string_starts_with(shader_name, grob_name) ||
            String::string_ends_with(shader_name, "Shader")
        ){
            Logger::err("ModuleMaker")
                << "shader_name should be like \'Param\' "
                << "for \'ParamSurfaceShader\'" << std::endl;
            return;
        }
        
        std::string base_class_name = base_class_name_in;
        
        if(base_class_name == "") {
            base_class_name = grob_class_name + "Shader";
        }
        
        MetaClass* base_mclass = dynamic_cast<MetaClass*>(
            Meta::instance()->resolve_meta_type(base_class_name) 
        );
        
        if(base_mclass == nullptr) {
            Logger::err("ModuleMaker")
                << base_class_name << ": no such gom_class" << std::endl;
            return;
        }
        
        if(!base_mclass->is_a(
               Meta::instance()->resolve_meta_type("OGF::Shader"))
        ) {
            Logger::err("ModuleMaker")
                << base_class_name << ": not a Shader" << std::endl;
            return;
        }

        std::string class_name =
            grob_name + shader_name + "Shader";

        TextUtils::Environment env;
        env.set_value("grob_class_name", grob_class_name);

        create_gom_class(
            module_name, "shaders", base_class_name, class_name,
            "devel/shader.h.skel", "devel/shader.cpp.skel", env
        );

        insert(
            "common/" + package_ + "_common.cpp", 
            "[includes insertion point]",
            "#include <OGF/" + package_ + "/shaders/" +
            class_name_to_file_name(class_name) + ".h>"
        );

        insert(
            "common/" + package_ + "_common.cpp", 
            "[source insertion point]",
            "        ogf_register_grob_shader<" +
            grob_class_name + "," + class_name + ">();"
        );
	touch_root_cmake();	
    }

    void ModuleMaker::create_tool(
        const std::string& module_name,
        const std::string& grob_class_name,
        const std::string& tool_name,
        const std::string& base_class_name_in
    ) {
        if(!set_module(module_name)) {
            return;
        }

        MetaClass* grob_mclass = dynamic_cast<MetaClass*>(
            Meta::instance()->resolve_meta_type(grob_class_name) 
        );
        if(grob_mclass == nullptr) {
            Logger::err("ModuleMaker")
                << grob_class_name << ": no such gom_class" << std::endl;
            return;
        }
        if(!grob_mclass->is_a(
               Meta::instance()->resolve_meta_type("OGF::Grob"))
        ) {
            Logger::err("ModuleMaker")
                << grob_class_name << ": not a Grob class" << std::endl;
            return;
        }

        std::string grob_name = grob_class_name;
        if(String::string_starts_with(grob_name, "OGF::")) {
            grob_name = grob_name.substr(
                5, grob_class_name.length()-5
            );
        }

        if(
            String::string_starts_with(tool_name, "OGF::")   ||
            String::string_starts_with(tool_name, grob_name) ||
            String::string_ends_with(tool_name, "Tool")
        ){
            Logger::err("ModuleMaker")
                << "tool_name should be like \'Edit\' "
                << "for \'SurfaceEditTool\'" << std::endl;
            return;
        }


        std::string base_class_name = base_class_name_in;
        
        if(base_class_name == "") {
            base_class_name = grob_class_name + "Tool";
        }
        
        MetaClass* base_mclass = dynamic_cast<MetaClass*>(
            Meta::instance()->resolve_meta_type(base_class_name) 
        );
        
        if(base_mclass == nullptr) {
            Logger::err("ModuleMaker")
                << base_class_name << ": no such gom_class" << std::endl;
            return;
        }
        
        if(!base_mclass->is_a(
               Meta::instance()->resolve_meta_type("OGF::Tool"))
        ) {
            Logger::err("ModuleMaker")
                << base_class_name << ": not a Tool" << std::endl;
            return;
        }
        
        std::string class_name =
            grob_name + tool_name + "Tool"; 

        TextUtils::Environment env;
        env.set_value("grob_class_name", grob_class_name);
        env.set_value("base_class_name", base_class_name);
        env.set_value("tool_name", tool_name);
        
        if (
            create_gom_class(
                module_name, "tools", base_class_name, class_name,
                "devel/tool.h.skel", "devel/tool.cpp.skel", env
            )
        ) {

            insert(
                "common/" + package_ + "_common.cpp",
                "[includes insertion point]",
                "#include <OGF/" + package_ + "/tools/" +
                class_name_to_file_name(class_name) + ".h>"
            );

            insert(
                "common/" + package_ + "_common.cpp", 
                "[source insertion point]",
                "        ogf_register_grob_tool<" + grob_class_name +
                "," + class_name + ">();"
            );

        }
	touch_root_cmake();	
    }


    void ModuleMaker::create_grob(
        const std::string& module_name,
        const std::string& grob_name,
        const std::string& file_extension,
        const std::string& base_class_name_in
    ) {
        if(!set_module(module_name)) {
            return;
        }
        std::string base_class_name = base_class_name_in;
        if(base_class_name == "") {
            base_class_name = "OGF::Grob";
        }
        MetaClass* base_mclass = dynamic_cast<MetaClass*>(
            Meta::instance()->resolve_meta_type(base_class_name) 
        );
        if(base_mclass == nullptr) {
            Logger::err("ModuleMaker")
                << base_class_name << ": no such gom_class" << std::endl;
            return;
        }
        if(
            !base_mclass->is_a(Meta::instance()->resolve_meta_type("OGF::Grob"))
        ) {
            Logger::err("ModuleMaker")
                << base_class_name << ": not a Grob" << std::endl;
            return;
        }

        TextUtils::Environment env;
        env.set_value("grob_class_name", grob_name);

        create_gom_class(
            module_name, "grob", base_class_name, grob_name,
            "devel/grob.h.skel", "devel/grob.cpp.skel", env
        );

        env.set_value("grob_file", env.value("file"));
        env.set_value("grob_lowercase", class_name_to_file_name(grob_name));

        create_gom_class(
            module_name, "commands", "OGF::Commands", grob_name + "Commands",
            "devel/grob_commands.h.skel",
            "devel/grob_commands.cpp.skel",
            env
        );

        create_gom_class(
            module_name, "shaders", "OGF::Shader", grob_name + "Shader",
            "devel/grob_shader.h.skel", "devel/grob_shader.cpp.skel",
            env
        );

        create_gom_class(
            module_name, "tools", "OGF::Tool", grob_name + "Tool",
            "devel/grob_tool.h.skel", "devel/grob_tool.cpp.skel",
            env
        );

        insert(
            "common/" + package_ + "_common.cpp", 
            "[includes insertion point]",
            "#include <OGF/" + package_ + "/grob/" +
            class_name_to_file_name(grob_name) + ".h>"
        );

        insert(
            "common/" + package_ + "_common.cpp", 
            "[includes insertion point]",
            "#include <OGF/" + package_ + "/shaders/" +
            class_name_to_file_name(grob_name) + "_shader" + ".h>"
        );

        insert(
            "common/" + package_ + "_common.cpp", 
            "[source insertion point]",
            "        ogf_register_grob_type<" + grob_name + ">();"
        );

        insert(
            "common/" + package_ + "_common.cpp", 
            "[source insertion point]",
            "        ogf_register_grob_shader<" + grob_name + "," +
            "Plain" + grob_name + "Shader>();"
        );

        if(file_extension.length() != 0) {
            insert(
                "common/" + package_ + "_common.cpp", 
                "[source insertion point]",
                std::string("        SceneGraphLibrary::instance()->") +
                "register_grob_read_file_extension(\"" +
                std::string("OGF::") + grob_name + "\",\""
                + file_extension + "\");"
            );
        }
	touch_root_cmake();	
    }

    bool ModuleMaker::set_module(const std::string& module_name, bool check) {
        project_root_ = FileManager::instance()->ogf_path()[0];
        package_ = module_name;
        if (
            FileSystem::is_directory(
                project_root_ + "/src/lib/OGF/" + module_name
            )
        ) {
	    package_path_ = project_root_ + "/src/lib/OGF/" + module_name;
        } else {
	    package_path_ = project_root_ + "/plugins/OGF/" + module_name;
        }
        bool module_exists = FileSystem::is_directory(package_path_);
        if(module_exists) {
            FileSystem::touch(package_path_ + "/CMakeLists.txt");
        }
        if(check && !module_exists) {
            Logger::err("ModuleMaker") 
                << "Directory \'" << package_path_
                << "\' does not exist" << std::endl;
        }
        Module* module = ModuleManager::instance()->resolve_module(package_);
        if(check && module == nullptr) {
            Logger::err("ModuleMaker") 
                << "Module \'" << package_
                << "\' is not loaded" << std::endl;
        }
        if(module != nullptr) {
            author_ = module->vendor();            
        }
        return module_exists;
    }

    void ModuleMaker::set_module_name(TextUtils::Environment& env) {
        std::string package    = package_;
        std::string Package    = package_; 
        Package[0] = char_toupper(Package[0]);
        std::string PACKAGE = package_; 
        String::to_uppercase(PACKAGE);
        env.set_value("package", package);
        env.set_value("Package", Package);
        env.set_value("PACKAGE", PACKAGE);
        env.set_value("PACKAGE_API", PACKAGE + "_API");
        env.set_value("author", author_);
    }

    bool ModuleMaker::create_file_from_skel(
        const std::string& file_name, const std::string& skel_name_in,
        const TextUtils::Environment& env,
        bool insert_header
    ) {

        std::ofstream out((package_path_ + "/" + file_name).c_str());
        if(!out) {
            Logger::err("ModuleMaker")
                << "Could not create file: \'" 
                << package_path_ + "/" + file_name << "\'" << std::endl; 
            return false;
        }

        if(insert_header) {
            std::string header_file_name = "devel/header.skel";
            if(!FileManager::instance()->find_file(header_file_name)) {
                Logger::err("ModuleMaker") 
                    << "Could not find skeleton: \'"
                    << header_file_name << "\'" << std::endl; 
            }
            std::ifstream in(header_file_name.c_str());
            TextUtils::find_and_replace(in, out, env);            
        }

        std::string skel_name = skel_name_in;
        if(!FileManager::instance()->find_file(skel_name)) {
            Logger::err("ModuleMaker")
                << "Could not find skeleton: \'" << skel_name << "\'"
                << std::endl; 
        }
        std::ifstream in(skel_name.c_str());
        if(!in) {
            Logger::err("ModuleMaker") 
                << "Could not open skeleton file: \'" << skel_name << "\'"
                << std::endl; 
            return false;
        }

        
        TextUtils::find_and_replace(in, out, env);

        Logger::out("ModuleMaker") << "Successfully created file: \'" 
                                   << package_path_ + "/" + file_name
                                   << "\'" << std::endl;
        
        return true;
    }

    bool ModuleMaker::create_directory(const std::string& dir_name) {
        if (FileSystem::is_directory(package_path_ + "/" + dir_name)) {
            return true;
        }

        if (!FileSystem::create_directory(package_path_ + "/" + dir_name)) {
            Logger::err("ModuleMaker") << "Could not create directory \'"
                                       << package_path_ + "/" + dir_name
                                       << "\'" << std::endl;
            return false;
        }
        add_directory_to_cmake(dir_name);
        return true;
    }

    std::string ModuleMaker::class_name_to_file_name(
        const std::string& class_name_in
    ) {
        std::string class_name = strip_namespaces(class_name_in);
        std::string result;
        for(unsigned int i=0; i<class_name.length(); i++) {
            char c = class_name[i];
            if(i != 0 && isupper(c)) {
                result += "_";
            }
            result += String::char_to_string(char_tolower(c));
        }
        return result;
    }

    std::string ModuleMaker::file_name_to_include_guard(
        const std::string& file_name
    ) {
        std::string result = "H__OGF_" + package_ + "_" + file_name + "__H";
        for(unsigned int i=0; i<result.length(); i++) {        
            if(result[i] == '/') {
                result[i] = '_';
            } else {
                result[i] = char_toupper(result[i]);
            }
        }
        return result;
    }

    bool ModuleMaker::insert(
        const std::string& file_name, const std::string& insertion_point,
        const std::string& line_to_insert
    ) {

        {
            std::ifstream in((package_path_ + "/" + file_name).c_str());
            if(!in) {
                Logger::err("ModuleMaker")
                    << "Could not open file \'" 
                    << package_path_ + "/" + file_name << std::endl;
                return false;
            }

            std::ofstream out((package_path_ + "/tmp.txt").c_str());
            std::string line;
            
            while(getline(in,line)) {
                if(line.find(insertion_point) != std::string::npos) {
                    out << line_to_insert << std::endl;
                }
                out << line << std::endl;
            }
        }

        FileSystem::copy_file(
            package_path_ + "/tmp.txt", package_path_ + "/" + file_name
        );
        FileSystem::delete_file(package_path_ + "/tmp.txt");

        return true;
    }

    void ModuleMaker::add_directory_to_cmake(
        const std::string& directory
    ) {
        Logger::out("ModuleMaker") << "add directory to cmake:"
                                   << directory << std::endl;
        std::ifstream in((package_path_ + "/CMakeLists.txt").c_str());
        if(!in) {
            Logger::err("ModuleMaker")
                << "Could not open file \'" 
                << package_path_ + "/CMakeLists.txt" << std::endl;
            return;
        }

        // Maybe would be better with a [directories insert point] tag
        std::ofstream out((package_path_ + "/tmp.txt").c_str());
        std::string line;
        while(getline(in,line)) {
            if (line.find("gomgen") != std::string::npos) {
                out << "aux_source_directories(SOURCES \"Source Files\\\\"
                    << directory << "\" "
                    << directory
                    << ")" << std::endl;
            }
            out << line << std::endl;
        }
        out.close();
        in.close();
        
        FileSystem::copy_file(
            package_path_ + "/tmp.txt", package_path_ + "/CMakeLists.txt"
        );
        FileSystem::delete_file(package_path_ + "/tmp.txt");
    }

    void ModuleMaker::touch_root_cmake() {
	FileSystem::touch(project_root_ + "/CMakeLists.txt");
    }
}
