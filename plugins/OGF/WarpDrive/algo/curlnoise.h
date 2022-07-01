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
 */
 

#ifndef OGF_WARPDRIVE_ALGO_CURLNOISE
#define OGF_WARPDRIVE_ALGO_CURLNOISE

#include <OGF/WarpDrive/common/common.h>
#include <OGF/WarpDrive/algo/velocity_field.h>
#include <geogram/basic/numeric.h>

namespace OGF {

    /**
     * \brief Implements curlnoise velocity field (Brochu and Bridson).
     *  Typical test scenario uses a sphere of radius 1 centered on
     *  (0,1,0).
     * \note Initial implementation from ElTopo, modifications by
     *  David Lopez (June 2010).
     */
    
    class WarpDrive_API CurlNoiseVelocityField : public VelocityField {
      public:
      CurlNoiseVelocityField() :
	noise_lengthscale(1),
	noise_gain(1) {
	    t_ = 0; 
	    delta_x = 1e-4; 
	    noise_lengthscale[0]=1.5;
	    noise_gain[0]=1.3;
	    seed_ = 171717; 
	    spin_variation_ = 0.2; 
	    flow_noise3(seed_, spin_variation_);
	}
                
	~CurlNoiseVelocityField() override;
	
	void get_velocity(
	    double current_time, const vec3& vertex, vec3& velocity
	) const override;

      protected: 

	void reinitialize(unsigned int seed);
	double calc(double x, double y, double z) const;
	double calc(const vec3& x) const {
	    return calc(x.x, x.y, x.z);
	}

	static const unsigned int n=128;
	vec3 basis[n];
	unsigned int perm[n];

	void set_time(double t); // period of repetition is approximately 1
	
	vec3 original_basis[n];
	double spin_rate[n];
	vec3 spin_axis[n];
	
	std::vector<double> noise_lengthscale, noise_gain;
	double t_;          // time
	double delta_x;    // used for finite difference approximations of curl
	unsigned int seed_; 
	double spin_variation_;
	
      private:

	// directly from el_topo code ...

	void noise3(unsigned int seed);   
	void flow_noise3(unsigned int seed, double spin_variation);   
	
	double noise2(double x, double y, double z) const {
	    return calc(z-203.994, x+169.47, y-205.31);
	}
	
	vec3 potential(double x, double y, double z) const;

	unsigned int hash_index(int si, int sj, int sk) const {
	    geo_debug_assert(si >= 0);
	    geo_debug_assert(sj >= 0);
	    geo_debug_assert(sk >= 0);
	    unsigned int i = (unsigned int)si;
	    unsigned int j = (unsigned int)sj;
	    unsigned int k = (unsigned int)sk;
	    return perm[(perm[(perm[i%n]+j)%n]+k)%n];
	}

	inline unsigned int randhash(unsigned int seed_in) {
	    unsigned int i=(seed_in^12345391u)*2654435769u;
	    i^=(i<<6)^(i>>26);
	    i*=2654435769u;
	    i+=(i<<5)^(i>>12);
	    return i;
	}

	inline double randhashd(unsigned int seed_in, double a, double b) {
            const unsigned int A =
		std::numeric_limits<unsigned int>::max();	    
	    return (b-a)*double(randhash(seed_in))/double(A) + a;
	}

	inline float randhashf(unsigned int seed_in, float a, float b) {
            const unsigned int A =
		std::numeric_limits<unsigned int>::max();	    
	    return (b-a)*float(randhash(seed_in))/float(A) + a;
	}

	inline double lerp(
	    const double& value0, const double& value1, double f
	) const {
	    return (1-f)*value0 + f*value1;
	}


	inline double bilerp(
	    const double& v00, const double& v10,
	    const double& v01, const double& v11,
	    double fx, double fy
	) const {
	    return lerp(lerp(v00, v10, fx),
			lerp(v01, v11, fx),
			fy);
	}

	inline double trilerp(
	    const double& v000, const double& v100,
	    const double& v010, const double& v110,
	    const double& v001, const double& v101,
	    const double& v011, const double& v111,
	    double fx, double fy, double fz) const
	{
	    return lerp(bilerp(v000, v100, v010, v110, fx, fy),
			bilerp(v001, v101, v011, v111, fx, fy),
			fz);
	}


	inline vec3 sample_sphere(unsigned int &seed_in) {
	    vec3 v;
	    double m2;
	    do {
		for(unsigned int i=0; i<3; ++i) {
		    v[i]=randhashd(seed_in++,-1.0,1.0);
		}
		m2=length2(v);
	    }
	    while(m2>1 || m2==0);
	    return v/std::sqrt(m2);
	}
    };
}
#endif

