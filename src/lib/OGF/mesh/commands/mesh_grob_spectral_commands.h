/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2016 INRIA - Project ALICE
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

#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_SPECTRAL_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_SPECTRAL_COMMANDS_H

#include <OGF/mesh/commands/mesh_grob_commands.h>
#include <geogram/mesh/mesh_manifold_harmonics.h>

/**
 * \file OGF/mesh/commands/mesh_grob_spectral_commands.h
 * \brief Commands for spectral mesh processing.
 */


namespace OGF {

    /**
     * \brief Commands that create simple shapes.
     */
    gom_class MeshGrobSpectralCommands : public MeshGrobCommands {
    public:
        
        /**
         * \brief MeshGrobSpectralCommands constructor.
         */
        MeshGrobSpectralCommands();


        /**
         * \brief MeshGrobSpectralCommands destructor.
         */
         ~MeshGrobSpectralCommands() override;
	
    gom_slots:
        /**
         * \brief Computes manifold harmonics (Laplacien eigenfunctions)
         * \menu /Surface/Spectral
	 * \param[in] nb_eigens number of eigenfunctions to compute
	 * \param[in] discretization discretization of the Laplace Beltrami
	 *  operator
	 * \param[in] attribute name of the attribute used to store the 
	 *  eigenvectors
	 * \param[in] shift eigen shift applied to explore a certain part
	 *  of the spectrum.
	 * \param[in] nb_eigens_per_band if non-zero, 
	 *   use band-by-band computation.
	 * \param[in] print_spectrum if true, prints eigenvalue to the terminal.
         */
	void compute_manifold_harmonics(
	    index_t nb_eigens = 30,
	    LaplaceBeltramiDiscretization discretization = FEM_P1_LUMPED,
	    const std::string& attribute = "eigen",
	    double shift = 0.0,
	    index_t nb_eigens_per_band = 0,
	    bool print_spectrum = false
	);


        /**
         * \brief Computes manifold harmonics (Laplacien eigenfunctions)
         * \menu /Surface/Spectral
         * \brief Computes a spectral embedding.
         * \param[in] x_eigen the eigenfunction to be used for x
         * \param[in] y_eigen the eigenfunction to be used for y
         * \param[in] z_eigen the eigenfunction to be used for z
         */
        void compute_spectral_embedding(
            index_t x_eigen=1,
            index_t y_eigen=2,
            index_t z_eigen=3
        );
    };
    
}


#endif
