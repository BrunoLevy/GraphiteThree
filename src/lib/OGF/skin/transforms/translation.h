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
 

#ifndef H_OGF_GUI_TRANSFORMS_TRANSLATION_H
#define H_OGF_GUI_TRANSFORMS_TRANSLATION_H

#include <OGF/skin/common/common.h>
#include <OGF/gom/types/node.h>
#include <geogram/basic/geometry.h>

/**
 * \file OGF/skin/transforms/translation.h
 * \brief Controls a 2d translation from user mouse input.
 */

namespace OGF {

//_________________________________________________________

   /**
    * \brief Enables to interactively define a 2d translation. 
    */
    gom_class SKIN_API Translation : public Object {
    public:

	/**
	 * \brief Translation constructor.
	 */
        Translation();

	/**
	 * \brief Translation destructor.
	 */
	 ~Translation() override;
	
    gom_slots:
        /**
         * \brief Callback called when the mouse is clicked.
         * \param[in] value the picked point, in normalized device
         *  coordinates (x and y both in [-1.0, 1.0]).
         */
        void grab(const vec2& value) ;

        /**
         * \brief Callback called when the mouse is moved.
         * \param[in] value the point under the mouse pointer, 
         *  in normalized device coordinates (x and y both in [-1.0, 1.0]).
         */
        void drag(const vec2& value) ;

        /**
         * \brief Callback called when the mouse button is released.
         * \param[in] value the point under the mouse pointer, 
         *  in normalized device coordinates (x and y both in [-1.0, 1.0]).
         */
        void release(const vec2& value) ;
        
    gom_signals:
        /**
         * \brief Signal triggered whenever the translation changes.
         * \param[in] value of the translation, as a 2d vector
         */
        void value_changed(const vec3& value) ;

    private:
        vec2 last_position_ ;
    } ;

//_________________________________________________________

}
#endif

