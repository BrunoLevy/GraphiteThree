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
 

#ifndef H_OGF_GUI_TRANSFORMS_TRANSFORM3D_H
#define H_OGF_GUI_TRANSFORMS_TRANSFORM3D_H

#include <OGF/skin/common/common.h>
#include <OGF/gom/types/node.h>
#include <OGF/basic/math/geometry.h>

/**
 * \file OGF/skin/transforms/transform3d.h
 * \brief Controls a 3d transform by different means.
 */

namespace OGF {

//_________________________________________________________


    /**
     * \brief Controls a 3d transform by different means.
     */
    gom_class SKIN_API Transform3d : public Object {
    public:
        /**
         * \brief Transform3d constructor.
         */
        Transform3d();

        /**
         * \brief Transform3d destructor.
         */
         ~Transform3d() override;

        
    gom_properties:
        /**
         * \brief Gets the origin.
         * \return the origin
         */
        const vec3& get_look_at() const {
            return look_at_;
        }

        /**
         * \brief Sets the origin.
         * \param[in] value the origin
         */
        void set_look_at(const vec3& value) {
            look_at_ = value;
            update_matrix();
        }

        /**
         * \brief Gets the u vector.
         * \details The u vector points to the right.
         * \return the u vector
         */
        const vec3& get_u() const {
            return u_;
        }

        /**
         * \brief Sets the u vector.
         * \details The u vector points to the right.
         * \param[in] value the u vector
         */
        void set_u(const vec3& value) {
            u_ = value;
            update_matrix();
        }

        /**
         * \brief Gets the v vector.
         * \details The v vector points to the top.
         * \return the v vector
         */
        const vec3& get_v() const {
            return v_;
        }

        /**
         * \brief Sets the v vector.
         * \details The v vector points to the top.
         * \param[in] value the v vector
         */
        void set_v(const vec3& value) {
            v_ = value;
            update_matrix();
        }

        /**
         * \brief Gets the w vector.
         * \details The w vector points to the front.
         * \return the w vector
         */
        const vec3& get_w() const {
            return w_;
        }

        /**
         * \brief Sets the w vector.
         * \details The w vector points to the front.
         * \param[in] value the w vector
         */
        void set_w(const vec3& value) {
            w_ = value;
            update_matrix();
        }

        /**
         * \brief Gets the increment for position changes.
         * \return the increment for position changes
         * \see move_up(), move_down(), 
         *  move_right(), move_left(),
         *  move_forward(), move_backward()
         */
        double get_delta_location() const {
            return delta_location_;
        }

        /**
         * \brief Sets the increment for position changes.
         * \param[in] value the increment for position changes
         * \see move_up(), move_down(), 
         *  move_right(), move_left(),
         *  move_forward(), move_backward()
         */
        void set_delta_location(double value) {
            delta_location_ = value;
        }

        /**
         * \brief Gets the increment for angle changes.
         * \return the increment for angle changes
         * \see turn_left(), turn_right(),
         *  tilt_left(), tilt_right(),
         *  tilt_up(), tilt_down()
         */
        double get_delta_angle() const {
            return delta_angle_;
        }

        /**
         * \brief Sets the increment for angle changes.
         * \param[in] value the increment for angle changes
         * \see turn_left(), turn_right(),
         *  tilt_left(), tilt_right(),
         *  tilt_up(), tilt_down()
         */
        void set_delta_angle(double value) {
            delta_angle_ = value;
        }

        /**
         * \brief Gets the zooming factor.
         * \return the zooming factor
         */
        double get_zoom() const {
            return zoom_;
        }

        /**
         * \brief Sets the zooming factor.
         * \param[in] value the zooming factor
         */
        void set_zoom(double value) {
            zoom_ = value ;
            update_matrix() ;
        }

        /**
         * \brief Gets the increment for zooming.
         * \return the increment for zooming
         * \see zoom_in(), zoom_out()
         */
        double get_delta_zoom() const {
            return delta_zoom_;
        }

        /**
         * \brief Sets the increment for zooming.
         * \param[in] value the increment for zooming
         * \see zoom_in(), zoom_out()
         */
        void set_delta_zoom(double value) {
            delta_zoom_ = value;
        }

        /**
         * \brief Gets the current transform.
         * \return the current transform, as a 4x4 homogeneous-
         *  coordinates matrix
         */
        const mat4& get_matrix() const {
            return matrix_;
        }

        /**
         * \brief Sets the current transform.
         * \param[in] value the transform, as a 4x4 homogeneous-
         *  coordinates matrix
         * \TODO compute u,v,w,lookat from matrix.
         */
        void set_matrix(const mat4& value) {
            matrix_ = value;
        }

        /**
         * \brief Gets the rotational part of the current transform.
         * \return the rotational part of the current transform, 
         *  as a 4x4 homogeneous-coordinates matrix (with zero
         *  translation)
         */
        const mat4& get_rotation_matrix() const ;

        /**
         * \brief Sets the rotational part of the current transform.
         * \param[in] value the rotational part to be set in the current
         *  transform, as a 4x4 homogeneous-coordinates matrix (with zero
         *  translation)
         */
        void set_rotation_matrix(const mat4& value) ;

        /**
         * \brief Gets the default origin.
         * \return the default origin
         * \see reset()
         */
        const vec3& get_default_look_at() const {
            return default_look_at_;
        }

        /**
         * \brief Sets the default origin.
         * \param[in] value the default origin
         * \see reset()
         */
        void set_default_look_at(const vec3& value) {
            default_look_at_ = value;
        }

        /**
         * \brief Gets the default u vector
         * \return the default u vector.
         * \see reset()
         */
        const vec3& get_default_u() const {
            return default_u_;
        }

        /**
         * \brief Sets the default u vector.
         * \param[in] value the default u vector
         * \see reset()
         */
        void set_default_u(const vec3& value) {
            default_u_ = value;            
        }

        /**
         * \brief Gets the default v vector
         * \return the default v vector.
         * \see reset()
         */
        const vec3& get_default_v() const {
            return default_v_;
        }

        /**
         * \brief Sets the default v vector.
         * \param[in] value the default v vector
         * \see reset()
         */
        void set_default_v(const vec3& value) {
            default_v_ = value;
        }

        /**
         * \brief Gets the default w vector
         * \return the default w vector.
         * \see reset()
         */
        const vec3& get_default_w() const {
            return default_w_;
        }

        /**
         * \brief Sets the default w vector.
         * \param[in] value the default w vector
         * \see reset()
         */
        void set_default_w(const vec3& value) {
            default_w_ = value;
        }

        /**
         * \brief Gets the default zoom factor.
         * \return the default zoom factor
         * \see reset()
         */
        double get_default_zoom() const {
            return default_zoom_;
        }

        /**
         * \brief Sets the default zoom factor.
         * \param[in] value the default zoom factor
         * \see reset()
         */
        void set_default_zoom(double value) {
            default_zoom_ = value;
        }

     gom_slots:
        /**
         * \brief resets look_at, u, v, w and zoom
         *  to their default values.
         */
        void reset() ;

        /**
         * \brief translates to the left, along the u vector.
         * \param[in] value the length of the displacement, that will
         *  be multiplied by delta_location before being applied.
         */
        void move_left(double value = 1.0) ;

        /**
         * \brief translates to the right, along the u vector.
         * \param[in] value the length of the displacement, that will
         *  be multiplied by delta_location before being applied.
         */
        void move_right(double value = 1.0) ;

        /**
         * \brief translates to the up, along the v vector.
         * \param[in] value the length of the displacement, that will
         *  be multiplied by delta_location before being applied.
         */
        void move_up(double value = 1.0) ;

        /**
         * \brief translates to the bottom, along the v vector.
         * \param[in] value the length of the displacement, that will
         *  be multiplied by delta_location before being applied.
         */
        void move_down(double value = 1.0) ;

        /**
         * \brief translates to the front, along the w vector.
         * \param[in] value the length of the displacement, that will
         *  be multiplied by delta_location before being applied.
         */
        void move_forward(double value = 1.0) ;

        /**
         * \brief translates to the back, along the w vector.
         * \param[in] value the length of the displacement, that will
         *  be multiplied by delta_location before being applied.
         */
        void move_backward(double value = 1.0) ;

        /**
         * \brief Turn to the left.
         * \param[in] value the rotation angle, multiplied by delta_angle
         *  before being applied
         * \TODO not implemented yet!
         */
        void turn_left(double value = 1.0) ;

        /**
         * \brief Turn to the right.
         * \param[in] value the rotation angle, multiplied by delta_angle
         *  before being applied
         * \TODO not implemented yet!
         */ 
        void turn_right(double value = 1.0) ;

        /**
         * \brief Tilt towards the up.
         * \details Like pulling the stick in a plane.
         * \param[in] value the rotation angle, multiplied by delta_angle
         *  before being applied
         * \TODO not implemented yet!
         */
        void tilt_up(double value = 1.0) ;

        /**
         * \brief Tilt towards the bottom.
         * \details Like pushing the stick in a plane.
         * \param[in] value the rotation angle, multiplied by delta_angle
         *  before being applied
         * \TODO not implemented yet!
         */
        void tilt_down(double value = 1.0) ;

        /**
         * \brief Tilt towards the left.
         * \param[in] value the rotation angle, multiplied by delta_angle
         *  before being applied
         * \TODO not implemented yet!
         */
        void tilt_left(double value = 1.0) ;

        /**
         * \brief Tilt towards the right.
         * \param[in] value the rotation angle, multiplied by delta_angle
         *  before being applied
         * \TODO not implemented yet!
         */
        void tilt_right(double value = 1.0) ;

        /**
         * \brief Applies a translation.
         * \details This function is used to implement panning in Graphite's
         *  3D view.
         * \param[in] value the translation vector, only its X and Y
         *  components are used. They are multiplied by the zooming
         *  factor before beeing added to look_at. 
         */
        void translate(const vec3& value) ;

        /**
         * \brief Applies a rotation.
         * \param[in] value the rotational part of the transform is 
         *  composed with the specified rotation
         */  
        void rotate(const mat4& value) ;

        /**
         * \brief Zooms in.
         * \param[in] value number of times zooming factor
         *  is multiplied by delta_zoom.
         */
        void zoom_in(double value = 1.0) ;

        /**
         * \brief Zooms out.
         * \param[in] value number of times zooming factor
         *  is divided by delta_zoom.
         */
        void zoom_out(double value = 1.0) ;

        /**
         * \brief Applies a scaling to the current zoom.
         * \param[in] value scaling applied to the current
         *  zooming factor.
         */
        void zoom_mult(double value = 1.0) ;

    gom_signals:
        /**
         * \brief A signal that is triggered each time the
         *  transform changes.
         * \param[in] value the 3d transform, as a 4x4 homogeneous-
         *  coordinates matrix.
         */
        void value_changed(const mat4& value) ;

        /**
         * \brief A signal that is triggered each time the
         *  rotational component of the transform changes.
         * \param[in] value the rotation, as a 4x4 homogeneous-
         *  coordinates matrix, with zero translation.
         */
        void rotation_changed(const mat4& value) ;

    protected:
        /**
         * \brief Updates the matrix representation of the transform.
         * \param[in] send_signal if true, value_changed() and 
         *  rotation_changed() are triggered.
         */
        void update_matrix(bool send_signal = true) ;
    
    private:
        vec3 look_at_ ;
        vec3 u_ ;
        vec3 v_ ;
        vec3 w_ ;

        vec3 default_look_at_ ;
        vec3 default_u_ ;
        vec3 default_v_ ;
        vec3 default_w_ ;
        double default_zoom_ ;

        double delta_location_ ;
        double delta_angle_ ;
        double zoom_ ;
        double delta_zoom_ ;
        mat4 matrix_ ;
    } ;

//_________________________________________________________

}
#endif

