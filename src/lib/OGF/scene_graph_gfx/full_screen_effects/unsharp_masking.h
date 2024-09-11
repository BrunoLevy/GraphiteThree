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

#ifndef H_OGF_SCENE_GRAPH_GFX_FULL_SCREEN_EFFECTS_UM_H
#define H_OGF_SCENE_GRAPH_GFX_FULL_SCREEN_EFFECTS_UM_H

#include <OGF/scene_graph_gfx/common/common.h>
#include <OGF/scene_graph_gfx/full_screen_effects/full_screen_effect.h>
#include <geogram_gfx/full_screen_effects/unsharp_masking.h>

/**
 * \file OGF/scene_graph_gfx/full_screen_effects/unsharp_masking.h
 * \brief GOM wrapper around UnsharpMaskingImpl.
 */

namespace OGF {

    /**
     * \brief A GOM wrapper around UnsharpMaskingImpl.
     */
    gom_class SCENE_GRAPH_GFX_API UnsharpMasking : public FullScreenEffect {
    public:

        /**
         * \brief UnsharpMasking constructor.
	 * \param[in] scene_graph a pointer to the SceneGraph.
         */
        UnsharpMasking(SceneGraph* scene_graph);

        /**
         * \copydoc FullScreenEffect::implementation()
         */
         FullScreenEffectImpl* implementation() override;

    gom_properties:

        /**
         * \copydoc UnsharpMaskingImpl::get_intensity()
         */
        index_t get_intensity() const {
            return impl_->get_intensity();
        }

        /**
         * \copydoc UnsharpMaskingImpl::set_intensity()
         */
        void set_intensity(index_t value) {
            impl_->set_intensity(value);
            update();
        }

        /**
         * \copydoc UnsharpMaskingImpl::get_contrast()
         */
        index_t get_contrast() const {
            return impl_->get_contrast();
        }

        /**
         * \copydoc UnsharpMaskingImpl::set_contrast()
         */
        void set_contrast(index_t value) {
            impl_->set_contrast(value);
            update();
        }

        /**
         * \copydoc UnsharpMaskingImpl::get_blur_width()
         */
        gom_attribute(help, "Size (in pixels) of the Gaussian blur kernel")
        index_t get_blur_width() const {
            return impl_->get_blur_width();
        }

        /**
         * \copydoc UnsharpMaskingImpl::set_blur_width()
         */
        void set_blur_width(index_t value) {
            impl_->set_blur_width(value);
            update();
        }


        /**
         * \copydoc UnsharpMaskingImpl::get_halos()
         */
        gom_attribute(help, "Draw halos around the object")
        bool get_halos() const {
            return impl_->get_halos();
        }

        /**
         * \copydoc UnsharpMaskingImpl::set_halos()
         */
        void set_halos(bool value) {
            impl_->set_halos(value);
            update();
        }


        /**
         * \copydoc UnsharpMaskingImpl::get_positive_shadows()
         */
        gom_attribute(
            help, "Draw white shadows on zones opposite to dark shadows"
        )
        bool get_positive_shadows() const {
            return impl_->get_positive_shadows();
        }

        /**
         * \copydoc UnsharpMaskingImpl::set_positive_shadows()
         */
        void set_positive_shadows(bool value) {
            impl_->set_positive_shadows(value);
            update();
        }

    private:
        UnsharpMaskingImpl_var impl_;
    };

}

#endif
