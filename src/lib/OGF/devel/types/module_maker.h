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

#ifndef H_OGF_DEVEL_TYPES_MODULE_MAKER_H
#define H_OGF_DEVEL_TYPES_MODULE_MAKER_H

#include <OGF/devel/common/common.h>
#include <OGF/basic/os/text_utils.h>

/**
 * \file OGF/devel/types/module_maker.h
 * \brief Class to generate plugins from skeleton files.
 */

namespace OGF {

    /**
     * \brief Generates plugins using skeletons in lib/devel/
     * \details Automates the generation of all the tedious to write
     *  code for creating new object types, commands, shaders and tools.
     */
    class DEVEL_API ModuleMaker {
    public:
        /**
         * \brief ModuleMaker constructor.
         */
        ModuleMaker();

        /**
         * \brief Creates a new module (a plugin).
         * \details Generates C++ sources and configuration files
         *  for a new module.
         * \param[in] module_name name of the module
         * \param[in] author_name name of the author, to be copied
         *  in header files and in generated module information
         */
        bool create_module(
            const std::string& module_name,
            const std::string& author_name
        );

        /**
         * \brief Creates a new C++ source in a module.
         * \details In the skeleton files, the variables 
         *  "file", "FILE", "package", "Package", "PACKAGE", 
         *  "PACKAGE_API" and "author" are substituted.
         * \param[in] module_name name of the module
         * \param[in] subdirectory subdirectory where the file will 
         *  be created. If the subdirectory does not exist, it will
         *  be created as well
         * \param[in] file_name name of the file, without the extension
         * \param[in] header_skel name of the skeleton to be used for the
         *  header, relative to the lib/ subdirectory of Graphite
         * \param[in] source_skel name of the skeleton to be used for the
         *  source, relative to the lib/ subdirectory of Graphite
         */
        void create_file(
            const std::string& module_name,
            const std::string& subdirectory,
            const std::string& file_name,
            const std::string& header_skel = "devel/file.h.skel", 
            const std::string& source_skel = "devel/file.cpp.skel"
        );

        /**
         * \brief Creates sources for a new C++ class in a module.
         * \details The generated files are named like the class, with
         *  underscore-separated words (instead of camel-case).
         *  In the skeleton files, the variables "class_name",
         *  "file", "FILE", "package", "Package", "PACKAGE", 
         *  "PACKAGE_API" and "author" are substituted.
         * \param[in] module_name name of the module
         * \param[in] subdirectory subdirectory where the file will 
         *  be created. If the subdirectory does not exist, it will
         *  be created as well
         * \param[in] class_name name of the class
         * \param[in] header_skel name of the skeleton to be used for the
         *  header, relative to the lib/ subdirectory of Graphite
         * \param[in] source_skel name of the skeleton to be used for the
         *  source, relative to the lib/ subdirectory of Graphite
         */
        void create_class(
            const std::string& module_name,
            const std::string& subdirectory,
            const std::string& class_name,
            const std::string& header_skel = "devel/class.h.skel", 
            const std::string& source_skel = "devel/class.cpp.skel"
        );


        /**
         * \brief Creates sources for a new C++ GOM class in a module.
         * \details The generated files are named like the class, with
         *  underscore-separated words (instead of camel-case).
         *  In the skeleton files, the variables "class_name", 
         *  "base_class_name", "base_class_header_file", "file", 
         *  "FILE", "package", "Package", "PACKAGE", 
         *  "PACKAGE_API" and "author" are substituted.
         * \param[in] module_name name of the module
         * \param[in] subdirectory subdirectory where the file will 
         *  be created. If the subdirectory does not exist, it will
         *  be created as well
         * \param[in] base_class_name name of the base class (inherited
         *  by the class), with the "OGF::" prefix
         * \param[in] class_name name of the class
         * \param[in] header_skel name of the skeleton to be used for the
         *  header, relative to the lib/ subdirectory of Graphite
         * \param[in] source_skel name of the skeleton to be used for the
         *  source, relative to the lib/ subdirectory of Graphite
         */
        bool create_gom_class(
            const std::string& module_name,
            const std::string& subdirectory, 
            const std::string& base_class_name,
            const std::string& class_name,
            const std::string& header_skel = "devel/gom_class.h.skel", 
            const std::string& source_skel = "devel/gom_class.cpp.skel"
        );

        /**
         * \brief Creates sources for a new C++ GOM class in a module.
         * \details The generated files are named like the class, with
         *  underscore-separated words (instead of camel-case).
         *  In the skeleton files, the variables "class_name", 
         *  "base_class_name", "base_class_header_file", "file", 
         *  "FILE", "package", "Package", "PACKAGE", "PACKAGE_API", 
         *  "author" as well as the ones specified in \p env 
         *  are substituted.
         * \param[in] module_name name of the module
         * \param[in] subdirectory subdirectory where the file will 
         *  be created. If the subdirectory does not exist, it will
         *  be created as well
         * \param[in] base_class_name name of the base class (inherited
         *  by the class)
         * \param[in] class_name name of the class
         * \param[in] header_skel name of the skeleton to be used for the
         *  header, relative to the lib/ subdirectory of Graphite
         * \param[in] source_skel name of the skeleton to be used for the
         *  source, relative to the lib/ subdirectory of Graphite
         * \param[in] env a reference to a TextUtils::Environment used to 
         *  specified additional substitutions
         */
        bool create_gom_class(
            const std::string& module_name,
            const std::string& subdirectory, 
            const std::string& base_class_name,
            const std::string& class_name,
            const std::string& header_skel,
            const std::string& source_skel,
            TextUtils::Environment& env
        );

        /**
         * \brief Creates source files for a new Commands class.
         * \details It also inserts the code to register the created
         *  Commands class in the initializer of the library.
         * \param[in] module_name name of the module
         * \param[in] grob_class_name name of the Grob class
         *  these Commands should be attached to, with the "OGF::" 
         *  prefix
         * \param[in] commands_name name of the commands without
         *  any prefix/suffix (e.g., use "Remesh" for 
         *  "OGF::MeshGrobRemeshCommands").
         */
        void create_commands(
            const std::string& module_name, 
            const std::string& grob_class_name, 
            const std::string& commands_name
        );

        /**
         * \brief Creates source files for a new Shader class.
         * \details It also inserts the code to register the created
         *  Commands class in the initializer of the library.
         * \param[in] module_name name of the module
         * \param[in] grob_class_name name of the Grob class
         *  these Commands should be attached to, with the "OGF::" 
         *  prefix
         * \param[in] shader_name name of the shader without
         *  any prefix/suffix (e.g., use "Param" for 
         *  "OGF::ParamSurfaceShader").
         * \param[in] base_class_name name of the base class (inherited
         *  by the class), with the "OGF::" prefix
         */
        void create_shader(
            const std::string& module_name,
            const std::string& grob_class_name,
            const std::string& shader_name,
            const std::string& base_class_name = ""
        );

        /**
         * \brief Creates source files for a new Tool class.
         * \details It also inserts the code to register the created
         *  Tool class in the initializer of the library.
         * \param[in] module_name name of the module
         * \param[in] grob_class_name name of the Grob class
         *  this Tool should be attached to, with the "OGF::" 
         *  prefix
         * \param[in] tool_name name of the commands without
         *  any prefix/suffix (e.g., use "Edit" for 
         *  "OGF::MeshGrobEditTool").
         * \param[in] base_class_name name of the base class (inherited
         *  by the class), with the "OGF::" prefix
         */
        void create_tool(
            const std::string& module_name,
            const std::string& grob_class_name,
            const std::string& tool_name,
            const std::string& base_class_name = ""
        );

        /**
         * \brief Creates source files for a new Grob class.
         * \details It also inserts the code to register the created
         *  Grob class in the initializer of the library.
         * \param[in] module_name name of the module
         * \param[in] grob_name name of the Grob class without
         *  the "OGF::" prefix
         * \param[in] file_extension extension of the files used to
         *  save/load objects with this Grob class
         * \param[in] base_class_name optional name of the base class 
         *  (inherited by the class), with the "OGF::" prefix
         */
        void create_grob(
            const std::string& module_name,
            const std::string& grob_name,
            const std::string& file_extension = "",
            const std::string& base_class_name = "OGF::Grob"
        );

   protected:

        /**
         * \brief Sets the name of the current module where files should
         *  be generated.
         * \param[in] module_name the name of the module
         * \param[in] check if true, test whether the module already exists
         * \retval true if the module already exists or if \p check is false
         * \retval false if the module does not exist and \p check is true
         */
        bool set_module(const std::string& module_name, bool check = true);

        /**
         * \brief Sets the substitution rules deduced from the name of the
         *  current module.
         * \details This sets "package", "Package", "PACKAGE", "PACKAGE_API"
         *  and "author".
         * \param[out] env where to store the substitution rules
         * \see set_module()
         */
        void set_module_name(TextUtils::Environment& env);

        /**
         * \brief Creates a file from an skeleton file and a set of substitution
         *  rules.
         * \param[in] file_name name of the file to be created, relative to
         *  the path of the current module
         * \param[in] skel_name name of the skeleton file, relative to 
         *  lib/devel
         * \param[in] env a const reference to a TextUtils::Environment, with
         *  the substitution rules
         * \param[in] insert_header if true, the C++ header (with the copyright,
         *  author information etc...) is inserted at the beginning of the
         *  generated file
         * \retval true if the file could be created and the skeleton file
         *  exists
         * \retval false otherwise
         */
        bool create_file_from_skel(
            const std::string& file_name,
            const std::string& skel_name,
            const TextUtils::Environment& env,
            bool insert_header = false 
        );

        /**
         * \brief Creates a new directory.
         * \details The created directory is added to the list of directories
         *  where source files should be gathered in the CMake file.
         * \param[in] dir_name directory to be created, relative to the 
         *  path of the current module
         * \retval true if the directory could be created or already exists
         * \retval false otherwise
         */
        bool create_directory(
            const std::string& dir_name
        );

        /**
         * \brief Converts a class name to a file name.
         * \param[in] class_name the class name, possibly with the "OGF::"
         *  prefix, and in CamelCase
         * \return the file name, all lower case, with one underscore between
         *  each word
         */
        std::string class_name_to_file_name(const std::string& class_name);

        /**
         * \brief Converts a file name to an include guard.
         * \param[in] file_name the file name
         * \return the include guard, all upper case, with slashes
         *  converted into underscores, a "__OGF_" prefix and a "__" suffix
         */
        std::string file_name_to_include_guard(const std::string& file_name);

        /**
         * \brief Inserts a new line in a file.
         * \param[in] file_name the name of the file, relative to the
         *  path of the current module.
         * \param[in] insertion_point a string to be recognized in the
         *  file, that indicates where the line should be inserted. It
         *  will be inserted before the insertion point.
         * \param[in] line the line to be inserted
         * \retval true if the line could be inserted
         * \retval false otherwise
         */
        bool insert(
            const std::string& file_name,
            const std::string& insertion_point,
            const std::string& line
        );

        /**
         * \brief Adds a directory to the list of directories
         *  where source files should be gathered in the CMake file.
         * \param[in] directory the directory to be added, relative
         *  to the path of the current module
         */
        void add_directory_to_cmake(const std::string& directory);


	/**
	 * \brief Changes the modification time of the root CMakeLists.txt
	 *  in order to trigger recompilation.
	 */
	void touch_root_cmake();
	
   private:
        std::string project_root_;
        std::string package_;
        std::string package_path_;
        std::string author_;
    };

}

#endif
