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
 

#ifndef H_OGF_SKIN_IMGUI_COMMON_COMMON_H
#define H_OGF_SKIN_IMGUI_COMMON_COMMON_H

#include <OGF/basic/common/common.h>
#ifdef skin_imgui_EXPORTS
#   define SKIN_IMGUI_API OGF_EXPORT
#else
#   define SKIN_IMGUI_API OGF_IMPORT
#endif


#include <iostream>
// iostream should be included before anything
// else, otherwise 'cin', 'cout' and 'cerr' will
// be uninitialized.

//#include <geogram_gfx/third_party/glew/glew.h>
#include <OGF/skin/common/common.h>
#include <geogram_gfx/basic/common.h>

// Lots of "conditional expression is constant" warnings 
// in Qt include files. Deactivating this warning for this
// library.

#ifdef GEO_COMPILER_MSVC
# pragma warning(disable: 4127)
#endif

// Lots of sign conversion and conversion warnings in Qt,
// disabling them.
#ifdef GEO_COMPILER_GCC
# pragma GCC diagnostic ignored "-Wsign-conversion"
# pragma GCC diagnostic ignored "-Wconversion"
#endif

/**
 * \file OGF/skin_imgui/common/common.h
 * \brief Definitions common to all include files in the skin_imgui library.
 */
namespace OGF {

    static class SKIN_IMGUI_API skin_imgui_libinit {
    public:
        skin_imgui_libinit();
        ~skin_imgui_libinit();

        static void increment_users();
        static void decrement_users();

    private:
        static void initialize();
        static void terminate();
        static int count_;
    } skin_imgui_libinit_instance;

}

#endif
