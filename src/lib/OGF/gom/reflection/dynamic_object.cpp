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

#include <OGF/gom/reflection/dynamic_object.h>
#include <OGF/gom/reflection/meta.h>

namespace OGF {

    /*************************************************************************/
    
    bool DynamicObject::set_property(
        const std::string& name, const Any& value
    ) {
        // We first need to test if property is one of
        // Object properties. Then we let Object do the
        // job and access the property in Object fields.
        if(
            ogf_meta<OGF::Object>::meta_class()->find_property(name) != nullptr
        ) {
            return Object::set_property(name, value);
        }
        
        if(!has_property(name)) {
            Logger::err("GOM")
                << meta_class()->name() << "::" << name << " : no such property"
                << std::endl;
            return false;
        }
        properties_[name] = value;
        return true;
    }

    bool DynamicObject::get_property(
        const std::string& name, Any& value
    ) const {

        // We first need to test if property is one of
        // Object properties. Then we let Object do the
        // job and access the property in Object fields.
        if(
            ogf_meta<OGF::Object>::meta_class()->find_property(name) != nullptr
        ) {
            return Object::get_property(name, value);
        }
        
        if(!has_property(name)) {
            Logger::err("GOM")
                << meta_class()->name() << "::" << name << " : no such property"
                << std::endl;
            return false;
        }
        value = properties_[name];
        return true;
    }

    /*************************************************************************/

    Object* DynamicFactoryMetaClass::create(const ArgList& args) {
        Object* result = new DynamicObject;
        result->set_meta_class(meta_class());

        if(action_.is_null()) {
            // Default constructor initializes object's properties from args
            for(index_t i=0; i<args.nb_args(); ++i) {
                MetaProperty* mprop = meta_class()->find_property(
                    args.ith_arg_name(i)
                );
                if(mprop != nullptr && !mprop->read_only()) {
                    result->set_property(
                        args.ith_arg_name(i), args.ith_arg_value(i)
                    );
                }
            }
        } else {
            // If an action was specified, initialize 'self' and call it
            ArgList args2 = args;
            args2.create_arg("self", result);
            action_->invoke(args2);
        }
        return result;
    }
    
    /*************************************************************************/
    
    DynamicMetaSlot::DynamicMetaSlot(
        const std::string& name, MetaClass* container,
        Callable* action,
        const std::string& return_type_name
    ) : MetaSlot(name, container, return_type_name), action_(action) {
        set_method_adapter(
            [](
                Object* target, 
                const std::string& method_name, const ArgList& args,
                Any& ret_val
            )->bool {
                DynamicMetaSlot* mmethod = dynamic_cast<DynamicMetaSlot*>(
                    target->meta_class()->find_slot(method_name)
                );
                if(mmethod == nullptr) {
                    Logger::err("GOM") << method_name << ": no such method"
                                       << std::endl;
                    return false;
                }
                ArgList args2 = args;
                args2.create_arg("self",target);
                args2.create_arg("method",method_name);
                return mmethod->action_->invoke(args2, ret_val);
            }
        );
        // I've got a seg fault when deallocating the Callable
        // Quick and dirty fix: artificially increase ref count
        // so that it is never deleted (small memory leak here)
        action_->ref();
    }

    void DynamicMetaSlot::add_arg(
        const std::string& name, MetaType* type,
        const std::string& default_value
    ) {
        MetaArg marg(name, type);
        if(default_value != "") {
            marg.default_value().set_value(default_value);
        }
        MetaSlot::add_arg(marg);
    }

    void DynamicMetaSlot::create_arg_custom_attribute(
        const std::string& arg_name,
        const std::string& name, const std::string& value
    ) {
        MetaArg* marg = find_arg(arg_name);
        if(marg == nullptr) {
            Logger::err("GOM")
                << meta_class()->name() << "::" << this->name() << "() :"
                << arg_name << ":no such argument"
                << std::endl;
            return;
        }
        marg->create_custom_attribute(name,value);
    }
    
    /**********************************************************************/
    
    DynamicMetaClass::DynamicMetaClass(
        const std::string& class_name, 
        const std::string& super_class_name,
        bool is_abstract
    ) : MetaClass(class_name, super_class_name, is_abstract) {
    }

    MetaConstructor* DynamicMetaClass::add_constructor(Callable* action) {
        MetaConstructor* result = new MetaConstructor(this);
        result->set_factory(new DynamicFactoryMetaClass(this, action));
        return result;
    }
    
    DynamicMetaSlot* DynamicMetaClass::add_slot(
        const std::string& name, Callable* action,
        const std::string& return_type
    ) {
        return new DynamicMetaSlot(name, this, action, return_type);
    }
    
}
