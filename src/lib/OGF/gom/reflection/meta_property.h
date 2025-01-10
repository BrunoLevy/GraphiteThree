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

#ifndef H_OGF_META_MEMBERS_META_PROPERTY_H
#define H_OGF_META_MEMBERS_META_PROPERTY_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_member.h>
#include <OGF/gom/reflection/meta_method.h>

/**
 * \file OGF/gom/reflection/meta_property.h
 * \brief representation of properties in the Meta system
 */

namespace OGF {

    class Object ;
    class MetaProperty ;

    /**
     * \brief A MetaMethod that corresponds to the setter of
     *  a property.
     */
    gom_class GOM_API MetaMethodSetProperty : public MetaMethod {
    public:

        /**
         * \brief MetaMethodSetProperty constructor.
         * \param[in] name name of the property
         * \param[in] property the MetaProperty this setter corresponds to
         */
        MetaMethodSetProperty(
            const std::string& name, MetaProperty* property
        ) ;

        /**
         * \brief MetaMethodSetProperty destructor.
         */
        ~MetaMethodSetProperty() override ;

        /**
         * \brief Invokes the setter on a target object.
         * \param[in] target the object the setter should be invoked with
         * \param[in] args a const reference to the arguments list
         * \param[out] return_value the return value, as an Any
         */
        bool invoke(
            Object* target, const ArgList& args, Any& return_value
        ) override ;

        /**
         * \brief Gets the MetaProperty
         * \return the MetaProperty this setter is associated with
         */
        MetaProperty* property() const {
            return property_ ;
        }

    private:
        MetaProperty* property_ ;
    } ;

    /**
     * \brief A MetaMethod that corresponds to the getter of
     *  a property.
     */
    gom_class GOM_API MetaMethodGetProperty : public MetaMethod {
    public:

        /**
         * \brief MetaMethodGetProperty constructor.
         * \param[in] name name of the property
         * \param[in] property the MetaProperty this getter corresponds to
         */
        MetaMethodGetProperty(
            const std::string& name, MetaProperty* property
        ) ;

        /**
         * \brief MetaMethodGetProperty destructor.
         */
        ~MetaMethodGetProperty() override ;

        /**
         * \brief Invokes the getter on a target object.
         * \param[in] target the object the getter should be invoked with
         * \param[in] args a const reference to the arguments list
         * \param[out] return_value the return value, as an Any
         */
        bool invoke(
            Object* target, const ArgList& args, Any& return_value
        ) override ;

        /**
         * \brief Gets the MetaProperty
         * \return the MetaProperty this getter is associated with
         */
        MetaProperty* property() const {
            return property_ ;
        }

    private:
        MetaProperty* property_ ;
    } ;

/****************************************************************/

    /**
     * \brief The representation of a property in the Meta repository.
     */
    gom_class GOM_API MetaProperty : public MetaMember {
    public:

        /**
         * \brief MetaProperty constructor
         * \param[in] name name of the property
         * \param[in] container the MetaClass this MetaProperty belongs to
         * \param[in] type_name the C++ type name of this property, as a string
         * \param[in] read_only true if the method has a getter but no setter
         */
        MetaProperty(
            const std::string& name, MetaClass* container,
            const std::string& type_name,
            bool read_only = false
        ) ;

        /**
         * \brief MetaProperty constructor
         * \param[in] name name of the property
         * \param[in] container the MetaClass this MetaProperty belongs to
         * \param[in] meta_type a pointer to the MetaType of the property
         * \param[in] read_only true if the method has a getter but no setter
         */
        MetaProperty(
            const std::string& name, MetaClass* container,
            MetaType* meta_type,
            bool read_only = false
        ) ;

        /**
         * \brief MetaProperty destructor.
         */
        ~MetaProperty() override ;

      gom_slots:

        /**
         * \brief Tests whether this MetaProperty is read only.
         * \retval true if this MetaProperty is read only
         * \retval false otherwise
         */
        bool read_only() const  {
            return read_only_ ;
        }

        /**
         * \brief Gets the type name.
         * \return the C++ type name of this MetaProperty, as a string.
         */
        const std::string& type_name() const {
            return type_name_ ;
        }

        /**
         * \brief Gets the type.
         * \return a pointer to the MetaType of this property.
         */
        MetaType* type() const ;

        /**
         * \brief Sets the value of this property in a target object.
         * \param[in] target pointer to the object
         * \param[in] value new value of the property, as a string
         */
        virtual bool set_value(Object* target, const std::string& value) ;

        /**
         * \brief Gets the value of this property from a target object.
         * \param[in] target pointer to the object
         * \param[out] value value of the property, as a string
         */
        virtual bool get_value(const Object* target, std::string& value) ;

      public:

        /**
         * \brief Sets the value of this property in a target object.
         * \param[in] target pointer to the object
         * \param[in] value new value of the property, as an Any
         */
        virtual bool set_value(Object* target, const Any& value) ;

        /**
         * \brief Gets the value of this property from a target object.
         * \param[in] target pointer to the object
         * \param[out] value value of the property, as an Any
         */
        virtual bool get_value(const Object* target, Any& value) ;


        /**
         * \brief Specify whether this MetaProperty is read only.
         * \param[in] x true if the property is set to read only, false
         *  otherwise
         */
        void set_read_only(bool x) ;


        /**
         * \brief Gets the getter.
         * \return a pointer to the getter, as a MetaMethod.
         */
        MetaMethod* meta_method_get() {
            return meta_method_get_ ;
        }

        /**
         * \brief Gets the setter.
         * \return a pointer to the setter, as a MetaMethod.
         */
        MetaMethod* meta_method_set() {
            return meta_method_set_ ;
        }

        /**
         * \brief Sets the getter.
         * \param[in] getter a pointer to the getter, as a MetaMethod. Ownership
         *  is transfered to this MetaProperty.
         */
        void set_meta_method_get(MetaMethod* getter) {
            meta_method_get_ = getter ;
        }

        /**
         * \brief Sets the setter.
         * \param[in] setter a pointer to the setter, as a MetaMethod. Ownership
         *  is transfered to this MetaProperty.
         */
        void set_meta_method_set(MetaMethod* setter) {
            meta_method_set_ = setter ;
        }

    protected:
        /**
         * \brief Sets the type name of this property.
         * \param[in] type_name_in the C++ type name of this property,
         *  as a string
         */
        void set_type_name(const std::string& type_name_in) {
            type_name_ = type_name_in ;
        }

    private:
        std::string type_name_ ;
        bool read_only_ ;
        MetaMethod_var meta_method_get_ ;
        MetaMethod_var meta_method_set_ ;
    } ;

    /**
     * \brief Automatic reference-counted pointer to a MetaProperty.
     */
    typedef SmartPointer<MetaProperty> MetaProperty_var ;

//______________________________________________________

}
#endif
