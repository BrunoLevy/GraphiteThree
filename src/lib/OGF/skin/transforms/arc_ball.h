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
 

#ifndef H_OGF_GUI_TRANSFORMS_ARCBALL_H
#define H_OGF_GUI_TRANSFORMS_ARCBALL_H

#include <OGF/skin/common/common.h>
#include <OGF/gom/types/object.h>
#include <geogram_gfx/gui/arc_ball.h>

/**
 * \file OGF/skin/transforms/arc_ball.h
 * \brief Controls a 3d rotation from user mouse input.
 */

namespace OGF {

    class RenderingContext;
    
//_________________________________________________________
    
   /**
    * \brief Enables to interactively define a rotation. 
    * \details This class is inspired by an implementation written by 
    * Paul Rademacher, in his glui library.
    * Initial documentation by Paul Rademacher:  
    * A C++ class that implements the Arcball, 
    * as described by Ken Shoemake in Graphics Gems IV.  
    * This class takes as input mouse events (mouse down, mouse drag,
    * mouse up), and creates the appropriate quaternions and 4x4 matrices
    * to represent the rotation given by the mouse.  
    */
    gom_class SKIN_API ArcBall : public Object {
    public:
        /**
         * \brief ArcBall constructor.
         */
        ArcBall();

	/**
	 * \brief ArcBall destructor.
	 */
	~ArcBall() override;
	
    gom_properties:

        /**
         * \brief Gets the value of the rotation.
         * \return the computed rotation, as a 4x4 matrix
         *  (with a zero translational component).
         */
        const mat4& get_value() const {
	    return impl_.get_value();
        }

        /**
         * \brief Sets the value of the rotation.
         * \param[in] value the rotation, as a 4x4 matrix
         *  (with a zero translational component).
         */
        void set_value(const mat4& value) {
	    impl_.set_value(value);
	    value_changed(value);
	}

        /**
         * \brief Tests whether the X axis is constrained.
         * \retval true if the X axis is constrained
         * \retval false otherwise
         */
        bool get_x_constraint() const {
            return impl_.get_x_constraint();
        }

        /**
         * \brief Specifies whether the X axis is constrained.
         * \param[in] value true if the X axis should be constrained,
         *  false otherwise
         */
        void set_x_constraint(bool value) {
	    impl_.set_x_constraint(value);
        }

        /**
         * \brief Tests whether the Y axis is constrained.
         * \retval true if the Y axis is constrained
         * \retval false otherwise
         */
        bool get_y_constraint() const {
	    return impl_.get_y_constraint();
        }

        /**
         * \brief Specifies whether the Y axis is constrained.
         * \param[in] value true if the Y axis should be constrained,
         *  false otherwise
         */
        void set_y_constraint(bool value) {
            impl_.set_y_constraint(value);
        }

        /**
	 * \brief Tests whether this ArcBall is grabbed.
	 * \retval true if this ArcBall is grabbed.
	 * \retval false otherwise.
	 */
        bool get_grabbed() const {
	    return impl_.grabbed();
	}
	
    gom_slots:
        /**
         * \brief Callback called when the mouse is clicked.
         * \param[in] value the picked point, in normalized device
         *  coordinates (x and y both in [-1.0, 1.0]).
         */
        void grab(const vec2& value);

        /**
         * \brief Callback called when the mouse is moved.
         * \param[in] value the point under the mouse pointer, 
         *  in normalized device coordinates (x and y both in [-1.0, 1.0]).
         */
        void drag(const vec2& value);

        /**
         * \brief Callback called when the mouse button is released.
         * \param[in] value the point under the mouse pointer, 
         *  in normalized device coordinates (x and y both in [-1.0, 1.0]).
         */
        void release(const vec2& value);

    gom_signals:
        /**
         * \brief Signal triggered whenever the rotation changes.
         * \param[in] value of the rotation, as a 4x4 matrix (with zero
         *  translational component).
         */
        void value_changed(const mat4& value);

    private:
	GEO::ArcBall impl_;
    };

    /**
     * \brief An automatic reference-counted pointer to an ArcBall.
     */
    typedef SmartPointer<ArcBall> ArcBall_var;
    
/****************************************************************************/

}
#endif

