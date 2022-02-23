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

#ifndef H_OGF_META_MEMBERS_META_CONSTRUCTOR_H
#define H_OGF_META_MEMBERS_META_CONSTRUCTOR_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_method.h>
#include <OGF/gom/services/factory.h>

/**
 * \file OGF/gom/reflection/meta_constructor.h
 * \brief Meta-information attached to constructors.
 */

namespace OGF {

    /**
     * \brief The representation of a constructor in the Meta repository.
     */
    gom_class GOM_API MetaConstructor : public MetaMethod {
    public:
        /**
         * \brief Constructs a new MetaConstructor.
         * \details The constructed MetaConstructor is 
         *  automatically added to the MetaClass. The
         *  generic method adapter for constructors is
         *  automatically defined as the method adapter.
         * \param[in] mclass a pointer to the MetaClass
         */
        explicit MetaConstructor(MetaClass* mclass);

	/**
	 * \brief MetaConstructor destructor.
	 */
	virtual ~MetaConstructor();
	
        /**
         * \brief Gets the factory associated
         *  with this MetaConstructor.
         * \return a pointer to the factory associated with
         *  this MetaConstructor, or nil if there is no factory.
         */
        Factory* factory() const {
            return factory_;
        }

        /**
         * \brief Sets the factory associated
         *  with this MetaConstructor.
         * \details Factories are typically automatically generated
         *  by the GOMGEN compiler.
         * \param[in] f a pointer to the factory
         */
        void set_factory(Factory* f) {
            factory_ = f;
        }

    protected:
        /**
         * \brief The method adapter used for constructor.
         * \details The same method adapter can be used by all
         *  constructors. It uses the factory registered with
         *  the MetaConstructor.
         * \param[in] target the meta class
         * \param[in] method_name name of the constructor
         * \param[in] args a const reference to the list of arguments
         * \param[out] ret_val the constructed object, in an Any.
         * \retval true if successful
         * \retval false otherwise
         */
        static bool constructor_method_adapter(
            Object* target, 
            const std::string& method_name, const ArgList& args,
            Any& ret_val
        );

    private:
        Factory_var factory_;
    };
}

#endif
