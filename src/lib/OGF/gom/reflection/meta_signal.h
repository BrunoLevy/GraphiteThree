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


#ifndef H_OGF_META_MEMBERS_META_SIGNAL_H
#define H_OGF_META_MEMBERS_META_SIGNAL_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_method.h>

/**
 * \file OGF/gom/reflection/meta_signal.h
 * \brief Meta-information attached to signals.
 */

namespace OGF {

    /**
     * \brief The representation of a signal in the Meta repository.
     */
    gom_class GOM_API MetaSignal : public MetaMethod {
    public:
        
        /**
         * \brief MetaSignal constructor.
         * \param[in] name name of the signal
         * \param[in] container the MetaClass this MetaSignal belongs to
         */
        MetaSignal(const std::string& name, MetaClass* container) ;

        /**
         * \brief MetaSignal destructor.
         */
        virtual ~MetaSignal() ;

    protected:
        
        /**
         * \brief the method adapter for signals.
         * \param[in] target the object which signal should be invoked
         * \param[in] method_name the name of the signal
         * \param[in] args a const reference to the arguments list
         * \param[out] ret_val the return value, as a string
         */
        static bool signal_method_adapter(
            Object* target, 
            const std::string& method_name, const ArgList& args,
            Any& ret_val
        ) ;
    } ;

    /**
     * \brief Automatic reference-counted pointer to a MetaSignal.
     */
    typedef SmartPointer<MetaSignal> MetaSignal_var ;
    
//______________________________________________________
    
}
#endif 

