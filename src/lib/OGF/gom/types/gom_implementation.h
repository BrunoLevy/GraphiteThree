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

#ifndef H_OGF_GOM_TYPES_GOM_IMPL_H
#define H_OGF_GOM_TYPES_GOM_IMPL_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/types/gom.h>
#include <OGF/gom/types/arg_list.h>
#include <OGF/gom/reflection/meta_type.h>
#include <OGF/gom/reflection/meta_builtin.h>
#include <OGF/gom/reflection/meta_enum.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta_struct.h>
#include <OGF/gom/services/serializer.h>

/**
 * \file OGF/gom/types/gom_implementation.h
 * \brief Functions to declare GOM meta-types.
 * \details Used by the C++ code created by GOMGEN.
 *  Client code should not use these functions.
 */

namespace OGF {

//_________________________________________________________________________

    /**
     * \brief A class to declare a new builtin type.
     * \tparam T the builtin type to be declared
     * \details Example of use:
     * \code
     *    MetaBuiltinType* mtype = ogf_declare_builtin_type<Color>("color");
     * \endcode
     * \note Used by the C++ code created by GOMGEN.
     *  Client code should not use these functions.
     */
    template <class T> class ogf_declare_builtin_type {
    public:
        /**
         * \brief Declares a new builtin type.
         * \param[in] type_name the C++ type name of the type
         *  to be declared.
         */
        explicit ogf_declare_builtin_type(const std::string& type_name) {
            //   We create the MetaType using a SmartPointer, because
            // it may be already registered in the Meta repository. If
            // it is the case, then the SmartPointer will deallocate it.
            //   We need though to call Meta::bind_meta_type() since the
            // type may be an alias. The result is finally retreived in
            // the Meta instance (if there was a previous MetaType declared
            // with the same name, it will use this one).
            MetaBuiltinType_var meta_type = new MetaBuiltinType(type_name);
            meta_type->set_serializer(new GenericSerializer<T>);
            meta_type->set_life_cycle(new GenericLifeCycle<T>);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
            result_ = dynamic_cast<MetaBuiltinType*>(
                Meta::instance()->resolve_meta_type(type_name)
            );
        }
        /**
         * \brief Gets the created MetaType object.
         * \return a pointer to the created MetaType object.
         */
        operator MetaBuiltinType*() {
            return result_;
        }
    private:
        MetaBuiltinType* result_;
    };

   /**
    * \brief ogf_declare_builtin_type specialization for bool.
    * \note Used by the C++ code created by GOMGEN.
    *  Client code should not use these functions.
    */
    template<> class ogf_declare_builtin_type<bool> {
    public:
        /**
         * \copydoc ogf_declare_builtin_type::ogf_declare_builtin_type()
         */
        explicit ogf_declare_builtin_type(const std::string& type_name) {
            //   We create the MetaType using a SmartPointer, because
            // it may be already registered in the Meta repository. If
            // it is the case, then the SmartPointer will deallocate it.
            //   We need though to call Meta::bind_meta_type() since the
            // type may be an alias. The result is finally retreived in
            // the Meta instance (if there was a previous MetaType declared
            // with the same name, it will use this one).
            MetaBuiltinType_var meta_type = new MetaBuiltinType(type_name);
            meta_type->set_serializer(new BoolSerializer);
            meta_type->set_life_cycle(new GenericLifeCycle<bool>);

            Meta::instance()->bind_meta_type(
                meta_type, typeid(bool).name()
            );
            result_ = dynamic_cast<MetaBuiltinType*>(
                Meta::instance()->resolve_meta_type(type_name)
            );
        }
        /**
         * \copydoc ogf_declare_builtin_type::operator MetaBuiltinType*()
         */
        operator MetaBuiltinType*() {
            return result_;
        }
    private:
        MetaBuiltinType* result_;
    };

   /**
    * \brief ogf_declare_builtin_type specialization for void.
    * \note Used by the C++ code created by GOMGEN.
    *  Client code should not use these functions.
    */
    template<> class ogf_declare_builtin_type<void> {
    public:
        /**
         * \copydoc ogf_declare_builtin_type::ogf_declare_builtin_type()
         */
        explicit ogf_declare_builtin_type(const std::string& type_name) {
            //   We create the MetaType using a SmartPointer, because
            // it may be already registered in the Meta repository. If
            // it is the case, then the SmartPointer will deallocate it.
            //   We need though to call Meta::bind_meta_type() since the
            // type may be an alias. The result is finally retreived in
            // the Meta instance (if there was a previous MetaType declared
            // with the same name, it will use this one).
            MetaBuiltinType_var meta_type = new MetaBuiltinType(type_name);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(void).name()
            );
            result_ = dynamic_cast<MetaBuiltinType*>(
                Meta::instance()->resolve_meta_type(type_name)
            );
        }
        /**
         * \copydoc ogf_declare_builtin_type::operator MetaBuiltinType*()
         */
        operator MetaBuiltinType*() {
            return result_;
        }
    private:
        MetaBuiltinType* result_;
    };

   /**
    * \brief ogf_declare_builtin_type specialization for std::string.
    * \note Used by the C++ code created by GOMGEN.
    *  Client code should not use these functions.
    */
    template<> class ogf_declare_builtin_type<std::string> {
    public:
        /**
         * \copydoc ogf_declare_builtin_type::ogf_declare_builtin_type()
         */
        explicit ogf_declare_builtin_type(const std::string& type_name) {
            //   We create the MetaType using a SmartPointer, because
            // it may be already registered in the Meta repository. If
            // it is the case, then the SmartPointer will deallocate it.
            //   We need though to call Meta::bind_meta_type() since the
            // type may be an alias. The result is finally retreived in
            // the Meta instance (if there was a previous MetaType declared
            // with the same name, it will use this one).
            MetaBuiltinType_var meta_type = new MetaBuiltinType(type_name);
            meta_type->set_serializer(new StringSerializer);
            meta_type->set_life_cycle(new GenericLifeCycle<std::string>);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(std::string).name()
            );
            result_ = dynamic_cast<MetaBuiltinType*>(
                Meta::instance()->resolve_meta_type(type_name)
            );
        }
        /**
         * \copydoc ogf_declare_builtin_type::operator MetaBuiltinType*()
         */
        operator MetaBuiltinType*() {
            return result_;
        }
    private:
        MetaBuiltinType* result_;
    };

   /**
    * \brief a version of ogf_declare_builtin_type specialization
    *  for pointers.
    * \details We did not use a specialization as in the code below,
    *  because MSVC 6 does not seem to support it.
    * \code
    *   template <class T> class ogf_declare_builtin_type<T*> {
    *   ...
    *   };
    * \endcode
    * \TODO test whether it works with more modern versions of MSVC.
    * \note Used by the C++ code created by GOMGEN.
    *  Client code should not use these functions.
    */
    template <class T> class ogf_declare_pointer_type {
    public:
        /**
         * \brief Declares a new builtin pointer type.
         * \param[in] type_name the C++ type name of the type
         *  to be declared.
         */
        explicit ogf_declare_pointer_type(const std::string& type_name) {
            //   We create the MetaType using a SmartPointer, because
            // it may be already registered in the Meta repository. If
            // it is the case, then the SmartPointer will deallocate it.
            //   We need though to call Meta::bind_meta_type() since the
            // type may be an alias. The result is finally retreived in
            // the Meta instance (if there was a previous MetaType declared
            // with the same name, it will use this one).
            MetaBuiltinType_var meta_type = new MetaBuiltinType(type_name);
            meta_type->set_serializer(new PointerSerializer);
            meta_type->set_life_cycle(new GenericLifeCycle<T>);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
            result_ = dynamic_cast<MetaBuiltinType*>(
                Meta::instance()->resolve_meta_type(type_name)
            );
        }
        /**
         * \brief Gets the created MetaType object.
         * \return a pointer to the created MetaType object.
         */
        operator MetaBuiltinType*() {
            return result_;
        }
    private:
        MetaBuiltinType* result_;
    };

    /**
     * \brief A class to declare a new enum type.
     * \tparam T the enum type to be declared
     * \details Example of use:
     * \code
     *    MetaEnum* mtype = ogf_declare_builtin_type<Sign>("OGF::Sign");
     *    mtype->add_value("NEGATIVE",-1);
     *    mtype->add_value("ZERO",0);
     *    mtype->add_value("POSITIVE",1);
     * \endcode
     * \note Used by the C++ code created by GOMGEN.
     *  Client code should not use these functions.
     */
    template <class T> class ogf_declare_enum {
    public:
        /**
         * \brief Declares a new enum type.
         * \param[in] type_name the C++ type name of the enum
         *  to be declared
         */
        explicit ogf_declare_enum(const std::string& type_name) {
            MetaEnum* meta_type = new MetaEnum(type_name);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
	    meta_type->set_life_cycle(new GenericLifeCycle<T>);
            result_ = meta_type;
        }

        /**
         * \brief Gets the created MetaEnum object.
         * \return a pointer to the created MetaEnum object
         */
        operator MetaEnum*() {
            return result_;
        }
    private:
        MetaEnum* result_;
    };

    /***************************************************************************/

#define ogf_offset(structtype,field) \
	&(reinterpret_cast<structtype*>(0)->field)

#define ogf_add_field(structtype, field) \
    add_field(#field, ogf_offset(structtype, field))

    /**
     * \brief A class to declare a new struct type.
     * \tparam T the struct type to be declared
     * \details Example of use:
     * \code
     *    ogf_declare_struct<SurfaceStyle>("OGF::SurfaceStyle")
     *     ->ogf_add_field(SurfaceStyle, visible)
     *     ->ogf_add_field(SurfaceStyle, color);
     * \endcode
     * \note Used by the C++ code created by GOMGEN.
     *  Client code should not use these functions.
     */
    template <class T> class ogf_declare_struct {
    public:
        /**
         * \brief Declares a new struct type.
         * \param[in] type_name the C++ type name of the struct
         *  to be declared
         */
        explicit ogf_declare_struct(const std::string& type_name) {
            MetaBuiltinStruct* meta_type = new MetaBuiltinStruct(type_name);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
            //meta_type->set_serializer(new GenericSerializer<T>);
            meta_type->set_life_cycle(new GenericLifeCycle<T>);
            result_ = meta_type;
        }

        /**
         * \brief Gets the created MetaBuiltinStruct object.
         * \return a pointer to the created MetaBuiltinStruct object
         */
        operator MetaBuiltinStruct*() {
            return result_;
        }

        /**
         * \brief Gets the created MetaBuiltinStruct object.
	 * \details To be able to write
	 *   ogf_declare_struct<XXX>("XXX")->add_field<YYY>("zzz")
         * \return a pointer to the created MetaBuiltinStruct object
         */
	MetaBuiltinStruct* operator->() {
            return result_;
	}

    private:
        MetaBuiltinStruct* result_;
    };

    /***************************************************************************/

    /**
     * \brief A class to declare a class type.
     * \tparam T the class type to be declared
     * \details Example of use:
     * \code
     *    MetaClass* mclass = ogf_declare_class<ComboBox>(
     *     "OGF::ComboBox","OGF::Widget"
     *    );
     * \endcode
     * \note Used by the C++ code created by GOMGEN.
     *  Client code should not use these functions.
     */
    template <class T> class ogf_declare_class {
    public:
        /**
         * \brief Declares a new class type.
         * \param[in] class_name the C++ type name of the class
         *  to be declared
         * \param[in] super_class a pointer to the MetaClass of the
         *  super class
         */
	ogf_declare_class(
            const std::string& class_name, MetaClass* super_class
        ) {
            MetaClass* meta_type = new MetaClass(class_name, super_class);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
            result_ = meta_type;
        }

        /**
         * \brief Declares a new class type.
         * \param[in] class_name the C++ type name of the class
         *  to be declared
         * \param[in] super_class_name the C++ type name of the
         *  superclass
         */
        ogf_declare_class(
            const std::string& class_name, const std::string& super_class_name
        ) {
            MetaClass* meta_type = new MetaClass(class_name, super_class_name);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
            result_ = meta_type;
        }

        /**
         * \brief Declares a new class type.
         * \param[in] class_name the C++ type name of the class
         *  to be declared
         */
        explicit ogf_declare_class(const std::string& class_name) {
            MetaClass* meta_type = new MetaClass(class_name);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
            result_ = meta_type;
        }

        /**
         * \brief Gets the created MetaClass object.
         * \return a pointer to the created MetaEnum object
         */
        operator MetaClass*() {
            return result_;
        }
    private:
        MetaClass* result_;
    };


    /**
     * \brief A class to declare an abstract class type.
     * \tparam T the class type to be declared
     * \details Example of use:
     * \code
     *    MetaClass* mclass = ogf_declare_abstract_class<Widget>(
     *     "OGF::Widget","OGF::Object"
     *    );
     * \endcode
     * \note Used by the C++ code created by GOMGEN.
     *  Client code should not use these functions.
     */
    template <class T> class ogf_declare_abstract_class {
    public:

        /**
         * \brief Declares a new abstract class type.
         * \param[in] class_name the C++ type name of the class
         *  to be declared
         * \param[in] super_class a pointer to the MetaClass of the
         *  super class
         */
        ogf_declare_abstract_class(
            const std::string& class_name, MetaClass* super_class
        ) {
            MetaClass* meta_type = new MetaClass(class_name, super_class);
            meta_type->set_abstract(true);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
            result_ = meta_type;
        }

        /**
         * \brief Declares a new abstract class type.
         * \param[in] class_name the C++ type name of the class
         *  to be declared
         * \param[in] super_class_name the C++ type name of the
         *  superclass
         */
        ogf_declare_abstract_class(
            const std::string& class_name, const std::string& super_class_name
        ) {
            MetaClass* meta_type = new MetaClass(class_name, super_class_name);
            meta_type->set_abstract(true);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
            result_ = meta_type;
        }

        /**
         * \brief Declares a new abstract class type.
         * \param[in] class_name the C++ type name of the class
         *  to be declared
         */
        explicit ogf_declare_abstract_class(const std::string& class_name) {
            MetaClass* meta_type = new MetaClass(class_name);
            meta_type->set_abstract(true);
            Meta::instance()->bind_meta_type(
                meta_type, typeid(T).name()
            );
            result_ = meta_type;
        }

        /**
         * \brief Gets the created MetaClass object.
         * \return a pointer to the created MetaEnum object
         */
        operator MetaClass*() {
            return result_;
        }
    private:
        MetaClass* result_;
    };

//_________________________________________________________________________

}

#endif
