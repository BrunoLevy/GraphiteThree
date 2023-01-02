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
 

#ifndef H_OGF_GEOGRAMPLUG_GROB_MESH_GROB_H
#define H_OGF_GEOGRAMPLUG_GROB_MESH_GROB_H

#include <OGF/mesh/common/common.h>
#include <OGF/scene_graph/grob/grob.h>
#include <geogram/mesh/mesh.h>

/**
 * \file OGF/mesh/grob/mesh_grob.h
 * \brief the MeshGrob class.
 */
namespace OGF {

    /**
     * \brief A Grob wrapper around Geogram's Mesh class.
     */
    gom_class MESH_API MeshGrob : public Grob, public GEO::Mesh {
    public:
        /**
         * \brief MeshGrob constructor.
         * \param[in] parent a pointer to the container (the scenegraph 
         *  in most cases).
         */
        MeshGrob(CompositeGrob* parent);


	/**
	 * \brief MeshGrob destructor;
	 */
	~MeshGrob() override;
	
        /**
         * \copydoc Grob::load()
         */
        bool load(const FileName& value) override;

        /**
         * \copydoc Grob::append()
         */
        bool append(const FileName& value) override;

        /**
         * \copydoc Grob::save()
         */
        bool save(const NewFileName& value) override;

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
         * \return a ';'-separated list of all attributes. Each attribute
         *  name is prefixed by the subelement is is bound to.
         */
        std::string get_attributes() const;
	
        /**
         * \brief Gets the list of all scalar attributes.
         * \return a ';'-separated list of all scalar attributes. Each
         *  attribute name is prefixed by the subelement it is bound to. For
         *  vector attributes, all their components are listed.
         */
        std::string get_scalar_attributes() const;

        /**
         * \brief Gets the list of all selections.
         * \return a ';'-separated list of all selections. Each selection
         *  name is prefixed by the subelement is is bound to.
         */
        std::string get_selections() const;

        /**
         * \brief Gets the list of all filters.
         * \return a ';'-separated list of all selections. Each filter
         *  name is prefixed by the subelement is is bound to.
         */
        std::string get_filters() const;
        
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
         * \brief Finds or creates a MeshGrob with the specified name
         * \param[in] sg a pointer to the SceneGraph
         * \param[in] name the name
         * \return a pointer to the MeshGrob named as \p name in the
         *  SceneGraph \p sg if it exists, or a newly created MeshGrob
         *  otherwise.
         */
        static MeshGrob* find_or_create(
            SceneGraph* sg, const std::string& name
        );

        /**
         * \brief Finds a MeshGrob by name
         * \param[in] sg a pointer to the SceneGraph
         * \param[in] name the name
         * \return a pointer to the MeshGrob named as \p name in the
         *  SceneGraph \p sg if it exists, or nil otherwise.
         */
        static MeshGrob* find(SceneGraph* sg, const std::string& name);

        /**
         * \brief Registers all Geogram file extensions in Graphite.
         * \note This function is automatically called at Graphite startup,
         *  in the initialization function of the mesh library.
         */
        static void register_geogram_file_extensions();
        
    private:
    };

    /**
     * \brief The name of an existing MeshGrob in the SceneGraph.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with all the MeshGrob objects found in the SceneGraph.
     */
    typedef Name<MeshGrob*> MeshGrobName;

    /**
     * \brief The name of an (existing or not) MeshGrob in the SceneGraph.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with all the MeshGrob objects found in the SceneGraph.
     *  In additon, the field can be edited, and the user can enter in 
     *  it a new name, not already present in the SceneGraph.     
     */
    typedef Name<MeshGrob*,true> NewMeshGrobName;
}
#endif

