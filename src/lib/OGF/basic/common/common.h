/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 Bruno Levy
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

#ifndef H_OGF_BASIC_COMMON_COMMON_H
#define H_OGF_BASIC_COMMON_COMMON_H

/**
 * \file OGF/basic/common/common.h
 * \brief Definitions common to all include files in the basic library.
 */

#include <geogram/basic/common.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/argused.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/string.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/logger.h>
#include <iostream>
#include <stdlib.h>

#define OGF_EXPORT GEO_EXPORT
#define OGF_IMPORT GEO_IMPORT

#ifdef basic_EXPORTS
#   define BASIC_API OGF_EXPORT
#else
#   define BASIC_API OGF_IMPORT
#endif

#define ogf_assert(x)            geo_assert(x)
#define ogf_debug_assert(x)      geo_debug_assert(x)
#define ogf_assert_not_reached   geo_assert_not_reached
#define ogf_argused(x)           ::GEO::geo_argused(x)
#define ogf_range_assert(x,xmin,xmax) geo_assert((x) >= (xmin) && (x) <= (xmax))

/**
 * \brief Global Graphite namespace.
 * \details OGF stands for Open Graphics Foundation classes.
 */
namespace OGF {
    using namespace GEO;
    template <class T> Sign ogf_sgn(const T& x) {
        return geo_sgn(x);
    }
    template <class T> void ogf_clamp(T& x, T xmin, T xmax) {
        geo_clamp(x,xmin,xmax);
    }
    template <class T> T ogf_sqr(const T& x) {
        return geo_sqr(x);
    }
}

namespace OGF {

    static class BASIC_API basic_libinit {
    public:
        basic_libinit() ;
        ~basic_libinit() ;
        
        static void increment_users() ;
        static void decrement_users() ;
        
    private:
        static void initialize() ;
        static void terminate() ;
        static int count_ ;
    } basic_libinit_instance ;
    
}

#endif
