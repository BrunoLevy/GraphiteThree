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

#ifndef H_OGF_SCENE_GRAPH_GFX_SHADERS_SHADER_MANAGER_H
#define H_OGF_SCENE_GRAPH_GFX_SHADERS_SHADER_MANAGER_H

#include <OGF/scene_graph_gfx/common/common.h>
#include <OGF/scene_graph_gfx/shaders/shader.h>
#include <OGF/gom/types/node.h>

#include <map>

/**
 * \file OGF/scene_graph_gfx/shaders/shader_manager.h
 * \brief The class that manages the shaders associated with a Grob.
 */

namespace OGF {

    class SceneGraphShaderManager;

    /**
     * \brief Manages the Shader objects associated with a Grob.
     */
    gom_class SCENE_GRAPH_GFX_API ShaderManager : public Object {
    public:
        /**
         * \brief ShaderManager constructor.
         * \param[in] grob a pointer to the Grob
         * \param[in] sg_shader_manager a pointer to the SceneGraphShaderManager
         */
        ShaderManager(Grob* grob, SceneGraphShaderManager* sg_shader_manager);

        /**
         * \brief ShaderManager destructor.
         */
         ~ShaderManager() override;

    gom_slots:
        /**
         * \brief Gets the current Shader.
         * \return a pointer to the current Shader
         */
        Shader* current_shader() {
            return current_shader_;
        }

        /**
         * \brief Gets the Grob.
         * \return a pointer to the Grob
         */
        Grob* grob() {
            return grob_;
        }

        /**
         * \brief Draws the Grob using the current Shader.
         */
        void draw();

        /**
         * \brief Updates the current shader.
         * \details Updating the current shader copies all the values from
         *  the GUI element into the Shader's variables, and fires an update
         *  event that redraws the SceneGraph.
         */
        void update();

        /**
         * \brief Gets the SceneGraphShaderManager.
         * \return a pointer to the SceneGraphShaderManager
         */
        SceneGraphShaderManager* scene_graph_shader_manager() const {
            return sg_shader_manager_;
        }

    gom_properties:
        /**
         * \brief Sets the Shader by class name.
         * \param[in] value the Shader class name as a string, with
         *  the "OGF::" prefix.
         */
        void set_shader(const std::string& value);

        /**
         * \brief Gets the class name of the current shader.
         * \return the class name of the current shader as a string, with
         *  the "OGF::" prefix.
         */
        std::string get_shader() const;

	/**
	 * \brief Gets the current Shader object.
	 * \return a pointer to the current Shader object.
	 */
	gom_attribute(aggregate_properties,"true")
	Shader* get_shader_object() const {
	    return current_shader_;
	}

    private:
        Grob* grob_;
        SceneGraphShaderManager* sg_shader_manager_;
        Shader* current_shader_;
        /**
         * \brief Maps shader class names to Shader objects.
         */
        typedef std::map<std::string, Shader_var> ShaderMap;
        ShaderMap shaders_;
    };

    /**
     * \brief An automatic reference-counted pointer to a ShaderManager.
     */
    typedef SmartPointer<ShaderManager> ShaderManager_var;
}

#endif
