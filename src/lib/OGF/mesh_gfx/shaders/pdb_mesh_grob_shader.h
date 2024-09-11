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
 * As an exception to the GPL, Graphite can be linked with
 *  the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#ifndef H_OGF_MESH_GFX_SHADERS_MESH_GROB_PDB_SHADER_H
#define H_OGF_MESH_GFX_SHADERS_MESH_GROB_PDB_SHADER_H

#include <OGF/mesh_gfx/common/common.h>
#include <OGF/mesh_gfx/shaders/mesh_grob_shader.h>

/**
 * \file OGF/mesh_gfx/shaders/mesh_grob_pdb_shader.h
 * \brief Classes for drawing PDB (Protein DataBase) files
 */
namespace OGF {

    gom_class MESH_GFX_API PDBMeshGrobShader : public MeshGrobShader {
    public:

        /**
         * \brief PDBMeshGrobShader constructor.
         * \param[in] grob a pointer to the MeshGrob this shader is attached to
         */
        PDBMeshGrobShader(MeshGrob* grob);

        /**
         * \brief PDBMeshGrobShader destructor.
         */
         ~PDBMeshGrobShader() override;


	enum AtomColoring { constant, atom, chain };

    gom_properties:
        /**
         * \brief Sets whether lighting should be used.
         * \param[in] value true if lighting is enabled, false
         *  otherwise
         */
        void set_lighting(bool value) {
            lighting_ = value;
            update();
        }

        /**
         * \brief Gets whether lighting is used.
         * \retval true if lighting is used
         * \retval false otherwise
         */
        bool get_lighting() const {
	    return lighting_;
        }

	void set_atom_colors(AtomColoring x) {
	    atom_colors_ = x;
	    update();
	}

	AtomColoring get_atom_colors() const {
	    return atom_colors_;
	}

	void set_atom_size(index_t value) {
	    atom_size_ = value;
	    update();
	}

	index_t get_atom_size() const {
	    return atom_size_;
	}

      public:

        /**
         * \copydoc Shader::draw()
         */
         void draw() override;

      private:
	bool lighting_;
	AtomColoring atom_colors_;
	index_t atom_size_;
    };

}

#endif
