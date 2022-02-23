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

#ifndef H_OGF_META_MEMBERS_MEMBER_H
#define H_OGF_META_MEMBERS_MEMBER_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_type.h>
#include <string>

/**
 * \file OGF/gom/reflection/meta_member.h
 * \brief Meta-information attached to class members.
 */

namespace OGF {

    class MetaClass ;

    /**
     * \brief The base class for class members in the Meta repository.
     */
    gom_class GOM_API MetaMember : public MetaInformation {
    public:
        /**
         * \brief MetaMember constructor
         * \param[in] name name of the member
         * \param[in] container the MetaClass this MetaMember belongs to
         * \note MetaMember's constructor does not automatically add 
         * the MetaMember to the container class, 
         * since MetaProperty has a getter 
         * and setter MetaMethods which are not directly contained by 
         * the MetaClass.
         */
        MetaMember(const std::string& name, MetaClass* container) ;

        /**
         * \brief MetaMember destructor.
         */
	~MetaMember() override;

        /**
         * \brief Gets the name of this MetaMember.
         * \return a const reference to the name
         */
        const std::string& name() const {
            return name_ ;
        }

	/**
	 * \brief Removes all variables that use the meta type system
	 *  before deleting.
	 * \details If we do not do that, when deleting the meta type system,
	 *  we can delete the meta information in the wrong order, and delete
	 *  first meta classes that we needed to delete other ones.
	 */
	virtual void pre_delete();
	
    gom_slots:
        /**
         * \brief Gets the MetaClass this MetaMember belongs to.
         * \return a pointer to the containing MetaClass
         */
        MetaClass* container_meta_class() const {
            return container_ ;
        }

    gom_properties:
        /**
         * \brief Gets the name of this MetaMember.
         * \return a const reference to the name
         */
        const std::string& get_name() const {
            return name_;
        }
        
    private:
        std::string name_ ;
        MetaClass* container_ ;
    } ;

    /**
     * \brief Automatic reference-counted pointer to a MetaMember.
     */
    typedef SmartPointer<MetaMember> MetaMember_var ;

}
#endif 

