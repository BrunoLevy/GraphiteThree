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
 * As an exception to the GPL, Graphite can be linked with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/grob/mesh_grob.h>
#include <OGF/scene_graph/commands/commands.h>

/**
 * \file OGF/mesh/commands/mesh_grob_commands.h
 * \brief Base class for Commands related with a MeshGrob object.
 */
namespace OGF {

    /**
     * \brief Base class for Commands related with a MeshGrob object.
     */
    gom_attribute(abstract,"true") 
    gom_class MESH_API MeshGrobCommands : public Commands {
    public:

        /**
         * \brief MeshGrobCommands constructor.
         */
        MeshGrobCommands();

        /**
         * \brief MeshGrobCommands destructor.
         */
        ~MeshGrobCommands() override;

        /**
         * \brief Gets the MeshGrob
         * \return a pointer to the MeshGrob these Commands are 
         *  associated with
         */
        MeshGrob* mesh_grob() const {
            return dynamic_cast<MeshGrob*>(grob());
        }

    protected:

        /**
         * \brief Hides an attribute shown by show_attribute().
         */
        virtual void hide_attribute();
        
	/**
	 * \brief Shows an attribute.
	 * \param[in] attribute_name the name of the attribute to be
	 *  displayed, prefixed by the element (e.g., "vertices.density").
	 * \param[in] M an optional pointer to a MeshGrob
	 */
	virtual void show_attribute(
	    const std::string& attribute_name, MeshGrob* M = nullptr
	);


	/**
	 * \brief Shows the charts, stored in the "chart" facet attribute.
	 */
	virtual void show_charts();

	/**
	 * \brief Shows the mesh.
	 * \param[in] M an optional pointer to a MeshGrob
	 */
	virtual void show_mesh(MeshGrob* M = nullptr);


        /**
         * \nrief Shows the vertices.
         */
        virtual void show_vertices();
        

	/** 
	 * \brief Shows the parameterization of a mesh.
	 * \param[in] UV_attribute_name optional name of an attribute with
	 *   the texture coordinates
	 * \param[in] M an optional pointer to a MeshGrob
	 */
	virtual void show_UV(
	    const std::string& UV_attribute_name = "facet_corners.tex_coord",
	    MeshGrob* M = nullptr
	);

	/**
	 * \brief Shows the colors of a mesh
	 * \param[in] attribute optional name of the attribute with the colors
	 * \param[in] M an optional pointer to a MeshGrob
	 */
	virtual void show_colors(
	    const std::string& attribute = "vertices.colors",
	    MeshGrob* M = nullptr
	);
	
    };
}
#endif

