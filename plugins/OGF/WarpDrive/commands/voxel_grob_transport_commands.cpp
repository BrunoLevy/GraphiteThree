
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

#include <OGF/WarpDrive/commands/voxel_grob_transport_commands.h>
#include <geogram/NL/nl.h>

namespace OGF {

    VoxelGrobTransportCommands::VoxelGrobTransportCommands() {
    }

    VoxelGrobTransportCommands::~VoxelGrobTransportCommands() {
    }

    void VoxelGrobTransportCommands::init_from_pointset(
	const MeshGrobName& points_name
    ) {
	// Important note: following Farnik's files, W/Z index is fast index
	// (flipped as compared to Graphite/geogram that uses U/X fast index)

	MeshGrob* pointset = MeshGrob::find(scene_graph(), points_name);
	if(pointset == nullptr) {
	    Logger::err("VoxelGrob") << points_name << " :no such MeshGrob"
				     << std::endl;
	    return;
	}

	index_t NUVW[3] = {0,0,0};
	bool UVW_done[3] = {false, false, false};
	vec3 prev_p;
	vec3 p_min(
	    Numeric::max_float64(),
	    Numeric::max_float64(),
	    Numeric::max_float64()
	);
	vec3 p_max(
	    -Numeric::max_float64(),
	    -Numeric::max_float64(),
	    -Numeric::max_float64()
	);

	for(index_t v: pointset->vertices) {
	    vec3 p = pointset->vertices.point(v);

	    if(v == 0) {
		++NUVW[0];
		++NUVW[1];
		++NUVW[2];
		prev_p = p;
		continue;
	    }

	    int varying_c = 2;
	    for(int c=2; c>=0; --c) {
		if(p[index_t(c)] > prev_p[index_t(c)]) {
		    varying_c = c;
		    break;
		}
	    }

	    if(!UVW_done[varying_c]) {
		++NUVW[varying_c];
	    }

	    for(int c=varying_c+1; c<3; ++c) {
		UVW_done[index_t(c)] = true;
	    }

	    p_min = vec3(
		std::min(p_min.x, p.x),
		std::min(p_min.y, p.y),
		std::min(p_min.z, p.z)
	    );

	    p_max = vec3(
		std::max(p_max.x, p.x),
		std::max(p_max.y, p.y),
		std::max(p_max.z, p.z)
	    );

	    prev_p = p;
	}

	std::cerr << NUVW[0] << " " << NUVW[1] << " " << NUVW[2] << std::endl;

	if(pointset->vertices.nb() != NUVW[0]*NUVW[1]*NUVW[2]) {
	    Logger::err("VoxelGrob")
		<< "Did not find NU,NV,NW, invalid pointset size"
		<< std::endl;
	    return;
	}

        vec3 origin = p_min;
        vec3 U(p_max.x - p_min.x, 0.0, 0.0);
        vec3 V(0.0, p_max.y - p_min.y, 0.0);
        vec3 W(0.0, 0.0, p_max.z - p_min.z);

        voxel_grob()->set_box(origin, U, V, W);
        voxel_grob()->resize(NUVW[0], NUVW[1], NUVW[2]);

	Attribute<double> pointset_mass;
	pointset_mass.bind_if_is_defined(pointset->vertices.attributes(),"mass");
	if(pointset_mass.is_bound()) {
	    Attribute<float> voxel_mass(voxel_grob()->attributes(), "mass");
	    for(index_t u=0; u<NUVW[0]; ++u) {
		for(index_t v=0; v<NUVW[1]; ++v) {
		    for(index_t w=0; w<NUVW[2]; ++w) {
			double point_mass = pointset_mass [
			    w + v * NUVW[2] + u * NUVW[2]*NUVW[1]
			] ;
			voxel_mass[voxel_grob()->linear_index(u,v,w)] =
			    float(point_mass);
		    }
		}
	    }
	}

        voxel_grob()->update();
    }


    void VoxelGrobTransportCommands::interpolate_attribute(
	const std::string& attribute_name,
	double bkgnd_value, index_t margin_width
    ) {
	Attribute<float> attribute;
	attribute.bind_if_is_defined(voxel_grob()->attributes(), attribute_name);
	if(!attribute.is_bound()) {
	    Logger::err("VoxelGrob") << attribute_name << ": no such attribute"
				     << std::endl;
	    return;
	}

	index_t nu = voxel_grob()->nu();
	index_t nv = voxel_grob()->nv();
	index_t nw = voxel_grob()->nw();

	constexpr char VOX_BKGND = 0;
	constexpr char VOX_DATA = 1;
	constexpr char VOX_INTERP = 2;
	constexpr char VOX_NEXT_INTERP = 3;

	vector<char> cell_tag(nu*nv*nw);

	for(index_t i=0; i<nu*nv*nw; ++i) {
	    double val = double(attribute[i]);
	    cell_tag[i] = (val == bkgnd_value) ? VOX_BKGND : VOX_DATA;
	}

	for(index_t k=0; k<margin_width; ++k) {

	    Logger::out("VoxelGrob") << "expand margin: " << k << std::endl;

	    for(index_t w=0; w<nw; ++w) {
		for(index_t v=0; v<nv; ++v) {
		    for(index_t u=0; u<nu; ++u) {
			index_t uvw = voxel_grob()->linear_index(u,v,w);
			if(cell_tag[uvw] == VOX_BKGND) {
			    for(int dw=-1; dw<=1; ++dw) {
				for(int dv=-1; dv<=1; ++dv) {
				    for(int du=-1; du<=1; ++du) {
					int u2 = int(u)+du;
					u2 = (u2 < 0) ? int(nu - 1) : u2;
					int v2 = int(v)+dv;
					v2 = (v2 < 0) ? int(nv - 1) : v2;
					int w2 = int(w)+dw;
					w2 = (w2 < 0) ? int(nw - 1) : w2;
					char t = cell_tag[
					    voxel_grob()->linear_index(
						index_t(u2),
						index_t(v2),
						index_t(w2)
					    )
					];
					if(t == VOX_DATA || t == VOX_INTERP) {
					    cell_tag[uvw] = VOX_NEXT_INTERP;
					}
				    }
				}
			    }
			}
		    }
		}
	    }


	    for(index_t i=0; i<nu*nv*nw; ++i) {
		if(cell_tag[i] == VOX_NEXT_INTERP) {
		    cell_tag[i] = VOX_INTERP;
		}
	    }
	}


	nlNewContext();
	nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
	nlSolverParameteri(NL_NB_VARIABLES, NLint(nu*nv*nw));
	nlEnable(NL_VERBOSE);

	nlBegin(NL_SYSTEM);

	index_t nb_locked = 0;
	for(index_t i=0; i<nu*nv*nw; ++i) {
	    double val = double(attribute[i]);
	    nlSetVariable(i,val);
	    if(cell_tag[i] != VOX_INTERP) {
		nlLockVariable(i);
		++nb_locked;
	    }
	}

	Logger::out("VoxelGrob") << nb_locked << " locked "
				 << double(nb_locked) * 100.0 / double(nu*nv*nw)
				 << "%"
				 << std::endl;


	nlBegin(NL_MATRIX);

	for(index_t w=0; w<nw; ++w) {
	    for(index_t v=0; v<nv; ++v) {
		for(index_t u=0; u<nu; ++u) {
		    index_t um = (u + nu - 1) % nu;
		    index_t up = (u + 1) % nu;
		    index_t vm = (v + nv - 1) % nv;
		    index_t vp = (v + 1) % nv;
		    index_t wm = (w + nw - 1) % nw;
		    index_t wp = (w + 1) % nw;
		    nlBegin(NL_ROW);
		    nlCoefficient(voxel_grob()->linear_index(u,v,w), -6.0);
		    nlCoefficient(voxel_grob()->linear_index(um,v,w), 1.0);
		    nlCoefficient(voxel_grob()->linear_index(up,v,w), 1.0);
		    nlCoefficient(voxel_grob()->linear_index(u,vm,w), 1.0);
		    nlCoefficient(voxel_grob()->linear_index(u,vp,w), 1.0);
		    nlCoefficient(voxel_grob()->linear_index(u,v,wm), 1.0);
		    nlCoefficient(voxel_grob()->linear_index(u,v,wp), 1.0);
		    nlEnd(NL_ROW);
		}
	    }
	}

	nlEnd(NL_MATRIX);
	nlEnd(NL_SYSTEM);

	nlSolve();

	for(index_t i=0; i<nu*nv*nw; ++i) {
	    attribute[i] = float(std::max(nlGetVariable(i),0.0));
	}

	nlDeleteContext(nlGetCurrent());

	voxel_grob()->update();
    }
}
