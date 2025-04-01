
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

#ifndef H_OGF_MESH_GFX_TOOLS_MESH_GROB_EDGE_TOOLS_H
#define H_OGF_MESH_GFX_TOOLS_MESH_GROB_EDGE_TOOLS_H

#include <OGF/mesh_gfx/common/common.h>
#include <OGF/mesh_gfx/tools/mesh_grob_tool.h>

/**
 * \file OGF/mesh_gfx/tools/mesh_grob_edge_tools.h
 * \brief Tools to edit surface mesh edges.
 */

namespace OGF {

    /****************************************************************/

    /**
     * \brief A tool that creates an edge between two vertices.
     */
    class MeshGrobCreateEdge : public MeshGrobTool {
    public:

        /**
         * \brief MeshGrobsCreateEdge constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobCreateEdge(
            ToolsManager* parent
        ) :
            MeshGrobTool(parent),
            v1_(NO_VERTEX),
            v2_(NO_VERTEX) {
        }

        /**
         * \copydoc Tool::grab()
         */
	void grab(const RayPick& p_ndc) override;

        /**
         * \copydoc Tool::reset()
         */
        void reset() override;
    private:
        index_t v1_;
        index_t v2_;
    };

    /****************************************************************/

    /**
     * \brief A tool that creates an edge between two vertices.
     */
    class MeshGrobDeleteEdge : public MeshGrobTool {
    public:

        /**
         * \brief MeshGrobsCreateEdge constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobDeleteEdge(
            ToolsManager* parent
        ) : MeshGrobTool(parent) {
        }

        /**
         * \copydoc Tool::grab()
         */
	void grab(const RayPick& p_ndc) override;
    };

    /****************************************************************/

    /**
     * \brief A tool that create/deletes edges.
     * \see MeshGrobCreateEdge, MeshObjDeleteEdge
     */
    gom_attribute(icon, "create_edge")
    gom_attribute(help, "create edge / delete edge")
    gom_attribute(message,
       "btn1: create edge (select vertices); btn3: delete edge (select edge)"
    )
    gom_class MESH_GFX_API MeshGrobEditEdge : public MultiTool {
    public:
        /**
         * \brief MeshGrobEditEdge constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobEditEdge(ToolsManager* parent) : MultiTool(parent) {
            set_tool(MOUSE_BUTTON_LEFT, new MeshGrobCreateEdge(parent));
            set_tool(MOUSE_BUTTON_RIGHT, new MeshGrobDeleteEdge(parent));
        }
    };


    /****************************************************************/
}

#endif
