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

#ifndef H_OGF_META_MEMBERS_META_SLOT_H
#define H_OGF_META_MEMBERS_META_SLOT_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_method.h>
#include <OGF/gom/reflection/meta_arg.h>

/**
 * \file OGF/gom/reflection/meta_slot.h
 * \brief Meta-information attached to slots.
 */

namespace OGF {

    
    /**
     * \brief The representation of a slot in the Meta repository.
     */
    gom_class GOM_API MetaSlot : public MetaMethod {
    public:

        /**
         * \brief MetaSlot constructor.
         * \param[in] name name of the slot
         * \param[in] container the MetaClass this MetaSlot belongs to
         * \param[in] return_type_name the C++ return type name, as a 
         *  string
         */
        MetaSlot(
            const std::string& name, MetaClass* container,
            const std::string& return_type_name = "void"
        ) ;

        /**
         * \brief MetaSlot constructor.
         * \param[in] name name of the slot
         * \param[in] container the MetaClass this MetaSlot belongs to
         * \param[in] return_type a pointer to the MetaType that corresponds
         *  to the return type
         */
        MetaSlot(
            const std::string& name, MetaClass* container,
            MetaType* return_type 
        ) ;

        /**
         * \brief MetaSlot destructor.
         */
        virtual ~MetaSlot() ;
    } ;

    /**
     * \brief Automatic reference-counted pointer to a MetaSlot.
     */
    typedef SmartPointer<MetaSlot> MetaSlot_var ;
    
//______________________________________________________

}
#endif 

