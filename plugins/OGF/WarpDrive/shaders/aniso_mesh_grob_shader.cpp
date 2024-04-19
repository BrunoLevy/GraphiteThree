
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
        program_ = 0;
        ellipsoids_ = true;
    }
        
    AnisoMeshGrobShader::~AnisoMeshGrobShader() {
        if(program_ != 0) {
            glDeleteProgram(program_);
        }
    }        

    void AnisoMeshGrobShader::draw() {

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
    }

    void AnisoMeshGrobShader::draw_ellipsoids() {

        if(program_ == 0) {

            std::string profile = glupCurrentProfileName();
            if(profile != "GLUP150" && profile != "GLUP440") {
                Logger::err("AnisoShader")
                    << "Only supported with GLUP150 or GLUP440"
                    << std::endl;
                return;
            }

            program_ = glupCompileProgram(
                "//primitive GLUP_POINTS\n"

                // Vertex shader
                // Input point and basis as GLUP_POINTS + attributes
                // basis is encoded in (color,tex_color,normal)
                // Transforms points and basis into clip space
                // Computes inverse transform of basis (Up, Vp, Wp)
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUP/current_profile/vertex_shader_preamble.h>\n"
                "//import <GLUPGLSL/state.h>\n"
                "//import <GLUP/stdglup.h>\n"
                "//import <GLUP/current_profile/toggles.h>\n"
                "//import <GLUP/current_profile/primitive.h>\n"
                "in vec4 vertex_in;\n"
                "in vec4 color_in;\n"                          
                "in vec4 tex_coord_in;\n"
                "in vec4 normal_in;\n"
                "out VertexData {\n"
                "  vec3 p;\n"
                "  vec3 Up;\n" // [  Up  ]     [   |   |   ] -1
                "  vec3 Vp;\n" // [  Vp  ]  =  [ U | V | W ]
                "  vec3 Wp;\n" // [  Wp  ]     [   |   |   ]
                "  vec3 U_clip_space;\n"
                "  vec3 V_clip_space;\n"
                "  vec3 W_clip_space;\n"
                "} VertexOut;\n"
                "void main(void) {\n"
                "   VertexOut.p  = vertex_in.xyz;\n"
                "   VertexOut.Up = color_in.xyz;\n"
                "   VertexOut.Vp = tex_coord_in.xyz;\n"
                "   VertexOut.Wp = normal_in.xyz;\n"
                "   gl_Position = GLUP.modelviewprojection_matrix*vertex_in;\n"
                "   mat3 M = mat3(GLUP.modelviewprojection_matrix);\n"
                "   VertexOut.U_clip_space = M*VertexOut.Up;\n"
                "   VertexOut.V_clip_space = M*VertexOut.Vp;\n"
                "   VertexOut.W_clip_space = M*VertexOut.Wp;\n"
                "   VertexOut.Up = (1.0/dot(VertexOut.Up,VertexOut.Up))*VertexOut.Up;\n"
                "   VertexOut.Vp = (1.0/dot(VertexOut.Vp,VertexOut.Vp))*VertexOut.Vp;\n"
                "   VertexOut.Wp = (1.0/dot(VertexOut.Wp,VertexOut.Wp))*VertexOut.Wp;\n"                
                "}\n"

                // Geometry shader
                // Generates a box around each ellipsoid
                "//stage GL_GEOMETRY_SHADER\n"
                "#version 150\n"
                "#define GLUP_GEOMETRY_SHADER\n"
                "layout(points) in;\n"
                "layout(triangle_strip, max_vertices = 18) out;\n"
                "in VertexData {\n"
                "  vec3 p;\n"
                "  vec3 Up;\n"
                "  vec3 Vp;\n"
                "  vec3 Wp;\n"
                "  vec3 U_clip_space;\n"
                "  vec3 V_clip_space;\n"
                "  vec3 W_clip_space;\n"
                "} VertexIn[];\n"
                "out VertexData {\n"
                "  flat vec3 p;\n"
                "  flat vec3 Up;\n"
                "  flat vec3 Vp;\n"
                "  flat vec3 Wp;\n"
                "} VertexOut;\n"
                "void emit_vertex_2(int i) {\n"
                "  vec3 delta = vec3(float(i&1),float((i&2)>>1),float((i&4)>>2));\n"
                "  delta = vec3(-1.0, -1.0, -1.0) + 2.0*delta;\n"
                "  gl_Position = gl_in[0].gl_Position + \n"
                "     delta.x*vec4(VertexIn[0].U_clip_space,0.0)+\n"
                "     delta.y*vec4(VertexIn[0].V_clip_space,0.0)+\n"
                "     delta.z*vec4(VertexIn[0].W_clip_space,0.0);\n"
                "  EmitVertex();"
                "}\n"
                "void main() {\n"
                "  VertexOut.p  = VertexIn[0].p;\n"
                "  VertexOut.Up = VertexIn[0].Up;\n"
                "  VertexOut.Vp = VertexIn[0].Vp;\n"
                "  VertexOut.Wp = VertexIn[0].Wp;\n"
                "  emit_vertex_2(6);\n"
                "  emit_vertex_2(7);\n"
                "  emit_vertex_2(4);\n"
                "  emit_vertex_2(5);\n"
                "  emit_vertex_2(0);\n"
                "  emit_vertex_2(1);\n"
                "  emit_vertex_2(2);\n"
                "  emit_vertex_2(3);\n"
                "  emit_vertex_2(6);\n"
                "  emit_vertex_2(7);\n"
                "  EndPrimitive();\n"
                "  emit_vertex_2(4);\n"
                "  emit_vertex_2(0);\n"
                "  emit_vertex_2(6);\n"
                "  emit_vertex_2(2);\n"
                "  EndPrimitive();\n"
                "  emit_vertex_2(1);\n"
                "  emit_vertex_2(5);\n"
                "  emit_vertex_2(3);\n"
                "  emit_vertex_2(7);\n"
                "  EndPrimitive();\n"  
                "}\n"

                // Fragment shader
                // Displays ellipsoids by ray-tracing
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUP/current_profile/fragment_shader_preamble.h>\n"
                "//import <GLUPGLSL/state.h>\n"
                "//import <GLUP/stdglup.h>\n"
                "//import <GLUP/current_profile/toggles.h>\n"
                "//import <GLUP/fragment_shader_utils.h>\n"
                "//import <GLUP/fragment_ray_tracing.h>\n"
                "in VertexData {\n"
                "  flat vec3 p;\n"
                "  flat vec3 Up;\n"
                "  flat vec3 Vp;\n"
                "  flat vec3 Wp;\n"
                "} FragmentIn;\n"
                "void main() {\n"
                "  if(!gl_FrontFacing) discard; \n"
                "  mat3 Mt = mat3(FragmentIn.Up, FragmentIn.Vp, FragmentIn.Wp); \n"
                "  mat3 M  = transpose(Mt); \n" // M is the inverse basis transform
                "  Ray R = glup_primary_ray();\n"
                "  vec3 D = M * (R.O - FragmentIn.p); \n"
                "  vec3 v = M * R.V; \n"
                "  float a   = dot(v,v); \n"
                "  float b_p = dot(D,v); \n"
                "  float c   = dot(D,D) - 1.0; \n"
                "  float delta_p = b_p*b_p - a*c; \n"
                "  if(delta_p < 0.0) discard; \n"
                "  float t = -(b_p+sqrt(delta_p))/a;\n"
                "  vec3 I = R.O + t*R.V; \n"
                "  glup_update_depth(I); \n"
                "  vec4 result = GLUP.front_color;\n"
                "  if(glupIsEnabled(GLUP_LIGHTING)) {\n"
                "     vec3 w = I-FragmentIn.p; \n"
                "     vec3 N = Mt*(M*w); \n"
                "     N = normalize(GLUP.normal_matrix*N); \n"
                "     if(result.r < 0.01 && result.g < 0.01 && result.b < 0.01) {\n"
                "        result = vec4(0.5*(N+vec3(1.0, 1.0, 1.0)),1.0);\n"
                "     }\n"
                "     result = glup_lighting(result, N);\n"
                "  }\n"
                "  glup_FragColor = result;\n"
                "}\n"
            );
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
        glupUseProgram(program_);
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
    
}
