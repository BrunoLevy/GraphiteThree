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
 
#ifndef H_OGF_SCENE_GRAPH_TYPES_TOOL_H
#define H_OGF_SCENE_GRAPH_TYPES_TOOL_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/skin/transforms/ray_picker.h>
#include <OGF/gom/types/node.h>
#include <OGF/basic/math/geometry.h>

/**
 * \file OGF/scene_graph/tools/tool.h
 * \brief Base classes for the tools.
 */

namespace OGF {

    class ToolsManager;
    class Grob;
    class SceneGraph;

    /**
     * \brief Base class for the tools.
     */
    gom_attribute(abstract, "true") 
    gom_class SCENE_GRAPH_API Tool : public Node {
    public:

        /**
         * \brief Tool constructor.
         * \param[in] mgr a pointer to the ToolsManager
         */
        Tool(ToolsManager* mgr) : tools_manager_(mgr) {
        }

        /**
         * \brief Outputs a message in the status bar.
         * \details The call is routed to the SceneGraphToolsManager::status()
         *  function, that in turns fires the 
         *  SceneGraphToolsManager::status_message() signal.
         * \param[in] value the message to be displayed
         */
        void status(const std::string& value) ;

	/**
	 * \brief Gets the SceneGraph.
	 * \return a pointer to the SceneGraph.
	 */
	SceneGraph* scene_graph();
	
    gom_slots:
        /**
         * \brief The event handler for mouse pick events.
         * \param[in] value a const reference to the picking event
         */
        virtual void grab(const RayPick& value) ;

        /**
         * \brief The event handler for mouse drag events.
         * \param[in] value a const reference to the picking event
         */
        virtual void drag(const RayPick& value) ;

        /**
         * \brief The event handler for mouse release events.
         * \param[in] value a const reference to the picking event
         */
        virtual void release(const RayPick& value) ;

        /**
         * \brief Resets this Tool.
         * \details If this Tool has some state variables, this function
         *  can be overloaded in subclasses to reset it, for instance when
         *  the user changes the active Tool.
         */
        virtual void reset() ;

        /**
         * \brief Configures this Tool.
         * \details This opens a dialog that makes it possible to change
         *  all the gom_properties of this Tool.
         */
        virtual void configure() ;

    public:

        /**
         * \brief Gets the ToolsManager.
         * \return a pointer to the ToolsManager
         */
        ToolsManager* tools_manager() const {
            return tools_manager_ ;
        }

        /**
         * \brief Gets the current focus matrix.
         * \details Used to have the same transform for picking and for
         *  rendering.
         * \return a const reference to the current focus matrix
         */
        const mat4& focus() const ;

        /**
         * \brief Gets the RenderingContext.
         * \return a pointer to the RenderingContext
         */
        RenderingContext* rendering_context() const ;

        /**
         * \brief Gets the current object.
         * \return a pointer to the current Grob.
         */
        Grob* object() const ;

    protected:
        ToolsManager* tools_manager_ ;
    } ;

    /**
     * \brief An automatic reference-counted pointer to a Tool.
     */
    typedef SmartPointer<Tool> Tool_var ;
    
    //___________________________________________________________________

    /**
     * \brief A Tool that can associate up to three different
     *  tools to the three buttons of the mouse.
     */
    gom_attribute(abstract, "true") 
    gom_class SCENE_GRAPH_API MultiTool : public Tool {
    public:

        /**
         * \brief MultiTool constructor.
         * \param[in] mgr a pointer to the ToolsManager
         */
        MultiTool(ToolsManager* mgr) ;

        /**
         * \copydoc Tool::grab()
         * \details Routes the event to one of the (up to three)
         *  sub-tools, according to the pushed mouse button.
         */
         void grab(const RayPick& value) override;

        /**
         * \copydoc Tool::drag()
         * \details Routes the event to one of the (up to three)
         *  sub-tools, according to the pushed mouse button.
         */
	 void drag(const RayPick& value) override;

        /**
         * \copydoc Tool::release()
         * \details Routes the event to one of the (up to three)
         *  sub-tools, according to the pushed mouse button.
         */
	 void release(const RayPick& value) override;

        /**
         * \copydoc Tool::reset()
         * \details Resets the (up to three) sub-tools.
         */
	 void reset() override;
      
    protected:
        /**
         * \brief Associates a tool to one of the buttons.
         * \param[in] button one of (1,2,3)
         * \param[in] tool a pointer to the Tool. Ownership is
         *  transfered to this MultiTool.
         */
        void set_tool(int button, Tool* tool) ;

        /**
         * \brief Gets a Tool by button id.
         * \param[in] button one of (1,2,3)
         * \return a pointer to the Tool associated with button \p button
         *  or nil if there is no such Tool.
         */
        Tool* get_tool(int button) const ;
      
    protected:
        Tool_var tools_[3] ;
    } ; 

    //_________________________________________________________________

}

#endif
