
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

#ifndef H_OGF_MESH_TOOLS_MESH_GROB_BORDER_TOOLS_H
#define H_OGF_MESH_TOOLS_MESH_GROB_BORDER_TOOLS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/tools/mesh_grob_tool.h>

/**
 * \file OGF/mesh/tools/mesh_grob_border_tools.h
 * \brief Tools to edit surface mesh borders.
 */

namespace OGF {

    /**
     * \brief A Tool that glues edges on the border.
     */
    class MeshGrobGlueEdges : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobGlueEdges constructor.
         * \param[in] manager a pointer to the ToolsManager
         */
        MeshGrobGlueEdges(ToolsManager* manager) : MeshGrobTool(manager) {
        }

        /**
         * \copydoc Tool::grab()
         */
         void grab(const RayPick& p_ndc) override;
    };

    /**
     * \brief A Tool that unglues two edges, thus forming a new border.
     */
    class MeshGrobUnglueEdges : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobUnGlueEdges constructor.
         * \param[in] manager a pointer to the ToolsManager
         */
        MeshGrobUnglueEdges(ToolsManager* manager) : MeshGrobTool(manager) {
        }

        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
    };

    /**
     * \brief A Tool that glues/unglues edges depending on the pushed
     *  mouse button.
     */
    gom_attribute(category, "borders")
    gom_attribute(icon, "cut_edges")
    gom_attribute(help, "glue edges / unglue edges")
    gom_attribute(message,
       "btn1: glue edge; btn3: unglue edge"
    )
    gom_class MeshGrobGlueUnglueEdges : public MultiTool {
    public:
        /**
         * \brief MeshGrobGlueUnglueEdges constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobGlueUnglueEdges(ToolsManager* parent) : MultiTool(parent) {
            set_tool(1, new MeshGrobGlueEdges(parent));            
            set_tool(3, new MeshGrobUnglueEdges(parent));
        }
        /**
         * \copydoc Tool::reset()
         */
         void reset() override;
    };

    /**
     * \brief A Tool that zips two edges on the border, starting from
     *  their common vertex.
     */
    class MeshGrobZipEdges : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobZipEdges constructor.
         * \param[in] manager a pointer to the ToolsManager
         */
        MeshGrobZipEdges(ToolsManager* manager) : MeshGrobTool(manager) {
        }

        /**
         * \copydoc Tool::grab()
         */
         void grab(const RayPick& p_ndc) override;
    };


    /**
     * \brief A Tool that zips/unzips edges depending on the pushed
     *  mouse button.
     */
    gom_attribute(category, "borders")
    gom_attribute(icon, "zip_edge")
    gom_attribute(help, "zip edges / unzip edges")
    gom_attribute(message,
       "btn1: zip edges; btn3: unzip edge"
    )
    gom_class MeshGrobZipUnzipEdges : public MultiTool {
    public:
        /**
         * \brief MeshGrobZipUnzipEdges constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobZipUnzipEdges(ToolsManager* parent) : MultiTool(parent) {
            set_tool(1, new MeshGrobZipEdges(parent));
            set_tool(3, new MeshGrobUnglueEdges(parent));
        }
        
        /**
         * \copydoc Tool::reset()
         */
         void reset() override;
    };


    /**
     * \brief A Tool that connects two picked mesh facet edges on
     *  the border.
     */
    class MeshGrobConnectEdges : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobConnectEdges constructor.
         * \param[in] manager a pointer to the ToolsManager
         */
        MeshGrobConnectEdges(ToolsManager* manager) : MeshGrobTool(manager) {
            f_ = NO_FACET;
            c_ = NO_CORNER;
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
        index_t f_;
        index_t c_;
    };


    /**
     * \brief A Tool that zips/unzips edges depending on the pushed
     *  mouse button.
     */
    gom_attribute(category, "borders")
    gom_attribute(icon, "connect_edges")
    gom_attribute(help, "connect edges / disconnect edges")
    gom_attribute(message,
       "btn1: connect edges; btn3: disconnect edges"
    )
    gom_class MeshGrobConnectDisconnectEdges : public MultiTool {
    public:
        /**
         * \brief MeshGrobConnectDisconnectEdges constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobConnectDisconnectEdges(
            ToolsManager* parent
        ) : MultiTool(parent) {
            set_tool(1, new MeshGrobConnectEdges(parent));
            set_tool(3, new MeshGrobUnglueEdges(parent));
        }
        
        /**
         * \copydoc Tool::reset()
         */
        void reset() override;
    };
}

#endif
