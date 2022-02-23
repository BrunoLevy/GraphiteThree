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

#include <OGF/skin/transforms/transform3d.h>

namespace OGF {

//_________________________________________________________

    Transform3d::Transform3d() {

        default_look_at_ = vec3(0,0,0) ;
        default_u_ = vec3(1,0,0) ;
        default_v_ = vec3(0,1,0) ;
        default_w_ = vec3(0,0,1) ;
        default_zoom_ = 1.0 ;
    
        look_at_ = default_look_at_ ;
        u_ = default_u_ ;
        v_ = default_v_ ;
        w_ = default_w_ ;
        zoom_ = default_zoom_ ;
    
        delta_location_ = 0.05 ;
        delta_angle_ = 0.1 ;
        delta_zoom_ = 1.1 ;

        update_matrix(false) ;
    }

    Transform3d::~Transform3d() {
    }

    void Transform3d::reset() {
        look_at_ = default_look_at_ ;
        u_ = default_u_ ;
        v_ = default_v_ ;
        w_ = default_w_ ;
        zoom_ = default_zoom_ ;

        delta_location_ = 0.05 ;
        delta_angle_ = 0.1 ;
        delta_zoom_ = 1.1 ;
        update_matrix() ;
    }

    void Transform3d::move_left(double value) {
        look_at_ = look_at_ - value * delta_location_ * u_ ;
        update_matrix() ;
    }

    void Transform3d::move_right(double value) {
        look_at_ = look_at_ + value * delta_location_ * u_ ;
        update_matrix() ;
    }

    void Transform3d::move_up(double value) {
        look_at_ = look_at_ + value * delta_location_ * v_ ;
        update_matrix() ;
    }

    void Transform3d::move_down(double value) {
        look_at_ = look_at_ - value * delta_location_ * v_ ;
        update_matrix() ;
    }

    void Transform3d::move_forward(double value) {
        look_at_ = look_at_ + value * delta_location_ * w_ ;
        update_matrix() ;
    }

    void Transform3d::move_backward(double value) {
        look_at_ = look_at_ - value * delta_location_ * w_ ;
        update_matrix() ;
    }


    void Transform3d::turn_left(double value) {
        geo_argused(value);
//        double angle = value * delta_angle_ ;
        bool implemented = false ;
        ogf_assert(implemented) ;
        /// TODO ...
            update_matrix() ;
    }

    void Transform3d::turn_right(double value) {
        geo_argused(value);        
//        double angle = value * delta_angle_ ;
        bool implemented = false ;
        ogf_assert(implemented) ;
        /// TODO ...
            update_matrix() ;
    }

    void Transform3d::tilt_up(double value) {
        geo_argused(value);        
//        double angle = value * delta_angle_ ;
        bool implemented = false ;
        ogf_assert(implemented) ;
        /// TODO ...
            update_matrix() ;
    }

    void Transform3d::tilt_down(double value) {
        geo_argused(value);        
//        double angle = value * delta_angle_ ;
        bool implemented = false ;
        ogf_assert(implemented) ;
        /// TODO ...
            update_matrix() ;
    }

    void Transform3d::tilt_left(double value) {
        geo_argused(value);        
//        double angle = value * delta_angle_ ;
        bool implemented = false ;
        ogf_assert(implemented) ;
        /// TODO ...
            update_matrix() ;
    }

    void Transform3d::tilt_right(double value) {
        geo_argused(value);        
//        double angle = value * delta_angle_ ;
        bool implemented = false ;
        ogf_assert(implemented) ;
        /// TODO ...
            update_matrix() ;
    }

    void Transform3d::translate(const vec3& value) {
        vec3 v = zoom_ * value ;
        look_at_ = look_at_ + v.x * u_ + v.y * v_ ;
        update_matrix() ;
    }

    void Transform3d::rotate(const mat4& value) {
        // TODO: m = m * value ; extract u_, v_, w_ ; update matrix
        u_ = transform_vector(u_, value);
        v_ = transform_vector(v_, value);
        w_ = transform_vector(w_, value);
        update_matrix() ;
    }

    void Transform3d::zoom_in(double value) {
        if(value > 0) {
            for(int i=0; i<value; i++) {
                zoom_ /= delta_zoom_ ;
            }
        } else {
            for(int i=0; i<-value; i++) {
                zoom_ *= delta_zoom_ ;
            }
        }
        update_matrix() ;
    }

    void Transform3d::zoom_out(double value) {
        if(value > 0) {
            for(int i=0; i<value; i++) {
                zoom_ *= delta_zoom_ ;
            }
        } else {
            for(int i=0; i<-value; i++) {
                zoom_ /= delta_zoom_ ;
            }
        }
        update_matrix() ;
    }    

    void Transform3d::zoom_mult(double value) {
        zoom_ *= value ;
        update_matrix() ;
    }


    void Transform3d::update_matrix(bool send_signal) {
        matrix_.load_identity() ;
        matrix_(0,0) = u_.x ;
        matrix_(0,1) = u_.y ;
        matrix_(0,2) = u_.z ;
        matrix_(1,0) = v_.x ;
        matrix_(1,1) = v_.y ;
        matrix_(1,2) = v_.z ;
        matrix_(2,0) = w_.x ;
        matrix_(2,1) = w_.y ;
        matrix_(2,2) = w_.z ;
        matrix_(3,0) = look_at_.x ;
        matrix_(3,1) = look_at_.y ;
        matrix_(3,2) = look_at_.z ;
        matrix_ = matrix_.inverse() ;
        matrix_(3,2) += 0.5 ;
        matrix_(3,3) = zoom_ ;

        if(send_signal) {
            disable_slots() ;
            value_changed(matrix_) ;
            
            mat4 rotation ;
            rotation.load_identity() ;
            rotation(0,0) = u_.x ;
            rotation(0,1) = u_.y ;
            rotation(0,2) = u_.z ;
            rotation(1,0) = v_.x ;
            rotation(1,1) = v_.y ;
            rotation(1,2) = v_.z ;
            rotation(2,0) = w_.x ;
            rotation(2,1) = w_.y ;
            rotation(2,2) = w_.z ;
            rotation = rotation.inverse() ;
            
            rotation_changed(rotation) ;
            enable_slots() ;
        }
    }

    const mat4& Transform3d::get_rotation_matrix() const {
        static mat4 m ;
        m.load_identity() ;
        m(0,0) = u_.x ;
        m(0,1) = u_.y ;
        m(0,2) = u_.z ;
        m(1,0) = v_.x ;
        m(1,1) = v_.y ;
        m(1,2) = v_.z ;
        m(2,0) = w_.x ;
        m(2,1) = w_.y ;
        m(2,2) = w_.z ;
        m=m.inverse() ;
        return m ;
    }

    void Transform3d::set_rotation_matrix(const mat4& m) {
        mat4 m_inv = m.inverse() ;
        u_ = transform_vector(vec3(1,0,0), m_inv) ;
        v_ = transform_vector(vec3(0,1,0), m_inv) ;
        w_ = transform_vector(vec3(0,0,1), m_inv) ;
        update_matrix() ;
    }

//_________________________________________________________

}

