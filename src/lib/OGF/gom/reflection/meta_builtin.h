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

#ifndef H_OGF_META_TYPES_META_BUILTIN_H
#define H_OGF_META_TYPES_META_BUILTIN_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_type.h>

/**
 * \file OGF/gom/reflection/meta_builtin.h
 * \brief MetaType for builting types.
 */

namespace OGF {

    /**
     * \brief MetaType for builting types.
     * \details Builtin types are C++ base types,
     *   strings and pointers.
     */
    gom_class GOM_API MetaBuiltinType : public MetaType {
    public:
        /**
         * \brief MetaBuiltinType constructor.
         * \param[in] name C++ type name
         */
        MetaBuiltinType(const std::string& name) ;

        /**
         * \brief MetaBuiltinType destructor.
         */
        virtual ~MetaBuiltinType() ;

    gom_slots:
        /**
         * \brief Tests whether this builtin type is a pointer type.
         * \retval true if this builtin type is a pointer type
         * \retval false otherwise
         */
        bool is_pointer_type() const ;
    } ;

    /**
     * \brief Automatic reference-counted pointer to a MetaBuiltinType.
     */
    typedef SmartPointer<MetaBuiltinType> MetaBuiltinType_var ;   
}

#endif 

