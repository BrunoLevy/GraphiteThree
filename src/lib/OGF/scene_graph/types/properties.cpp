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

#include <OGF/scene_graph/types/properties.h>
#include <OGF/gom/reflection/meta.h>

namespace OGF {

    //________________________________________________________________

    std::ostream& operator<<(std::ostream& out, const PointStyle& ps) {
        if(ps.visible) {
            out << "true";
        } else {
            out << "false";
        }
        out << ";";
        out << ps.color;
        out << ";";
        out << ps.size;
        return out;
    }

    std::istream& operator>>(std::istream& in, PointStyle& ps) {
        std::string line;
        std::getline(in,line);
        std::vector<std::string> words;
        String::split_string(line, ';', words);
        if(words.size() != 3) {
            return in;
        }
        ogf_convert_from_string(words[0], ps.visible);
        ogf_convert_from_string(words[1], ps.color);
        ogf_convert_from_string(words[2], ps.size);
        return in;
    }
    

    //________________________________________________________________

    std::ostream& operator<<(std::ostream& out, const EdgeStyle& es) {
        if(es.visible) {
            out << "true";
        } else {
            out << "false";
        }
        out << ";";
        out << es.color;
        out << ";";
        out << es.width;
        return out;
    }

    std::istream& operator>>(std::istream& in, EdgeStyle& es) {
        std::string line;
        std::getline(in,line);
        std::vector<std::string> words;
        String::split_string(line, ';', words);
        if(words.size() != 3) {
            return in;
        }
        ogf_convert_from_string(words[0], es.visible);
        ogf_convert_from_string(words[1], es.color);
        ogf_convert_from_string(words[2], es.width);
        return in;
    }
    

    //________________________________________________________________

    std::ostream& operator<<(std::ostream& out, const SurfaceStyle& ss) {
        if(ss.visible) {
            out << "true";
        } else {
            out << "false";
        }
        out << ";";
        out << ss.color;
        return out;
    }

    std::istream& operator>>(std::istream& in, SurfaceStyle& es) {
        std::string line;
        std::getline(in,line);
        std::vector<std::string> words;
        String::split_string(line, ';', words);
        if(words.size() != 2) {
            return in;
        }
        ogf_convert_from_string(words[0], es.visible);
        ogf_convert_from_string(words[1], es.color);
        return in;
    }
    
    //________________________________________________________________

    std::ostream& operator<<(
        std::ostream& out, const ColormapStyle& cms
    ) {
        out << cms.colormap_name << ";";
        out << (cms.smooth ? "true" : "false") << ";" ;
        out << cms.repeat << ";";
        out << (cms.show ? "true" : "false") << ";" ;
	out << (cms.flip ? "true" : "false") << ";" ;
        return out;
    }

    std::istream& operator>>(
        std::istream& in, ColormapStyle& cms
    ) {
        std::string line;
        std::getline(in,line);
        std::vector<std::string> words;
        String::split_string(line, ';', words);
	// words.size() == 4: backward compatibility
	// (when ColormapStyle did not have "flip").
        if(words.size() != 4 && words.size() != 5) {
            return in;
        }
        cms.colormap_name = words[0];
        ogf_convert_from_string(words[1], cms.smooth);
        ogf_convert_from_string(words[2], cms.repeat);
        ogf_convert_from_string(words[3], cms.show);
	if(words.size() == 5) {
	    ogf_convert_from_string(words[4], cms.flip);	    
	} else {
	    cms.flip = false;
	}
        return in;
    }
    
    //________________________________________________________________    
}
