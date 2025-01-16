/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
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

#ifndef H_OGF_GOM_PYTHON_PYTHON_H
#define H_OGF_GOM_PYTHON_PYTHON_H

#include <OGF/gompy/common/common.h>

#ifdef GEO_COMPILER_CLANG
// Python uses 'long long' considered to be not c++98 by Clang.
#pragma GCC diagnostic ignored "-Wc++98-compat-pedantic"
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#endif

#ifdef GEO_COMPILER_GCC_FAMILY
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif


#ifdef GEO_OS_WINDOWS
// Python's distribution under Windows
// has some problems, seems they have
// forgotten to include the debugging
// library.
#   ifdef _DEBUG
#      undef _DEBUG
#      include <Python.h>
#      define _DEBUG
#   else
#      include <Python.h>
#   endif
#else
#  include <Python.h>
#endif

#endif
