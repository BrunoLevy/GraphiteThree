
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

#ifndef H_OGF_MESH_TOOLS_MESH_GROB_FACET_TOOLS_H
#define H_OGF_MESH_TOOLS_MESH_GROB_FACET_TOOLS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/tools/mesh_grob_tool.h>

/**
 * \file OGF/mesh/tools/mesh_grob_facet_tools.h
 * \brief Tools to edit surface mesh facets.
 */

namespace OGF {

    /****************************************************************/

    /**
     * \brief A tool that creates a vertex in the center of a facet.
     */
    class MESH_API MeshGrobCreateCenterVertex : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobCreateCenterVertex constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobCreateCenterVertex(
            ToolsManager* parent
        ) : MeshGrobTool(parent),
            new_vertex_(NO_VERTEX) {
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
        index_t new_vertex_;
    };

    /**
     * \brief A tool that removes a vertex and merges all incident facets.
     */
    class MESH_API MeshGrobRemoveCenterVertex : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobRemoveCenterVertex constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobRemoveCenterVertex(
            ToolsManager* parent
        ) : MeshGrobTool(parent) {
        }
        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
    };


    /**
     * \brief A tool that creates/removes a vertex in a facet depending
     *  on the pushed mouse button.
     * \see MeshGrobCreateCenterVertex, MeshGrobRemoveCenterVertex.
     */
    gom_attribute(category, "facets")
    gom_attribute(icon, "create_center_vertex")
    gom_attribute(help, "create new vertex in facet / remove center vertex")
    gom_attribute(message,
                  "btn1: create center vertex; btn3: remove center vertex"
    )
    gom_class MESH_API MeshGrobEditCenterVertex : public MultiTool {
    public:
        /**
         * \brief MeshGrobEditCenterVertex constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobEditCenterVertex(ToolsManager* parent) : MultiTool(parent) {
            set_tool(
                MOUSE_BUTTON_LEFT, new MeshGrobCreateCenterVertex(parent)
            );
            set_tool(
                MOUSE_BUTTON_RIGHT, new MeshGrobRemoveCenterVertex(parent)
            );
        }
        /**
         * \copydoc Tool::reset()
         */
        void reset() override;
    };    
    

    /****************************************************************/

    /**
     * \brief A tool that removes all facets incident to a vertex.
     */
    gom_attribute(category, "facets")
    gom_attribute(icon, "remove_incident_facets")
    gom_attribute(help, "remove facets incident to a vertex")
    gom_attribute(message, "btn1: remove facets incident to vertex")
    gom_class MESH_API MeshGrobRemoveIncidentFacets : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobRemoveIncidentFacets constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobRemoveIncidentFacets(
            ToolsManager* parent
        ) : MeshGrobTool(parent) {
        }
        
        /**
         * \copydoc Tool::reset()
         */
        void reset() override;

        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
    };
    
    /****************************************************************/

    /**
     * \brief A tool that removes a facet.
     */
    class MESH_API MeshGrobRemoveFacet : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobRemoveFacet constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobRemoveFacet(ToolsManager* parent) : MeshGrobTool(parent) {
        }

        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
    };

    /**
     * \brief A tool that fills a hole by creating a new facet.
     */
    class MESH_API MeshGrobFillHole : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobFillHole constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobFillHole(ToolsManager* parent) : MeshGrobTool(parent) {
        }

        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
    };

    /**
     * \brief A tool that create or removes a facet, depending on the
     *  pushed mouse button.
     * \see MeshGrobRemoveFacet, MeshGrobFillHole.
     */
    gom_attribute(category, "facets")
    gom_attribute(icon, "fill_hole")
    gom_attribute(help, "fill hole / remove facet")
    gom_attribute(message, "btn1: fill hole; btn3: remove facet")
    gom_class MESH_API MeshGrobEditHole : public MultiTool {
    public:

        /**
         * \brief MeshGrobEditHole constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobEditHole(ToolsManager* parent) : MultiTool(parent) {
            set_tool(MOUSE_BUTTON_LEFT, new MeshGrobFillHole(parent));
            set_tool(MOUSE_BUTTON_RIGHT, new MeshGrobRemoveFacet(parent));
        }

	/**
	 * \brief MeshGrobEditHole destructor.
	 */
	~MeshGrobEditHole() override;
    };    

    /****************************************************************/

    /**
     * \brief A tool that translates/scales/rotates a facet depending on
     *  the pushed mouse button.
     */
    gom_attribute(category, "facets")
    gom_attribute(icon, "move_facet")
    gom_attribute(help, "move facet / resize facet / rotate facet")
    gom_attribute(message,
                  "btn1: move facet; btn2: resize facet; btn3: rotate facet"
    )
    gom_class MeshGrobTransformFacet : public MeshGrobTransformTool {
    public:
        /**
         * \brief MeshGrobTransformFacet constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobTransformFacet(
            ToolsManager* parent
        ) : MeshGrobTransformTool(parent) {
        }

    protected:
        /**
         * \copydoc MeshGrobTransformTool::pick_subset()
         */
        void pick_subset(
            MeshGrobTransformSubset* tool, const RayPick& rp
        ) override;

        /**
         * \copydoc MeshGrobTransformTool::transform_subset()
         */
        void transform_subset(const mat4& M) override;

        index_t picked_facet_;
    };

    /****************************************************************/

    /**
     * \brief A tool that joins two adjacent facet to form a single one.
     */
    class MeshGrobJoinFacets : public MeshGrobTool {
    public:

        /**
         * \brief MeshGrobJoinFacets constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobJoinFacets(
            ToolsManager* parent
        ) : MeshGrobTool(parent) {
        }

        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
    };


    /**
     * \brief A tool that splits a facet by creating a new edge between
     *  two picked vertices.
     */
    class MeshGrobSplitFacet : public MeshGrobTool {
    public:

        /**
         * \brief MeshGrobsSplitFacet constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobSplitFacet(
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

    /**
     * \brief A tool that splits/merges facets depending on the pushed
     *  mouse button.
     * \see MeshGrobSplitFacet, MeshGrobMergeFacets.
     */
    gom_attribute(category, "facets")
    gom_attribute(icon, "split_facet")
    gom_attribute(help, "split facet / merge facets")
    gom_attribute(message,
       "btn1: split facet (select vertices); btn3: merge facets (select edge)"
    )
    gom_class MESH_API MeshGrobEditFacetEdge : public MultiTool {
    public:
        /**
         * \brief MeshGrobEditFacetEdge constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobEditFacetEdge(ToolsManager* parent) : MultiTool(parent) {
            set_tool(MOUSE_BUTTON_LEFT, new MeshGrobSplitFacet(parent));
            set_tool(MOUSE_BUTTON_RIGHT, new MeshGrobJoinFacets(parent));
        }
        
        /**
         * \copydoc Tool::reset()
         */
        void reset() override;
    };    
}

#endif

