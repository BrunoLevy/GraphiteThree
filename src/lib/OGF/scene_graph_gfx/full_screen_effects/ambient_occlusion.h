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

#ifndef H_OGF_SCENE_GRAPH_GFX_FULL_SCREEN_EFFECTS_AO_H
#define H_OGF_SCENE_GRAPH_GFX_FULL_SCREEN_EFFECTS_AO_H

#include <OGF/scene_graph_gfx/common/common.h>
#include <OGF/scene_graph_gfx/full_screen_effects/full_screen_effect.h>
#include <geogram_gfx/full_screen_effects/ambient_occlusion.h>

/**
 * \file OGF/scene_graph_gfx/full_screen_effects/ambient_occlusion.h
 * \brief GOM wrapper around AmbientOcclusionImpl.
 */

namespace OGF {

    /**
     * \brief GOM wrapper around AmbientOcclusionImpl.
     */
    gom_class SCENE_GRAPH_GFX_API AmbientOcclusion : public FullScreenEffect {
    public:
        /**
         * \brief AmbientOcclusion constructor.
	 * \param[in] scene_graph a pointer to the SceneGraph.
         */
        AmbientOcclusion(SceneGraph* scene_graph);

        /**
         * \copydoc FullScreenEffect::implementation()
         */
         FullScreenEffectImpl* implementation() override;

    gom_properties:

        /**
         * \copydoc AmbientOcclusionImpl::get_lightness()
         */
        index_t get_lightness() const {
            return impl_->get_lightness();
        }

        /**
         * \copydoc AmbientOcclusionImpl::set_lightness()
         */
        void set_lightness(index_t value) {
            impl_->set_lightness(value);
            update();
        }

        /**
         * \copydoc AmbientOcclusionImpl::get_contrast()
         */
        index_t get_contrast() const {
            return impl_->get_contrast();
        }

        /**
         * \copydoc AmbientOcclusionImpl::set_contrast()
         */
        void set_contrast(index_t value) {
            impl_->set_contrast(value);
            update();
        }

        /**
         * \copydoc AmbientOcclusionImpl::get_blur_width()
         */
        gom_attribute(help, "Size (in pixels) of the Gaussian blur kernel")
        index_t get_blur_width() const {
            return impl_->get_blur_width();
        }

        /**
         * \copydoc AmbientOcclusionImpl::set_blur_width()
         */
        void set_blur_width(index_t value) {
            impl_->set_blur_width(value);
            update();
        }

        /**
         * \copydoc AmbientOcclusionImpl::get_nb_directions()
         */
        gom_attribute(help, "The higher, the better (but the slower !!)")
        index_t get_nb_directions() const {
            return impl_->get_nb_directions();
        }

        /**
         * \copydoc AmbientOcclusionImpl::set_nb_directions()
         */
        void set_nb_directions(index_t value) {
            impl_->set_nb_directions(value);
            update();
        }

        /**
         * \copydoc AmbientOcclusionImpl::get_max_radius()
         */
	gom_attribute(help, "In 0.0 ... 1.0. The higher, the better (but the slower !!)")
	double get_max_radius() const {
	    return impl_->get_max_radius();
	}

        /**
         * \copydoc AmbientOcclusionImpl::set_max_radius()
         */
	void set_max_radius(double x) {
	    impl_->set_max_radius(x);
	    update();
	}

        /**
         * \copydoc AmbientOcclusionImpl::get_step_mul()
         */
	gom_attribute(help, "> 1.0. The smaller, the better (but the slower !!)")
	double get_step_mul() const {
	    return impl_->get_step_mul();
	}

        /**
         * \copydoc AmbientOcclusionImpl::set_step_mul()
         */
	void set_step_mul(double x) {
	    impl_->set_step_mul(x);
	    update();
	}

    private:
        AmbientOcclusionImpl_var impl_;
    };

}

#endif
