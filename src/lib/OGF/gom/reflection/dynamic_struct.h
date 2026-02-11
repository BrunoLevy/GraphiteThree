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

#ifndef H_OGF_META_TYPES_META_STRUCT_H
#define H_OGF_META_TYPES_META_STRUCT_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta_builtin.h>

#include <typeinfo>

namespace OGF {


    /**
     * \brief MetaType for GOM struct objects, such as StructPropertyRef.
     */
    gom_class GOM_API MetaStruct : public MetaClass {
    public:
	/**
         * \brief Constructs a new MetaStruct
         * \param[in] struct_name the C++ struct name
	 */
	MetaStruct(const std::string& struct_name);

        /**
         * \brief MetaStruct destructor.
         */
	~MetaStruct() override;
    };

    typedef SmartPointer<MetaStruct> MetaStruct_var;

    /********************************************************************/

    /**
     * \brief MetaType for raw C++ structs
     */
    gom_class GOM_API MetaBuiltinStruct : public MetaBuiltinType {
    public:
	/**
         * \brief Constructs a new MetaStruct
         * \param[in] struct_name the C++ struct name
	 */
	MetaBuiltinStruct(const std::string& struct_name);

        /**
         * \brief MetaStruct destructor.
         */
	~MetaBuiltinStruct() override;


    gom_properties:

	/**
	 * \brief Gets a MetaStruct object with the fields and types
	 *  of the corresponding struct.
	 */
	MetaStruct* get_meta_struct() const {
	    return meta_struct_;
	}


    public:
	/**
	 * \brief Declares a new field of this MetaBuiltinStruct
	 * \tparam T the type of the field
	 * \param[in] name the name of the field
	 * \param[in] offset the offset of the field in memory, that is,
	 *   the address of the field supposing that the address of the struct
	 *   is zero
	 * \return a pointer to this MetaBuiltinStruct, used to chain
	 *   calls to add_field()
	 */
	template <class T> MetaBuiltinStruct* add_field(
	    const std::string& name, T* offset
	) {
	    MetaProperty* mprop =
		add_property_by_typeid_name(name, typeid(T).name());
	    mprop->create_custom_attribute(
		"offset", String::to_string(size_t(offset))
	    );
	    return this;
	}

    protected:
	/**
	 * \brief Adds a new property to this MetaBuiltinStruct
	 * \param[in] property_name the name of the property
	 * \param[in] property_type a pointer to the MetaType of the property
	 * \return a pointer to the created MetaProperty
	 */
	MetaProperty* add_property_by_typeid_name(
	    const std::string& property_name,
	    const std::string& typeid_name
	);


    private:
	MetaStruct* meta_struct_;
    };


}

#endif
