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
 *  with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_GRAPHITE_VOXEL_VOXEL_GROB_H
#define H_OGF_GRAPHITE_VOXEL_VOXEL_GROB_H

#include <OGF/voxel/common/common.h>
#include <OGF/scene_graph/grob/grob.h>
#include <geogram/basic/attributes.h>

/**
 * \file OGF/voxel/grob/voxel_grob.h
 * \brief the VoxelGrob class.
 */
namespace OGF {

    /**
     * \brief A Grob class for voxel grids.
     */
    gom_class VOXEL_API VoxelGrob : public Grob {
    public:
        /**
         * \brief VoxelGrob constructor.
         * \param[in] parent a pointer to the container (the scenegraph 
         *  in most cases).
         */
        VoxelGrob(CompositeGrob* parent);

        /**
         * \brief Gets the origin.
         * \return a const pointer to the origin
         */
        const vec3& origin() const {
            return origin_;
        }

        /**
         * \brief Gets the first axis.
         * \return a const reference to the first axis
         */
        const vec3& U() const {
            return U_;
        }

        /**
         * \brief Gets the second axis.
         * \return a const reference to the second axis
         */
        const vec3& V() const {
            return V_;
        }

        /**
         * \brief Gets the third axis.
         * \return a const reference to the third axis
         */
        const vec3& W() const {
            return W_;
        }

        /**
         * \brief Gets the size of the first axis.
         * \return the number of cells along the first axis
         */
        index_t nu() const {
            return nu_;
        }

        /**
         * \brief Gets the size of the second axis.
         * \return the number of cells along the second axis
         */
        index_t nv() const {
            return nv_;
        }

        /**
         * \brief Gets the size of the third axis.
         * \return the number of cells along the third axis
         */
        index_t nw() const {
            return nw_;
        }

        /**
         * \brief Defines the sizes in cells of the three axes.
         * \param[in] nu , nv , nw the sizes in cells of the three
         *  axes
         * \details All the attributes are deleted.
         */
        void resize(index_t nu, index_t nv, index_t nw);

        /**
         * \brief Sets the geometry of the box
         * \param[in] origin a const reference to the origin
         * \param[in] U a const reference to the first axis
         * \param[in] V a const reference to the second axis
         * \param[in] W a const reference to the third axis
         */
        void set_box(
            const vec3& origin, const vec3& U, const vec3& V, const vec3& W
        );

        /**
         * \brief Converts (u,v,w) indices into a linear index.
         * \details The linear index can be used to access properties.
         * \param[in] u , v , w the coordinates of the cell
         * \return the linear index of the cell
         */
        index_t linear_index(index_t u, index_t v, index_t w) const {
            geo_debug_assert(u < nu());
            geo_debug_assert(v < nv());
            geo_debug_assert(w < nw());
            return u + nu_ * (v + nv_ * w);
        }
        
        /**
         * \copydoc Grob::clear()
         */
        void clear() override;

        /**
         * \copydoc Grob::duplicate()
         */
        Grob* duplicate(SceneGraph* sg) override;

        /**
         * \copydoc Grob::is_serializable()
         */
        bool is_serializable() const override;

        /**
         * \copydoc Grob::serialize_read()
         */
        bool serialize_read(InputGraphiteFile& geofile) override;

        /**
         * \copydoc Grob::serialize_write()
         */
        bool serialize_write(OutputGraphiteFile& geofile) override;


    gom_properties:
        
        /**
         * \brief Gets the list of all attributes.
         * \return a ';'-separated list of all scalar attributes.
         */
        std::string get_displayable_attributes() const;


        
    public:
        /**
         * \copydoc Grob::update()
         */
        void update() override;

        
        /**
         * \copydoc Grob::bbox()
         */
        Box3d bbox() const override;

        /**
         * \brief Finds or creates a VoxelGrob with the specified name
         * \param[in] sg a pointer to the SceneGraph
         * \param[in] name the name
         * \return a pointer to the VoxelGrob named as \p name in the
         *  SceneGraph \p sg if it exists, or a newly created VoxelGrob
         *  otherwise.
         */
        static VoxelGrob* find_or_create(
            SceneGraph* sg, const std::string& name
        );

        /**
         * \brief Finds a VoxelGrob by name
         * \param[in] sg a pointer to the SceneGraph
         * \param[in] name the name
         * \return a pointer to the VoxelGrob named as \p name in the
         *  SceneGraph \p sg if it exists, or nullptr otherwise.
         */
        static VoxelGrob* find(SceneGraph* sg, const std::string& name);

        /**
         * \brief Gets the attributes manager.
         * \details The returned reference is not a const one,
         *  so that attributes can be bound / unbound / accessed
         *  even if the VoxelGrob is const.
         * \return a modifiable reference to the attributes manager.
         */
        AttributesManager& attributes() const {
            return const_cast<AttributesManager&>(attributes_);
        }

    private:
        AttributesManager attributes_;
        index_t nu_;
        index_t nv_;
        index_t nw_;
        vec3 origin_;
        vec3 U_;
        vec3 V_;
        vec3 W_;
    };

    /**
     * \brief The name of an existing VoxelGrob in the SceneGraph.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with all the VoxelGrob objects found in the SceneGraph.
     */
    typedef Name<VoxelGrob*> VoxelGrobName;

    /**
     * \brief The name of an (existing or not) VoxelGrob in the SceneGraph.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with all the VoxelGrob objects found in the SceneGraph.
     *  In additon, the field can be edited, and the user can enter in 
     *  it a new name, not already present in the SceneGraph.     
     */
    typedef Name<VoxelGrob*,true> NewVoxelGrobName;
}
#endif

