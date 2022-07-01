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
 *  (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_WARPDRIVE_COMMANDS_MESH_GROB_TRANSPORT_INTERFACE_H
#define H_OGF_WARPDRIVE_COMMANDS_MESH_GROB_TRANSPORT_INTERFACE_H

#include <OGF/WarpDrive/common/common.h>
#include <OGF/WarpDrive/commands/mesh_grob_transport_commands.h>
#include <OGF/mesh/grob/mesh_grob.h>
#include <OGF/scene_graph/commands/commands.h>

namespace OGF {
    namespace NL {
	class Vector;
	class Matrix;
    }
}

namespace OGF {

    /**
     * \brief A wrapper to script low-level editing operations 
     *  on a MeshGrob.
     */
    gom_class WarpDrive_API MeshGrobTransport : public Interface {
      public:
	/**
	 * \brief MeshGrobEditor constructor.
	 */
	MeshGrobTransport();

	/**
	 * \brief MeshGrobEditor destrutor.
	 */
	~MeshGrobTransport() override;

	/**
	 * \brief Gets the wrapped MeshGrob.
	 * \return a pointer to the MeshGrob or nullptr.
	 */
	MeshGrob* mesh_grob() const {
	    return dynamic_cast<MeshGrob*>(grob());
	}

      gom_slots:

	/**
	 * \brief Computes the centroids of the Laguerre cells
	 *  that realize an optimal transport from a domain to
	 *  the vertices of the current mesh.
	 * \param[in] Omega the domain, either a surfacic or a 
	 *  volumetric mesh.
	 * \param[out] centroids the coordinates of the centroids
	 *  of the Laguerre cells. If mode is EULER_2D, then there
	 *  are two coordinates per vertex, else there are three
	 *  coordinates per vertex.
	 * \param[out] weights an optional vector to get the computed weights.
	 * \param[in] mode one of EULER_2D, EULER_3D or EULER_ON_SURFACE.
	 */
	void compute_optimal_Laguerre_cells_centroids(
	    MeshGrob* Omega, NL::Vector* centroids,
	    NL::Vector* weights = nullptr,
	    MeshGrobTransportCommands::EulerMode mode = MeshGrobTransportCommands::EULER_2D
	);

	/**
	 * \brief Computes the measures of the Laguerre cells.
	 * \param[in] Omega the domain, either a surfacic or a 
	 *  volumetric mesh.
	 * \param[in] weights the weights of the Laguerre diagram.
	 * \param[out] measures the measures of all Laguerre cells.
	 * \param[in] mode one of EULER_2D, EULER_3D or EULER_ON_SURFACE.
	 */
	void compute_Laguerre_cells_measures(
	    MeshGrob* Omega,
	    NL::Vector* weights,
	    NL::Vector* measures,
	    MeshGrobTransportCommands::EulerMode mode
	);

	/**
	 * \brief Computes the P1 Laplacian of the Laguerre cells.
	 * \param[in] Omega the domain, either a surfacic or a 
	 *  volumetric mesh.
	 * \param[in] weights the weights of the Laguerre diagram.
	 * \param[out] Laplacian P1 Laplacian of the Laguerre diagram or nullptr if 
	 *  not needed.
	 * \param[out] measures optional measures the measures of 
	 *  all Laguerre cells, or nullptr if not needed.
	 * \param[in] mode one of EULER_2D, EULER_3D or EULER_ON_SURFACE.
	 */
	void compute_Laguerre_cells_P1_Laplacian(
	    MeshGrob* Omega,
	    NL::Vector* weights,
	    NL::Matrix* Laplacian,
	    NL::Vector* measures,
	    MeshGrobTransportCommands::EulerMode mode
	);

	/**
	 * \brief Computes the Laguerre diagram.
	 * \param[in] Omega the domain, either a surfacic or a 
	 *  volumetric mesh.
	 * \param[in] weights the weights of the Laguerre diagram.
	 * \param[out] RVD MeshGrob with the resulting Laguerre diagram.
	 * \param[in] mode one of EULER_2D, EULER_3D or EULER_ON_SURFACE.
	 */
	void compute_Laguerre_diagram(
	    MeshGrob* Omega,
	    NL::Vector* weights,
	    MeshGrob* RVD,
	    MeshGrobTransportCommands::EulerMode mode	    
	);
	
	/**
	 * \brief Computes the measure of the domain.
	 *  \param[in] Omega the domain, either a surfacic or a 
	 *  volumetric mesh.
	 * \param[in] mode one of EULER_2D, EULER_3D or EULER_ON_SURFACE.
	 */
	double Omega_measure(
	    MeshGrob* Omega,
	    MeshGrobTransportCommands::EulerMode mode	    
	);

	
    };    
    
}


#endif
