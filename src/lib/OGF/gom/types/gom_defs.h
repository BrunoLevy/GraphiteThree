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

#ifndef H_OGF_GOM_TYPES_GOM_DEFS_H
#define H_OGF_GOM_TYPES_GOM_DEFS_H

#include <OGF/gom/common/common.h>
#include <string>

/**
 * \file OGF/gom/types/gom_defs.h
 * \details Some macros to extend C++ with additional keywords understood
 *  by the GOMGEN compiler.
 */

#ifdef GOMGEN

#define gom_class                  %pragma(gom) gomclass;      class
#define gom_slots                  %pragma(gom) gomslots;      public
#define gom_signals                %pragma(gom) gomsignals;    public
#define gom_properties             %pragma(gom) gomproperties; public
#define gom_attribute(x,y)         %pragma(gomattribute) x=y
#define gom_arg_attribute(arg,x,y) %pragma(gomattribute) gomarg ## $ ## arg ## $ ## x=y

#else

#define gom_class      class
#define gom_slots      public
#define gom_signals    public
#define gom_properties public
#define gom_attribute(x,y) 
#define gom_arg_attribute(arg,x,y)

#endif

#define gom_package_initialize(x) extern void gom_package_initialize_##x() ; gom_package_initialize_##x()  

#endif
