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

#ifndef H_OGF_WARPDRIVE_ALGOS_VSDM_H
#define H_OGF_WARPDRIVE_ALGOS_VSDM_H

#include <OGF/WarpDrive/common/common.h>
#include <exploragram/optimal_transport/VSDM.h>

namespace OGF {

    /**
     * \brief An implementation of VSDM that displays the fitting 
     *  process by updating Graphite's graphics at each iteration.
     */
    class WarpDrive_API VisualVSDM : public VSDM {
      public:
	/**
	 * \brief VisualVSDM constructor.
	 * \param[in] S the surface mesh to be fitted.
	 * \param[in] T the target surface mesh.
	 */
	VisualVSDM(Mesh* S, Mesh* T);


      protected:

	/**
	 * \copydoc VSDM::newiteration()
	 */
	void newiteration() override;
    };
    
}

#endif
