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

#ifndef H_SKIN_IMGUI_CAMERA_H
#define H_SKIN_IMGUI_CAMERA_H

#include <OGF/skin_imgui/common/common.h>
#include <OGF/skin_imgui/widgets/render_area.h>
#include <OGF/scene_graph/types/properties.h>
#include <OGF/gom/types/node.h>
#include <geogram/image/color.h>

namespace OGF {

    class SceneGraphShaderManager;
    class Application;
    class FullScreenEffect;

    /**
     * \brief Clipping configuration
     */
    struct ClippingConfig {
	ClippingConfig() :
	    active(false),
	    axis('z'),
	    mode(GLUP_CLIP_STRADDLING_CELLS),
	    shift(0),
	    rotation{{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}},
	    invert(false) {
	}
	bool active;
	char axis;   /**< one of 'x', 'y', 'z', 'd' */
	GLUPclipMode mode;
	int shift;   /**< integer shifting of clipping plane along axis */
	mat4 rotation;
	bool invert;
    };


    /**
     * \brief Application attributes and methods related
     *  with camera management.
     * \details Camera does not store any information by
     *  itself. It only routes attributes and slots to
     *  other GOM objects (SceneGraphShaderManager and
     *  RenderArea). Gathering them in a single place
     *  makes it simpler to generate the GUI.
     */
    gom_class SKIN_IMGUI_API Camera : public Object {
      public:
	/**
	 * \brief Camera constructor.
	 * \param[in] app a pointer to the Application
	 */
	Camera(Application* app);

	/**
	 * \brief Camera destructor.
	 */
	 ~Camera() override;

      gom_properties:
        /**
         * \brief Sets autofocus mode.
         * \details In autofocus mode, the focus matrix adapts the
         *  camera to display the current object.
         * \param[in] value true if autofocus should be activated,
         *  false otherwise
         */
        void set_auto_focus(bool value);

        /**
         * \brief Tests whether autofocus is active.
         * \details In autofocus mode, the focus matrix adapts the
         *  camera to display the current object.
         * \retval true if autofocus is active
         * \retval false otherwise
         */
        bool get_auto_focus() const;

        /**
         * \brief Sets whether only the selected object should be drawn.
         * \param[in] value true if only the selected object is drawn,
         *  false if the whole scene graph is drawn
         */
        void set_draw_selected_only(bool value);

        /**
         * \brief Tests whether only the selected object should be drawn.
         * \retval true if only the selected object is drawn
         * \retval false if the whole scene graph is drawn
         */
        bool get_draw_selected_only() const;


        /**
         * \brief Sets the primary background color.
         * \param[in] value the background color
         */
        void set_bkg_color_1(const Color& value);

        /**
         * \brief Gets the background color.
         * \return the background color
         */
        const Color& get_bkg_color_1() const;

        /**
         * \brief Sets the secondary background color.
         * \details If different from the (primary) background
         *  color, then a color-ramp between both is generated.
         * \param[in] value the secondary background color
         */
        void set_bkg_color_2(const Color& value);

        /**
         * \brief Gets the secondary background color.
         * \details If different from the (primary) background
         *  color, then a color-ramp between both is generated.
         */
        const Color& get_bkg_color_2() const;

	/**
	 * \brief Sets the full screen effect.
	 * \param[in] effect the user effect name.
	 */
	void set_effect(const FullScreenEffectName& effect);

	/**
	 * \brief Gets the full screen effect.
	 * \return the user effect name.
	 */
	const FullScreenEffectName& get_effect() const;

	/**
	 * \brief Gets the full screen effect object.
	 * \return a pointer to the FullScreenEffect.
	 */
	gom_attribute(aggregate_properties,"true")
	FullScreenEffect* get_effect_properties() const;

        /**
         * \brief Sets the current clipping configuration
         * \param[in] value a const reference to a ClippingConfig.
         */
        void set_clipping(const ClippingConfig& value) {
	    clipping_config_ = value;
	    update_clipping_config();
	}

        /**
         * \brief Gets the current clipping configuration
	 * \return a const reference to a ClippingConfig
         */
        const ClippingConfig& get_clipping() const {
	    return clipping_config_;
	}

	/**
	 * \brief Gets the lighting matrix.
	 * \return the lighting matrix.
	 */
	gom_attribute(visible,"false")
	mat4 get_lighting_matrix() const;

	/**
	 * \brief Sets the lighting matrix.
	 * \param[in] value the new lighting matrix.
	 */
	void set_lighting_matrix(const mat4& value);

      gom_slots:

	/**
	 * \brief Gets the SceneGraphShaderManager
	 * \return a pointer to the SceneGraphShaderManager
	 */
	SceneGraphShaderManager* scene_graph_shader_manager() const;

      public:
	/**
	 * \copydoc Object::set_property
	 */
        bool set_property(
            const std::string& name, const Any& value
        ) override;

      protected:
	void update_clipping_config();

      private:
	Application* application_;
	ClippingConfig clipping_config_;
    };

    typedef SmartPointer<Camera> Camera_var;
}

#endif
