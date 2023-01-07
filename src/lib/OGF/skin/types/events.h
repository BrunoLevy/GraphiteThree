/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
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
 

#ifndef H_OGF_GUI_TYPES_EVENTS_H
#define H_OGF_GUI_TYPES_EVENTS_H

#include <OGF/skin/common/common.h>

/**
 * \file OGF/skin/types/events.h
 * \brief Events
 */

namespace OGF {
    
    /**
     * \brief Symbolic constants for mouse buttons
     */
    enum MouseButton {
        MOUSE_BUTTON_NONE       = 0,
        MOUSE_BUTTON_LEFT       = 1,
        MOUSE_BUTTON_MIDDLE     = 2,
        MOUSE_BUTTON_RIGHT      = 3,
        MOUSE_BUTTON_WHEEL_UP   = 4,
        MOUSE_BUTTON_WHEEL_DOWN = 5,
        MOUSE_BUTTON_AUX1       = 6,
        MOUSE_BUTTON_AUX2       = 7,
        MOUSE_BUTTONS_NB        = 8
    };
    
}

#endif
