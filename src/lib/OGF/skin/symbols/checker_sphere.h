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
 

#ifndef H_OGF_GUI_SYMBOLS_CHECKER_SPHERE_H
#define H_OGF_GUI_SYMBOLS_CHECKER_SPHERE_H

#include <OGF/skin/common/common.h>
#include <OGF/gom/types/object.h>
#include <OGF/basic/math/geometry.h>
#include <geogram/image/color.h>

/**
 * \file OGF/skin/symbols/checker_sphere.h
 * \brief Class to draw a sphere with a checkerboard
 */

namespace OGF {

    class RenderingContext;

//_________________________________________________________

    /**
     * \brief Draws a sphere with a colored checkerboard.
     */
    gom_class SKIN_API CheckerSphere : public Object {
    public:

        /**
         * \brief CheckerSphere constructor.
         */
        CheckerSphere();

        /**
         * \brief CheckerShpere destructor.
         */
         ~CheckerSphere() override;

    gom_properties:

        /**
         * \brief Gets the number of segments.
         * \return the number of segments used to discretize the sphere
         */
        index_t get_nb_segments() const { 
            return nb_segments_; 
        }

        /**
         * \brief Sets the number of segments.
         * \param[in] value the number of segments 
         *  used to discretize the sphere
         */
        void set_nb_segments(index_t value) { 
            nb_segments_ = value; 
        }

        /**
         * Gets the checker size.
         * \return the size of a square of the checkerboard,
         *  in number of segments.
         */
        index_t get_checker_size() const {
            return checker_size_;
        }

        /**
         * Sets the checker size.
         * \param[in] value the size of a square of the checkerboard,
         *  in number of segments.
         */
        void set_checker_size(index_t value) {
            checker_size_ = value;
        }

        /**
         * \brief Gets the rendering context.
         * \return a pointer to the RenderingContext
         * \TODO we probably do not need this function anymore
         */
        RenderingContext* get_rendering_context() const {
            return rendering_context_;
        }

        /**
         * \brief Sets the rendering context.
         * \param[in] value a pointer to the RenderingContext
         * \TODO we probably do not need this function anymore
         */
        void set_rendering_context(RenderingContext* value) {
            rendering_context_ = value;
        }

        /**
         * \brief Tests whether wireframe mode is active.
         * \retval true if wireframe mode is active
         * \retval false otherwise
         */
        bool get_wireframe() const {
            return wireframe_;
        }

        /**
         * \brief Sets wireframe mode.
         * \param[in] value true if wireframe mode should be used,
         *  false if filled polygons should be used
         */
        void set_wireframe(bool value) {
            wireframe_ = value;
        }

        /**
         * \brief Gets the mesh color
         * \return the color used to draw the mes
         */
        const Color& get_mesh_color() const {
            return mesh_color_;
        }

        /**
         * \brief Sets the mesh color
         * \param[in] value the color used to draw the mes
         */
        void set_mesh_color(const Color& value) {
            mesh_color_ = value;
        }

        /**
         * \brief Gets one of the colors used to draw the checkerboard.
         * \return the color
         */
        const Color& get_color1() const {
            return color1_;
        }

        /**
         * \brief Sets one of the colors used to draw the checkerboard.
         * \param[in] value the color
         */
        void set_color1(const Color& value) {
            color1_ = value;
        }

        /**
         * \brief Gets the other color used to draw the checkerboard.
         * \return the color
         */
        const Color& get_color2() const {
            return color2_;
        }

        /**
         * \brief Sets the other color used to draw the checkerboard.
         * \param[in] value the color
         */
        void set_color2(const Color& value) {
            color2_ = value;
        }

        /**
         * \brief Tests whether lighting is used.
         * \retval true if lighting is on
         * \retval false otherwise
         */
        bool get_lighting() const {
            return lighting_;
        }

        /**
         * \brief Sets whether lighting is used.
         * \param[in] value true if lighting should be used,
         *  false otherwise
         */
        void set_lighting(bool value) {
            lighting_ = value;
        }

    gom_slots:
        /**
         * \brief Draws the sphere.
         * \param[in] rendering_context the rendering_context
         *  where the sphere should be drawn. 
         */
        void draw(RenderingContext* rendering_context = nullptr);

    private:
        RenderingContext* rendering_context_;
        index_t nb_segments_;
        index_t checker_size_;
        bool wireframe_;
        Color mesh_color_;
        Color color1_;
        Color color2_;
        bool lighting_;
        vec3 lighting_vector_;
    };

//_________________________________________________________

}
#endif

