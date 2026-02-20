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

#ifndef H_OGF_SCENE_GRAPH_GFX_FULL_SCREEN_EFFECTS_FSE_H
#define H_OGF_SCENE_GRAPH_GFX_FULL_SCREEN_EFFECTS_FSE_H

#include <OGF/scene_graph_gfx/common/common.h>
#include <OGF/gom/types/node.h>

/**
 * \file OGF/scene_graph_gfx/full_screen_effects/full_screen_effect.h
 * \brief Base classes for full screen effects.
 */

namespace GEO {
    class FullScreenEffectImpl;
}

namespace OGF {

    class SceneGraph;

    /**
     * \brief A Full screen effect.
     * \details FullScreenEffect is a wrapper around a FullScreenEffectImpl.
     */
    gom_attribute(abstract,"true")
    gom_class SCENE_GRAPH_GFX_API FullScreenEffect : public Object {
    public:

        /**
         * \brief FullScreenEffect constructor.
         */
        FullScreenEffect(SceneGraph* scene_graph);

        /**
         * \brief Gets the implementation.
         * \return a pointer to the FullScreenEffectImpl that implements this
         *  FullScreenEffect.
         */
        virtual FullScreenEffectImpl* implementation() = 0;

        /**
         * \brief Triggers an update of both the implementation and the 3D
         *  view.
         * \details Derived classes are supposed to call this function
         *  whenever a property is changed.
         */
        virtual void update();

	/**
	 * \copydoc Object::set_property
	 */
        bool set_property(
            const std::string& name, const Any& value
        ) override;

      private:
	SceneGraph* scene_graph_;
    };

    /**
     * \brief An automatic reference-counted pointer to a FulLScreenEffect.
     */
    typedef SmartPointer<FullScreenEffect> FullScreenEffect_var;

    /***************************************************************/

    /**
     * \brief The default dummy implementation of FullScreenEffect, that
     *  does nothing.
     */
    gom_class SCENE_GRAPH_GFX_API PlainFullScreenEffect :
	public FullScreenEffect {
    public:
        /**
         * \brief PlainFullScreenEffect constructor.
	 * \param[in] sg a pointer to the SceneGraph.
         */
        PlainFullScreenEffect(SceneGraph* sg);
        /**
         * \copydoc FullScreenEffect::implementation()
         * \details For this dummy FullScreenEffect, this function
         *  returns nil.
         */
         FullScreenEffectImpl* implementation() override;
    };

    /***************************************************************/
}

#endif
