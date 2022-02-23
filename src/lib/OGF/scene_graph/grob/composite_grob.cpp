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
 
 
#include <OGF/scene_graph/grob/composite_grob.h>
#include <OGF/scene_graph/types/scene_graph.h>

namespace OGF {

//_________________________________________________________

    CompositeGrob::CompositeGrob(
        CompositeGrob* parent
    ) : Grob(parent) {
    }


    bool CompositeGrob::is_bound(const std::string& name) const {
        return (resolve(name) != nullptr) ;
    }

    Grob* CompositeGrob::ith_child(index_t i) const {
        Grob* result = dynamic_cast<Grob*>(Grob::ith_child(i)) ;
        ogf_assert(result != nullptr) ;
        return result ;
    }
    
    Grob* CompositeGrob::resolve(const std::string& name) const {
        for(index_t i=0; i<get_nb_children(); i++) {
            Grob* cur = ith_child(i) ;
            if(cur != nullptr && cur->get_name() == name) {
                return cur ;
            }
        }
        return nullptr ;
    }

    Box3d CompositeGrob::bbox() const {
        Box3d result ;
        for(index_t i=0; i<get_nb_children(); i++) {
            Grob* cur = ith_child(i) ;
            Box3d cur_box = cur->bbox();
            result.add_box(cur_box);
        }
        if(!result.initialized()) {
            result.add_point(vec3(0,0,0)) ;
            result.add_point(vec3(1,1,1)) ;
        }
        return result ;
    }

    Box3d CompositeGrob::world_bbox() const {
        Box3d result ;
        for(index_t i=0; i<get_nb_children(); i++) {
            Grob* cur = ith_child(i) ;
            Box3d cur_box = cur->world_bbox();
            result.add_box(cur_box);
        }
        if(!result.initialized()) {
            result.add_point(vec3(0,0,0)) ;
            result.add_point(vec3(1,1,1)) ;
        }
        return result ;
    }
    
    void CompositeGrob::add_child(Node* child) {
        Grob::add_child(child) ;
    }

    void CompositeGrob::remove_child(Node* child) {
        Grob::remove_child(child) ;
    }
    
/************************************************************/

    CompositeGrobScope::CompositeGrobScope(
	CompositeGrob* grob
    ) : Scope(grob) {
    }

    CompositeGrobScope::~CompositeGrobScope() {
    }

    Any CompositeGrobScope::resolve(const std::string& name){
	CompositeGrob* cgrob = dynamic_cast<CompositeGrob*>(object_);
	geo_assert(cgrob != nullptr);
	Any result;
	result.set_value(cgrob->resolve(name));
	return result;
    }

    void CompositeGrobScope::list_names(std::vector<std::string>& names) const {
	CompositeGrob* cgrob = dynamic_cast<CompositeGrob*>(object_);
	geo_assert(cgrob != nullptr);
	names.clear();
	for(index_t i=0; i<cgrob->get_nb_children(); ++i) {
	    Grob* grob = cgrob->ith_child(i);
	    if(grob != nullptr) {
		names.push_back(grob->name());
	    }
	}
    }
    

}

