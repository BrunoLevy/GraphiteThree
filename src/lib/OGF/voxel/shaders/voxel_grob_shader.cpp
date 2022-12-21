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
 

#include <OGF/voxel/shaders/voxel_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>

namespace OGF {

    VoxelGrobShader::VoxelGrobShader(
        VoxelGrob* grob
    ) : Shader(grob) {
       no_grob_update_ = true;
    }
        
    VoxelGrobShader::~VoxelGrobShader() { 
    }
    
    void VoxelGrobShader::blink() {
    }

    void VoxelGrobShader::draw() {
        Shader::draw();
    }

    void VoxelGrobShader::pick_object(index_t object_id) {
        geo_argused(object_id);
    }

    /*********************************************************************/
    
    PlainVoxelGrobShader::PlainVoxelGrobShader(
        VoxelGrob* grob
    ) :
        VoxelGrobShader(grob)
    {
        attribute_name_ = "";
        attribute_min_ = 0.0;
        attribute_max_ = 0.0;
        
        picking_ = false;
        clipping_ = true;
        lighting_ = true;

        box_style_.visible = true;
        box_style_.color   = Color(0.0,0.0,0.0,1.0);
        box_style_.width   = 2;
    }
    
    PlainVoxelGrobShader::~PlainVoxelGrobShader() {
    }

    void PlainVoxelGrobShader::set_attribute(const std::string& value) {
        attribute_name_ = value;
        attribute_texture_.reset();
        if(attribute_min_ == 0.0 && attribute_max_ == 0.0) {
            autorange();
        } else {
            update();
        }
    }
    
    void PlainVoxelGrobShader::autorange() {
        ReadOnlyScalarAttributeAdapter attribute(
            voxel_grob()->attributes(), attribute_name_
        );
        if(attribute.is_bound()) {
            attribute_min_ = Numeric::max_float64();
            attribute_max_ = Numeric::min_float64();
            
            index_t nuvw =
                voxel_grob()->nu() * voxel_grob()->nv() * voxel_grob()->nw();
            
            for(index_t i=0; i<nuvw; ++i) {
                attribute_min_ = std::min(attribute_min_, attribute[i]);
                attribute_max_ = std::max(attribute_max_, attribute[i]);
            }
        }
        update();
    }
    
    void PlainVoxelGrobShader::draw() {

        VoxelGrobShader::draw();
        
        GLUPboolean clipping_backup = glupIsEnabled(GLUP_CLIPPING);
        
        if(!clipping_) {
            glupDisable(GLUP_CLIPPING);
            glDisable(GL_CLIP_PLANE0);
        }
        
        if(voxel_grob()->graphics_are_locked()) {
            return;
        }

        if(voxel_grob()->dirty()) {
            attribute_texture_.reset();
            voxel_grob()->up_to_date();
        }
        
        
        if(box_style_.visible) {
            draw_wireframe_box();
        }

        create_textures_if_needed();

        if(!attribute_texture_.is_null() && !colormap_texture_.is_null()) {

            index_t repeat = colormap_style_.repeat;
            if(
                String::string_starts_with(
                    colormap_style_.colormap_name,"iso"
                )
            ) {
                if(repeat <= 1) {
                    repeat = 4;
                }
            }
            repeat = std::max(repeat, 1u);

	    double attribute_min = attribute_min_;
	    double attribute_max = attribute_max_;
	    
	    if(colormap_style_.flip) {
		std::swap(attribute_min, attribute_max);
	    }
            glupDisable(GLUP_LIGHTING);
            if(!glupIsEnabled(GLUP_CLIPPING)) {
                draw_volume(
                    colormap_texture_,
                    attribute_min, attribute_max, repeat
                );                
            } else {
                GLUPclipMode clip_mode = glupGetClipMode();
                if(
                    clip_mode == GLUP_CLIP_STANDARD 
                ) {
                    draw_volume(
                        colormap_texture_,
                        attribute_min, attribute_max, repeat
                    );
                } else if(clip_mode == GLUP_CLIP_SLICE_CELLS) {
                    draw_volume(
                        colormap_texture_,
                        attribute_min, attribute_max, repeat
                    );
                } else {
                    glupClipMode(GLUP_CLIP_STANDARD);
                    draw_volume(
                        colormap_texture_,
                        attribute_min, attribute_max, repeat
                    );
                    glupDisable(GLUP_LIGHTING);
                    glupClipMode(GLUP_CLIP_SLICE_CELLS);
                    draw_volume(
                        colormap_texture_,
                        attribute_min, attribute_max, repeat
                    );
                    glupClipMode(clip_mode);
                }
                glupEnable(GLUP_LIGHTING);                
            }
        }

        if(!clipping_ && clipping_backup) {
            glupEnable(GLUP_CLIPPING);
            glEnable(GL_CLIP_PLANE0);
        }
    }


    void PlainVoxelGrobShader::create_textures_if_needed() {
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
        if(attribute_texture_.is_null()) {
            ReadOnlyScalarAttributeAdapter attribute(
                voxel_grob()->attributes(), attribute_name_
            );

            if(attribute.is_bound()) {

                Image::ComponentEncoding comp_encoding = Image::BYTE;
                    
                switch(attribute.element_type()) {
                case ReadOnlyScalarAttributeAdapter::ET_UINT8:
                case ReadOnlyScalarAttributeAdapter::ET_INT8:
                    comp_encoding = Image::BYTE;
                    break;
                case ReadOnlyScalarAttributeAdapter::ET_UINT32:
                case ReadOnlyScalarAttributeAdapter::ET_INT32:
                    comp_encoding = Image::INT32;
                    break;
                case ReadOnlyScalarAttributeAdapter::ET_FLOAT32:
                    comp_encoding = Image::FLOAT32;
                    break;
                case ReadOnlyScalarAttributeAdapter::ET_FLOAT64:
                    comp_encoding = Image::FLOAT64;
                    break;
                case ReadOnlyScalarAttributeAdapter::ET_NONE:
                case ReadOnlyScalarAttributeAdapter::ET_VEC2:
                case ReadOnlyScalarAttributeAdapter::ET_VEC3:		    
                    break;
                }
                
                attribute_texture_ = new Texture;
                attribute_texture_->create_from_data(
                    Memory::pointer(
                        attribute.attribute_store()->data()
                    ),
                    Image::GRAY,
                    comp_encoding,
                    voxel_grob()->nu(),
                    voxel_grob()->nv(),
                    voxel_grob()->nw(),
                    colormap_style_.smooth ? GL_LINEAR : GL_NEAREST,
                    GL_CLAMP_TO_EDGE
               );
            }
        }
	if(!attribute_texture_.is_null()) {
	    attribute_texture_->set_filtering(
		colormap_style_.smooth ? GL_LINEAR : GL_NEAREST
	    );
	    attribute_texture_->set_wrapping(GL_CLAMP_TO_EDGE);
	}
    }

    void PlainVoxelGrobShader::draw_wireframe_box() {
        const vec3& origin = voxel_grob()->origin();
        const vec3& U = voxel_grob()->U();
        const vec3& V = voxel_grob()->V();
        const vec3& W = voxel_grob()->W();            
        
        vec3 p000 = origin;
        vec3 p100 = origin + U;
        vec3 p010 = origin + V;
        vec3 p001 = origin + W;
        vec3 p011 = origin + V + W;
        vec3 p101 = origin + U + W;
        vec3 p110 = origin + U + V;
        vec3 p111 = origin + U + V + W;
        
        glupDisable(GLUP_VERTEX_COLORS);
        glupDisable(GLUP_TEXTURING);
        glupSetColor4dv(GLUP_MESH_COLOR, box_style_.color.data());
        glupSetMeshWidth(GLUPint(box_style_.width));
            
        glupBegin(GLUP_LINES);
        glupVertex(p000);
        glupVertex(p100);
        glupVertex(p100);
        glupVertex(p110);
        glupVertex(p110);
        glupVertex(p010);
        glupVertex(p010);            
        glupVertex(p000);
        glupVertex(p001);
        glupVertex(p101);
        glupVertex(p101);
        glupVertex(p111);
        glupVertex(p111);
        glupVertex(p011);
        glupVertex(p011);            
        glupVertex(p001);
        glupVertex(p000);
        glupVertex(p001);
        glupVertex(p100);
        glupVertex(p101);
        glupVertex(p110);
        glupVertex(p111);
        glupVertex(p010);            
        glupVertex(p011);
        glupEnd();
    }
    
    void PlainVoxelGrobShader::draw_volume(
        Texture* colormap_texture,
        double attribute_min,
        double attribute_max,
        index_t repeat
    ) {
        const vec3& origin = voxel_grob()->origin();
        const vec3& U = voxel_grob()->U();
        const vec3& V = voxel_grob()->V();
        const vec3& W = voxel_grob()->W();            
        
        vec3 p000 = origin;
        vec3 p100 = origin + U;
        vec3 p010 = origin + V;
        vec3 p001 = origin + W;
        vec3 p011 = origin + V + W;
        vec3 p101 = origin + U + W;
        vec3 p110 = origin + U + V;
        vec3 p111 = origin + U + V + W;

        vec3 u000(0.0, 0.0, 0.0);
        vec3 u100(1.0, 0.0, 0.0);
        vec3 u010(0.0, 1.0, 0.0);
        vec3 u001(0.0, 0.0, 1.0);
        vec3 u011(0.0, 1.0, 1.0);
        vec3 u101(1.0, 0.0, 1.0);
        vec3 u110(1.0, 1.0, 0.0);
        vec3 u111(1.0, 1.0, 1.0);        

        colormap_texture->bind();
        attribute_texture_->bind();
                
        glupEnable(GLUP_INDIRECT_TEXTURING);
        glupMapTexCoords1d(attribute_min, attribute_max, repeat);
        
        glupBegin(GLUP_HEXAHEDRA);
        glupTexCoord(u000);
        glupVertex(p000);
        glupTexCoord(u100);
        glupVertex(p100);
        glupTexCoord(u010);
        glupVertex(p010);
        glupTexCoord(u110);
        glupVertex(p110);
        glupTexCoord(u001);
        glupVertex(p001);
        glupTexCoord(u101);
        glupVertex(p101);
        glupTexCoord(u011);
        glupVertex(p011);
        glupTexCoord(u111);
        glupVertex(p111);
        glupEnd();

        glupDisable(GLUP_INDIRECT_TEXTURING);
        
        attribute_texture_->unbind();
        colormap_texture->unbind();

        glupMatrixMode(GLUP_TEXTURE_MATRIX);
        glupLoadIdentity();
        glupMatrixMode(GLUP_MODELVIEW_MATRIX);        
        
    }
    
    void PlainVoxelGrobShader::pick_object(index_t object_id) {
        geo_argused(object_id);
        // TODO
        picking_ = true;
        draw();
        picking_ = false;
    }

    void PlainVoxelGrobShader::blink() {
        // TODO
        update();
    }
}

