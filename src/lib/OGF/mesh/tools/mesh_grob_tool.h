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
 * As an exception to the GPL, Graphite can be linked with the following 
 *  (non-GPL) libraries: Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_MESH_TOOLS_MESH_GROB_TOOL_H
#define H_OGF_MESH_TOOLS_MESH_GROB_TOOL_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/grob/mesh_grob.h>
#include <OGF/scene_graph/tools/tool.h>
#include <OGF/scene_graph/tools/tools_manager.h>
#include <OGF/skin/transforms/arc_ball.h>

/**
 * \file OGF/mesh/tools/mesh_grob_tool.h
 * \brief Base classes for tools that operate on MeshGrob
 */

namespace OGF {

    /*******************************************************************/

    /**
     * \brief Base class for tools that operate on MeshGrob
     */
    gom_attribute(abstract,"true") 
    gom_class MESH_API MeshGrobTool : public Tool {
    public:

        /**
         * \brief MeshGrobTool constructor.
         * \param[in] parent the ToolsManager this MeshGrobTool belongs to
         */
        MeshGrobTool(ToolsManager* parent) ;

        /**
         * \brief MeshGrobTool destructor.
         */
        ~MeshGrobTool() override;
	
        /**
         * \brief Gets the MeshGrob this MeshGrobTool operates on
         * \return a pointer to the MeshGrob
         */
        MeshGrob* mesh_grob() const { 
            return dynamic_cast<MeshGrob*>(object()) ;              
        }

        /**
         * \brief Picks a vertex
         * \details The picked point, depth and normalized device coordinates
         *  can then be queried by using picked_point(), picked_depth(), 
         *  picked_ndc() respectively.
         * \param[in] rp a RayPick as returned by RenderArea events
         * \return the index of the picked vertex or NO_VERTEX if no
         *  vertex was picked.
         */
        index_t pick_vertex(const RayPick& rp);

        /**
         * \brief Picks a edge
         * \details Edges are the ones that are explicitly stored in the mesh. 
         *  To pick an edge of a facet, use pick_facet_edge() instead.
         *  The picked point, depth and normalized device coordinates
         *  can then be queried by using picked_point(), picked_depth(), 
         *  picked_ndc() respectively.
         * \param[in] rp a RayPick as returned by RenderArea events
         * \return the index of the picked edge or NO_EDGE if no
         *  edge was picked.
         */
        index_t pick_edge(const RayPick& rp);

        /**
         * \brief Picks a facet
         * \details The picked point, depth and normalized device coordinates
         *  can then be queried by using picked_point(), picked_depth(), 
         *  picked_ndc() respectively.
         * \param[in] rp a RayPick as returned by RenderArea events
         * \return the index of the picked facet or NO_FACET if no
         *  facet was picked.
         */
        index_t pick_facet(const RayPick& rp);

        /**
         * \brief Picks a cell
         * \details The picked point, depth and normalized device coordinates
         *  can then be queried by using picked_point(), picked_depth(), 
         *  picked_ndc() respectively.
         * \param[in] rp a RayPick as returned by RenderArea events
         * \return the index of the picked cell or NO_CELL if no
         *  cell was picked.
         */
        index_t pick_cell(const RayPick& rp);

        /**
         * \brief Picks an edge of a facet
         * \details The picked point, depth and normalized device coordinates
         *  can then be queried by using picked_point(), picked_depth(), 
         *  picked_ndc() respectively.
         * \param[in] rp a RayPick as returned by RenderArea events
         * \param[out] facet the picked facet
         * \param[out] corner the first facet corner of the picked edge
         * \retval true if a facet edge was picked
         * \retval false otherwise
         */
        bool pick_facet_edge(
            const RayPick& rp, index_t& facet, index_t& corner
        );

        /**
         * \brief Picks an element of the mesh
         * \details The picked point, depth and normalized device coordinates
         *  can then be queried by using picked_point(), picked_depth(), 
         *  picked_ndc() respectively.
         * \param[in] rp a RayPick as returned by RenderArea events
         * \param[in] what one of (MESH_VERTICES, MESH_EDGES, MESH_FACETS, 
         *  MESH_CELLS). It is also possible to use a bitwise-or combination
         *  of them (but then it is no longer possible to know what element
         *  type was picked).
         * \param[in] image a pointer to an optional image to get the picking 
         *  framebuffer (can be used by selection tools). The image is supposed
         *  to be uninitialized. The function allocates it to the correct size
         *  and reads the pixels from the picking buffer.
         * \param[in] x0 , y0 , width , height optional image bounds. If let
         *  unspecified, the entire picking buffer is copied.
         * \return the index of the picked element or index_t(-1) if nothing
         *  was picked.
         */
        index_t pick(
            const RayPick& rp, MeshElementsFlags what,
            Image* image=nullptr,
            index_t x0=0, index_t y0=0,
            index_t width=0, index_t height=0
        );

        /**
         * \brief Gets the 3D coordinate of a point dragged from the latest
         *  picked point to the actual mouse position in a direction parallel
         *  to the screen.
         * \param[in] rp a RayPick as returned by RenderArea events
         * \return the dragged point
         */
        vec3 drag_point(const RayPick& rp) const;

        /**
         * \brief Gets the picked point in world coordinates.
         * \return a const reference to the 3D world coordinates 
         *  of the latest point picked by one of the pick_xxx() functions.
         */
        const vec3& picked_point() const {
            return picked_point_;
        }

        /**
         * \brief Gets the picked point in normalized device coordinates.
         * \return a const reference to the normalized device coordinates
         *  (x and y in [-1.0,1.0]) of the latest point picked by one 
         *  of the pick_xxx() functions.
         */
        const vec2& picked_ndc() const {
            return picked_ndc_;
        }

        /**
         * \brief Gets the depth of the picked point.
         * \return the depth of the latest point picked by one 
         *  of the pick_xxx() functions.
         */
        double picked_depth() const {
            return picked_depth_;
        }

        
    protected:
        vec3 picked_point_;
        vec2 picked_ndc_;
        double picked_depth_;
    } ;

    /*******************************************************************/

    class MeshGrobTransformTool;

    /**
     * \brief Base class of tools that apply 3d transform to a subset
     *  of a MeshGrob and that are attached to a MultiTool 
     *  (or a MeshGrobTransformTool).
     */
    class MeshGrobTransformSubset : public MeshGrobTool {
    public:

        /**
         * \brief MeshGrobTransformSubset constructor.
         * \param[in] parent a pointer to the MeshGrobTransformTool (MultiTool)
         *  this MeshGrobTransformSubset belongs to.
         */
        MeshGrobTransformSubset(MeshGrobTransformTool* parent);

        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
        
    protected:
        /**
         * \brief Applies the transform M to the subset.
         * \details The transform /p M is applied to the subset as
         *  it was when grab() was called (any previously applied transform
         *  is canceled).
         * \param[in] M the transform to be applied
         */
        void update_transform_subset(const mat4& M);

        /**
         * \brief Gets the center of the subset.
         */
        const vec3& center() const;
        
        MeshGrobTransformTool* transform_tool_;
    };

    /**
     * \brief Applies a mouse-controlled translation to a subset.
     */
    class MeshGrobMoveSubset : public MeshGrobTransformSubset {
    public:
        /**
         * \brief MeshGrobMoveSubset constructor.
         * \param[in] parent a pointer to the MeshGrobTransformTool (MultiTool)
         *  this MeshGrobMoveSubset belongs to.
         */
        MeshGrobMoveSubset(
            MeshGrobTransformTool* parent
        ) : MeshGrobTransformSubset(parent) {
        }
        
        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
        
        /**
         * \copydoc Tool::drag()
         */
        void drag(const RayPick& p_ndc) override;
    };

    /**
     * \brief Applies a mouse-controlled scaling to a subset.
     */
    class MeshGrobResizeSubset : public MeshGrobTransformSubset {
    public:

        /**
         * \brief MeshGrobResizeSubset constructor.
         * \param[in] parent a pointer to the MeshGrobTransformTool (MultiTool)
         *  this MeshGrobResizeSubset belongs to.
         */
        MeshGrobResizeSubset(
            MeshGrobTransformTool* parent
        ) : MeshGrobTransformSubset(parent) {
        }
        
        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;

        /**
         * \copydoc Tool::drag()
         */
        void drag(const RayPick& p_ndc) override;
    };

    /**
     * \brief Applies a mouse-controlled rotation to a subset.
     */
    class MeshGrobRotateSubset : public MeshGrobTransformSubset {
    public:

        /**
         * \brief MeshGrobRotateSubset constructor.
         * \param[in] parent a pointer to the MeshGrobTransformTool (MultiTool)
         *  this MeshGrobRotateSubset belongs to.
         */
        MeshGrobRotateSubset(
            MeshGrobTransformTool* parent
        ) : MeshGrobTransformSubset(parent),
            arc_ball_(new ArcBall) {
        }

        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;

        /**
         * \copydoc Tool::drag()
         */
        void drag(const RayPick& p_ndc) override;

        /**
         * \copydoc Tool::release()
         */
        void release(const RayPick& p_ndc) override;
        
    protected:
        ArcBall_var arc_ball_;
    };

    /**
     * \brief A MultiTool that applies a mouse-controlled transform to a subset.
     * \details Left mouse button controls a translation, center mouse button
     *  controls a scaling, and right mouse button controls a rotation.
     */
    gom_attribute(abstract, "true")
    gom_class MeshGrobTransformTool : public MultiTool {
    public:
        MeshGrobTransformTool(ToolsManager* parent) : MultiTool(parent) {
            set_tool(1, new MeshGrobMoveSubset(this));
            set_tool(2, new MeshGrobResizeSubset(this));
            set_tool(3, new MeshGrobRotateSubset(this));
            prev_inverse_transform_.load_identity();
        }

        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;

        /**
         * \copydoc Tool::release()
         */
        void release(const RayPick& p_ndc) override;

        /**
         * \brief Gets the MeshGrob this MeshGrobTransformTool operates on
         * \return a pointer to the MeshGrob
         */
        MeshGrob* mesh_grob() const { 
            return dynamic_cast<MeshGrob*>(object()) ;              
        }
        
    protected:

        /**
         * \brief Applies the transform M to the subset.
         * \details The transform /p M is applied to the subset as
         *  it was when grab() was called (any previously applied transform
         *  is canceled).
         * \param[in] M the transform to be applied
         */
        void update_transform_subset(const mat4& M) {
            mat4 MM = M * prev_inverse_transform_;
            transform_subset(MM);
            prev_inverse_transform_ = M.inverse();
        }

        /**
         * \brief Gets the center of the subset.
         */
        const vec3& center() const {
            return center_;
        }

        /**
         * \brief Gets the subset by picking.
         * \details This function is called when the mouse is pressed.
         *  It is meant to be overloaded in derived classes, to memorize
         *  the picked subset in some state variables.
         * \param[in] tool a pointer to the MeshGrobTransformSubset
         * \param[in] rp a RayPick as returned by RenderArea events
         */
        virtual void pick_subset(
            MeshGrobTransformSubset* tool, const RayPick& rp
        ) = 0;

        /**
         * \brief Applies a 3D transform to the subset.
         * \param[in] M a const reference of the homogeneous coordinates
         *  matrix of the transform.
         */
        virtual void transform_subset(const mat4& M) = 0;

        /**
         * \brief Clears the state variables that store the subset.
         * \details Default implementation does nothing.
         */
        virtual void clear_subset();

        /**
         * \brief The inverse of the previous transform.
         * \details It is used when the transform is updated, to
         *  undo the previous transform.
         */
        mat4 prev_inverse_transform_;
        friend class MeshGrobTransformSubset;
        vec3 center_;
    };
} 


#endif

