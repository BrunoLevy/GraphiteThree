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

#include <OGF/gom/types/object.h>
#include <OGF/gom/types/connection.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_type.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta_method.h>
#include <sstream>


namespace OGF {

//___________________________________________________________________________

    class GOM_API ConnectionTable :
        public std::map<std::string, ConnectionList > {
    public:
    };

    std::map<index_t, Object*>* Object::id_to_object_ = nullptr;

    Object::Object(bool transient) {
        slots_enabled_ = true;
        signals_enabled_ = true;
        meta_class_ = nullptr;
	if(transient) {
	    id_ = 0;
	    connections_ = nullptr;
	} else {
	    static unsigned int cur_id = 1;
	    connections_ = new ConnectionTable;
	    id_ = cur_id;
	    cur_id++;
	    if(id_to_object_ == nullptr) {
		id_to_object_ = new std::map<index_t, Object*>;
	    }
	    (*id_to_object_)[id_] = this;
	}
	
        //   Note: meta_class_ cannot be initialized in constructor
        // since in constructor we got typeid(*this) == Object ...
    }
    
    Object::~Object() {
        delete connections_; 
        connections_ = nullptr;
	if(id_ != 0) {
	    auto it = id_to_object_->find(id_);
	    geo_assert(it != id_to_object_->end());
	    id_to_object_->erase(it);
	    if(id_to_object_->size() == 0) {
		delete id_to_object_;
		id_to_object_ = nullptr;
	    }
	}
    }
    
    void Object::disconnect() {
	delete connections_;
	connections_ = nullptr;
    }
    
    std::string Object::string_id() const {
        std::ostringstream out;
        out << "@" << meta_class()->name() << "::#" << id_;
        return out.str();
    }

    MetaClass* Object::meta_class() const {
        // meta_class_ is mutable (cached for efficiency)
        // It's a bit ugly, I'd prefer to initialize meta
        // information in constructor (but I can't since
        // dynamic type is not available at construction
        // time).
        Object* non_const_this = const_cast<Object*>(this);
        if(meta_class_ == nullptr) {
            MetaType* m_type = ogf_dynamic_type(*this);
            if(m_type == nullptr) {
                std::cerr << "Missing meta information for type:"
                          << typeid(*this).name() 
                          << std::endl;
            }
            ogf_assert(m_type != nullptr);
            non_const_this->meta_class_ = dynamic_cast<MetaClass*>(m_type);
            ogf_assert(meta_class_ != nullptr);
        }
        return meta_class_;
    }

    void Object::set_meta_class(MetaClass* mclass) {
        meta_class_ = mclass;
    }

    bool Object::is_a(const MetaType* type) const {
        return ogf_is_a(*this, type);
    }

    bool Object::has_method(const std::string& method_name) const {
	return meta_class()->find_method(method_name) != nullptr;	
    }
    
    bool Object::invoke_method(
        const std::string& method_name,
        const ArgList& args, Any& ret_val
    ) {

        if( !slots_enabled_ && 
            method_name != "enable_slots" &&
            method_name != "slots_enabled"
        ) {
            return true;
        }

        // Both slots and signals are methods.
        MetaMethod* method = meta_class()->find_method(method_name);
        if(method != nullptr) {
            return method->invoke(this, args, ret_val);
        }

        // If no method is found, try property (setter)
        MetaProperty* property = meta_class()->find_property(method_name);
        if(property != nullptr) {
            if(property->read_only()) {
            Logger::err("Object") 
                << "Property "
                << meta_class()->name() << "::" << method_name 
                << " is read-only" << std::endl;
            } else {
                return property->meta_method_set()->invoke(
                    this, args, ret_val
                );
            }
        }
        Logger::err("Object") 
            << "No such method: "
            << meta_class()->name() << "::" << method_name << std::endl;
        return false;
    }

    bool Object::set_property(
        const std::string& prop_name, const Any& prop_value
    ) {
        if( !slots_enabled_ && prop_name != "slots_enabled" ) {
            return true;
        }
        MetaProperty* prop = meta_class()->find_property(prop_name);
        if(prop != nullptr) {
            return prop->set_value(this, prop_value);
        }
        Logger::err("Object") 
            << "No such property: "
            << meta_class()->name() << "::" << prop_name << std::endl;
        return false;
    }

    bool Object::has_property(const std::string& prop_name) const {
	return (meta_class()->find_property(prop_name) != nullptr);
    }
    
    bool Object::get_property(
        const std::string& prop_name, Any& prop_value
    ) const {
        MetaProperty* prop = meta_class()->find_property(prop_name);
        if(prop != nullptr) {
            bool result = prop->get_value(this, prop_value);
	    return result;
        }
        Logger::err("Object") 
            << "No such property: "
            << meta_class()->name() << "::" << prop_name << std::endl;
        return false;
    }

    
    bool Object::set_property(
        const std::string& prop_name, const std::string& prop_value
    ) {
	Any prop_value_any;
	prop_value_any.set_value(prop_value);
	return set_property(prop_name, prop_value_any);
    }

    bool Object::get_property(
        const std::string& prop_name, std::string& prop_value
    ) const {
	Any prop_value_any;
	bool ok = get_property(prop_name, prop_value_any);
	ok = ok & prop_value_any.get_value(prop_value);
	return ok;
    }

    void Object::set_properties(const ArgList& args) {
        for(unsigned int i=0; i<args.nb_args(); i++) {
            if(meta_class()->find_property(args.ith_arg_name(i))) {
                set_property(
                    args.ith_arg_name(i), args.ith_arg_value(i)
                );
            }
        }
    } 

    Connection* Object::connect_signal_to_slot(
        const std::string& signal_name, 
        Object* to, const std::string& slot_name
    ) {
        return new SlotConnection(this, signal_name, to, slot_name);
    }

    void Object::add_connection(Connection* connection) {
        auto it = connections_->find(connection->signal_name());
        if(it == connections_->end()) {
            (*connections_)[connection->signal_name()] = ConnectionList();
            it = connections_->find(
                connection->signal_name()
            );
        }
        it->second.push_back(connection);

        //   TODO: check that we have all the arguments required by
        // the slot (and that their types match).
    }

    bool Object::emit_signal(
        const std::string& signal_name, const ArgList& args,
        bool called_from_slot
    ) {

        ogf_argused(called_from_slot);
        
        if(!signals_enabled_) {
            return true;
        }

        bool result = true;
        
        auto it = connections_->find(signal_name);
        if(it == connections_->end()) {
            return true;
        }
        ConnectionList& connections = it->second;
	connections.invoke(args);
        return result;
    }

    index_t Object::get_nb_elements() const {
	return 0;
    }
    
    void Object::get_element(index_t i, Any& value) const {
	geo_argused(i);
	geo_argused(value);
	Logger::err("GOM") << meta_class()->name() << "::get_element()"
			   << " not implemented"
			   << std::endl;
    }

    void Object::set_element(index_t i, const Any& value) {
	geo_argused(i);
	geo_argused(value);
	Logger::err("GOM") << meta_class()->name() << "::set_element()"
			   << " not implemented"
			   << std::endl;
    }
    
/******************************************************************/
    
}
