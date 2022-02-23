/*
 *  GXML/Graphite: Geometry and Graphics Programming Library + Utilities
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

#include <OGF/skin/types/preferences.h>
#include <OGF/basic/os/file_manager.h>

#include <geogram/basic/file_system.h>
#include <geogram/basic/environment.h>
#include <geogram/basic/command_line.h>

#include <vector>
#include <fstream>
 
namespace OGF {
    namespace Preferences {

        void save_preferences() {
            if(!Environment::instance()->has_value("preferences_variables")) {
                Logger::warn("Preferences") 
                    << "No specified preference_variables" 
                    << std::endl;
                return;
            }
	    
            std::string pref_vars_str =
                Environment::instance()->get_value("preferences_variables");
            Logger::out("Preference")
                << "saving variables: " << pref_vars_str << std::endl;
            std::vector<std::string> pref_vars;
            String::split_string(pref_vars_str, ';', pref_vars);

	    std::string file_name =
		FileSystem::home_directory() + "/" +
		CmdLine::get_config_file_name();
	    std::ofstream out(file_name.c_str());
	    if(!out) {
		Logger::err("Preferences") << "Could not save to:"
					   << file_name << std::endl;
	    } else {
		for(unsigned int i=0; i<pref_vars.size(); i++) {
		    std::string name = pref_vars[i];
		    std::string value;
		    if(!Environment::instance()->has_value(name)) {
			Logger::warn("Preferences") 
			    << "Undefined variable \'" << name << "\'"
			    << std::endl;
		    } else {
			value = Environment::instance()->get_value(name);
			out << name << "=" << value << std::endl;
		    }
		}
	    }
        }

	bool preference_variable_is_declared(
	    const std::string& name
	) {
	    std::string var_name = "preferences_variables";
	    if(!Environment::instance()->has_value(var_name)) {
		return false;
	    }
	    std::string value = Environment::instance()->get_value(var_name);
	    std::vector<std::string> values;
	    String::split_string(value, ';', values);
	    for(size_t i=0; i<values.size(); ++i) {
		if(values[i] == name) {
		    return true;
		}
	    }
	    return false;
	}

	void declare_preference_variable(const std::string& name) {
	    geo_assert(CmdLine::arg_is_declared(name));
	    if(preference_variable_is_declared(name)) {
		return;
	    }
	    std::string var_name = "preferences_variables";
	    std::string values;
	    if(Environment::instance()->has_value(var_name)) {
		values = Environment::instance()->get_value(var_name) + ";";
	    }
	    values += name;
	    Environment::instance()->set_value(var_name, values);
	}
	
        void declare_preference_variable(
	    const std::string& name, const char* value,
	    const std::string& help
	) {
	    CmdLine::declare_arg(name, value, help);
	    declare_preference_variable(name);
        }

        void declare_preference_variable(
	    const std::string& name, int value, const std::string& help
	) {
	    CmdLine::declare_arg(name, value, help);
	    declare_preference_variable(name);
        }

        void declare_preference_variable(
	    const std::string& name, bool value, const std::string& help
	) {
	    CmdLine::declare_arg(name, value, help);
	    declare_preference_variable(name);
        }
	
    }
}
