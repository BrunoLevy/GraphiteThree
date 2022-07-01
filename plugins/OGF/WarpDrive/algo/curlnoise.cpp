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
 */
 
#include <OGF/WarpDrive/algo/curlnoise.h>

namespace OGF {

    CurlNoiseVelocityField::~CurlNoiseVelocityField() { 
    }

    void CurlNoiseVelocityField::get_velocity(
	double current_time, const vec3& vertex, vec3 &velocity
    ) const {
	geo_argused(current_time);
	
	double x = vertex.x ;
	double y = vertex.y ;
	double z = vertex.z ;
	
	velocity= vec3(
	    (
		(potential(x, y+delta_x, z).z - potential(x, y-delta_x, z).z)
		-(potential(x, y, z+delta_x).y - potential(x, y, z-delta_x).y)
	    ) / (2*delta_x),
	    
	    (
		(potential(x, y, z+delta_x).x - potential(x, y, z-delta_x).x)
		-(potential(x+delta_x, y, z).z - potential(x-delta_x, y, z).z)
	    ) / (2*delta_x) ,

	    (
		(potential(x+delta_x, y, z).y - potential(x-delta_x, y, z).y)
		-(potential(x, y+delta_x, z).x - potential(x, y-delta_x, z).x)
	    ) / (2*delta_x)
	    
	    );
        }
        
        vec3 CurlNoiseVelocityField::potential(
	    double x, double y , double z
	) const {
	    vec3 psi(0,0,0);
	    double height_factor=0.5;
	    
	    static const vec3 centre( 0.0, 1.0, 0.0 );
	    static double radius = 4.0;
	    
	    for(unsigned int i=0; i<noise_lengthscale.size(); ++i) {
		double sx=x/noise_lengthscale[i];
		double sy=y/noise_lengthscale[i];
		double sz=z/noise_lengthscale[i];
		
		vec3 psi_i(0,0, noise2(sx,sy,sz)) ;
		//        Vector3d psi_i( 0.f, 0.f, noise2(sx,sy,sz));
		
		double dist = length(vec3(x,y,z) - centre );
		double scale = std::max((radius - dist)/radius, 0.0 );
		psi_i = vec3(psi_i.x * scale, psi_i.y * scale, psi_i.z * scale);
		
		psi+=height_factor*noise_gain[i]*psi_i;
	    }

	    return psi;
	}

    void CurlNoiseVelocityField::flow_noise3(
	unsigned int seed, double spin_variation
    ) {
	noise3(seed) ;
	seed+=8*n; // probably avoids overlap with sequence
	              // used in initializing superclass Noise3
	for(unsigned int i=0; i<n; ++i){
	    original_basis[i]=basis[i];
	    spin_axis[i]=sample_sphere(seed);
	    spin_rate[i]=2.0*M_PI*randhashd(
		seed++, 0.1 - 0.5*spin_variation, 0.1 + 0.5*spin_variation
	    );
	}
    }

    void CurlNoiseVelocityField::noise3(unsigned int seed) {
	for(unsigned int i=0; i<n; ++i){
	    basis[i]=sample_sphere(seed);
	    perm[i]=i;
	}
	reinitialize(seed);
    }
    
    void CurlNoiseVelocityField::reinitialize(unsigned int seed) {
	for(unsigned int i=1; i<n; ++i){
	    unsigned int j=randhash(seed++)%(i+1);
	    std::swap(perm[i], perm[j]);
	}
    }

    double CurlNoiseVelocityField::calc(double x, double y, double z) const {
	double floorx=std::floor(x), floory=std::floor(y), floorz=std::floor(z);
	int i=(int)floorx, j=(int)floory, k=(int)floorz;
	const vec3 &n000=basis[hash_index(i,j,k)];
	const vec3 &n100=basis[hash_index(i+1,j,k)];
	const vec3 &n010=basis[hash_index(i,j+1,k)];
	const vec3 &n110=basis[hash_index(i+1,j+1,k)];
	const vec3 &n001=basis[hash_index(i,j,k+1)];
	const vec3 &n101=basis[hash_index(i+1,j,k+1)];
	const vec3 &n011=basis[hash_index(i,j+1,k+1)];
	const vec3 &n111=basis[hash_index(i+1,j+1,k+1)];
	
	double fx=x-floorx, fy=y-floory, fz=z-floorz;
	double sx=fx*fx*fx*(10-fx*(15-fx*6));
	double sy=fy*fy*fy*(10-fy*(15-fy*6));
	double sz=fz*fz*fz*(10-fz*(15-fz*6));

	return trilerp(    fx*n000.x +     fy*n000.y +     fz*n000.z,
			   (fx-1)*n100.x +     fy*n100.y +     fz*n100.z,
			   fx*n010.x + (fy-1)*n010.y +     fz*n010.z,
			   (fx-1)*n110.x + (fy-1)*n110.y +     fz*n110.z,
			   fx*n001.x +     fy*n001.y + (fz-1)*n001.z,
			   (fx-1)*n101.x +     fy*n101.y + (fz-1)*n101.z,
			   fx*n011.x + (fy-1)*n011.y + (fz-1)*n011.z,
			   (fx-1)*n111.x + (fy-1)*n111.y + (fz-1)*n111.z,
			   sx, sy, sz);
    }

    void CurlNoiseVelocityField::set_time(double t) {
	t_ = t;
	for(unsigned int i=0; i<n; ++i){
	    double theta=spin_rate[i]*t;
	    double c=std::cos(theta), s=std::sin(theta);
	    // form rotation matrix
	    double R00=c+(1-c)*geo_sqr(spin_axis[i][0]);
	    double R01=(1-c)*spin_axis[i][0]*spin_axis[i][1]-s*spin_axis[i][2];
	    double R02=(1-c)*spin_axis[i][0]*spin_axis[i][2]+s*spin_axis[i][1];
	    double R10=(1-c)*spin_axis[i][0]*spin_axis[i][1]+s*spin_axis[i][2];
	    double R11=c+(1-c)*geo_sqr(spin_axis[i][1]);
	    double R12=(1-c)*spin_axis[i][1]*spin_axis[i][2]-s*spin_axis[i][0];
	    double R20=(1-c)*spin_axis[i][0]*spin_axis[i][2]-s*spin_axis[i][1];
	    double R21=(1-c)*spin_axis[i][1]*spin_axis[i][2]+s*spin_axis[i][0];
	    double R22=c+(1-c)*geo_sqr(spin_axis[i][2]);
	    basis[i][0]=
		R00*original_basis[i][0] +
		R01*original_basis[i][1] +
		R02*original_basis[i][2];
	    
	    basis[i][1]=
		R10*original_basis[i][0] +
		R11*original_basis[i][1] +
		R12*original_basis[i][2];
	    
	    basis[i][2]=
		R20*original_basis[i][0] +
		R21*original_basis[i][1] +
		R22*original_basis[i][2];
	}
    }

} 

