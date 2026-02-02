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


#ifndef H_OGF_SCENE_GRAPH_GFX_SHADERS_SCENE_GRAPH_SHDR_MGR_H
#define H_OGF_SCENE_GRAPH_GFX_SHADERS_SCENE_GRAPH_SHDR_MGR_H

#include <OGF/scene_graph_gfx/common/common.h>
#include <OGF/scene_graph_gfx/full_screen_effects/full_screen_effect.h>
#include <OGF/scene_graph/types/properties.h>
#include <map>

/**
 * \file OGF/scene_graph_gfx/shaders/scene_graph_shader_manager.h
 * \brief scene graph shaders management.
 */

namespace OGF {

    class Grob;
    class Shader;
    class ShaderManager;
    class SceneGraph;

    /**
     * \brief Manages the shaders and full screen effects for the entire
     *  SceneGraph.
     */
    gom_class SCENE_GRAPH_GFX_API SceneGraphShaderManager : public Object {
    public:

        /**
         * \brief SceneGraphShaderManager constructor
         */
        SceneGraphShaderManager();

	/**
	 * \brief SceneGraphShaderManager destructor.
	 */
	~SceneGraphShaderManager() override;

        /**
         * \brief Gets the Shader associated with a Grob and its properties
         * \param[in] grob a pointer to the Grob
         * \param[out] classname class name of the Shader
         * \param[out] args the properties of the Shader
         * \param[in] pointers if true, gets the properties with pointer
         *  types, else ignore them
         */
        void get_grob_shader(
            Grob* grob, std::string& classname,
            ArgList& args, bool pointers = true
        );

        /**
         * \brief Sets the Shader associated with a Grob and its properties
         * \param[in] grob a pointer to the Grob
         * \param[in] classname class name of the Shader
         * \param[in] args the properties of the Shader
         */
        void set_grob_shader(
            Grob* grob, const std::string& classname, const ArgList& args
        );


	/**
	 * \brief Gets the main Interpreter.
	 * \return a pointer to the main Interpreter.
	 */
	Interpreter* interpreter();

    gom_slots:
        /**
         * \brief Updates the focus matrix
         * \details Recomputes the focus matrix using the bounding box
         *  of the entire SceneGraph, or the bounding box of the current
         *  object if auto focus mode is active. Then it triggers a
         *  focus_changed() signal.
         */
        void update_focus();

        /**
         * \brief Notify this SceneGraphShaderManager that the
         *  current object changed.
         * \param[in] value name of the new current object
         */
        void current_object(const std::string& value);

        /**
         * \brief Change the shader associated with the current
         *  object.
         * \param[in] value the shader user name
         */
        void shader(const std::string& value);

        /**
         * \brief Draws the SceneGraph using the shaders associated
         *  with each object and the FullScreenEffect if one was
         *  specified.
         */
        void draw();


        /**
         * \brief Draws the SceneGraph using the picking shader,
         *  that will encode the id of each object in the backbuffer.
         */
        void pick_object();

        /**
         * \brief Copies the properties of the current Shader to all
         *  other Shaders in the SceneGraph associated with objects
         *  that have the same class as the current object.
         * \param[in] visible_only if set, copies the properties
         *  of the current shader only to objects that are visible.
         * \param[in] selected_only if set, copies the properties
         *  of the current shader only to objects that are selected.
         */
        void apply_to_scene_graph(
	    bool visible_only = false, bool selected_only = false
	);

        /**
         * \brief Gets the shader of the current object.
         * \return a pointer to the Shader of the current object
         */
        Shader* current();

	/**
	 * \brief Gets the current FullScreenEffect.
	 * \return a pointer to the current FullScreenEffect.
	 */
	FullScreenEffect* current_effect();

        /**
         * \brief Sets the FullScreenEffect.
         * \param[in] value the user name for the FullScreenEffect
         */
        void full_screen_effect(const std::string& value);


        /**
         * \brief Finds the ShaderManager associated with a given Grob.
         * \param[in] grob a pointer to the Grob
         * \return a pointer to the ShaderManager
         */
        ShaderManager* resolve_shader_manager(Grob* grob);

    gom_signals:
        /**
         * \brief a signal that is triggered whenever the focus matrix
         *  changes
         * \param[in] value the focus matrix
         */
        void focus_changed(const mat4& value);

    gom_properties:

        /**
         * \brief Sets autofocus mode.
         * \details In autofocus mode, the focus matrix adapts the
         *  camera to display the current object.
         * \param[in] value true if autofocus should be activated,
         *  false otherwise
         */
        void set_auto_focus(bool value) {
            auto_focus_ = value;
            update_focus();
        }


        /**
         * \brief Tests whether autofocus is active.
         * \details In autofocus mode, the focus matrix adapts the
         *  camera to display the current object.
         * \retval true if autofocus is active
         * \retval false otherwise
         */
        bool get_auto_focus() const {
            return auto_focus_;
        }

        /**
         * \brief Sets highlighting mode.
         * \details In highlighting mode, the selected object blinks
         *  each time it changes.
         * \param[in] value true if highlighting mode should be activated,
         *  false otherwise
         */
        void set_highlight_selected(bool value);

        /**
         * \brief Tests whether highlighting mode is active.
         * \details In highlighting mode, the selected object blinks
         *  each time it changes.
         * \retval true if highlighting mode is active
         * \retval false otherwise
         */
        bool get_highlight_selected() const {
            return highlight_selected_;
        }

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
        bool get_draw_selected_only() const {
            return draw_selected_only_;
        }

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
	 * \brief Gets the focus matrix.
	 * \details The focus matrix corresponds to the transform
	 *  to be applied to display the current object when autofocus
	 *  is set.
	 * \return a const reference to the focus matrix.
	 */
	const mat4& get_focus_matrix() const {
	    return focus_;
	}


    private:
        SceneGraph* scene_graph_;

        Grob* current_object_;

        bool auto_focus_;
        mat4 focus_;

        bool highlight_selected_;
        bool draw_selected_only_;

        std::string last_shader_;

        typedef std::map<std::string, FullScreenEffect_var> EffectsMap;
        EffectsMap effects_;

        FullScreenEffect* effect_;
	FullScreenEffectName user_effect_name_;
    };
}

#endif
