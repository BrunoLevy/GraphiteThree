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
 * As an exception to the GPL, Graphite can be linked with the 
 *  following (non-GPL) libraries: Qt, SuperLU, WildMagic and CGAL
 */
 
#ifndef H_OGF_VOXEL_INTERFACES_VOXEL_GROB_EDITOR_INTERFACE_H
#define H_OGF_VOXEL_INTERFACES_VOXEL_GROB_EDITOR_INTERFACE_H

#include <OGF/voxel/common/common.h>
#include <OGF/voxel/grob/voxel_grob.h>
#include <OGF/scene_graph/commands/commands.h>
#include <geogram/basic/attributes.h>

namespace OGF {

    namespace NL {
	class Vector;
    }
    
    /**
     * \brief A wrapper to script low-level editing operations 
     *  on a VoxelGrob.
     */
    gom_class VOXEL_API VoxelGrobEditor : public Interface {
      public:
	/**
	 * \brief VoxelGrobEditor constructor.
	 */
	VoxelGrobEditor();

	/**
	 * \brief VoxelGrobEditor destrutor.
	 */
	~VoxelGrobEditor() override;

	/**
	 * \brief Gets the wrapped VoxelGrob.
	 * \return a pointer to the VoxelGrob or nullptr.
	 */
	VoxelGrob* voxel_grob() const {
	    return dynamic_cast<VoxelGrob*>(grob());
	}
	
    gom_properties:
        
        const vec3& get_origin() const {
            return voxel_grob()->origin();
        }

        const vec3& get_U() const {
            return voxel_grob()->U();
        }

        const vec3& get_V() const {
            return voxel_grob()->V();
        }

        const vec3& get_W() const {
            return voxel_grob()->W();
        }

        index_t get_nu() const {
            return voxel_grob()->nu();
        }

        index_t get_nv() const {
            return voxel_grob()->nv();
        }

        index_t get_nw() const {
            return voxel_grob()->nw();
        }
        
	
      gom_slots:

	/**
	 * \brief Gets a wrapper around an attribute.
	 * \param[in] attribute_name the name of the attribute
	 * \param[in] quiet if true, do not display any error message if the
	 *  attribute does not exist.
	 * \return a pointer to the NL::Vector or nullptr if
	 *  there is no such attribute.
	 */
	NL::Vector* find_attribute(
	    const std::string& attribute_name, bool quiet=false
	);	

	/**
	 * \brief Creates an attribute.
	 * \param[in] attribute_name the name of the attribute/
	 * \param[in] dimension number of elements per item. Default is 1.
	 * \param[in] type optional meta type for the attribute elements. 
	 *  Default is double precision number.
	 * \return a wrapper around the newly created attribute.
	 */
	NL::Vector* create_attribute(
	    const std::string& attribute_name,
	    index_t dimension=1, MetaType* type=nullptr
	);

	/**
	 * \brief Creates an attribute.
	 * \param[in] attribute_name the name of the attribute.
	 * \param[in] dimension number of elements per item. Default is 1.
	 * \param[in] type optional meta type for the attribute elements. 
	 *  Default is double precision number.
	 * \return a wrapper around the attribute.
	 */
	NL::Vector* find_or_create_attribute(
	    const std::string& attribute_name,
	    index_t dimension=1, MetaType* type=nullptr
	);

	/**
	 * \brief Tests whether the voxel has an attribute.
	 * \param[in] attribute_name the name of the attribute.
	 * \retval true if the voxel has an attribute of the specified name.
	 * \retval false otherwise.
	 */
	bool has_attribute(const std::string& attribute_name) const;

	
      protected:

	/**
	 * \brief Checks whether VoxelGrob is valid.
	 * \details Displays an error message if not.
	 * \retval true if VoxelGrob is not nullptr.
	 * \retval false otherwise.
	 */
	bool check_voxel_grob() const;

	/**
	 * \brief Redisplays the wrapped VoxelGrob.
	 */
	void update();
    };
}

#endif
