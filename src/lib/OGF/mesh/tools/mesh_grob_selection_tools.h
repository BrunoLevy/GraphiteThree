
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
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
 *  Contact: Bruno Levy - levy@loria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked 
 *  with the following (non-GPL) libraries: Qt, SuperLU, WildMagic and CGAL
 */

#ifndef H_OGF_MESH_TOOLS_MESH_GROB_SELECTION_TOOLS_H
#define H_OGF_MESH_TOOLS_MESH_GROB_SELECTION_TOOLS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/tools/mesh_grob_tool.h>

/**
 * \file OGF/mesh/tools/mesh_grob_selection_tools.h
 * \brief Tools to manipulate mesh selections.
 */

namespace OGF {

    /**
     * \brief A tool that selects a vertex.
     */
    class MESH_API MeshGrobSelectVertex : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobSelectTool
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobSelectVertex(
            ToolsManager* parent
	) :
	    MeshGrobTool(parent),
	    vertex_(index_t(-1)) {
        }
        
        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;

        /**
         * \copydoc Tool::drag()
         */
        void drag(const RayPick& p_ndc) override;
	
      private:
	index_t vertex_;
    };

    /**
     * \brief A tool that unselects a vertex.
     */
    class MESH_API MeshGrobUnselectVertex : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobSelectTool
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobUnselectVertex(
            ToolsManager* parent
        ) : MeshGrobTool(parent) {
            
        }
        
        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
    };
    

    /**
     * \brief A tool that selects/unselects a vertex depending 
     *  on the pushed mouse button.
     * \see MeshGrobSelectVertex, MeshGrobUnselectVertex.
     */
    gom_attribute(category, "selection")
    gom_attribute(icon, "select_vertex")
    gom_attribute(help, "select vertex / unselect vertex")
    gom_attribute(message,
                  "btn1: select/move vertex; btn3: unselect vertex"
    )
    gom_class MESH_API MeshGrobSelectUnselectVertex : public MultiTool {
    public:
        /**
         * \brief MeshGrobSelectUnselectVertex constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobSelectUnselectVertex(ToolsManager* parent) : MultiTool(parent) {
            set_tool(MOUSE_BUTTON_LEFT, new MeshGrobSelectVertex(parent));
            set_tool(MOUSE_BUTTON_RIGHT, new MeshGrobUnselectVertex(parent));
        }

        /**
         * \copydoc Tool::reset()
         */
	void reset() override;
    };    
    
}

#endif

