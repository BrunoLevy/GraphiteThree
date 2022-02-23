/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 Bruno Levy
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


#include <OGF/basic/common/common.h>

// Swig includes need to be AFTER OGF includes
// else it causes a problem under Windows.
#include "doxyparse.h"

#include <sstream>
#include <iostream>
#include <vector>


static bool strip_keyword(std::string& line, const std::string& kw) {
    size_t p = line.find(kw);
    if(p != std::string::npos) {
        size_t l = kw.length();
        line = line.substr(p+l, line.length()-p-l);
        return true;
    }
    return false;
}

static void strip_leading_spaces(std::string& line) {
    size_t i=0;
    for(i=0; i<line.length(); ++i) {
        if(line[i] != ' ') {
            break;
        }
    }
    if(line[i] != 0) {
        line = line.substr(i, line.length()-i);
    }
}

static void strip_all_spaces(std::string& line) {
    size_t i=0;
    for(i=0; i<line.length(); ++i) {
        if(line[i] != ' ') {
            break;
        }
    }
    size_t j=line.length();
    for(j=line.length(); j>0; --j) {
        if(line[j-1] != ' ') {
            break;
        }
    }
    if(j>i) {
        line = line.substr(i, j-i);
    } else {
        line = "";
    }
}
    
static std::string strip_first_word(std::string& line) {
    strip_leading_spaces(line);
    size_t i=0;
    for(i=0; i<line.length(); ++i) {
        if(line[i] == ' ') {
            break;
        }
    }
    std::string result = line.substr(0, i);
    for(; i<line.length(); ++i) {
        if(line[i] != ' ') {
            break;
        }
    }
    line = line.substr(i, line.length()-i);
    return result;
}

typedef std::map<std::string, std::string> DoxyArgs;

static void doxyargs_to_gom(
    const DoxyArgs& doxyargs, String* preprocessor_output
) {
    if(doxyargs.size() != 0) {
        Printf(preprocessor_output,"\n");
    }
    for(
        DoxyArgs::const_iterator it = doxyargs.begin();
        it != doxyargs.end(); ++it
    ) {
        const std::string& argname = it->first;

        // For now, replace single and double quotes with spaces
        // (TODO: replace them with escape sequences)
        std::string argval = it->second;
        for(size_t i=0; i<argval.size(); ++i) {
            if(argval[i] == '\"' || argval[i] == '\'') {
                argval[i] = ' ';
            }
        }

        // argname may contain gomarg$x1,x2,x3$help
        // if a multiarg \param command was encountered.
        // For now such args are skipped...
        // (TODO: expand them)
        if(argname.find(',') != std::string::npos) {
            continue;
        }
        
        // Cannot put '%' in Printf, and cannot escape it,
        // so I 'Putc' it before 'Printingf' the rest...
        Putc('%',preprocessor_output);        
        Printf(
            preprocessor_output,
            "pragma(gomattribute) %s=\"%s\"\n",
            argname.c_str(),argval.c_str()
        );
    }
}

void doxyparse(String* text_in, String* preprocessor_output) {
    
    std::string text(Char(text_in));
    
    if(text.find("\\file") != std::string::npos) {
        return;
    }

    std::string line;
    std::istringstream in(text);
    std::string current_arg;
    DoxyArgs args;
    bool advanced_arg = false;
    while((std::getline(in,line))) {
        if(line.find("/**") == std::string::npos &&
           line.find("*/") == std::string::npos
        ) {
            if(strip_keyword(line,"\\")) {
                if(strip_keyword(line,"brief")) {
                    current_arg = "help"; 
                } else if(strip_keyword(line, "menu")) {
                    current_arg = "menu";
                } else if(strip_keyword(line, "advanced")) {
                    advanced_arg = true;
                    continue;
                } else if(strip_keyword(line, "param")) {
                    strip_keyword(line, "]"); // Works for [in], [out], [in,out]
                    std::string arg_name= strip_first_word(line);
                    current_arg = "gomarg$" +  arg_name + "$help";
                    if(advanced_arg) {
                        args["gomarg$" + arg_name + "$advanced"] = "true";
                    }
                } else {
                    current_arg = "";
                }
            }
            if(current_arg !="") {
                strip_keyword(line, "*");
                strip_all_spaces(line);
                DoxyArgs::iterator it = args.find(current_arg);
                if(it == args.end()) {
                    args[current_arg] += line;
                } else {
                    it->second += " ";
                    it->second += line;
                }
            }
        }
    }
    doxyargs_to_gom(args, preprocessor_output);
}
