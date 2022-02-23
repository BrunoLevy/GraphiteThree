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

#include <OGF/gom/reflection/meta.h>

//___________________________________________________

namespace OGF {

    Meta* Meta::instance_ = nullptr ;

    Meta::Meta() {
    }

    Meta::~Meta() {
	// First delete all the ArgLists stored in the Meta information,
	// because deleting Any requires to access the LifeCycle objects
	// that are stored in the Meta information (do not saw the branch...)
	for(auto& it : type_name_to_meta_type_) {
	    it.second->pre_delete();
	}
	// Now we are good to go !
        type_name_to_meta_type_.clear() ;
        typeid_name_to_meta_type_.clear() ;
    }

    void Meta::initialize() {
        ogf_assert(instance_ == nullptr) ;
        instance_ = new Meta ;
    }
    
    void Meta::terminate() {
        delete instance_ ;
        instance_ = nullptr ;
    }


    Meta* Meta::instance() {
        ogf_assert(instance_ != nullptr) ;
        return instance_ ;
    }

    bool Meta::meta_type_is_bound(const std::string& name) const {
        return ( 
            type_name_to_meta_type_.find(name) != 
            type_name_to_meta_type_.end()
        ) ;
    }

    bool Meta::bind_meta_type(MetaType* meta_type) {
        if(meta_type_is_bound(meta_type->name())) {
            return false ;
        }
        type_name_to_meta_type_[meta_type->name()] = meta_type ;
        return true ;
    }

    bool Meta::bind_meta_type(
        MetaType* meta_type, const std::string& typeid_name
    ) {
        // TODO: It would be better to have separate functions
        // to bind aliases and existing meta types. See usage
        // in gom_implementation.h, that creates SmartPointers
        // that are subsequently deallocated if the type is
        // already bound (not very clean)...
        
        //  If meta type is already bound with same "user name",
        // then there is nothing to do (same meta type can be
        // registered several times in GOMGEN generated code).
        if(meta_type_is_bound(meta_type->name())) {
            return false;
        }
        //  If meta type is already bound *by typeid name*, then
        // we are declaring an alias (and we bind the already
        // registered meta type to the "user name").
        if(typeid_name_is_bound(typeid_name)) {
            type_name_to_meta_type_[meta_type->name()] =
                typeid_name_to_meta_type_[typeid_name];
        } else {
            type_name_to_meta_type_[meta_type->name()]   = meta_type ;
            typeid_name_to_meta_type_[typeid_name]       = meta_type ;
            meta_type->set_typeid_name(typeid_name) ;
        }
        return true ;
    }
    
    bool Meta::unbind_meta_type(const std::string& name) {
        MetaType* type = nullptr ;            
        {
            auto it = type_name_to_meta_type_.find(name) ;
            if(it == type_name_to_meta_type_.end()) {
                return false ;
            }
            type = it->second ;
            type_name_to_meta_type_.erase(it) ;
        }

        {
            for(
                auto it = typeid_name_to_meta_type_.begin(); 
                it != typeid_name_to_meta_type_.end(); ++it
            ) {
                if(it->second == type) {
                    typeid_name_to_meta_type_.erase(it) ;
                    break ;
                }
            }
        }
        return true ;
    }

    MetaType* Meta::resolve_meta_type(const std::string& type_name) const {
        auto it = type_name_to_meta_type_.find(type_name) ;
        if(it == type_name_to_meta_type_.end()) {
            return nullptr ;
        }
        return it->second ;
    }

    bool Meta::typeid_name_is_bound(const std::string& typeid_name) const {
        return ( 
            typeid_name_to_meta_type_.find(typeid_name) != 
            typeid_name_to_meta_type_.end()
        ) ;
    }

    MetaType* Meta::resolve_meta_type_by_typeid_name(
        const std::string& typeid_name
    ) const {
        auto it = typeid_name_to_meta_type_.find(typeid_name) ;
        if(it == typeid_name_to_meta_type_.end()) {
            return nullptr ;
        }
        return it->second ;
    }

    void Meta::list_types(std::vector<MetaType*>& types) {
        types.clear() ;
        for(auto& it : type_name_to_meta_type_) {
            types.push_back(it.second) ;
        }
    }
}



