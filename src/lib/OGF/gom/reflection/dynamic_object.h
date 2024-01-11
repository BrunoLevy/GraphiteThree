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

#ifndef H_OGF_META_TYPES_DYNAMIC_OBJECT_H
#define H_OGF_META_TYPES_DYNAMIC_OBJECT_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/types/callable.h>

/**
 * \file OGF/gom/reflection/dynamic_object.h
 * \brief Dynamic creation of classes and objects in Lua
 */

namespace OGF {

    /**
     * \brief An object with a class that can be created in Lua.
     * \details It is possible to create new GOM classes and objects in Lua.
     */
    gom_class GOM_API DynamicObject : public Object {
    public:
        /**
         * \copydoc Object::set_property()
         */
        bool set_property(
            const std::string& name, const Any& value
        ) override;

        /**
         * \copydoc Object::get_property()
         */
        bool get_property(
            const std::string& prop_name, Any& prop_value
        ) const override;

    private:
        mutable std::map<std::string, Any> properties_;
    };


    gom_class GOM_API DynamicMetaSlot : public MetaSlot {
    public:
        /**
         * \brief DynamicMetaSlot constructor.
         * \param[in] name name of the slot
         * \param[in] container the MetaClass this MetaSlot belongs to
         * \param[in] action a callable object with the action to be
         *  executed each time the slot is called.
         * \param[in] return_type_name the C++ return type name, as a 
         *  string
         */
        DynamicMetaSlot(
            const std::string& name, MetaClass* container,
            Callable* action,
            const std::string& return_type_name = "void"
        );

    gom_slots:
        /**
         * \brief Adds an argument to a DynamicMetaSlot
         * \param[in] name the name of the argument
         * \param[in] type_name a string with the C++ type name of the argument
         */
        void add_arg(const std::string& name, const std::string& type_name);

        
        // TODO: default value
        // TODO: custom attribute for args
        
    protected:
        Callable_var action_;
    };
    
    /**
     * \brief A MetaClass that can be created in Lua.
     * \details It is possible to create new GOM classes and objects in Lua.
     */
    gom_class GOM_API DynamicMetaClass : public MetaClass {
    public:
        /**
         * \brief Constructs a new DynamicMetaClass
         * \param[in] class_name the C++ class name
         * \param[in] super_class_name the C++ name of the super class
         * \param[in] is_abstract indicates whether the class is abstract
         *  (e.g. with pure virtual methods) or not. Abstract classes 
         *  cannot be constructed.
         */
        explicit DynamicMetaClass(
            const std::string& class_name, 
            const std::string& super_class_name,
            bool is_abstract = false
        );
    };

}



#endif


