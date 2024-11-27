
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


#include <OGF/RayTracing/shaders/mesh_grob_ray_tracing_shader.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/image/image_library.h>
#include <geogram/basic/stopwatch.h>

namespace OGF {

    RayTracingMeshGrobShader::RayTracingMeshGrobShader(
        OGF::MeshGrob* grob
    ):
	MeshGrobShader(grob),
	texture_(0),
	AABB_(*grob)
    {
	// AABB changed facet order, need to notify
	mesh_grob()->update();

	supersampling_ = 1;
        color_ = Color(0.5, 0.5, 1.0, 0.5);
	spec_ = 1.0;
	spec_factor_ = 20;
	smooth_ = false;
	shadows_ = false;
	transp_ = false;
	refract_index_ = 1.0;
	ext_ = 5.0;
	nb_layers_ = 4;
	xray_ = false;
	raytrace_background_ = false;

	facet_normal_.bind(
	    mesh_grob()->facets.attributes(), "normal"
	);
	vertex_normal_.bind(
	    mesh_grob()->vertices.attributes(), "normal"
	);
	facet_corner_normal_.bind_if_is_defined(
	    mesh_grob()->facet_corners.attributes(), "normal"
	);
	bool has_facet_corner_normals =
	    (facet_corner_normal_.is_bound());
	if(!has_facet_corner_normals) {
	    facet_corner_normal_.bind(
		mesh_grob()->facet_corners.attributes(), "normal"
	    );
	}
	for(index_t f: mesh_grob()->facets) {
	    if(has_facet_corner_normals) {
		facet_normal_[f] = vec3(0.0, 0.0, 0.0);
		for(index_t c: mesh_grob()->facets.corners(f)) {
		    facet_normal_[f] += facet_corner_normal_[c];
		}
		facet_normal_[f] = normalize(facet_normal_[f]);
	    } else {
		facet_normal_[f] = Geom::mesh_facet_normal(*mesh_grob(), f);
	    }
	}
	FOR(v, mesh_grob()->vertices.nb()) {
	    vertex_normal_[v] = vec3(0.0, 0.0, 0.0);
	}
	FOR(f, mesh_grob()->facets.nb()) {
	    for(index_t c=mesh_grob()->facets.corners_begin(f);
		c < mesh_grob()->facets.corners_end(f); ++c
	    ) {
		index_t v = mesh_grob()->facet_corners.vertex(c);
		vertex_normal_[v] += facet_normal_[f];
	    }
	}
	FOR(v, mesh_grob()->vertices.nb()) {
	    vertex_normal_[v] = normalize(vertex_normal_[v]);
	}
	FOR(f, mesh_grob()->facets.nb()) {
	    facet_normal_[f] = normalize(facet_normal_[f]);
	}
	FOR(c, mesh_grob()->facet_corners.nb()) {
	    index_t v = mesh_grob()->facet_corners.vertex(c);
	    if(!has_facet_corner_normals) {
		facet_corner_normal_[c] = vertex_normal_[v];
	    }
	}
	bbox_diag_ = bbox_diagonal(*mesh_grob());
	copy_background_queued_ = false;
	save_background_queued_ = false;
	show_stats_ = false;

	core_color_ = Color(0.0, 0.0, 0.0, 1.0);
    }

    RayTracingMeshGrobShader::~RayTracingMeshGrobShader() {
	if(texture_ != 0) {
	    glDeleteTextures(1, &texture_);
	    texture_ = 0;
	}
    }

    void RayTracingMeshGrobShader::draw() {
	create_or_resize_image_if_needed();
	update_viewing_parameters();
	if(copy_background_queued_) {
	    do_copy_background();
	    copy_background_queued_ = false;
	}
	if(save_background_queued_) {
	    do_save_background();
	    save_background_queued_ = false;
	}
	raytrace();
	draw_image();
    }

    void RayTracingMeshGrobShader::do_copy_background() {

	index_t w = image_->width();
	index_t h = image_->height();

	if(background_image_.is_null() ||
	   background_image_->width() != w ||
	   background_image_->height() != h
	) {
	    background_image_ = new Image(Image::RGB, Image::BYTE, w, h);
	}

	if(background_depth_.is_null() ||
	   background_depth_->width() != w ||
	   background_depth_->height() != h
	) {
	    background_depth_ = new Image(
		Image::GRAY, Image::FLOAT32, w, h
	    );
	}

	glReadPixels(
	    0, 0,
	    GLsizei(w), GLsizei(h),
	    GL_RGB, GL_UNSIGNED_BYTE,
	    background_image_->base_mem()
	);

	glReadPixels(
	    0, 0,
	    GLsizei(w), GLsizei(h),
	    GL_DEPTH_COMPONENT, GL_FLOAT,
	    background_depth_->base_mem()
	);

	background_mesh_.clear();
	if(!background_mesh_color_.is_bound()) {
	    background_mesh_color_.create_vector_attribute(
		background_mesh_.vertices.attributes(), "color", 3
	    );
	}

	FOR(Y, h) {
	    FOR(X, w) {
		// Add tiny displacement because my ray-AABB test is
		// broken for some degenerate configs (to be fixed).
		// HERE
		double x = double(X) + 1e-5;
		double y = double(Y) + 1e-5;
		double z = double(
		    *background_depth_->pixel_base_float32_ptr(X,Y)
		);
		vec4 p(
		    2.0*((x - viewport_[0]) / viewport_[2]-0.5),
		    2.0*((y - viewport_[1]) / viewport_[3]-0.5),
		    2.0*(z-0.5),
		    1.0
		);
		p = mult(inv_project_modelview_,p);
		index_t v = background_mesh_.vertices.create_vertex(
		    vec3(p.x/p.w, p.y/p.w, p.z/p.w).data()
	        );
		Memory::byte* c = background_image_->pixel_base(X,Y);
		background_mesh_color_[3*v  ] = double(c[0])/255.0;
		background_mesh_color_[3*v+1] = double(c[1])/255.0;
		background_mesh_color_[3*v+2] = double(c[2])/255.0;
	    }
	}

	FOR(Y, h-1) {
	    FOR(X, w-1) {
		if(
		    *background_depth_->pixel_base_float32_ptr(X,Y)   == 1.0f ||
		    *background_depth_->pixel_base_float32_ptr(X,Y+1) == 1.0f ||
		    *background_depth_->pixel_base_float32_ptr(X+1,Y) == 1.0f ||
		    *background_depth_->pixel_base_float32_ptr(X+1,Y+1) == 1.0f
		) {
		    continue;
		}
		index_t v11 =  X    +  Y    * w;
		index_t v12 =  X    + (Y+1) * w;
		index_t v21 = (X+1) +  Y    * w;
		index_t v22 = (X+1) + (Y+1) * w;
		background_mesh_.facets.create_triangle(v11, v12, v22);
		background_mesh_.facets.create_triangle(v11, v22, v21);
	    }
	}
	background_mesh_.vertices.remove_isolated();
	background_mesh_AABB_.initialize(background_mesh_);
    }

    void RayTracingMeshGrobShader::do_save_background() {
	mesh_save(background_mesh_, "background.geogram");
	std::ofstream out("background_camera.txt");
	out << "size " << image_->width() << " "
	    << image_->height() << std::endl;
	out << "viewport ";
	FOR(i,4) {
	    out << viewport_[i] << " ";
	}
	out << std::endl;
	mat4 modelview;
	glupGetMatrixdv(GLUP_MODELVIEW_MATRIX, modelview.data());
	out << "modelview " << modelview << std::endl;
	mat4 project;
	glupGetMatrixdv(GLUP_PROJECTION_MATRIX, project.data());
	out << "project " << project << std::endl;
	float Lf[3];
	glupGetLightVector3fv(Lf);
	out << "light " << Lf[0] << " " << Lf[1] << " " << Lf[2] << std::endl;
    }

    void RayTracingMeshGrobShader::create_or_resize_image_if_needed() {

	// Get window size
	Object* main=Interpreter::instance_by_language("Lua")->
	    resolve_object("main");
	index_t w,h;
	{
	    Any tmp;
	    main->get_property("width",tmp);
	    tmp.get_value(w);
	    main->get_property("height",tmp);
	    tmp.get_value(h);
	}

	if(image_.is_null() ||
	   image_->width() != w ||
	   image_->height() != h
	) {
	    image_ = new Image(Image::RGBA, Image::BYTE, w, h);
	    FOR(y, h) {
		FOR(x, w) {
		    Memory::byte* p = image_->pixel_base(x,y);
		    if(((x / 32) & 1) ^ ((y / 32) & 1)) {
			p[0] = 255;
			p[1] = 255;
			p[2] = 255;
			p[3] = 255;
		    } else {
			p[0] = 127;
			p[1] = 127;
			p[2] = 127;
			p[3] = 127;
		    }
		}
	    }
	}
    }

    void RayTracingMeshGrobShader::update_viewing_parameters() {
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	FOR(i,4) {
	    viewport_[i] = double(viewport[i]);
	}

	mat4 modelview;
	glupGetMatrixdv(GLUP_MODELVIEW_MATRIX, modelview.data());
	mat3 normalmatrix;
	FOR(i,3) {
	    FOR(j,3) {
		normalmatrix(i,j) = modelview(i,j);
	    }
	}
	modelview = modelview.transpose();
	mat4 project;
	glupGetMatrixdv(GLUP_PROJECTION_MATRIX, project.data());
	project = project.transpose();

	inv_project_modelview_ = (project*modelview).inverse();

	float Lf[3];
	glupGetLightVector3fv(Lf);

	L_ = vec3(Lf[0], Lf[1], Lf[2]);
	L_ = normalize(mult(normalmatrix,L_));
    }

    Ray RayTracingMeshGrobShader::primary_ray(double x, double y) {
	vec4 nearv(
	    2.0*((x - viewport_[0]) / viewport_[2]-0.5),
	    2.0*((y - viewport_[1]) / viewport_[3]-0.5),
	    -1.0,
	    1.0
	);
	vec4 ffar = nearv;
	ffar.z = 1.0;
	nearv = mult(inv_project_modelview_,nearv);
	ffar = mult(inv_project_modelview_,ffar);


	vec3 nearp = (1.0/nearv.w)*vec3(nearv.x, nearv.y, nearv.z);
	vec3 farp  = (1.0/ffar.w)*vec3(ffar.x, ffar.y, ffar.z);

	return Ray(nearp, farp-nearp);
    }

    void RayTracingMeshGrobShader::compute_normal(
	MeshFacetsAABB::Intersection& I
    ) {
	if(!smooth_) {
	    I.N = facet_normal_[I.f];
	    return;
	}

	// If facet is a triangle, interpolate normals
	// using barycentric coords in triangle.
	if(mesh_grob()->facets.nb_vertices(I.f) == 3) {
	    index_t c0 = mesh_grob()->facets.corners_begin(I.f);
	    index_t c1 = c0+1;
	    index_t c2 = c1+1;
	    I.N = normalize(
		(1.0 - I.u - I.v) * facet_corner_normal_[c0] +
		             I.u  * facet_corner_normal_[c1] +
		             I.v  * facet_corner_normal_[c2]
	    );
	    return;
	}

	// If facet is a polygon, decompose it on the fly into
	// triangles, find the triangle that contains the point,
	// (characterized by lambda1 + lambda2 + lambda3 = 1.0)
	// then interpolate normals in that triangle using
	// barycentric coordinates.
	// Normally we could find the first triangle such that
	// lambda1 + lambda2 + lambda3 = 1, but there are numerical
	// errors (or a bug !), so we find the one that *minimizes*
	// lambda1 + lambda2 + lambda3.

	vec3 g = Geom::mesh_facet_center(*mesh_grob(),I.f);

	vec3 result(0.0, 0.0, 0.0);
	double cur_sum = Numeric::max_float64();

	for(index_t c1 = mesh_grob()->facets.corners_begin(I.f);
	    c1<mesh_grob()->facets.corners_end(I.f); ++c1
	) {
	    index_t v1 = mesh_grob()->facet_corners.vertex(c1);
	    vec3 p1(mesh_grob()->vertices.point_ptr(v1));
	    index_t c2 = mesh_grob()->facets.next_corner_around_facet(I.f,c1);
	    index_t v2 = mesh_grob()->facet_corners.vertex(c2);
	    vec3 p2(mesh_grob()->vertices.point_ptr(v2));

	    double A =  Geom::triangle_area(g,p1,p2);
	    double l0 = Geom::triangle_area(I.p, p1,  p2 )/A;
	    double l1 = Geom::triangle_area(g,   I.p, p2 )/A;
	    double l2 = Geom::triangle_area(g,   p1,  I.p)/A;

	    // We need a bit of tolerance here, else it create
	    // black star-shaped patterns in the center of polygons.
	    if(l0 + l1 + l2 < cur_sum) {
		cur_sum = l0 + l1 + l2;
		result = normalize(
		    l0 * facet_normal_[I.f] +
		    l1 * facet_corner_normal_[c1] +
		    l2 * facet_corner_normal_[c2]
		);
	    }
	}
	I.N=result;
    }

    void RayTracingMeshGrobShader::raytrace() {
	Stopwatch W("Raytracing", show_stats_);
	parallel_for(0, image_->height(),
	   [this](index_t Y) {
		FOR(X, image_->width()) {
		    if(supersampling_ <= 1) {
			set_pixel(X, Y, raytrace_pixel(double(X), double(Y)));
		    } else {
			vec4 color(0.0, 0.0, 0.0, 0.0);
			for(index_t i=0; i<supersampling_; ++i) {
			    color += raytrace_pixel(
				double(X) + (Numeric::random_float64() - 0.5),
				double(Y) + (Numeric::random_float64() - 0.5)
			    );
			}
			color /= double(supersampling_);
			set_pixel(X, Y, color);
		    }
		}
	    }
	);
	if(show_stats_) {
	    index_t pixels = image_->width() * image_->height();
	    Logger::out("Raytracing")
		<< (double(pixels) / (1e6 * W.elapsed_time()))
		<< " Mpixels/s" << std::endl;
	}
    }

    vec4 RayTracingMeshGrobShader::raytrace_pixel(double x, double y) {
	vec4 color(0.0, 0.0, 0.0, 0.0);
	Ray ray = primary_ray(x,y);

	if(xray_) {
	    vector<MeshFacetsAABB::Intersection> isects;
	    AABB_.ray_all_intersections(
		ray,
		[&isects](const MeshFacetsAABB::Intersection& I) {
		    isects.push_back(I);
		}
	    );

	    std::sort(isects.begin(), isects.end(),
		      [](const MeshFacetsAABB::Intersection& I1,
			 const MeshFacetsAABB::Intersection& I2) -> bool {
			  return (I1.t < I2.t);
		      }
	    );

	    double traversed_len = 0;
	    for(index_t i=0; i<isects.size(); i+=2) {
		traversed_len += (isects[i+1].t - isects[i].t);
	    }
	    double d = (
		traversed_len * ext_ * length(ray.direction)
	    ) /	bbox_diag_;
	    geo_clamp(d, 0.0, 1.0);
	    return vec4(d, d, d, 1.0);
	}

	MeshFacetsAABB::Intersection I;
	if(AABB_.ray_nearest_intersection(ray, I)) {
	    color = vec4(0.0, 0.0, 0.0, 1.0 - transp_);
	    bool in_shadow = shadows_ && AABB_.ray_intersection(
		Ray(I.p,L_), Numeric::max_float64(), I.f
	    );

	    vec3 Kr(0.0, 0.0, 0.0);
	    vec3 Ks(0.0, 0.0, 0.0);
	    double spec=0.0;

	    compute_normal(I);

	    if(!in_shadow) {
		double diff = std::max(0.0, dot(L_,I.N));
		diff = std::min(diff, 1.0);

		Kr.x += diff * color_.r();
		Kr.y += diff * color_.g();
		Kr.z += diff * color_.b();

		if(spec_ != 0.0) {
		    vec3 R = normalize(reflect_vector(L_,I.N));
		    spec = dot(R,normalize(ray.direction));
		    spec = std::max(spec, 0.0);
		    spec = pow(spec, double(spec_factor_));
		    spec *= spec_;
		    Ks.x += spec;
		    Ks.y += spec;
		    Ks.z += spec;
		}
	    }
	    vec3 K;

	    if(transp_) {
		vec3 Kt(color_.r(), color_.g(), color_.b());
		double d = 0.0;
		double fresnel = 1.0 + dot(ray.direction,I.N) /
		                       length(ray.direction);
		geo_clamp(fresnel, 0.0, 1.0);
		if(transp_ && ext_ != 0) {
		    d = exp(-multi_refract(ray,I) * ext_ / bbox_diag_);
		    geo_clamp(d, 0.0, 1.0);
		    Kt.x *= d;
		    Kt.y *= d;
		    Kt.z *= d;
		    d = 1.0 - d;
		    Kt.x += d * core_color_.r();
		    Kt.y += d * core_color_.g();
		    Kt.z += d * core_color_.b();
		}
		vec3 K = Ks + fresnel * Kr + (1.0 - fresnel) * Kt;
		color.x = K.x;
		color.y = K.y;
		color.z = K.z;
		color.w = fresnel + (1.0 - fresnel) * d; // + spec;
		if(transp_ && ext_ != 0 &&
		   raytrace_background_ &&
		   background_mesh_AABB_.mesh() != nullptr
		) {
		    vec3 bkg_color = raytrace_background(ray);
		    color.x *= color.w;
		    color.y *= color.w;
		    color.z *= color.w;
		    color.x += (1.0 - color.w) * bkg_color.x;
		    color.y += (1.0 - color.w) * bkg_color.y;
		    color.z += (1.0 - color.w) * bkg_color.z;
		    color.w = 1.0;
		}
	    } else {
		vec3 K = (Kr + Ks);
		color.x = K.x;
		color.y = K.y;
		color.z = K.z;
		color.w = 1.0;
	    }
	} else {
	    if(
		raytrace_background_ &&
		background_mesh_AABB_.mesh() != nullptr
	    ) {
		vec3 c = raytrace_background(ray);
		color.x = c.x;
		color.y = c.y;
		color.z = c.z;
		color.w = 1.0;
	    } else {
		color.w = 0.0;
	    }
	}

	return color;
    }

    double RayTracingMeshGrobShader::multi_refract(
	Ray& r, MeshFacetsAABB::Intersection& I
    ) {
	double result = 0.0;
	for(index_t i=0; i<(nb_layers_*2); ++i) {
	    if(dot(r.direction, I.N) > 0.0) { // Exiting the object
		result += I.t;
		vec3 old_dir = r.direction;
		I.N = -I.N;
		r = refract_ray(r, I, refract_index_, 1.0);
		if(
		    r.direction.x == 0.0 &&
		    r.direction.y == 0.0 &&
		    r.direction.z == 0.0
		) {
		    r.direction = old_dir; // I should do total reflection here.
		}
	    } else { // Entering the object
		r = refract_ray(r, I, 1.0, refract_index_);
		r.direction = normalize(r.direction);
	    }
	    I.t = Numeric::max_float64();
	    if(i == (nb_layers_*2 - 1)) {
		break;
	    }
	    if(!AABB_.ray_nearest_intersection(r, I)) {
		break;
	    }
	    compute_normal(I);
	}
	return result;
    }

    vec3 RayTracingMeshGrobShader::raytrace_background(const Ray& r_in) {
	Ray r(r_in);
	r.origin -= 1e-6*r.direction;
	vec3 result(0.0, 0.0, 0.0);
	MeshFacetsAABB::Intersection I;
	if(background_mesh_AABB_.ray_nearest_intersection(r, I)) {
	    result.x =
		(1.0 - I.u - I.v) * background_mesh_color_[3*I.i] +
		I.u               * background_mesh_color_[3*I.j] +
		I.v               * background_mesh_color_[3*I.k] ;
	    result.y =
		(1.0 - I.u - I.v) * background_mesh_color_[3*I.i+1] +
		I.u               * background_mesh_color_[3*I.j+1] +
		I.v               * background_mesh_color_[3*I.k+1] ;
	    result.z =
		(1.0 - I.u - I.v) * background_mesh_color_[3*I.i+2] +
		I.u               * background_mesh_color_[3*I.j+2] +
		I.v               * background_mesh_color_[3*I.k+2] ;
	}
	return result;
    }

    void RayTracingMeshGrobShader::draw_image() {
	if(texture_ == 0) {
	    glGenTextures(1, &texture_);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glTexImage2D(
	    GL_TEXTURE_2D, 0, GL_RGBA,
	    GLsizei(image_->width()), GLsizei(image_->height()),
	    0, GL_RGBA, GL_UNSIGNED_BYTE,
	    image_->base_mem()
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glViewport(0, 0, GLsizei(image_->width()), GLsizei(image_->height()));
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	draw_unit_textured_quad();
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
    }


}
