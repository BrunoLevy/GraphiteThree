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
 

#include <OGF/basic/os/file_manager.h>
#include <OGF/basic/os/text_utils.h>
#include <OGF/basic/modules/modmgr.h>

#include <geogram/basic/file_system.h>
#include <geogram/basic/process.h>


#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <ctype.h>

namespace OGF {

    /****************************************************************/

    
    FileManager* FileManager::instance_ = nullptr ;

    void FileManager::initialize() {
        ogf_assert(instance_ == nullptr);
        instance_ = new FileManager() ;
        Environment::instance()->add_environment(instance_);        
    }

    void FileManager::terminate() {
        // Note: instance should not be deleted, since it
        // is registered in the Environment, it will be
        // deleted by the Environment (that has ownership).
        instance_ = nullptr ;
    }

    FileManager::FileManager() {
        //   The OGF path is found by starting from the full path of the
        // main executable, and moving upwards a certain number of times
        // (nb_path_components below), that depends on whether we are
        // under Windows or Unix (file structure is slightly different).
        std::string path = FileSystem::dir_name(Process::executable_filename());
        FileSystem::flip_slashes(path);

        // Linux build: binaries are in
        //   build/Linux-gcc-dynamic-(Release|Debug)/bin        
        index_t nb_path_components = 3;
        
#ifdef GEO_OS_WINDOWS
        // Windows build: binaries are in
        //   build/Windows/(Release|Debug)/bin/        
        nb_path_components = 4;

        // Detect a variant:
        // Windows binary distribution of Graphite,
        // binaries are in
        //   bin/win64
        if(FileSystem::base_name(path) == "win64") {
            nb_path_components = 2;
        }
#endif
        
        for(index_t i=0; i<nb_path_components; ++i) {
            if(i != 0) {
                libraries_subdirectory_ = "/" + libraries_subdirectory_;
            }
            std::string component = FileSystem::base_name(path);
           
#ifdef GEO_OS_WINDOWS
            //   Under Windows, dynamic libraries subdirectory
            // is the same as binaries subdirectory.
            libraries_subdirectory_ = component + libraries_subdirectory_;
#else
            //   Under Unix, dynamic libraries subdirectory is
            // in a "lib" directory, located in the parent
            // directory of "bin" (full path is obtained by
            // replacing "bin" with "lib" in the full binary path).
            libraries_subdirectory_ =
                ((component == "bin") ? "lib" : component) +
                libraries_subdirectory_;
#endif
           
            path = FileSystem::dir_name(path);
        }

        libraries_subdirectory_ += "/";
        
        ogf_path_.push_back(path);

        Logger::out("FileManager") << "   Main path=" << path << std::endl;
        Logger::out("FileManager") << "   Libraries subpath="
                                   << libraries_subdirectory_ << std::endl;
    }

    bool FileManager::find_file(
        std::string& file_name, bool verbose, const std::string& sub_path
    ) const {
        if(file_name == "") {
            return false ;
        }
       
        //   If is is an absolute path, do not search and
        // return it.
        if(file_name[0] == '/') {
            return true;
        }
       
#ifdef GEO_OS_WINDOWS
        //   If is is an absolute path, do not search and
        // return it (absolute path under windows stats with X:/...)
        if(file_name.length() >= 2 && file_name[1] == ':') {
            return true;
        }
#endif

       // Transform C++ scopes into subdirectories,
       // This is used for icon file names (OGF:: -> OGF/)
       // and for components file names (GML files).
       for(size_t i=2; i<file_name.length(); ++i) {
           if(file_name[i] == ':') {
               file_name[i] = '/';
           }
       }
       
       
       if(sub_path != "") {
           for(index_t i=0; i<ogf_path_.size(); ++i) {
               std::string cur_file_name = ogf_path_[i]+"/"+sub_path+file_name;
               if(FileSystem::is_file(cur_file_name)) {
                   file_name = cur_file_name;
                   return true;
               } 
           }
       }
       for(index_t i=0; i<ogf_path_.size(); ++i) {
           std::string cur_file_name = ogf_path_[i]+"/"+file_name;
           if(FileSystem::is_file(cur_file_name)) {
               file_name = cur_file_name;
               return true;
           } 
       }

       // If we still did not find it, try in the current working directory....
       std::string try_cwd =
           FileSystem::get_current_working_directory()+"/"+file_name;
       
       if(FileSystem::is_file(try_cwd)) {
           file_name = try_cwd;
           return true;
       }

       // Sorry, there is nothing I can do, this file is simply not there...
       if(verbose) {
           Logger::err("FileManager") << "did not find file:" 
                                      << file_name << std::endl ;
       }
       return false ;
    }

    bool FileManager::find_binary_file(
        std::string& file_name, bool verbose
    ) const {
        std::string full_file_name = dll_prefix() + file_name + dll_extension();
        bool result =
            find_file(full_file_name, false, libraries_subdirectory());
        if(result) {
            file_name = full_file_name;
            return true;
        }
        if(verbose) {
            Logger::err("FileManager") << "did not find binary file:" 
                                       << file_name << std::endl ;
        }
        return false ;
    }

    std::string FileManager::dll_prefix() const {
#ifdef GEO_OS_WINDOWS
        return "" ;
#elif defined (GEO_OS_APPLE)
        return "lib";
#else
        return "lib" ;
#endif
    }

    std::string FileManager::dll_extension() const {
#ifdef GEO_OS_WINDOWS
        return ".dll" ;
#elif defined (GEO_OS_APPLE)
        return ".dylib";
#else
        return ".so" ;
#endif
    }

    std::string FileManager::libraries_subdirectory() const {
        return libraries_subdirectory_;
    }
    
    FileManager* FileManager::instance() {
        return instance_ ;
    }

    bool FileManager::get_local_value(
        const std::string& name, std::string& value
    ) const {
        if(name == "PROJECT_ROOT") {
            value = project_root();
            return true;
        } else if(name == "OGF_PATH") {
            value = "";
            for(index_t i=0; i<ogf_path().size(); ++i) {
                if(value != "") {
                    value += ";";
                }
                value += ogf_path()[i];
            }
            return true;
        } else if(name == "LIBRARIES_SUBDIRECTORY") {
	    value = libraries_subdirectory_;
	    return true;
	} else if(name == "HOME_DIRECTORY") {
	    value = FileSystem::home_directory();
	    return true;
	} else if(name == "DLL_PREFIX") {
	    value = dll_prefix();
	    return true;
	} else if(name == "DLL_EXTENSION") {
	    value = dll_extension();
	    return true;
	}
        return false;
    }

    bool FileManager::set_local_value(
        const std::string& name, const std::string& value
    ) {
	if(name == "OGF_PATH") {
	    ogf_path_.clear();
	    String::split_string(value,';',ogf_path_);
	    return true;
	} else if(name == "LIBRARIES_SUBDIRECTORY") {
	    libraries_subdirectory_ = value;
	    return true;
	}
        return false;
    }
    
/****************************************************************/

}

