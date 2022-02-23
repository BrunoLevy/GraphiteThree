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
 
#include <OGF/basic/modules/modmgr.h>
#include <OGF/basic/modules/module.h>
#include <OGF/basic/os/file_manager.h>
#include <geogram/basic/file_system.h>

#include <string>

#include <iostream>
#include <stdlib.h>
#include <string.h>

#ifdef GEO_OS_WINDOWS
#  include <Psapi.h>
#else
#  include <dlfcn.h>
#endif


/******************************************************************************/
/** OS-independent functions **/

namespace OGF {

    ModuleManager* ModuleManager::instance_ = nullptr;
    
    ModuleManager* ModuleManager::instance() {
        return instance_;
    }
    
    void ModuleManager::initialize() {
        ogf_assert(instance_ == nullptr);
        instance_ = new ModuleManager();
        Environment::instance()->add_environment(instance_);
    }
    
    void ModuleManager::terminate() {
        // Note: instance should not be deleted, since it
        // is registered in the Environment, it will be
        // deleted by the Environment (that has ownership).
        instance_ = nullptr;
    }
    
    void ModuleManager::terminate_dynamic_modules() {
        instance()-> do_terminate_modules();
    }
    
    ModuleManager::ModuleManager() {
    }
    
    ModuleManager::~ModuleManager() {
    }
    
    bool ModuleManager::bind_module(
        const std::string& module_name, Module* module
    ) {
        if(resolve_module(module_name) != nullptr) {
            Logger::out("ModuleMgr") << "Note: Module \'" << module_name
                                     << "\' is already bound"
                                     << std::endl;
            return false;
        }
        modules_[module_name] = module;
        Environment::notify_observers("loaded_modules");
        Environment::notify_observers("loaded_dynamic_modules");
        return true;
    }
    
    bool ModuleManager::unbind_module(const std::string& module_name) {
	auto it = modules_.find(module_name);
        if(it == modules_.end()) {
            Logger::err("ModuleMgr") << "No such module: \'" << module_name
                                     << "\'" << std::endl;
            return false;
        }
        modules_.erase(it);
        Environment::notify_observers("loaded_modules");
        Environment::notify_observers("loaded_dynamic_modules");
        return true;
    }
    
    Module* ModuleManager::resolve_module(const std::string& module_name) {
        auto it = modules_.find(module_name);
        if(it == modules_.end()) {
            return nullptr;
        }
        return it->second;
    }
        
    bool ModuleManager::get_local_value(
        const std::string& name, std::string& value
    ) const {
        if(name == "loaded_modules") {
            value = "";
            for(auto& it : modules_) {
                if(value.length() != 0) {
                    value += ";";
                }
                value += it.first;
            }
            return true;
        } else if(name == "loaded_dynamic_modules") {
            value = "";
            for(auto& it : modules_) {
                if(!it.second->is_dynamic()) {
                    continue;
                }
                if(it.second->is_system()) {
                    continue;
                }
                if(value.length() != 0) {
                    value += ";";
                }
                value += it.first;
            }
            return true;
        } else {
            return false;
        }
    }

    bool ModuleManager::set_local_value(
        const std::string& name, const std::string& value
    ) {
        ogf_argused(name);
        ogf_argused(value);
        return false;        
    }

    ModuleManager::function_ptr ModuleManager::resolve_function(
	const std::string& name
    ) {
	static_assert(
	    sizeof(void*) == sizeof(OGF::ModuleManager::function_ptr),
	    "void* and function pointers need to have same size"
	);	
	void* gptr = resolve_symbol(name);
	function_ptr result = nullptr;
	memcpy(&result, &gptr, sizeof(void*));
	return result;
    }
}

/******************************************************************************/
/** OS-dependent functions **/

#ifdef GEO_OS_WINDOWS

namespace OGF {

    void ModuleManager::do_terminate_modules() {
        int nb_funcs = int(to_terminate_.size());
        for(int i = nb_funcs - 1; i >= 0; i--) {
            ModuleTerminateFunc module_terminate = to_terminate_[i];
            module_terminate();
        }
        int nb_modules = int(module_handles_.size());
        ogf_assert(nb_modules == nb_funcs);
        for(int i = nb_modules - 1; i >= 0; i--) {
            void* module_handle = module_handles_[i];
            FreeLibrary( (HMODULE) module_handle);
        }
    }

    bool ModuleManager::load_module(
        const std::string& module_name, bool quiet
    ) {
        std::string module_file_name = module_name;
        
        if(! FileManager::instance()->find_binary_file(
               module_file_name
        )) {
            if(!quiet) {
                Logger::err("ModuleMgr")  
                    << "module \'" << module_name 
                    << "\' not found" << std::endl;
            }
            return false;
        }

        void* module_handle = LoadLibrary(module_file_name.c_str());
    
        if(module_handle == nullptr) {
            // get the error message
            LPVOID lpMsgBuf;
            FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &lpMsgBuf,
                0,
                NULL 
            );
            // print the error
            Logger::err("ModuleMgr")  
                << "Could not open module \'" 
                << module_name << "\'" << std::endl 
                << "                     Error opening file \'"
                << module_file_name << "\'" << std::endl 
                << " reason: " << (LPTSTR) lpMsgBuf << std::endl;
            // free the error message
            LocalFree( lpMsgBuf );
            return  false;
        }
        
        // Retreive function pointers to initialize and
        //   terminate the module.

        std::string project_name = "OGF";
        std::string init_func_name =
            project_name + "_" + module_name + "_initialize";
        
        std::string term_func_name =
            project_name + "_" + module_name + "_terminate";        
        
        ModuleInitFunc p_init_func =
            (ModuleInitFunc)GetProcAddress(
                (HMODULE)module_handle, init_func_name.c_str()
            );
        
        ModuleTerminateFunc p_term_func =
            (ModuleTerminateFunc)GetProcAddress(
                (HMODULE)module_handle, term_func_name.c_str()
            );
        
        // Checks whether the module has init/termination functions.

        if(p_init_func == nullptr || p_term_func == nullptr) {
            Logger::warn("ModuleMgr")
                << "Could not find init/term of module \'" 
                << module_name << "\'" << std::endl;
        } else {
            // invoke the initialization function:
            // No need here to call p_init_func(), it is automatically 
            // called by ctor of objects at global scope
            
            // memorize the module handle
            module_handles_.push_back(module_handle);

            // memorize the termination function.
            to_terminate_.push_back(p_term_func);
        }
        
        Module* module_info = ModuleManager::resolve_module(module_name);
        if(module_info == nullptr) {
            module_info = new Module;
            module_info->set_name(module_name);
            ModuleManager::bind_module(module_name, module_info);
        } 
        module_info->set_is_dynamic(true);
        return true;
    }

    void* ModuleManager::resolve_symbol(
	const std::string& name
    ) {
	// Step 1: get the handles to all the modules used by the process
	vector<HMODULE> modules(16);
	DWORD cb=16*sizeof(HMODULE);
	DWORD needed_cb;
	EnumProcessModules(
	    GetCurrentProcess(),
	    modules.data(),
	    cb,
	    &needed_cb
	);
	modules.resize(needed_cb / sizeof(HMODULE));
	//   Do that again if result array was too small
	if(needed_cb > cb) {
	    EnumProcessModules(
		GetCurrentProcess(),
		modules.data(),
		cb,
		&needed_cb
	    );
	}
	// Step 2: lookup function in all loaded modules
	function_ptr result = nullptr;
	for(index_t i=0; i<modules.size(); ++i) {
	    result = function_ptr(GetProcAddress(
                modules[i], name.c_str()
	    ));	    
	}
	return result;
    }


    void ModuleManager::append_dynamic_libraries_path(const std::string& path_to_add) {
	// TODO: test AddDllDirectory() instead
	// (maybe needs SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS) as well)
	if(!SetDllDirectoryA(path_to_add.c_str())) {
	    DWORD err_code = GetLastError();
	    Logger::err("ModMgr") << "SetDllDirectoryA() returned " << err_code << std::endl;
	}
    }

}

#else

namespace {
    
    /**
     * \brief Calls dlsym and converts the result into
     *  a function pointer.
     * \details This functions is there to avoid warnings because
     *  it is illegal to directly cast a generic pointer to a
     *  function pointer.
     * \param[in] handle a pointer to a shared object, previously
     *  obtained through dlopen(), or nullptr (lookup function in 
     *  all modules).
     * \param[in] symbname the name of a symbol
     * \return a function pointer to the symbol, or NULL if there is
     *  no such function.
     */
    OGF::ModuleManager::function_ptr dlsym_fptr(
	void* handle, const char* symbname
    ) {
	OGF::ModuleManager::function_ptr result = nullptr;
	void* gptr = dlsym(handle, symbname);
	geo_assert(sizeof(void*) == sizeof(OGF::ModuleManager::function_ptr));
	memcpy(&result, &gptr, sizeof(void*));
	return result;
    }
}

namespace OGF {

    void ModuleManager::do_terminate_modules() {
        int nb_funcs = int(to_terminate_.size());
        for(int i = nb_funcs - 1; i >= 0; i--) {
            ModuleTerminateFunc module_terminate = to_terminate_[size_t(i)];
            module_terminate();
        }
        
        int nb_modules = int(module_handles_.size());
        ogf_assert(nb_modules == nb_funcs);
        //Commented-out, generates an assertion fail in dlclose:
        // Inconsistency detected by ld.so: dl-close.c: 764:
        //    _dl_close: Assertion `map->l_init_called' failed!
        // (to be investigated)
        /*
        for(int i = nb_modules - 1; i >= 0; i--) {
            void* module_handle = module_handles_[index_t(i)];
            Logger::out("ModuleMgr") << "dlclosing module " << i << std::endl;
            dlclose(module_handle);
        }
        */
    }

    bool ModuleManager::load_module(
        const std::string& module_name, bool quiet
    ) {
	if(!quiet) {
	    Logger::out("ModuleMgr") << "Loading module: "
				     << module_name << std::endl;
	}
        std::string module_file_name = module_name;

        if(! FileManager::instance()-> find_binary_file(
               module_file_name
        )) {
            if(!quiet) {
                Logger::err("ModuleMgr")  
                    << "module \'" << module_name 
                    << "\' not found" << std::endl;
            }
            return false;
        }

        // Invoke the dynamic linker with the following flags :
        //
        // RTLD_NOW:    I want errors to be notified as soon as possible
        // RTLD_GLOBAL: different modules may be inter-dependant.
        
        void* module_handle = dlopen(
            module_file_name.c_str(), RTLD_NOW | RTLD_GLOBAL 
        );
    
        if(module_handle == nullptr) {
            Logger::err("ModuleMgr")  
                << "Could not open module \'" 
                << module_name << "\'" << std::endl 
                << "                     Error opening file \'"
                << module_file_name << "\'" << std::endl 
                << " reason: " << dlerror() << std::endl;
            return  false;
        }
        
        // Retreive function pointers to initialize and
        //   terminate the module.

        std::string project_name = "OGF";
        std::string init_func_name =
            project_name + "_" + module_name + "_initialize";
        
        std::string term_func_name =
            project_name + "_" + module_name + "_terminate";        
        
        ModuleInitFunc p_init_func =
            (ModuleInitFunc)dlsym_fptr(module_handle, init_func_name.c_str());
        
        ModuleTerminateFunc p_term_func =
            (ModuleTerminateFunc)dlsym_fptr(
		module_handle, term_func_name.c_str()
	    );
        
        // Checks whether the module has init/termination functions.

        if(p_init_func == nullptr || p_term_func == nullptr) {
            Logger::warn("ModuleMgr")
                << "[ModuleManager]: "
                << "Could not find init/term of module \'" 
                << module_name << "\'" << std::endl;
        } else {
            // invoke the initialization function.
            // No need to call p_init_func(), automatically called
            // at DLL loading time by ctor of global objects.
            
            // memorize the module handle
            module_handles_.push_back(module_handle);

            // memorize the termination function.
            to_terminate_.push_back(p_term_func);
        }
        
        Module* module_info = ModuleManager::resolve_module(module_name);
        if(module_info == nullptr) {
            module_info = new Module;
            module_info->set_name(module_name);
            ModuleManager::bind_module(module_name, module_info);
        } 
        module_info->set_is_dynamic(true);
        return true;
    }

    void* ModuleManager::resolve_symbol(const std::string& name) {
	return dlsym(RTLD_DEFAULT, name.c_str());
    }

    void ModuleManager::append_dynamic_libraries_path(const std::string& path) {
	const char* env_ld_lib_path = getenv("LD_LIBRARY_PATH");
	std::string ld_lib_path;
	if(env_ld_lib_path != nullptr) {
	    ld_lib_path = env_ld_lib_path;
	    ld_lib_path = ld_lib_path + ";";
	}
	ld_lib_path += path;
	setenv("LD_LIBRARY_PATH", ld_lib_path.c_str(), 1);
    }    
}

#endif



