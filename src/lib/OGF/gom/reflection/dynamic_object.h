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

    /******************************************************************/

    class GOM_API DynamicFactoryMetaClass : public FactoryMetaClass {
    public:
        /**
         * \brief FactoryMetaClass constructor
         * \param[in] mclass a pointer to the meta class
         */
        DynamicFactoryMetaClass(MetaClass* mclass, Callable* action = nullptr) :
            FactoryMetaClass(mclass), action_(action) {
        }

        /**
         * \copydoc FactoryMetaClass::create()
         */
        Object* create(const ArgList& args) override;

    private:
        Callable_var action_;
    };
    
    /******************************************************************/

    /**
     * \brief A slot in a dynamically-created class.
     */
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
         * \param[in] type the MetaType of the argument
         * \param[in] default_value an optional default value as a string
         */
        void add_arg(
            const std::string& name, MetaType* type,
            const std::string& default_value = ""
        );


        /**
         * \brief Sets the default value for an arg.
         * \details There should be an existing arg with the specified name
         * \param[in] name the name of the argument
         * \param[in] default_value the default value as a string
         */
        void set_arg_default_value(
            const std::string& name, const std::string& default_value
        );
        
        /**
         * \brief Creates a new custom attribute
         * \param[in] arg_name name of the argument
         * \param[in] name name of the custom attribute
         * \param[in] value value of the custom attribute
         * \pre !ith_arg_has_custom_attribute(name)
         */
        void create_arg_custom_attribute(
            const std::string& arg_name,
            const std::string& name, const std::string& value
        );

    public:
        void pre_delete() override;
        
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

    gom_slots:

        /**
         * \brief Creates a new constructor
         * \param[in] action an optional action to be invoked each time this
         *  constructor is called
         * \return the created MetaConstructor
         * \details one can create the arguments by calling 
         *  MetaConstructor::add_arg() on the returned MetaConstructor
         */
        MetaConstructor* add_constructor(Callable* action=nullptr);
        
        /**
         * \brief Creates a new slot
         * \param[in] name the name of the slot
         * \param[in] action the function to be called when the slot is invoked
         * \param[in] return_type an optional string with the return type 
         * \return the created DynamicMetaSlot
         * \details one can create the arguments by calling 
         *  DynamicMetaSlot::add_arg() on the returned DynamicMetaSlot.
         */
        DynamicMetaSlot* add_slot(
            const std::string& name, Callable* action,
            const std::string& return_type="void"
        );
    };

}

#endif


