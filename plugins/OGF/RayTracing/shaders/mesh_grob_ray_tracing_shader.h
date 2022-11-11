
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
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
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Bruno Levy - Bruno.Levy@inria.fr
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
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */
 

#ifndef H__OGF_RAYTRACING_SHADERS_MESH_GROB_RAY_TRACING_SHADER__H
#define H__OGF_RAYTRACING_SHADERS_MESH_GROB_RAY_TRACING_SHADER__H

#include <OGF/RayTracing/common/common.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <geogram/mesh/mesh_AABB.h>

namespace OGF {

    gom_class RayTracing_API RayTracingMeshGrobShader : public MeshGrobShader {
    public:
        RayTracingMeshGrobShader(OGF::MeshGrob* grob);
        ~RayTracingMeshGrobShader() override;
        void draw() override;

    gom_properties:

        /**
         * \brief Number of rays per pixel.
         */
	index_t get_supersampling() const {
	    return supersampling_;
	}

	void set_supersampling(index_t x) {
	    supersampling_ = x;
	    update();
	}
	
        /**
         * \brief surface color.
         */
        const Color& get_color() const {
            return color_;
        }
        
        void set_color(const Color& x) {
            color_ = x;
            update();
        }

	/**
	 * \brief opaque core color.
	 */

	const Color& get_core_color() const {
	    return core_color_;
	}

	void set_core_color(const Color& x) {
	    core_color_ = x;
	    update();
	}

	
	/**
	 * \brief Specular intensity.
	 */

	double get_specular() const {
	    return spec_;
	}

	void set_specular(double x) {
	    spec_ = x;
	    update();
	}
	
	/**
	 * \brief Specular factor.
	 */
	index_t get_specular_factor() const {
	    return spec_factor_;
	}

	void set_specular_factor(index_t x) {
	    spec_factor_ = x;
	    update();
	}

	/**
	 * \brief Smooth shading.
	 */

	bool get_smooth() const {
	    return smooth_;
	}

	void set_smooth(bool x) {
	    smooth_ = x;
	    update();
	}

	/**
	 * \brief Compute shadows.
	 */
	
	bool get_shadows() const {
	    return shadows_;
	}

	void set_shadows(bool x) {
	    shadows_ = x;
	    update();
	}

	/**
	 * \brief Make object transparent.
	 */
	
	bool get_transparent() const {
	    return transp_;
	}

	void set_transparent(bool x) {
	    transp_ = x;
	    update();
	}

	/**
	 * \brief XRay mode
	 */
	bool get_xray() const {
	    return xray_ ;
	}

	void set_xray(bool x) {
	    xray_ = x;
	    update();
	}


	/**
	 * \brief Refraction index.
	 */
	double get_refract_index() const {
	    return refract_index_;
	}

	void set_refract_index(double x) {
	    refract_index_ = x;
	    update();
	}
	
	/**
	 * \brief Lighting extinction due to fluid thickness.
	 */
	double get_extinction() const {
	    return ext_;
	}

	void set_extinction(double x) {
	    ext_ = x;
	    update();
	}

	/**
	 * \brief Number of layers for transparency.
	 */
	index_t get_nb_layers() const {
	    return nb_layers_;
	}

	void set_nb_layers(index_t x) {
	    nb_layers_ = x;
	    update();
	}
	
	
	/**
	 * \brief Raytrace background.
	 */
	bool get_raytrace_background() const {
	    return raytrace_background_;
	}

	void set_raytrace_background(bool x) {
	    raytrace_background_ = x;
	    update();
	}

	/**
	 * \brief Show timing stats.
	 */
	bool get_show_stats() const {
	    return show_stats_;
	}

	void set_show_stats(bool x) {
	    show_stats_ = x;
	}

	
    gom_slots:
	/**
	 * \brief Copies background into image.
	 */
	void copy_background() {
	    copy_background_queued_ = true;
	    update();
	}

	/**
	 * \brief Saves background mesh and viewing parameters.
	 */
	void save_background() {
	    copy_background_queued_ = true;
	    save_background_queued_ = true;
	    update();
	}
	
    protected:

	void do_copy_background();
	void do_save_background();	
	
	/**
	 * \brief Launches a primary ray.
	 * \param[in] x , y the coordinates of the pixel,
	 *  in [0..width-1],[0..height-1]
	 */
	Ray primary_ray(double x, double y);

	/**
	 * \brief Creates the image the first time, then resizes it 
	 *  if the size of the window changed.
	 */
	void create_or_resize_image_if_needed();

	
	/**
	 * \brief Gets the viewing parameters from the current GLUP state.
	 * \details Gets the viewport, modelview and projection matrices.
	 */
	void update_viewing_parameters();

	/**
	 * \brief Raytraces the current image.
	 */
	void raytrace();

	vec4 raytrace_pixel(double x, double y);
	
	/**
	 * \brief Sets a pixel in the final image.
	 * \param[in] X , Y the pixel integer coordinates.
	 * \param[in] color the color with 4 components in [0,1].
	 */
	void set_pixel(index_t X, index_t Y, const vec4& color) {
	    Memory::byte* p = image_->pixel_base(X,Y);
	    FOR(i,4) {
		double comp = color[i];
		ogf_clamp(comp, 0.0, 1.0);
		p[i] = Memory::byte(comp*255.0);
	    }
	}
	
	/**
	 * \brief Draws the raytraced image on the screen.
	 */
	void draw_image();

	/**
	 * \brief Normalizes and optionally smooths 
	 *  the normal in an intersection.
	 * \param[in,out] I an intersection.
	 */
	void compute_normal(MeshFacetsAABB::Intersection& I);


	/**
	 * \brief Computes the direction of a reflected ray.
	 * \param[in] I the direction of incident ray.
	 * \param[in] N the unit normal vector.
	 * \return direction of reflected ray.
	 */
	static vec3 reflect_vector(
	    const vec3& I, const vec3& N
	) {
	    return -2.0*dot(N,I)*N + I;
	}

	/**
	 * \brief Computes a reflected ray.
	 * \param[in] ray incident ray.
	 * \param[in] P the point where reflection occurs.
	 * \param[in] N the unit vector at \p P.
	 * \return the reflected ray.
	 */
	static Ray reflect_ray(
	    const Ray& ray, const MeshFacetsAABB::Intersection& I
	) {
	    return Ray(I.p, reflect_vector(ray.direction, I.N));
	}

	/**
	 * \brief Computes the direction of a refracted ray.
	 * \param[in] I the direction of the incident ray.
	 * \param[in] N the unit normal vector.
	 * \param[in] eta ratio of the refraction indices.
	 * \return the direction of the refracted ray.
	 */
	static vec3 refract_vector(
	    const vec3& I, const vec3& N, double eta
	) {
	    double k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));
	    if (k < 0.0) {
		return vec3(0.0, 0.0, 0.0);
	    }
	    return eta * I - (eta * dot(N, I) + sqrt(k)) * N;
	}

	/**
	 * \brief Computes a refracted ray.
	 * \param[in] ray incident ray.
	 * \param[in] P the point where refraction occurs.
	 * \param[in] N the unit vector at \p P.
	 * \param[in] n1 the refraction index of the first material.
	 * \param[in] n2 the refraction index of the second material.
	 * \return the refracted ray.
	 */
	static Ray refract_ray(
	    const Ray& ray, const MeshFacetsAABB::Intersection& I,
	    double n1, double n2
	) {
	    vec3 Incid = normalize(ray.direction);
	    return Ray(I.p, refract_vector(Incid, I.N, n1/n2));
	}

	/**
	 * \brief Computes multiple refractions in the fluid surface.
	 * \param[in,out] r the current ray
	 * \param[in,out] I the current intersection
	 * \return the total traversed fluid length
	 */
	double multi_refract(Ray& r, MeshFacetsAABB::Intersection& I);

	vec3 raytrace_background(const Ray& r);
	
    private:
	index_t supersampling_;
	
        Color color_;
	double spec_;
	index_t spec_factor_;
	bool shadows_;
	bool smooth_;
	bool transp_;
	double ext_;
	double refract_index_;
	index_t nb_layers_;
	
	Image_var image_;
	
	bool raytrace_background_;
	Image_var background_image_;
	Image_var background_depth_;
	Mesh background_mesh_;
	Attribute<double> background_mesh_color_;
	MeshFacetsAABB background_mesh_AABB_;

	bool xray_;
	Color core_color_;
	
	GLuint texture_;
	MeshFacetsAABB AABB_;

	double viewport_[4];
	mat4 inv_project_modelview_;
	vec3 L_; /**< light vector in object space. */

	Attribute<vec3> facet_normal_;
	Attribute<vec3> vertex_normal_;
	Attribute<vec3> facet_corner_normal_;

	double bbox_diag_;

	bool copy_background_queued_;
	bool save_background_queued_;

	bool show_stats_;
    };
}

#endif

