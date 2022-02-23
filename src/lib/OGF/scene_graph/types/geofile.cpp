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

#include <OGF/scene_graph/types/geofile.h>

namespace OGF {
    
    /*************************************************************/
    
    InputGraphiteFile::InputGraphiteFile(
        const std::string& filename
    ) : InputGeoFile(filename) {
    }

    void InputGraphiteFile::read_scene_graph_header(ArgList& args) {
        geo_assert(current_chunk_class() == "SCNG");
        read_arg_list(args);
        check_chunk_size();
    }
    
    void InputGraphiteFile::read_grob_header(ArgList& args) {
        geo_assert(current_chunk_class() == "GROB");
        read_arg_list(args);
        check_chunk_size();
    }

    void InputGraphiteFile::read_shader(ArgList& args) {
        geo_assert(current_chunk_class() == "SHDR");
        read_arg_list(args);
        check_chunk_size();
    }

    void InputGraphiteFile::read_history(std::vector<std::string>& history) {
        geo_assert(current_chunk_class() == "HIST");
        read_string_array(history);
        check_chunk_size();
    }

    
    void InputGraphiteFile::read_arg_list(ArgList& args) {
        args.clear();
        index_t nb_args = read_int();
        for(index_t i=0; i<nb_args; ++i) {
            std::string arg_name = read_string();
            std::string arg_value = read_string();
            args.create_arg(arg_name, arg_value);
        }
    }
    
    /*************************************************************/

    OutputGraphiteFile::OutputGraphiteFile(
        const std::string& filename, index_t compression_level
    ) : OutputGeoFile(filename, compression_level) {
    }


    void OutputGraphiteFile::write_history(
        const std::vector<std::string>& history
    ) {
        write_chunk_header("HIST", string_array_size(history));
        write_string_array(history);
        check_chunk_size();
    }

    void OutputGraphiteFile::write_scene_graph_header(const ArgList& args) {
        write_chunk_header("SCNG", arg_list_size(args));
        write_arg_list(args);
        check_chunk_size();
    }
    
    void OutputGraphiteFile::write_grob_header(const ArgList& args) {
        write_chunk_header("GROB", arg_list_size(args));
        write_arg_list(args);
        check_chunk_size();
    }

    void OutputGraphiteFile::write_shader(const ArgList& args) {
        write_chunk_header("SHDR", arg_list_size(args));
        write_arg_list(args);
        check_chunk_size();
    }
    
    void OutputGraphiteFile::write_arg_list(const ArgList& args) {
        write_int(args.nb_args(), "the number of name-value pairs");
        for(index_t i=0; i<args.nb_args(); ++i) {
            write_string(args.ith_arg_name(i));
            write_string(args.ith_arg_value(i).as_string());
        }
    }
    
    size_t OutputGraphiteFile::arg_list_size(const ArgList& args) const {
        size_t result = sizeof(index_t);
        for(index_t i=0; i<args.nb_args(); ++i) {
            result += string_size(args.ith_arg_name(i));
            result += string_size(args.ith_arg_value(i).as_string());
        }
        return result;
    }

    
    
    /*************************************************************/    
}
