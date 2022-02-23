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
 

#ifndef H_OGF_GOM_GOM_COMMON_H
#define H_OGF_GOM_GOM_COMMON_H

/**
 * \file OGF/gom_gom/common/common.h
 * \brief Definitions common to all include files in the gom_gom library.
 */

#include <OGF/gom/common/common.h>
#ifdef gom_gom_EXPORTS
#   define GOM_GOM_API OGF_EXPORT
#else
#   define GOM_GOM_API OGF_IMPORT
#endif

#include <iostream>
// iostream should be included before anything
// else, otherwise 'cin', 'cout' and 'cerr' will
// be uninitialized.


namespace OGF {

    static class GOM_GOM_API gom_gom_libinit {
    public:
        gom_gom_libinit() ;
        ~gom_gom_libinit() ;
        
        static void increment_users() ;
        static void decrement_users() ;
        
    private:
        static void initialize() ;
        static void terminate() ;
        static int count_ ;
    } gom_gom_libinit_instance ;
    
}

#endif
