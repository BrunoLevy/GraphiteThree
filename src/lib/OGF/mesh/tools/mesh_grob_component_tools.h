
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

#ifndef H_OGF_MESH_TOOLS_MESH_GROB_COMPONENT_TOOLS_H
#define H_OGF_MESH_TOOLS_MESH_GROB_COMPONENT_TOOLS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/tools/mesh_grob_tool.h>

/**
 * \file OGF/mesh/tools/mesh_grob_component_tools.h
 * \brief Tools to edit mesh components
 */

namespace OGF {

    /****************************************************************/

    /**
     * \brief A tool that translates/scales/rotates a mesh connected component
     *  depending on the pushed mouse button.
     */
    gom_attribute(category, "components")
    gom_attribute(icon, "move_component")
    gom_attribute(help, "move component / resize component / rotate component")
    gom_attribute(
        message,
        "btn1: move; btn2: resize; btn3: rotate"
    )
    gom_class MeshGrobTransformComponent : public MeshGrobTransformTool {
    public:
        /**
         * \brief MeshGrobTransformComponent constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobTransformComponent(
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

        /**
         * \copydoc MeshGrobTransformTool::clear_subset()
         */
        void clear_subset() override;
        
        vector<bool> v_is_picked_;
    };

    /****************************************************************/

    /**
     * \brief A tool that removes a connected component of a mesh.
     */
    class MeshGrobRemoveComponent : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobRemoveComponent constructor.
         * \param[in] parent a pointer to the ToolsManager
         * \param[in] invert_selection if false, the picked component is
         *  deleted. If true, everything but the picked component is deleted.
         */
        MeshGrobRemoveComponent(
            ToolsManager* parent, bool invert_selection = false
        ) : MeshGrobTool(parent),
            invert_selection_(invert_selection) {
        }
        
        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
    protected:
        bool invert_selection_;
    };

    /****************************************************************/

    /**
     * \brief A tool that removes a connected component of a mesh or 
     *  its complement depending on the pushed mouse button.
     * \see MeshGrobRemoveComponent
     */
    gom_attribute(category, "components")
    gom_attribute(icon, "remove_component")
    gom_attribute(help, "remove component / keep component")
    gom_attribute(
        message,
        "btn1: remove component; btn3: keep component"
    )
    gom_class MeshGrobKeepOrRemoveComponent : public MultiTool {
    public:
        /**
         * \brief MeshGrobRemoveOrKeepComponent constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobKeepOrRemoveComponent(
            ToolsManager* parent
        ) : MultiTool(parent) {
            set_tool(
                MOUSE_BUTTON_LEFT, new MeshGrobRemoveComponent(parent,false)
            );
            set_tool(
                MOUSE_BUTTON_RIGHT, new MeshGrobRemoveComponent(parent,true)
            );
        }

	/**
	 * \brief MeshGrobKeepOrRemoveComponent destructor.
	 */
	 ~MeshGrobKeepOrRemoveComponent() override;
    };

    /****************************************************************/

    /**
     * \brief A tool that copies and drags a connected component of a mesh.
     */
    gom_attribute(category, "components")
    gom_attribute(icon, "copy_component")
    gom_attribute(help, "copy component / delete component")
    gom_attribute(
        message,
        "btn1: copy component; btn3: remove component"
    )
    gom_class MeshGrobCopyComponent : public MeshGrobTransformTool {
    public:
        /**
         * \brief MeshGrobCopyComponent constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobCopyComponent(
            ToolsManager* parent
        ) : MeshGrobTransformTool(parent) {
            set_tool(
                MOUSE_BUTTON_MIDDLE, nullptr
            );
            set_tool(
                MOUSE_BUTTON_RIGHT, new MeshGrobRemoveComponent(parent,false)
            );            
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

    private:
        index_t first_new_vertex_;
    };


    /****************************************************************/        

    /**
     * \brief A tool that flips the normals of a connected component.
     */
    gom_attribute(category, "components")
    gom_attribute(icon, "flip_component")
    gom_attribute(help, "flip component")
    gom_attribute(message, "btn1: flip")
    gom_class MeshGrobFlipComponent : public MeshGrobTool {
    public:
        /**
         * \brief MeshGrobFlipComponent constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        MeshGrobFlipComponent(ToolsManager* parent) :
	   MeshGrobTool(parent) {
        }
        
        /**
         * \copydoc Tool::grab()
         */
	void grab(const RayPick& p_ndc) override;
    };

}

#endif
