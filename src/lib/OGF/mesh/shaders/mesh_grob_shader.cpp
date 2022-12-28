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
 *   (non-GPL) libraries:  Qt, SuperLU, WildMagic and CGAL
 */
 

#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/basic/os/file_manager.h>

#include <geogram/image/image_library.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/stopwatch.h>

#include <time.h>

namespace OGF {

    MeshGrobShader::MeshGrobShader(
        MeshGrob* grob
    ) : Shader(grob) {
       no_grob_update_ = true;
    }
        
    MeshGrobShader::~MeshGrobShader() {
    }
    
    void MeshGrobShader::blink() {
    }

    void MeshGrobShader::show_vertices() {
    }

    void MeshGrobShader::hide_vertices() {
    }

    void MeshGrobShader::show_vertices_selection() {
    }

    void MeshGrobShader::hide_vertices_selection() {
    }
    
    void MeshGrobShader::show_mesh() {
    }

    void MeshGrobShader::hide_mesh() {
    }

    void MeshGrobShader::show_borders() {
    }

    void MeshGrobShader::hide_borders() {
    }

    void MeshGrobShader::draw() {
        Shader::draw();
    }

    void MeshGrobShader::pick(MeshElementsFlags what) {
        geo_argused(what);
    }

    void MeshGrobShader::pick_object(index_t object_id) {
        geo_argused(object_id);
    }
    
    PlainMeshGrobShader::PlainMeshGrobShader(
        MeshGrob* grob
    ) :
        MeshGrobShader(grob),
        gfx_() {
        gfx_.set_mesh(grob);

	painting_mode_ = SOLID_COLOR;
	
        attribute_ = "vertices.point[0]";
        attribute_subelements_ = MESH_VERTICES;
        attribute_name_ = "point[0]";
        attribute_min_ = 0.0;
        attribute_max_ = 0.0;

	tex_coord_subelements_ = MESH_VERTICES;
	tex_coord_attribute_ = "point";
	tex_coord_repeat_ = 1;
	tex_normal_mapping_ = false;
	
        surface_style_.visible = true;
        surface_style_.color = Color(0.5,0.5,0.5,1.0);
        facets_filter_ = false;
	culling_mode_ = NO_CULL;
	specular_ = 2;

        two_sided_ = false;
        
        volume_style_.visible = true;
        volume_style_.color = Color(1.0,1.0,0.0,1.0);
        cells_filter_ = false;
        
        edges_style_.visible = true;
        edges_style_.color   = Color(0.0,0.0,0.5,1.0);
        edges_style_.width   = 1;
        
        mesh_style_.visible = false;
        mesh_style_.color   = Color(0.0,0.0,0.0,1.0);
        mesh_style_.width   = 1;

        border_style_.visible = true;
        border_style_.color   = Color(0.0,0.0,0.5,1.0);
        border_style_.width   = 2;

        vertices_style_.visible = false;
        vertices_style_.color   = Color(0.0,1.0,0.0,1.0);
        vertices_style_.size    = 2;

        vertices_filter_ = false;
        
	vertices_transparency_ = 0.0;
	
        vertices_selection_style_.visible = true;
        vertices_selection_style_.color   = Color(1.0,0.0,0.0,1.0);
        vertices_selection_style_.size    = 3;
        
        shrink_ = 0;

        colored_cells_ = false;
        tets_ = true;        
        hexes_ = true;
        prisms_ = true;
        pyramids_ =true;
        connectors_=true;
        
        //  If we got only points, make sure that we will see
        // something by default !
        if(
            mesh_grob()->facets.nb() == 0 &&
            mesh_grob()->cells.nb() == 0 &&
            mesh_grob()->vertices.nb() != 0
        ) {
            vertices_style_.visible = true;
        }

        animate_ = false;
        time_ = 0.0;

        picking_ = false;

        slivers_ = 380.0;
        weird_cells_ = false;

        clipping_ = true;

	glsl_program_changed_ = false;
	glsl_program_ = 0;
	glsl_start_time_ = 0.0;
	glsl_frame_ = 0;
    }
    
    PlainMeshGrobShader::~PlainMeshGrobShader() {
	if(glsl_program_ != 0) {
	    glDeleteProgram(glsl_program_);
	    glsl_program_ = 0;
	}
    }

    void PlainMeshGrobShader::set_painting(PaintingMode value) {
	painting_mode_ = value;
	if(painting_mode_ == ATTRIBUTE) {
	    //  Update GUI because attribute-related
	    // GUI elements visibility depend on it.
	    if(attribute_min_ == 0.0 && attribute_max_ == 0.0) {
		autorange();
	    } 
	}
	if(painting_mode_ == COLOR) {
	    if(texture_filename_ != "rgbcube") {
		texture_filename_ = "rgbcube";
		texture_.reset();
	    }
	}
	update();
    }

    void PlainMeshGrobShader::set_attribute(const std::string& value) {
        attribute_ = value;
        std::string subelements_name;
        String::split_string(
            attribute_, '.',
            subelements_name,
            attribute_name_
        );
        attribute_subelements_ =
            mesh_grob()->name_to_subelements_type(subelements_name);
        if(attribute_min_ == 0.0 && attribute_max_ == 0.0) {
            autorange();
        } else {
            update();
        }
    }

    void PlainMeshGrobShader::set_tex_coords(const std::string& value) {
	tex_coord_attribute_ = value;
        std::string subelements_name;
        String::split_string(
            tex_coord_attribute_, '.',
            subelements_name,
            tex_coord_name_
        );
        tex_coord_subelements_ =
            mesh_grob()->name_to_subelements_type(subelements_name);
	update();
    }

    
    void PlainMeshGrobShader::autorange() {
        if(attribute_subelements_ != MESH_NONE) {
            attribute_min_ = 0.0;
            attribute_max_ = 0.0;
            const MeshSubElementsStore& subelements =
                mesh_grob()->get_subelements_by_type(attribute_subelements_);
            ReadOnlyScalarAttributeAdapter attribute(
                subelements.attributes(), attribute_name_
            );
            if(attribute.is_bound()) {
                // If boolean attribute, always use [0,1] range.
                if(
                    attribute.element_type() ==
                                       ReadOnlyScalarAttributeAdapter::ET_UINT8
                ) {
                    attribute_min_ = 0.0;
                    attribute_max_ = 1.0;
                } else {
                    attribute_min_ = Numeric::max_float64();
                    attribute_max_ = Numeric::min_float64();
                    for(index_t i: subelements) {
                        attribute_min_ = std::min(attribute_min_, attribute[i]);
                        attribute_max_ = std::max(attribute_max_, attribute[i]);
                    }
                }
            } 
        }
        update();
    }
    
    void PlainMeshGrobShader::draw() {
        MeshGrobShader::draw();
        
	if(glsl_program_changed_) {
	    update_glsl_program();
	}
	
        GLUPboolean clipping_backup = glupIsEnabled(GLUP_CLIPPING);
        
        if(!clipping_) {
            glupDisable(GLUP_CLIPPING);
            glDisable(GL_CLIP_PLANE0);
        }
        
        if(mesh_grob()->graphics_are_locked()) {
            return;
        }
        
        if(mesh_grob()->dirty()) {
            gfx_.set_mesh(mesh_grob());
            mesh_grob()->up_to_date();
        }

	if(get_texturing() || get_coloring()) {
	    glupEnable(GLUP_ALPHA_DISCARD);
	    glupSetAlphaThreshold(0.05f);
	    
	    if(texture_.is_null()) {
		if(FileSystem::is_file(texture_filename_)) {
		    texture_ = create_texture_from_file(
			texture_filename_, GL_LINEAR, GL_REPEAT
		    );
		} else {
		    if(texture_filename_ == "rgbcube") {
			Image_var image =
			    new Image(Image::RGB,Image::BYTE,16,16,16);
			for(index_t u=0; u<16; ++u) {
			    for(index_t v=0; v<16; ++v) {
				for(index_t w=0; w<16; ++w) {
				    Memory::byte* p = image->pixel_base(u,v,w);
				    p[0] = Memory::byte(u*16);
				    p[1] = Memory::byte(v*16);
				    p[2] = Memory::byte(w*16);
				}
			    }
			}
			texture_ = new Texture;
			texture_->create_from_image(
			    image, GL_LINEAR, GL_CLAMP_TO_EDGE
			);
		    } else if(texture_filename_ == "cube3") {
			Image_var image =
			    new Image(Image::RGB,Image::BYTE,4,4,4);
			for(index_t u=0; u<4; ++u) {
			    for(index_t v=0; v<4; ++v) {
				for(index_t w=0; w<4; ++w) {
				    Memory::byte* p = image->pixel_base(u,v,w);
				    if(((u/2)&1)^((v/2)&1)^((w/2)&1)) {
					p[0] = p[1] = p[2] = 255;
				    } else {
					p[0] = p[1] = p[2] = 0;
				    }
				}
			    }
			}
			texture_ = new Texture;
			texture_->create_from_image(
			    image, GL_NEAREST, GL_REPEAT
			);
		    } else if(texture_filename_ == "cube4") {
			Image_var image =
			    new Image(Image::RGB,Image::BYTE,4,4,4);
			for(index_t u=0; u<4; ++u) {
			    for(index_t v=0; v<4; ++v) {
				for(index_t w=0; w<4; ++w) {
				    Memory::byte* p = image->pixel_base(u,v,w);
				    index_t uu = (u + 1)%4;
				    index_t vv = (v + 1)%4;
				    index_t ww = (w + 1)%4;
				    if(((uu/2)&1)^((vv/2)&1)^((ww/2)&1)) {
					p[0] = p[1] = p[2] = 255;
				    } else {
					p[0] = p[1] = p[2] = 0;
				    }
				}
			    }
			}
			texture_ = new Texture;
			texture_->create_from_image(
			    image, GL_NEAREST, GL_REPEAT
			);
		    } else {
			std::string filename = "textures/checkerboard_gray.xpm";
			if(FileManager::instance()->find_file(filename)) {
			    texture_ = create_texture_from_file(
				filename, GL_NEAREST, GL_REPEAT
			    );
			}
		    }
		}
	    }
            texture_->bind();
            gfx_.set_texturing(
                tex_coord_subelements_,
		tex_coord_name_,
                texture_->id(),
		texture_->dimension(),
		tex_coord_repeat_
            );
            texture_->unbind();
	} else if(get_attributes()) {
            index_t repeat = colormap_style_.repeat;
            GLint filtering = colormap_style_.smooth ? GL_LINEAR : GL_NEAREST;
            if(
                String::string_starts_with(
                    colormap_style_.colormap_name,"iso"
                )
            ) {
                filtering = GL_LINEAR;
                if(repeat <= 1) {
                    repeat = 4;
                }
            }
            repeat = std::max(repeat, 1u);
            GLint clamping = repeat > 1 ? GL_REPEAT : GL_CLAMP_TO_EDGE;
            if(colormap_texture_.is_null()) {
                colormap_texture_ = create_texture_from_colormap_name(
                    colormap_style_.colormap_name,
                    filtering, clamping
                );
            }

	    // TODO: detect non-zero alpha in colormap.
	    if(
	       colormap_style_.colormap_name == "transparent" ||
	       colormap_style_.colormap_name == "transparent2" 
	    ) {
		glupEnable(GLUP_ALPHA_DISCARD);
		glupSetAlphaThreshold(0.05f);
	    }

	    double attribute_min = attribute_min_;
	    double attribute_max = attribute_max_;
	    
	    if(colormap_style_.flip) {
		std::swap(attribute_min, attribute_max);
	    }
	    
            colormap_texture_->bind();

            gfx_.set_scalar_attribute(
                attribute_subelements_, attribute_name_,
                attribute_min, attribute_max,
                colormap_texture_->id(), repeat
            );

            colormap_texture_->unbind();

        } else {
            gfx_.unset_scalar_attribute();
        }

        gfx_.set_mesh_color(
            float(mesh_style_.color.r()),
            float(mesh_style_.color.g()),
            float(mesh_style_.color.b())                
        );

        gfx_.set_show_mesh(mesh_style_.visible);
        gfx_.set_mesh_width(mesh_style_.width);

        gfx_.set_animate(animate_);
        gfx_.set_time(time_);

        if(vertices_selection_style_.visible) {
            gfx_.set_points_color(
                float(vertices_selection_style_.color.r()),
                float(vertices_selection_style_.color.g()),
                float(vertices_selection_style_.color.b())                
            );
            gfx_.set_points_size(float(vertices_selection_style_.size));
            gfx_.set_vertices_selection("selection");
            gfx_.draw_vertices();
            gfx_.set_vertices_selection("");            
        }
        
        if(vertices_style_.visible) {

	    if(vertices_transparency_ != 0.0) {
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	    }

            gfx_.set_points_color(
                float(vertices_style_.color.r()),
                float(vertices_style_.color.g()),
                float(vertices_style_.color.b()),
		float(1.0 - vertices_transparency_)
            );
	    
            gfx_.set_points_size(float(vertices_style_.size));
            gfx_.draw_vertices();

	    if(vertices_transparency_ != 0.0) {	    
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	    }
        }

	RenderingContext* context = RenderingContext::current();

        if(edges_style_.visible) {
            gfx_.set_mesh_color(
                float(edges_style_.color.r()),
                float(edges_style_.color.g()),
                float(edges_style_.color.b())                
            );
            gfx_.set_mesh_width(edges_style_.width);            
            gfx_.draw_edges();
        }
        
        if(border_style_.visible) {
            gfx_.set_mesh_color(
                float(border_style_.color.r()),
                float(border_style_.color.g()),
                float(border_style_.color.b())                
            );
            gfx_.draw_surface_borders();
        }
        
        if(surface_style_.visible) {
            gfx_.set_surface_color(
                float(surface_style_.color.r()),
                float(surface_style_.color.g()),
                float(surface_style_.color.b()),
                float(surface_style_.color.a())                		
            );
            gfx_.set_mesh_color(
                float(mesh_style_.color.r()),
                float(mesh_style_.color.g()),
                float(mesh_style_.color.b())                
            );
            gfx_.set_mesh_width(mesh_style_.width);	    
	    if(surface_style_.color.a() < 1.0) {
		Color bkg = 0.5 * (
		    context->background_color() +
		    context->background_color_2()
		);
		gfx_.set_surface_color(
		    float(bkg.r()),
		    float(bkg.g()),
		    float(bkg.b()),
		    0.0f
		);
		glupEnable(GLUP_ALPHA_DISCARD);
		glupSetAlphaThreshold(0.05f);
	    }
            if(two_sided_) {
                // If color is gray, replace with lighter
                // gray outside / darker gray inside so
                // that one can see the difference
                if(
                    surface_style_.color.r() == 0.5 &&
                    surface_style_.color.g() == 0.5 &&
                    surface_style_.color.b() == 0.5
                ) {
                    gfx_.set_surface_color(
                        0.9f, 0.9f, 0.9f
                    );
                    gfx_.set_backface_surface_color(
                        0.1f, 0.1f, 0.1f
                    );
                } else {
                    // Default case: inside color is the
                    // complementary color of surface color.
                    gfx_.set_backface_surface_color(
                        1.0f - float(surface_style_.color.r()),
                        1.0f - float(surface_style_.color.g()),
                        1.0f - float(surface_style_.color.b())                
                    );
                }
            }
	    if(tex_normal_mapping_) {
		glupEnable(GLUP_NORMAL_MAPPING);
	    }
	    float specular_backup = glupGetSpecular();
	    glupSetSpecular(float(specular_) / 10.0f);
	    switch(culling_mode_) {
		case NO_CULL:
		    break;
		case CULL_FRONT:
		    glEnable(GL_CULL_FACE);
		    glCullFace(GL_FRONT);
		    break;
		case CULL_BACK:
		    glEnable(GL_CULL_FACE);
		    glCullFace(GL_BACK);
		    break;
	    }
	    if(glsl_program_ != 0 && !picking_) {
		draw_surface_with_glsl_shader();
	    } else {
		gfx_.draw_surface();
	    }
	    glDisable(GL_CULL_FACE);
	    glupSetSpecular(specular_backup);
	    if(tex_normal_mapping_) {
		glupDisable(GLUP_NORMAL_MAPPING);
	    }
	    glupDisable(GLUP_ALPHA_DISCARD);	    
        }
	
        if(volume_style_.visible) {
            gfx_.set_mesh_width(mesh_style_.width);
            gfx_.set_mesh_color(
                float(mesh_style_.color.r()),
                float(mesh_style_.color.g()),
                float(mesh_style_.color.b())                
            );
            gfx_.set_show_mesh(mesh_style_.visible);
            gfx_.set_mesh_width(mesh_style_.width);

	    float specular_backup = glupGetSpecular();
	    glupSetSpecular(float(specular_) / 10.0f);
	    
            if(colored_cells_) {
                gfx_.set_cells_colors_by_type();
            } else {
                gfx_.set_cells_color(
                    float(volume_style_.color.r()),
                    float(volume_style_.color.g()),
                    float(volume_style_.color.b()),
                    float(volume_style_.color.a())
                );
		if(volume_style_.color.a() < 1.0) {
		    Color bkg = 0.5 * (
			context->background_color() +
			context->background_color_2()
		    );
		    gfx_.set_cells_color(
			float(bkg.r()),
			float(bkg.g()),
			float(bkg.b()),
			float(volume_style_.color.a())
		    );
		    glupEnable(GLUP_ALPHA_DISCARD);
		    glupSetAlphaThreshold(0.05f);
		}
            }
	    
	    
            gfx_.set_draw_cells(GEO::MESH_TET, tets_);
            gfx_.set_draw_cells(GEO::MESH_HEX, hexes_);
            gfx_.set_draw_cells(GEO::MESH_PRISM, prisms_);
            gfx_.set_draw_cells(GEO::MESH_PYRAMID, pyramids_);
            gfx_.set_draw_cells(GEO::MESH_CONNECTOR, connectors_);            

            // If clipping mode is slicing, we deactivate shading:
            // in particular when properties are mapped, shading
            // is very distracting, and properties variations can
            // be completely hidden by specular highlights.
            
            bool light_bkp = gfx_.get_lighting();

            // In addition, if we have a volume and we do not
            // have any surface, we draw the cells twice, once
            // with standard OpenGL mode, so that we can see
            // the border of the object.
            if(
                glupIsEnabled(GLUP_CLIPPING) && 
                (glupGetClipMode() == GLUP_CLIP_SLICE_CELLS)
            ) {
                if(mesh_grob()->facets.nb() == 0) {
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_FRONT);
                    glupClipMode(GLUP_CLIP_STANDARD);
                    gfx_.draw_volume();
                    glDisable(GL_CULL_FACE);
                }
                gfx_.set_lighting(false);                
                glupClipMode(GLUP_CLIP_SLICE_CELLS);
            }
            
            gfx_.draw_volume();
            gfx_.set_lighting(light_bkp);
	    glupDisable(GLUP_ALPHA_DISCARD);

	    glupSetSpecular(specular_backup);	    
        }

        if(slivers_ < 180.0) {
            draw_slivers();
        }

        if(weird_cells_) {
            draw_weird_cells();
        }

        if(!clipping_ && clipping_backup) {
            glupEnable(GLUP_CLIPPING);
            glEnable(GL_CLIP_PLANE0);
        }
    }

    void PlainMeshGrobShader::draw_slivers() {
        glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 1.0f, 0.0f, 0.0f);
        glupEnable(GLUP_DRAW_MESH);
        glupSetMeshWidth(1);
        glupSetCellsShrink(0.0f);
        glupBegin(GLUP_TETRAHEDRA);

        double max_cos = ::cos(slivers_ * M_PI / 180.0);
        double min_cos = -max_cos;

        for(index_t cell: mesh_grob()->cells) {
            if(mesh_grob()->cells.type(cell) == MESH_TET) {
                vec3 N[4];
                for(index_t lf=0; lf<4; ++lf) {
                    N[lf] = normalize(
                              mesh_cell_facet_normal(*mesh_grob(), cell, lf)
                            );
                }
                bool is_sliver = false;
                for(index_t lf1=0; lf1<4; ++lf1) {
                    for(index_t lf2=lf1+1; lf2<4; ++lf2) {
                        double cos_angle = dot(N[lf1], N[lf2]);
                        if(cos_angle < min_cos || cos_angle > max_cos) {
                            is_sliver = true;
                            break;
                        }
                    }
                    if(is_sliver) {
                        break;
                    }
                }
                if(is_sliver) {
                    for(index_t lv=0; lv<4; ++lv) {
                        index_t v = mesh_grob()->cells.vertex(cell,lv);
                        glupVertex3dv(mesh_grob()->vertices.point_ptr(v));
                    }
                }
            }
        }
        glupEnd();
    }

    void PlainMeshGrobShader::draw_weird_cells() {
        Attribute<bool> weird;
        weird.bind_if_is_defined(mesh_grob()->cells.attributes(), "weird");
        if(!weird.is_bound()) {
            return;
        }
        
        glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 1.0f, 0.0f, 0.0f);
        glupEnable(GLUP_DRAW_MESH);
        glupSetMeshWidth(1);
        glupSetCellsShrink(float(get_shrink())/10.0f);
        
        glupBegin(GLUP_TETRAHEDRA);
        for(index_t cell: mesh_grob()->cells) {
            if(mesh_grob()->cells.type(cell) == MESH_TET && weird[cell]) {
                for(index_t lv=0; lv<4; ++lv) {
                    index_t v = mesh_grob()->cells.vertex(cell,lv);
                    glupVertex3dv(mesh_grob()->vertices.point_ptr(v));
                }
            }
        }
        glupEnd();

        glupBegin(GLUP_HEXAHEDRA);
        for(index_t cell: mesh_grob()->cells) {
            if(mesh_grob()->cells.type(cell) == MESH_HEX && weird[cell]) {
                for(index_t lv=0; lv<8; ++lv) {
                    index_t v = mesh_grob()->cells.vertex(cell,lv);
                    glupVertex3dv(mesh_grob()->vertices.point_ptr(v));
                }
            }
        }
        glupEnd();
    }

    
    void PlainMeshGrobShader::pick_object(index_t object_id) {
        gfx_.set_picking_mode(MESH_NONE);
        gfx_.set_object_picking_id(object_id);
        picking_ = true;
        draw();
        gfx_.set_object_picking_id(index_t(-1));        
        picking_ = false;
    }

    
    void PlainMeshGrobShader::pick(MeshElementsFlags what) {
        gfx_.set_picking_mode(what);
        picking_ = true;
        draw();
        gfx_.set_picking_mode(MESH_NONE);
        picking_ = false;
    }

    void PlainMeshGrobShader::blink() {
        mesh_style_.visible = !mesh_style_.visible;
        update();
    }

    void PlainMeshGrobShader::show_vertices() {
        vertices_style_.visible=true;
        update();
    }

    void PlainMeshGrobShader::hide_vertices() {
        vertices_style_.visible=false;
        update();
    }

    void PlainMeshGrobShader::show_vertices_selection() {
        vertices_selection_style_.visible=true;
        update();
    }

    void PlainMeshGrobShader::hide_vertices_selection() {
        vertices_selection_style_.visible=false;
        update();
    }

    
    void PlainMeshGrobShader::show_mesh() {
        mesh_style_.visible=true;
        update();
    }

    void PlainMeshGrobShader::hide_mesh() {
        mesh_style_.visible=false;
        update();
    }

    void PlainMeshGrobShader::show_borders() {
        border_style_.visible=true;
        update();
    }

    void PlainMeshGrobShader::hide_borders() {
        border_style_.visible=false;
        update();
    }

    void PlainMeshGrobShader::update_glsl_program() {
	glsl_program_changed_ = true;
	if(glupCurrentContext() == nullptr) {
	    return;
	}
	glsl_start_time_ = SystemStopwatch::now();
	glsl_frame_ = 0;
	GEO_CHECK_GL();
	if(glsl_program_ != 0) {
	    glDeleteProgram(glsl_program_);
	    glsl_program_ = 0;
	}
	GEO_CHECK_GL();
	if(glsl_source_ != "") {
	    
	    std::string includes;
	    const char* begin = glsl_source_.c_str();
	    for(const char* p = begin; p != nullptr; p = strstr(p,"//import")) {
		p = strchr(p, '\n');
		if(p != nullptr) {
		    begin = p+1;
		    p = begin;
		}
	    }

	    if(begin != glsl_source_.c_str()) {
		std::string::size_type preamble_len = std::string::size_type(
		    begin - glsl_source_.c_str()
		);
		std::string preamble(glsl_source_, 0, preamble_len);
		index_t preamble_lines=0;
		for(index_t i=0; i<preamble.length(); ++i) {
		    if(preamble[i] == '\n') {
			++preamble_lines;
		    }
		}
		std::string directive =
		    "#line " + String::to_string(preamble_lines+1) + "\n";
		std::string source(begin);
		glsl_program_ = glupCompileProgram(
		    (preamble + directive + source).c_str()
		);
	    } else {
		glsl_program_ = glupCompileProgram(glsl_source_.c_str());
	    }
	}
	glsl_program_changed_ = false;
	update();
    }

    
    namespace {
	inline void draw_vertex(
	    MeshGrob* mesh, index_t v, const Attribute<double>& tex_coord
	) {
	    bool has_tex_coord = tex_coord.is_bound();
	    if(has_tex_coord) {
		glupTexCoord2dv(
		    &tex_coord[v*tex_coord.dimension()]
		);
	    }
	    if(mesh->vertices.single_precision()) {
		if(!has_tex_coord) {
		    glupTexCoord2fv(
			mesh->vertices.single_precision_point_ptr(v)
		    );
		}
		if(mesh->vertices.dimension() < 3) {
		    glupVertex2fv(
			mesh->vertices.single_precision_point_ptr(v)
		    );
		} else {
		    glupVertex3fv(
			mesh->vertices.single_precision_point_ptr(v)
		    );
		}
	    } else {
		if(!has_tex_coord) {
		    glupTexCoord2dv(
			mesh->vertices.point_ptr(v)
		    );
		}
		if(mesh->vertices.dimension() < 3) {
		    glupVertex2dv(
			mesh->vertices.point_ptr(v)
		    );
		} else {
		    glupVertex3dv(
			mesh->vertices.point_ptr(v)
		    );
		}
	    }
	}
    }
    
    void PlainMeshGrobShader::draw_surface_with_glsl_shader() {
	GEO_CHECK_GL();	
	glUseProgram(glsl_program_);

	// If shader has an iTime uniform (e.g. a ShaderToy shader),
	// then update it, and indicate that graphics should be
	// updated again and again (call update() at each frame).
	GLint iTime_loc = glGetUniformLocation(glsl_program_, "iTime");
	if(iTime_loc != -1) {
	    glUniform1f(
		iTime_loc, float(SystemStopwatch::now() - glsl_start_time_)
	    );
	    update();
	}

	GLint iFrame_loc = glGetUniformLocation(glsl_program_, "iFrame");
	if(iFrame_loc != -1) {
	    glUniform1f(
		iFrame_loc, float(glsl_frame_)
	    );
	    update();
	}

	GLint iDate_loc = glGetUniformLocation(glsl_program_, "iDate");
	if(iDate_loc != -1) {
	    time_t t = time(nullptr);
	    struct tm* tm = localtime(&t);
	    float datex = float(tm->tm_year + 1900);
	    float datey = float(tm->tm_mon);
	    float datez = float(tm->tm_mday);
	    float datew = float(tm->tm_sec) +
		          60.0f * float(tm->tm_min) +
		          3600.0f * float(tm->tm_hour);
	    glUniform4f(iDate_loc, datex, datey, datez, datew);
	    update();
	}

	++glsl_frame_;
	
	glUseProgram(0);
	GEO_CHECK_GL();
	
	glupDisable(GLUP_VERTEX_COLORS);
	glupEnable(GLUP_TEXTURING);
	glupSetColor4dv(GLUP_FRONT_AND_BACK_COLOR, surface_style_.color.data());
	glupUseProgram(glsl_program_);
	glupBegin(GLUP_TRIANGLES);
	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(
	    mesh_grob()->vertices.attributes(), "tex_coord"
	);
	for(index_t f: mesh_grob()->facets) {
            index_t v1 = mesh_grob()->facets.vertex(f,0);
            for(index_t lv=1; lv+1<mesh_grob()->facets.nb_vertices(f); ++lv) {
                index_t v2 = mesh_grob()->facets.vertex(f,lv);
                index_t v3 = mesh_grob()->facets.vertex(f,lv+1);
                draw_vertex(mesh_grob(),v1,tex_coord);
                draw_vertex(mesh_grob(),v2,tex_coord);
                draw_vertex(mesh_grob(),v3,tex_coord);
            }
	}
	if(tex_coord.is_bound()) {
	    tex_coord.unbind();
	}
	glupEnd();
	glupUseProgram(0);
	glupDisable(GLUP_TEXTURING);
	GEO_CHECK_GL();	
    }
    
}

