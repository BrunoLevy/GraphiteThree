/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
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
 *  Contact: Bruno Levy - levy@loria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with the following 
 *  (non-GPL) libraries:  Qt, SuperLU, WildMagic and CGAL
 */
 

#include <OGF/mesh/commands/filter.h>

namespace OGF {

    Filter::Filter(
        index_t size, const std::string& description, bool floating_point
    ) {
        size_ = size;
        if(floating_point) {
            parse_values(description);
        } else {
            parse_items(description);            
        }
    }

    void Filter::parse_items(const std::string& description) {
        std::vector<std::string> words;
        String::split_string(description,';',words);
        for(index_t i=0; i<words.size(); ++i) {
            if(words[i].size() == 0) {
                continue;
            }
            
            if(words[i] == "*") {
                include_intervals_.push_back(std::make_pair(0, size_-1));
                continue;
            }
            
            bool exclude = (words[i][0] == '!');
            if(exclude) {
                words[i] = words[i].substr(1);
            }
            size_t pos = words[i].find('-');
            if(pos == std::string::npos) {
                index_t item = String::to_uint(words[i]);
                if(item >= size_) {
                    throw(std::logic_error("index out of bounds"));
                }
                if(exclude) {
                    exclude_items_.push_back(double(item));
                } else {
                    include_items_.push_back(double(item));
                }
            } else {
                std::string from_str = words[i].substr(0,pos);
                std::string to_str   = words[i].substr(pos+1);
                index_t from = String::to_uint(from_str);
                index_t to   = String::to_uint(to_str);
                if(from >= size_ || to >= size_) {
                    throw(std::logic_error("index out of bounds"));
                }
                if(from >= to) {
                    throw(std::logic_error("malformed interval"));
                }
                if(exclude) {
                    exclude_intervals_.push_back(
                        std::make_pair(double(from),double(to))
                    );
                } else {
                    include_intervals_.push_back(
                        std::make_pair(double(from),double(to))
                    );
                }
            }
        }
    }

    void Filter::parse_values(const std::string& description) {
        std::vector<std::string> words;
        String::split_string(description,';',words);
        for(index_t i=0; i<words.size(); ++i) {
            if(words[i].size() == 0) {
                continue;
            }
            
            if(words[i] == "*") {
                include_intervals_.push_back(
                    std::make_pair(
                        -Numeric::max_float64(), Numeric::max_float64()
                    )
                );
                continue;
            }
            
            bool exclude = (words[i][0] == '!');
            if(exclude) {
                words[i] = words[i].substr(1);
            }
            size_t pos = words[i].find('-');
            if(pos == std::string::npos) {
                double value = String::to_double(words[i]);
                if(exclude) {
                    exclude_items_.push_back(value);
                } else {
                    include_items_.push_back(value);
                }
            } else {
                std::string from_str = words[i].substr(0,pos);
                std::string to_str   = words[i].substr(pos+1);
                double from = String::to_double(from_str);
                double to   = String::to_double(to_str);
                if(from >= to) {
                    throw(std::logic_error("malformed interval"));
                }
                if(exclude) {
                    exclude_intervals_.push_back(std::make_pair(from,to));
                } else {
                    include_intervals_.push_back(std::make_pair(from,to));
                }
            }
        }
    }
    
    bool Filter::test(double value) const {
        bool result = false;
        for(double v: include_items_) {
            if(value == v) {
                result = true;
            }
        }
        for(std::pair<double, double> I: include_intervals_) {
            if(value >= I.first && value <= I.second) {
                result = true;
            }
        }
        for(double v: exclude_items_) {
            if(value == v) {
                result = false;
            }
        }
        for(std::pair<double, double> I: exclude_intervals_) {
            if(value >= I.first && value <= I.second) {
                result = false;
            }
        }
        return result;
    }

    bool Filter::test(index_t item) const {
        geo_debug_assert(item < size_);
        return test(double(item));
    }
}

