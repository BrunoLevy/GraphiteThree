
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
 

#ifndef H_OGF_WARPDRIVE_COMMANDS_MESH_GROB_MARTINGALE_COMMANDS_H
#define H_OGF_WARPDRIVE_COMMANDS_MESH_GROB_MARTINGALE_COMMANDS_H

#include <OGF/WarpDrive/common/common.h>
#include <OGF/WarpDrive/algo/velocity_field.h>
#include <OGF/WarpDrive/algo/time_integrator.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

namespace OGF {

    gom_class WarpDrive_API MeshGrobMartingaleCommands :
        public MeshGrobCommands {
    public:
        MeshGrobMartingaleCommands() ;
        ~MeshGrobMartingaleCommands() override;

    gom_slots:
	/**
	 * \menu /Transport/Martingale
	 */
	void init_martingale(
	    const NewMeshGrobName& points = "points",
	    const NewMeshGrobName& laguerre = "Laguerre",
	    const NewMeshGrobName& centroids = "centroids",
	    index_t nb_points = 10,
	    bool use_exact_predicates = true
	);

	
	/**
	 * \menu /Transport/Martingale
	 * \brief Computes a martingale transport in 2d by iteratively
	 *  relocating the Dirac masses at the barycenters
	 * \param[in] points the generated pointset 
	 * \param[in] laguerre the generated Laguerre diagram
	 * \param[in] centroids the centroids of the Laguerre cells
	 */
	void recenter_martingale(
	    const MeshGrobName& points = "points",
	    const MeshGrobName& laguerre = "Laguerre",
	    const MeshGrobName& centroids = "centroids",
	    index_t nb_iter = 1,
	    bool use_exact_predicates = true	    
	);
	
    };
}

#endif
