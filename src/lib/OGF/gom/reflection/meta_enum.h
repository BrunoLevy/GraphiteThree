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

#ifndef H_OGF_META_TYPES_META_ENUM_H
#define H_OGF_META_TYPES_META_ENUM_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_type.h>
#include <vector>

/**
 * \file OGF/gom/reflection/meta_enum.h
 * \brief MetaType for enums.
 */

namespace OGF {

    /**
     * \brief MetaType for enums.
     * \details Gives the symbolic names
     *  and associated values.
     */
    gom_class GOM_API MetaEnum : public MetaType {
    public:

        /**
         * \brief MetaEnum constructor.
         * \param[in] enum_name C++ type name of the enum
         */
        MetaEnum(const std::string& enum_name) ;

        /**
         * \brief MetaEnum destructor.
         */
        virtual ~MetaEnum() ;

      gom_slots:
        /**
         * \brief Gets the number of values of the enum.
         * \return the number of values
         */
        size_t nb_values() const {
            return values_.size();
        }


        /**
         * \brief Gets a value by index.
         * \param[in] i index, in 0..nb_values() - 1
         * \return the value associated with index \p i
         */
        int ith_value(index_t i) const {
            ogf_assert(i < values_.size()) ;
            return values_[i].value ;
        }

        /**
         * \brief Gets a symbolic name by index.
         * \param[in] i index, in 0..nb_values() - 1
         * \return the symbolic name associated with index \p i
         */
        const std::string& ith_name(index_t i) const {
            ogf_assert(i < values_.size()) ;
            return values_[i].name ;
        }

        /**
         * \brief Tests whether a MetaEnum has a value
         * \param[in] value the value
         * \retval true if this MetaEnum has \p value
         * \retval false otherwise
         */
        bool has_value(int value) const ;

        /**
         * \brief Tests whether a MetaEnum has a symbolic name
         * \param[in] name the symbolic name
         * \retval true if this MetaEnum has a value with name \p name
         * \retval false otherwise
         */
        bool has_value(const std::string& name) const ;

        /**
         * \brief Gets a value by symbolic name.
         * \param[in] name the symbolic name
         * \return the value associated with \p name
         * \pre has_value(name)
         */
        int get_value_by_name(const std::string& name) const ;

        /**
         * \brief Gets a symbolic name by value
         * \param[in] value the value
         * \return the symbolic name associated with \p value
         * \pre has_value(value)
         */
        const std::string& get_name_by_value(int value) const ; 
	
    public:
        /**
         * \brief Adds a new value to this MetaEnum.
         * \param[in] name symbolic name
         * \param[in] value associated value. Can be negative.
         */
        void add_value(const std::string& name, int value) ;


    private:
        struct Value {
            std::string name ;
            int value ;
        } ;
        std::vector<Value> values_ ;
    } ;

    /**
     * \brief Automatic reference-counted pointer to a MetaEnum.
     */
    typedef SmartPointer<MetaEnum> MetaEnum_var ;
}

#endif 

