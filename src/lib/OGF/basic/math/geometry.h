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

#ifndef H_OGF_BASIC_MATH_GEOMETRY_H
#define H_OGF_BASIC_MATH_GEOMETRY_H

/**
 * \file OGF/basic/math/geometry.h
 * \brief Simple geometric objects and manipulations.
 */

#include <OGF/basic/common/common.h>
#include <geogram/basic/geometry.h>

namespace OGF {

    namespace Geom {
        using namespace ::GEO::Geom;
    }
    
//_________________________________________________________

    
    /**
     * \brief Applies a 3d transform to a 2d point.
     * \details Convention is the same as in OpenGL, i.e.
     *  vector is a row vector, multiplied on the left
     *  of the transform.
     * This function is typically used to transform
     *  screen coordinates using the inverse viewing matrix.
     * \param[in] v the input 2d point to be transformed,
     *  considered as a 3d point with z coordinate equal to zero
     * \param[in] m the transform, as a 4x4 matrix, using
     *  homogeneous coordinates
     * \tparam FT type of the coordinates
     * \return the transformed 2d point
     */
    // TODO: there seems to be something wrong here,
    // depending on transform, can be a 3D point ??
    // Check usage in tools.
    template <class FT> vecng<2,FT> transform_point(
        const vecng<2,FT>& v,
        const Matrix<4,FT>& m
    ) {
        FT result[4] ;
        FT w[4] ;
        index_t i,j ;

        for(i=0; i<4; i++) {
            result[i] = 0 ;
        }
        w[0] = v.x ;
        w[1] = v.y ;
        w[2] = FT(0) ;
        w[3] = FT(1) ;

        for(i=0; i<4; i++) {
            for(j=0; j<4; j++) {
                result[i] += w[j] * m(j,i) ;
            }
        }
        
        return vecng<2,FT> (
            result[0] / result[3],
            result[1] / result[3]
        ) ; 
    }

/************************************************************/

    /**
     * \brief A 2d axis aligned box.
     */
    class Box2d {
    public:
        /**
         * \brief Constructs a new uninitialized Box2d
         */
        Box2d() : initialized_(false) {
        }

        /**
         * \brief Adds a point to this Box2d.
         * \param[in] p a const reference to the point
         *  to be added.
         */
        void add_point(const vec2& p) {
            if(!initialized_) {
                for(index_t c=0; c<2; ++c) {
                    xy_min_[c] = xy_max_[c] = p[c];
                }
            } else {
                for(index_t c=0; c<2; ++c) {
                    xy_min_[c] = std::min(xy_min_[c],p[c]);
                    xy_max_[c] = std::max(xy_max_[c],p[c]);                    
                }
            }
        }

        /**
         * \brief Adds a box to this Box2d.
         * \details On exit, this box is replaced with the
         *   union of its previous value and \p B.
         * \param[in] B a const reference to the box 
         *  to be added.
         */
        void add_box(const Box2d& B) {
            if(B.initialized_) {
                add_point(vec2(B.xy_min_));
                add_point(vec2(B.xy_max_));
            }
        }

        /**
         * \brief Gets the width of this box
         * \return the width (i.e., xmax() - xmin()) of this box
         */
        double width() const {
            return initialized_ ? (xy_max_[0] - xy_min_[0]) : 0.0;
        }

        /**
         * \brief Gets the height of this box
         * \return the width (i.e., ymax() - ymin()) of this box
         */
        double height() const {
            return initialized_ ? (xy_max_[1] - xy_min_[1]) : 0.0;
        }

        /**
         * \brief Gets the minimum x coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the minimum x coordinate
         * \see initialized()
         */
        double x_min() const {
            return xy_min_[0];
        }

        /**
         * \brief Gets the maximum x coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the maximum x coordinate
         * \see initialized()
         */
        double x_max() const {
            return xy_max_[0];
        }

        /**
         * \brief Gets the minimum y coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the minimum y coordinate
         * \see initialized()
         */
        double y_min() const {
            return xy_min_[1];
        }

        /**
         * \brief Gets the maximum y coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the maximum y coordinate
         * \see initialized()
         */
        double y_max() const {
            return xy_max_[1];
        }
        
        /**
         * \brief Tests whether the box is initialized
         * \details At creation, a box is uninitialized.
         *   It becomes initialized whenever a add_xxx()
         *  function is called.
         */
        bool initialized() const {
            return initialized_;
        }

	/**
	 * \brief Clears the box.
	 */
	void clear() {
	    initialized_ = false;
	}
	
    private:
        double xy_min_[2];
        double xy_max_[2];
        bool initialized_;
    };

//_________________________________________________________    

    /**
     * \brief A 3d axis aligned box.
     */
    class Box3d : public Box {
    public:
        /**
         * \brief Constructs a new uninitialized Box3d
         */
        Box3d() : initialized_(false) {
        }


        /**
         * \brief Tests whether the box is initialized
         * \details At creation, a box is uninitialized.
         *   It becomes initialized whenever a add_xxx()
         *  function is called.
         */
        bool initialized() const {
            return initialized_;
        }

        /**
         * \brief Adds a point to this Box3d.
         * \param[in] p a const reference to the point
         *  to be added.
         */
        void add_point(const vec3& p) {
            if(initialized_) {
                for(index_t c=0; c<3; ++c) {            
                    Box::xyz_min[c] = std::min(Box::xyz_min[c],p[c]);
                    Box::xyz_max[c] = std::max(Box::xyz_max[c],p[c]);
                }
            } else {
                for(index_t c=0; c<3; ++c) {
                    Box::xyz_min[c] = Box::xyz_max[c] = p[c];
                }
                initialized_ = true;
            }
        }

        /**
         * \brief Adds a box to this Box3d.
         * \details On exit, this box is replaced with the
         *   union of its previous value and \p B.
         * \param[in] B a const reference to the box 
         *  to be added.
         */
        void add_box(const Box3d& B) {
            if(B.initialized()) {
                add_point(vec3(B.xyz_min));
                add_point(vec3(B.xyz_max));
            }
        }

        /**
         * \brief Gets the center of this Box3d
         * \return A 3d point at the center of this Box3d
         */
        vec3 center() const {
            vec3 result;
            for(index_t c=0; c<3; ++c) {
                result[c] = 0.5*(Box::xyz_min[c] + Box::xyz_max[c]);
            }
            return result;
        }

        /**
         * \brief Gets the radius of this Box3d
         * \return The distance between the center and any corner
         *   of this Box3d
         */
        double radius() const {
            double result = 0.0;
            for(index_t c=0; c<3; ++c) {
                result += ogf_sqr(Box::xyz_max[c] - Box::xyz_min[c]);
            }
            return 0.5*::sqrt(result);
        }

        /**
         * \brief Gets the minimum x coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the minimum x coordinate
         * \see initialized()
         */
        double x_min() const {
            return xyz_min[0];
        }

        /**
         * \brief Gets the minimum y coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the minimum y coordinate
         * \see initialized()
         */
        double y_min() const {
            return xyz_min[1];
        }

        /**
         * \brief Gets the minimum z coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the minimum z coordinate
         * \see initialized()
         */
        double z_min() const {
            return xyz_min[2];
        }

        /**
         * \brief Gets the maximum x coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the maximum x coordinate
         * \see initialized()
         */
        double x_max() const {
            return xyz_max[0];
        }

        /**
         * \brief Gets the maximum y coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the maximum y coordinate
         * \see initialized()
         */
        double y_max() const {
            return xyz_max[1];
        }

        /**
         * \brief Gets the maximum z coordinate in this box
         * \details Result is undetermined if the box is 
         *   not initialized.
         * \return the maximum z coordinate
         * \see initialized()
         */
        double z_max() const {
            return xyz_max[2];
        }

	/**
	 * \brief Clears the box.
	 */
	void clear() {
	    initialized_ = false;
	}
	
    private:
        bool initialized_;
    };
}

#endif
