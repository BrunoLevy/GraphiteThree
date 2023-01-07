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
 

#ifndef H_OGF_GUI_TRANSFORMS_RAY_PICKER_H
#define H_OGF_GUI_TRANSFORMS_RAY_PICKER_H

#include <OGF/skin/common/common.h>
#include <OGF/gom/types/node.h>
#include <OGF/basic/math/geometry.h>

/**
 * \file OGF/skin/transforms/ray_picker.h
 * \brief Transformation of user input between the screen and the 3D space.
 */

namespace OGF {

    class RenderingContext;

    /**************************************************************************/

    /**
     * \brief Represents the information related with a picking event.
     * \details For now, it just stores the 2D coordinates of the
     *   picked point in Normalized Device Coordinates. This class
     *   can be used as a placeholder for more efficient picking
     *   mechanisms (e.g. a stylus that could also have an orientation,
     *   thus defining a "picking ray").
     */
    struct RayPick {
    public:

        /**
         * \brief RayPick constructor.
         * \param[in] p_ndc_in picked point, in normalized device
         *  coordinates (X and Y both in [-1.0, 1.0])
         * \param[in] btn_in clicked button
         */
        RayPick(const vec2& p_ndc_in, int btn_in) :
            p_ndc(p_ndc_in),
            button(btn_in) {
        }
        
        /**
         * \brief RayPick constructor
         * \details Does not initialize the RayPick.
         */
        RayPick() {
        }

        /**
         * \brief picked point, in normalized device
         *  coordinates (X and Y both in [-1.0, 1.0]).
         */
        vec2 p_ndc;

        /**
         * \brief Clicked button.
         */
        int button;
    };

    /**
     * \brief Sends a RayPick to an output stream.
     * \param[in,out] out the output stream
     * \param[in] ev a const reference to the RayPick
     * \return the new state of the output stream once the RayPick has been
     *  send to it.
     */
    SKIN_API std::ostream& operator<<(std::ostream& out, const RayPick& ev);

    /**
     * \brief Reads a RayPick from an input stream.
     * \param[in,out] in the input stream
     * \param[out] ev a reference to the read RayPick
     * \return the new state of the output stream once the RayPick has been
     *  read from it
     */
    SKIN_API std::istream& operator>>(std::istream& in, RayPick& ev);

    
    /**
     * \brief Converts a 2D picking in a rendering window into 
     *  a ray picking event.
     * \details For now, just copies the 2D NDC coordinates of the event 
     *   into the target RayPick. It can be used as a placeholder for 
     *   more elaborate picking mechanisms.
     * \see RayPick
     */
    gom_class SKIN_API RayPicker : public Object {
    public:
        /**
         * \brief RayPicker constructor.
         */
        RayPicker();

	/**
	 * \brief RayPicker destructor.
	 */
	 ~RayPicker() override;

    gom_slots:

        /**
         * \brief Callback called when the mouse is clicked.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] point_ndc the picked point, in normalized device
         *  coordinates (x and y both in [-1.0, 1.0])
         * \param[in] button the button that was clicked
         */
        void grab(
            RenderingContext* rendering_context,
            const vec2& point_ndc, int button
        );

        /**
         * \brief Callback called when the mouse is dragged with a button
         *  pressed.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] point_ndc the point under the mouse pointer, 
         *  in normalized device coordinates (x and y both in [-1.0, 1.0])
         * \param[in] button the button that is pressed
         */
        void drag(
            RenderingContext* rendering_context,
            const vec2& point_ndc, int button
        );

        /**
         * \brief Callback called when a mouse button is released.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] point_ndc the point under the mouse pointer, 
         *  in normalized device coordinates (x and y both in [-1.0, 1.0])
         * \param[in] button the button that is released
         */
        void release(
            RenderingContext* rendering_context,
            const vec2& point_ndc, int button
        );
        
    gom_signals:
        /**
         * \brief The signal triggered when a mouse button is pressed.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] value the RayPick event generated from the mouse
         *  pointer location
         */
        void ray_grab(
            RenderingContext* rendering_context, const RayPick& value
        );

        /**
         * \brief The signal triggered when the mouse is dragged.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] value the RayPick event generated from the mouse
         *  pointer location
         */
        void ray_drag(
            RenderingContext* rendering_context, const RayPick& value
        );

        /**
         * \brief The signal triggered when a mouse button is released.
         * \param[in] rendering_context a pointer to the RenderingContext
         * \param[in] value the RayPick event generated from the mouse
         *  pointer location
         */
        void ray_release(
            RenderingContext* rendering_context, const RayPick& value
        );

    protected:
        /**
         * \brief Converts a mouse pointer location into a RayPick event.
         * \details For now, just copies \p p_ndc and \p button in the
         *  RayPick event. This can be a placeholder for more sophisticated
         *  picking mechanisms (e.g., with a stylus that has an orientation
         *  sensor).
         * \param[in] context a pointer to the RenderingContext
         * \param[in] p_ndc the point under the mouse pointer, in normalized
         *  device coordinates (X and Y both in [-1.0, 1.0])
         * \param[in] button the button that was pressed/dragged or released
         * \return the RayPick event
         */
        RayPick point_to_ray_pick(
            RenderingContext* context, const vec2& p_ndc, int button
        );
    };
    
    /*************************************************************************/
    
}
#endif

