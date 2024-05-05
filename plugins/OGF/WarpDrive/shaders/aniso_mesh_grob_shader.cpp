
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
 *  Contact for this Plugin: OGF
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
 

#include <OGF/WarpDrive/shaders/aniso_mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>

namespace {

    // Fast ellipsoid renderer, may have some precision issue
    // with very skinny ellipsoids
    const char* fp32_source =
        R"(
        //primitive GLUP_POINTS
        )"

        // Vertex shader
        // Input point and basis as GLUP_POINTS + attributes
        // basis is encoded in (color,tex_color,normal)
        // Transforms points and basis into clip space
        // Computes inverse transform of basis Minv
        R"(
        //stage GL_VERTEX_SHADER
        //import <GLUP/current_profile/vertex_shader_preamble.h>
        //import <GLUPGLSL/state.h>
        //import <GLUP/stdglup.h>
        //import <GLUP/current_profile/toggles.h>
        //import <GLUP/current_profile/primitive.h>
        in vec4 vertex_in;
        in vec4 color_in;                          
        in vec4 tex_coord_in;
        in vec4 normal_in;
        out VertexData {
          vec3 p;            // current point
          mat3 Minv;         // M = [U|V|W]; Minv = M^-1 
          mat3 M_clip_space; // M_clip_space = MVP * M
        } VertexOut;
        void main(void) {
           VertexOut.p  = vertex_in.xyz;
           gl_Position = GLUP.modelviewprojection_matrix*vertex_in;
           mat3 MVP33 = mat3(GLUP.modelviewprojection_matrix);
           VertexOut.M_clip_space = MVP33*mat3(
              color_in.xyz, tex_coord_in.xyz, normal_in.xyz
           );
           vec3 Up = vec3(color_in.xyz);
           vec3 Vp = vec3(tex_coord_in.xyz);
           vec3 Wp = vec3(normal_in.xyz);
           VertexOut.Minv = transpose(mat3(
              Up/dot(Up,Up), Vp/dot(Vp,Vp), Wp/dot(Wp,Wp)
           ));
        }
        )"
        
        // Geometry shader
        // Generates a box around each ellipsoid
        R"(
        //stage GL_GEOMETRY_SHADER
        #version 440
        #define GLUP_GEOMETRY_SHADER
        layout(points) in;
        layout(triangle_strip, max_vertices = 18) out;
        in VertexData {
           vec3 p;
           mat3 Minv;
           mat3 M_clip_space;
        } VertexIn[];
        out VertexData {
           flat vec3 p;
           flat mat3 Minv;
        } VertexOut;
        void cube_vrtx(int i) {
           vec3 delta = vec3(float(i&1),float((i&2)>>1),float((i&4)>>2));
           delta = vec3(-1.0, -1.0, -1.0) + 2.0*delta;
           gl_Position = gl_in[0].gl_Position + 
              vec4(VertexIn[0].M_clip_space*delta,0.0); 
           EmitVertex();
        }
        void main() {
           VertexOut.p    = VertexIn[0].p;
           VertexOut.Minv = VertexIn[0].Minv;
           cube_vrtx(6); cube_vrtx(7); cube_vrtx(4); cube_vrtx(5);
           cube_vrtx(0); cube_vrtx(1); cube_vrtx(2); cube_vrtx(3);
           cube_vrtx(6); cube_vrtx(7); 
           EndPrimitive();
           cube_vrtx(4); cube_vrtx(0); cube_vrtx(6); cube_vrtx(2);
           EndPrimitive();
           cube_vrtx(1); cube_vrtx(5); cube_vrtx(3); cube_vrtx(7);
           EndPrimitive();  
        }
        )"

        // Fragment shader
        // Displays ellipsoids by ray-tracing
        R"(
        //stage GL_FRAGMENT_SHADER
        //import <GLUP/current_profile/fragment_shader_preamble.h>
        //import <GLUPGLSL/state.h>
        //import <GLUP/stdglup.h>
        //import <GLUP/current_profile/toggles.h>
        //import <GLUP/fragment_shader_utils.h>
        //import <GLUP/fragment_ray_tracing.h>
        in VertexData {
           flat vec3  p;
           flat mat3 Minv;
        } FragmentIn;
        void main() {
           if(!gl_FrontFacing) discard; 
           if(glupIsEnabled(GLUP_CLIPPING)) {
              if(dot(vec4(FragmentIn.p,1.0),GLUP.world_clip_plane) < 0.0) {
                  discard; 
              }        
           }
           Ray R = glup_primary_ray();
           vec3 D = FragmentIn.Minv * (R.O - FragmentIn.p); 
           vec3 v = FragmentIn.Minv * R.V; 
           float a   = dot(v,v); 
           float b_p = dot(D,v); 
           float c   = dot(D,D) - 1.0; 
           float delta_p = b_p*b_p - a*c; 
           if(delta_p < 0.0) discard; 
           float t = -(b_p+sqrt(delta_p))/a;
           vec3 I = R.O + t*R.V; 
           glup_update_depth(I); 
           vec4 result = GLUP.front_color;
           if(glupIsEnabled(GLUP_LIGHTING)) {
              vec3 w = I-FragmentIn.p; 
              vec3 N = transpose(FragmentIn.Minv)*(FragmentIn.Minv*w); 
              N = normalize(GLUP.normal_matrix*N); 
              if(result.r<0.01 && result.g<0.01 && result.b<0.01) {
                 result = vec4(0.5*(N+vec3(1.0, 1.0, 1.0)),1.0);
              }
              result = glup_lighting(result, N);
          }
          glup_FragColor = result;
        }
        )";
    

    /***************************************************************/

    // More accurate ellipsoid renderer, uses double precision numbers
    // (much slower on most graphic boards)
    const char* fp64_source =
        R"(
        //primitive GLUP_POINTS
        )"
            
        // Vertex shader
        // Input point and basis as GLUP_POINTS + attributes
        // basis is encoded in (color,tex_color,normal)
        // Transforms points and basis into clip space
        // Computes inverse transform of basis Minv
        R"(
        //stage GL_VERTEX_SHADER
        //import <GLUP/current_profile/vertex_shader_preamble.h>
        //import <GLUPGLSL/state.h>
        //import <GLUP/stdglup.h>
        //import <GLUP/current_profile/toggles.h>
        //import <GLUP/current_profile/primitive.h>
        in vec4 vertex_in;
        in vec4 color_in;                          
        in vec4 tex_coord_in;
        in vec4 normal_in;
        out VertexData {
          vec3 p;             // current point
          dmat3 Minv;         // M = [U|V|W]; Minv = M^-1 
          mat3  M_clip_space; // M_clip_space = MVP * M
        } VertexOut;
        void main(void) {
           VertexOut.p  = vertex_in.xyz;
           gl_Position = GLUP.modelviewprojection_matrix*vertex_in;
           mat3 MVP33 = mat3(GLUP.modelviewprojection_matrix);
           VertexOut.M_clip_space = MVP33*mat3(
              color_in.xyz, tex_coord_in.xyz, normal_in.xyz
           );
           dvec3 Up = dvec3(color_in.xyz);
           dvec3 Vp = dvec3(tex_coord_in.xyz);
           dvec3 Wp = dvec3(normal_in.xyz);
           VertexOut.Minv = transpose(dmat3(
              Up/dot(Up,Up), Vp/dot(Vp,Vp), Wp/dot(Wp,Wp)
           ));
        }
        )"

        // Geometry shader
        // Generates a box around each ellipsoid
        R"(
        //stage GL_GEOMETRY_SHADER
        #version 440
        #define GLUP_GEOMETRY_SHADER
        layout(points) in;
        layout(triangle_strip, max_vertices = 18) out;
        in VertexData {
           vec3 p;
           dmat3 Minv;
           mat3 M_clip_space;
        } VertexIn[];
        out VertexData {
           flat vec3 p;
           flat dmat3 Minv;
        } VertexOut;
        void cube_vrtx(int i) {
           vec3 delta = vec3(float(i&1),float((i&2)>>1),float((i&4)>>2));
           delta = vec3(-1.0, -1.0, -1.0) + 2.0*delta;
           gl_Position = gl_in[0].gl_Position + 
              vec4(VertexIn[0].M_clip_space*delta,0.0); 
           EmitVertex();
        }
        void main() {
           VertexOut.p    = VertexIn[0].p;
           VertexOut.Minv = VertexIn[0].Minv;
           cube_vrtx(6); cube_vrtx(7); cube_vrtx(4); cube_vrtx(5);
           cube_vrtx(0); cube_vrtx(1); cube_vrtx(2); cube_vrtx(3);
           cube_vrtx(6); cube_vrtx(7); 
           EndPrimitive();
           cube_vrtx(4); cube_vrtx(0); cube_vrtx(6); cube_vrtx(2);
           EndPrimitive();
           cube_vrtx(1); cube_vrtx(5); cube_vrtx(3); cube_vrtx(7);
           EndPrimitive();  
        }
        )"
        
        // Fragment shader
        // Displays ellipsoids by ray-tracing
        R"(
        //stage GL_FRAGMENT_SHADER
        //import <GLUP/current_profile/fragment_shader_preamble.h>
        //import <GLUPGLSL/state.h>
        //import <GLUP/stdglup.h>
        //import <GLUP/current_profile/toggles.h>
        //import <GLUP/fragment_shader_utils.h>
        //import <GLUP/fragment_ray_tracing.h>
        in VertexData {
           flat vec3  p;
           flat dmat3 Minv;
        } FragmentIn;
        void main() {
           if(!gl_FrontFacing) discard; 
           if(glupIsEnabled(GLUP_CLIPPING)) {
              if(dot(vec4(FragmentIn.p,1.0),GLUP.world_clip_plane) < 0.0) {
                  discard; 
              }
           }
           Ray R = glup_primary_ray();
           dvec3 D = FragmentIn.Minv * (dvec3(R.O - FragmentIn.p)); 
           dvec3 v = FragmentIn.Minv * dvec3(R.V); 
           double a   = dot(v,v); 
           double b_p = dot(D,v); 
           double c   = dot(D,D) - 1.0; 
           double delta_p = b_p*b_p - a*c; 
           if(delta_p < 0.0) discard; 
           double t = -(b_p+sqrt(delta_p))/a;
           vec3 I = R.O + float(t)*R.V; 
           glup_update_depth(I); 
           vec4 result = GLUP.front_color;
           if(glupIsEnabled(GLUP_LIGHTING)) {
              vec3 w = I-FragmentIn.p; 
              mat3 fMinv = mat3(FragmentIn.Minv); 
              mat3 fMinvt = transpose(fMinv); 
              vec3 N = fMinvt*(fMinv*w); 
              N = normalize(GLUP.normal_matrix*N); 
              if(result.r<0.01 && result.g<0.01 && result.b<0.01) {
                 result = vec4(0.5*(N+vec3(1.0, 1.0, 1.0)),1.0);
              }
              result = glup_lighting(result, N);
          }
          glup_FragColor = result;
        };
        )";
}

namespace OGF {

    AnisoMeshGrobShader::AnisoMeshGrobShader(
        OGF::MeshGrob* grob
    ):MeshGrobShader(grob) {
        color_ = Color(0.0, 0.0, 0.0);
        scaling_ = 1.0;
        points_ = true;
        V0_ = true;
        V1_ = true;
        V2_ = true;
        fp64_program_ = 0;
        fp32_program_ = 0;
        ellipsoids_ = true;
        fp64_ = true;
        view_changed_ = false;
    }
        
    AnisoMeshGrobShader::~AnisoMeshGrobShader() {
        if(fp64_program_ != 0) {
            glDeleteProgram(fp64_program_);
        }
        if(fp32_program_ != 0) {
            glDeleteProgram(fp32_program_);
        }
    }        

    void AnisoMeshGrobShader::draw() {

        get_viewing_parameters();
        
        if(points_ && !ellipsoids_) {
            glupSetColor3d(
                GLUP_FRONT_AND_BACK_COLOR, color_.r(), color_.g(), color_.b()
            );
            glupSetPointSize(10.0);
            glupBegin(GLUP_POINTS);
            for(index_t v: mesh_grob()->vertices) {
                glupVertex3dv(mesh_grob()->vertices.point_ptr(v));
            }
            glupEnd();
        }

        if(ellipsoids_) {
            draw_ellipsoids();
        } else if(V0_ || V1_ || V2_) {
            draw_crosses();
        }

        if(view_changed_) {
            mesh_grob()->update(); // to make sure we redraw with fp64
        }
    }

    void AnisoMeshGrobShader::draw_ellipsoids() {

        if(fp64_program_ == 0 || fp32_program_ == 0) {

            std::string profile = glupCurrentProfileName();
            if(profile != "GLUP440") {
                Logger::err("AnisoShader")
                    << "Only supported with GLUP440"
                    << std::endl;
                return;
            }
            fp64_program_ = glupCompileProgram(fp64_source);
            fp32_program_ = glupCompileProgram(fp32_source);
        }
        
        Attribute<double> V0;
        Attribute<double> V1;
        Attribute<double> V2;

        V0.bind_if_is_defined(mesh_grob()->vertices.attributes(), "eigenV0");
        V1.bind_if_is_defined(mesh_grob()->vertices.attributes(), "eigenV1");
        V2.bind_if_is_defined(mesh_grob()->vertices.attributes(), "eigenV2");
        if(
            !V0.is_bound() || V0.dimension() != 3 ||
            !V1.is_bound() || V1.dimension() != 3 ||
            !V2.is_bound() || V2.dimension() != 3 
        ) {
            return;
        }

        glupSetColor3d(
            GLUP_FRONT_AND_BACK_COLOR, color_.r(), color_.g(), color_.b()
        );

        glupEnable(GLUP_VERTEX_COLORS);
        glupEnable(GLUP_VERTEX_NORMALS);
        glupEnable(GLUP_TEXTURING);
        glupUseProgram((fp64_&&!view_changed_) ? fp64_program_ : fp32_program_);
        glupBegin(GLUP_POINTS);
        for(index_t v: mesh_grob()->vertices) {
            vec3 p(mesh_grob()->vertices.point_ptr(v));
            vec3 U = scaling_ * vec3(V0[3*v], V0[3*v+1], V0[3*v+2]);
            vec3 V = scaling_ * vec3(V1[3*v], V1[3*v+1], V1[3*v+2]);
            vec3 W = scaling_ * vec3(V2[3*v], V2[3*v+1], V2[3*v+2]);
            glupColor3dv(U.data());
            glupTexCoord3dv(V.data());
            glupNormal3dv(W.data());
            glupVertex3dv(p.data());
        }
        glupEnd();
        glupUseProgram(0);
        glupDisable(GLUP_VERTEX_COLORS);
        glupDisable(GLUP_VERTEX_NORMALS);
        glupDisable(GLUP_TEXTURING);
        if(view_changed_) {
            mesh_grob()->update(); // to make sure we redraw with fp64 when still
        }
    }

    void AnisoMeshGrobShader::draw_crosses() {
        Attribute<double> V0;
        Attribute<double> V1;
        Attribute<double> V2;

        V0.bind_if_is_defined(mesh_grob()->vertices.attributes(), "eigenV0");
        V1.bind_if_is_defined(mesh_grob()->vertices.attributes(), "eigenV1");
        V2.bind_if_is_defined(mesh_grob()->vertices.attributes(), "eigenV2");
        if(
            !V0.is_bound() || V0.dimension() != 3 ||
            !V1.is_bound() || V1.dimension() != 3 ||
            !V2.is_bound() || V2.dimension() != 3 
        ) {
            return;
        }

        glupSetMeshWidth(2.0);
        glupSetColor3d(
            GLUP_MESH_COLOR, color_.r(), color_.g(), color_.b()
        );
        glupBegin(GLUP_LINES);
        for(index_t v: mesh_grob()->vertices) {
            vec3 p(mesh_grob()->vertices.point_ptr(v));

            if(V0_) {
                glupVertex3d(
                    p.x - scaling_*V0[3*v],
                    p.y - scaling_*V0[3*v+1],
                    p.z - scaling_*V0[3*v+2]
                );
            
                glupVertex3d(
                    p.x + scaling_*V0[3*v],
                    p.y + scaling_*V0[3*v+1],
                    p.z + scaling_*V0[3*v+2]
                );
            }

            if(V1_) {
                glupVertex3d(
                    p.x - scaling_*V1[3*v],
                    p.y - scaling_*V1[3*v+1],
                    p.z - scaling_*V1[3*v+2]
                );
                
                glupVertex3d(
                    p.x + scaling_*V1[3*v],
                    p.y + scaling_*V1[3*v+1],
                    p.z + scaling_*V1[3*v+2]
                );
            }

            if(V2_) {
                glupVertex3d(
                    p.x - scaling_*V2[3*v],
                    p.y - scaling_*V2[3*v+1],
                    p.z - scaling_*V2[3*v+2]
                );
                
                glupVertex3d(
                    p.x + scaling_*V2[3*v],
                    p.y + scaling_*V2[3*v+1],
                    p.z + scaling_*V2[3*v+2]
                );
            }
        }
        glupEnd();
    }


    void AnisoMeshGrobShader::get_viewing_parameters() {
        GLUPdouble modelview_bkp[16];
        GLUPdouble project_bkp[16];
        GLUPint viewport_bkp[4];

        Memory::copy(modelview_bkp, modelview_, sizeof(modelview_));
        Memory::copy(project_bkp, project_, sizeof(project_));
        Memory::copy(viewport_bkp, viewport_, sizeof(viewport_));

	glGetIntegerv(GL_VIEWPORT, viewport_);
	glupGetMatrixdv(GLUP_MODELVIEW_MATRIX, modelview_);
	glupGetMatrixdv(GLUP_PROJECTION_MATRIX, project_);
        
        view_changed_ = false;
        for(index_t i=0; i<16; ++i) {
            view_changed_ = view_changed_||(modelview_[i] != modelview_bkp[i]);
            view_changed_ = view_changed_||(project_[i] != project_bkp[i]);
        }
        for(index_t i=0; i<4; ++i) {
            view_changed_ = view_changed_||(viewport_[i] != viewport_bkp[i]);
        }
    }
    
}

