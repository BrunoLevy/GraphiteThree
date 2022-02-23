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

#include <OGF/basic/os/text_utils.h>
#include <geogram/basic/file_system.h>

#include <fstream>
#include <iostream>
#include <assert.h>

#ifdef GEO_COMPILER_MSVC
#include <string.h>
#else
#include <strings.h>
#endif

namespace OGF {
    namespace TextUtils {

        // this function is independent of the encoding of carriage
        // returns, i.e. \n or c10 or c13 ...
        static bool getline(std::istream& in, std::string& out) {
            if(!std::getline(in,out)) {
                return false;
            }
            // Strip spurious carriage returns encountered in
            // DOS files.
            if(out.length() > 0 && out[out.length()-1] == '\r') {
                out.resize(out.length()-1);
            }
            return true;
        }
        

        bool Environment::has_variable(const std::string& name) const {
            auto it = data_.find(name) ;
            return it != data_.end() ;
        }
    
        std::string Environment::value(const std::string& name) const {
            const std::vector<std::string>& vals = values(name) ;
            std::string result ;
            for(unsigned int i=0; i<vals.size(); i++) {
                if(i != 0) {
                    result += " " ;
                }
                result += vals[i] ;
            }
            return result ;
        }
    
        const std::vector<std::string>& Environment::values(const std::string& name) const {
            auto it = data_.find(name) ;
            ogf_assert(it != data_.end()) ;
            return it->second ;
        }
    
        void Environment::set_value(const std::string& name, const std::string& value) {
            data_[name].clear() ;
            append_value(name,value) ;
        }
    
        void Environment::set_values(
            const std::string& name, const std::vector<std::string>& values
        ) {
            data_[name].clear() ;
            append_values(name,values) ;
        }

        void Environment::append_value(const std::string& name, const std::string& value) {
            if(value[0] == '$') {
                std::string var_name = value.substr(1,value.length() - 1) ;
                if (has_variable(var_name)) {
                    // it's a local var
                    append_values(name,values(var_name)) ;
                } else {
                    // it's a windows var like (QTDIR)
                    data_[name].push_back(value) ;
                }
            } else {
                data_[name].push_back(value) ;
            }
        }
    
        void Environment::append_values(
            const std::string& name, const std::vector<std::string>& values
        ) {
            for(unsigned int i=0; i<values.size(); i++) {
                append_value(name,values[i]) ;
            }
        }
    
        void Environment::clear_value(const std::string& name) {
            ogf_assert(has_variable(name)) ;
            data_[name].clear() ;
        }

        void Environment::clear() {
            data_.clear() ;
        }

        void Environment::print(std::ostream& out) const {
            for(auto& it : data_) {
                out << it.first << "= \" " ;
                for(unsigned int i=0; i<it.second.size(); ++i) {
                    out << it.second[i] << " " ;
                }
                out << "\"" << std::endl ;
            }
        }

    
        //_________________________________________________________________________________________

        static void read_environment_variable(
            const std::string& variable,
            Environment& environment
        ) {
            char buff[1024] ;
            strcpy(buff, variable.c_str()) ;

            char* p_equal = strchr(buff, '=') ;
            ogf_assert(p_equal != nullptr) ;
            *p_equal = '\0' ;

            std::string variable_name(buff) ;

            char* p_value = p_equal + 1 ;
            p_value = strchr(p_value, '"') ;
            ogf_assert(p_value != nullptr) ;
            p_value++ ;

            char* p_value_end = strchr(p_value, '"') ;
            ogf_assert(p_value_end != nullptr) ;
            *p_value_end = '\0' ;

            std::string variable_values_str(p_value) ;
            std::vector<std::string> variable_values ;
            ::OGF::String::split_string(variable_values_str, ' ', variable_values) ;
            environment.set_values(variable_name, variable_values) ;
        }

        void read_environment_file(
            const std::string& file_name,
            Environment& environment
        ) {
            std::ifstream in(file_name.c_str()) ;
            ogf_assert(in) ;
        
            std::string buff;
            std::string line ;
            bool append = false ;

            while(getline(in,buff)) {
                size_t p = buff.find('#');
                if(p != std::string::npos) {
                    buff[p] = '\0';
                }
                if(buff[0] == '\0') {
                    continue ;
                }
                if(buff[buff.length() - 1] == '\\') {
                    append = true ;
                    buff[buff.length() - 1] = '\0' ;
                } else {
                    append = false ;
                }
                line += std::string(buff.c_str()) ;
                if(!append) {
                    read_environment_variable(line, environment) ;
                    line.clear() ;
                }
            }
        }

        void find_and_replace(
            std::istream& in, std::ostream& out,
            const Environment& env
        ) {
            std::string in_string;
            std::string var_name ;
            while(getline(in,in_string)) {
                std::string out_string ;
                bool in_var = false ;
                for(unsigned int i=0; i<in_string.length(); i++) {
                    char cur_char = in_string[i] ;
                    if(in_var) {
                        if(cur_char == '%') {
                            in_var = false ;
                            if(env.has_variable(var_name)) {
                                out_string += env.value(var_name) ;
                            } else {
                                // Maybe a variable for MSDOS, pass through...
                                out_string += ("%" + var_name + "%") ;
                            }
                            var_name.clear() ;
                        } else {
                            var_name += ::OGF::String::char_to_string(cur_char) ;
                        }
                    } else {
                        if(cur_char == '%') {
                            in_var = true ;
                        } else {
                            out_string += ::OGF::String::char_to_string(cur_char) ;
                        }
                    }
                }
                out << out_string << std::endl ;
            }
        }

        void concatenate(
            std::istream& in, std::ostream& out
        ) {
            std::string line;
            while(getline(in,line)) {
                out << line << std::endl ;
            }
        }

        bool file_contains_string(
            const std::string& file_name, const std::string& x
        ) {
            std::ifstream in(file_name.c_str()) ;
            std::string line;
            while(getline(in,line)) {
                if(line.find(x) != std::string::npos) {
                    return true ;
                }
            }
            return false ;
        }
    }
}
