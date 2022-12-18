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
 
#ifndef H_OGF_SCENE_GRAPH_TYPES_TOOLS_MANAGER_H
#define H_OGF_SCENE_GRAPH_TYPES_TOOLS_MANAGER_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/tools/tool.h>
#include <OGF/gom/types/node.h>
#include <map>

/**
 * \file OGF/scene_graph/tools/tools_manager.h
 * \brief A class to manage the Tool objects associated with
 *  a given Grob class.
 */

namespace OGF {

    class Grob;
    class RenderingContext;
    class SceneGraphToolsManager;

    /**
     * \brief Manages the Tool objects associated with a given
     *  Grob class.
     */
    gom_attribute(abstract, "true") 
    gom_class SCENE_GRAPH_API ToolsManager : public Node {
    public:
        /**
         * \brief ToolsManager constructor.
         * \param[in] mgr a pointer to the SceneGraphToolsManager
         * \param[in] grob_class_name the Grob class name as a string,
         *  with the "OGF::" prefix
         * \param[in] context a pointer to the RenderingContext associated 
         *  with the main OpenGL window
         */
        ToolsManager(
            SceneGraphToolsManager* mgr,
            const std::string& grob_class_name, 
            RenderingContext* context
        );

        /**
         * \brief ToolsManager destructor.
         */
         ~ToolsManager() override;

        /**
         * \brief Notifies this ToolsManager that the current Grob
         *  of the SceneGraph changed.
         * \details This shows the buttons to select the Tool objects
         *  associated with the grob class that this ToolsManager is 
         *  connected to.
         * \param[in] grob a pointer to the new current Grob
         */
        void activate(Grob* grob);

        /**
         * \brief Notifies this ToolsManager that the current Grob 
         *  of the SceneGraph is no longer of the class that it 
         *  is connected to.
         * \details This hides the buttons used to select the Tools
         *  objects associated with the grob class that this ToolsManager
         *  is connected to.
         */
        void deactivate();

        /**
         * \brief Sets the current Grob.
         * \details The ToolsManager is supposed to be already activated,
         *  and the specified Grob is supposed to be of the class managed
         *  by this ToolsManager.
         * \param[in] grob a pointer to the current Grob.
         * \TODO More information needed here. 
         *  When and where is this function used?
         */
        void set_grob(Grob* grob);

        /**
         * \brief Gets the Grob class name associated with this ToolsManager.
         * \return the Grob class name, as a string, with the "OGF::" prefix
         */
        const std::string& grob_class_name() {
            return grob_class_name_;
        }

        /**
         * \brief Gets the current Grob.
         * \return a pointer to the current Grob.
         */
        Grob* object() const {
            return grob_;
        }

        /**
         * \brief Gets the RenderingContext.
         * \return a pointer to the RenderingContext
         */
        RenderingContext* rendering_context() const;

        /**
         * \brief Sets the RenderingContext.
         * \param[in] rc a pointer to the RenderingContext
         */
        void set_rendering_context(RenderingContext* rc) {
            rendering_context_ = rc;
        }

        /**
         * \brief Routes a mouse pick event to the current tool.
         * \param[in] ray a const reference to the picking event
         */
        void grab(const RayPick& ray);

        /**
         * \brief Routes a mouse drag event to the current tool.
         * \param[in] ray a const reference to the picking event
         */
        void drag(const RayPick& ray);

        /**
         * \brief Routes a mouse release event to the current tool.
         * \param[in] ray a const reference to the picking event
         */
        void release(const RayPick& ray);

        /**
         * \brief Gets the SceneGraphToolsManager.
         * \return a pointer to the SceneGraphToolsManager
         */
        SceneGraphToolsManager* manager() const {
            return manager_;
        }

        /**
         * \brief Gets the current Tool.
         * \return a pointer to the current Tool
         */
        Tool* current_tool() {
            return current_tool_;
        }

        /**
         * \brief Displays a status message.
         * \details The call is routed to the SceneGraphToolsManager::status()
         *  function, that in turns fires the 
         *  SceneGraphToolsManager::status_message() signal.
         * \param[in] value the message to be displayed
         */
        void status(const std::string& value);

    gom_slots:
        
        /**
         * \brief Sets the current Tool.
         * \param[in] value the class name of the Tool to be set,
         *  as a string, with the "OGF::" prefix
         */
        void set_tool(const std::string& value);

        /**
         * \brief Open a dialog to edit the properties of a Tool.
         * \param[in] value the class name of the Tool to be configured,
         *  as a string, with the "OGF::" prefix
         */
        void configure_tool(const std::string& value);

    public:
        /**
         * \brief Finds a Tool by its class name.
         * \details The first time the function is called, the Tool is created
         *  and stored in a std::map. It is retreived by subsequent calls to
         *  the function.
         * \param[in] tool_class_name the class name of the Tool to be found
         *  as a string, with the "OGF::" prefix
         * \return A pointer to the Tool
         */
        Tool* resolve_tool(const std::string& tool_class_name);

    private:
        SceneGraphToolsManager* manager_;
        std::string grob_class_name_;
        mutable RenderingContext* rendering_context_;
        Grob* grob_;
        Tool* current_tool_;

        typedef std::map<std::string, Tool_var> ToolMap;
        ToolMap tools_;
    };

    /**
     * \brief An automatic reference-counted pointer to a ToolsManager.
     */
    typedef SmartPointer<ToolsManager> ToolsManager_var;
}

#endif
