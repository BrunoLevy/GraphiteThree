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
 

#ifndef H_OGF_BASIC_OS_FILE_MANAGER_H
#define H_OGF_BASIC_OS_FILE_MANAGER_H

#include <OGF/basic/common/common.h>
#include <string>
#include <vector>

/**
 * \file OGF/basic/os/file_manager.h
 * \brief management of graphite path and files.
 */

namespace OGF {

//_________________________________________________________


   /**
    * \brief FileManager retreives the files used by 
    *  Graphite (icons, plugins).
    * \details Internally, FileManager manages a 
    *  path where to search for the files. This path
    *  contains the main Graphite path and the path to 
    *  the loaded plugins.
    */
    class BASIC_API FileManager : public Environment {
    public:
        /**
         * \brief Initializes the FileManager
         * \details This function is called once when Graphite starts (does not
         *  need to be called by client code).
         */
        static void initialize() ;

        /**
         * \brief Terminates the FileManager
         * \details This function is called once when Graphite stops (does not
         *  need to be called by client code).
         */
        static void terminate() ;

        /**
         * \brief Gets a pointer to the instance of the FileManager.
         * \return A pointer to the instance of the FileManager.
         */
        static FileManager* instance() ;

        /**
         * \brief Finds a file.
         * \param[in,out] file_name the name to be found. 
         *  On exit, if found, the full path to the file
         * \param[in] verbose if true, messages are logged 
         *  on success and failure
         * \param[in] sub_path a relative path that is prepended 
         *  to the file name before searching
         * \retval true if the file was found
         * \retval false otherwise
         */
        bool find_file(
            std::string& file_name,
            bool verbose=true, const std::string& sub_path="lib/"
        ) const ;

        /**
         * \brief Finds a binary file, such as the DLL of a plugin.
         * \param[in,out] file_name the name to be found, 
         *  as specified by the user (without "lib" prefix, 
         *  without ".dll" or ".so" suffix). 
         *  On exit, if found, the full path to the file 
         *  (with "lib" prefix, with ".dll" or ".so" suffix).
         * \param[in] verbose if true, messages are logged 
         *  on success and failure
         * \retval true if the file was found
         * \retval false otherwise
         */
        bool find_binary_file(
            std::string& file_name, bool verbose=true
        ) const ;

        /**
         * \brief Gets the prefix of dynamically loadable libraries file names.
         * \details Under Unix, "lib", and under Windows, empty string.
         * \return the prefix of dynamically loadable libraries file names.
         */
        std::string dll_prefix() const ;

        /**
         * \brief Gets the extension of dynamically loadable 
         *  libraries file names.
         * \details Under Unix, ".so", and under Windows, ".dll".
         * \return the extension of dynamically loadable libraries file names.
         */
        std::string dll_extension() const ;

        /**
         * \brief Gets the relative subdirectory where dynamic libraries are 
         *  stored.
         * \details It is different under Unix and Windows. 
         *  Under Unix, they are in a separate "lib/" 
         *  subdirectory. Under Windows, they are in the same
         *  directory as the main executable.
         */
        std::string libraries_subdirectory() const;

        /**
         * \brief Gets the OGF Path, i.e. the list of directories where files
         *  are searched.
         * \return a const reference to an array of strings, each of which 
         *  corresponding an entry in the OGF path.
         */
        const std::vector<std::string>& ogf_path() const {
            return ogf_path_;
        }

        /**
         * \brief Gets Graphite project root.
         * \details The element of the OGF Path that contains Graphite's main
         *  executable. It is always the first element of the OGF Path.
         * \return Graphite project root.
         */
        const std::string& project_root() const {
            ogf_assert(ogf_path().size() >= 1);
            return ogf_path_[0];
        }

        /**
         * \copydoc Environment::get_local_value()
         * \details Defines PROJECT_ROOT and OGF_PATH environment variables.
         */
        bool get_local_value(
            const std::string& name, std::string& value
        ) const override;

        /**
         * \copydoc Environment::set_local_value()
         * \details Does not define any new variable (does nothing
         *  and returns false).
         */
        bool set_local_value(
            const std::string& name, const std::string& value
        ) override;
        
    protected:
        FileManager() ;


    private:
        static FileManager* instance_;
        std::vector<std::string> ogf_path_;
        std::string libraries_subdirectory_;
    } ;

//_________________________________________________________

}
#endif

