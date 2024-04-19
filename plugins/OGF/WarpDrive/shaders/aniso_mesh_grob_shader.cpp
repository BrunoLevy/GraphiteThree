
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
        "  vec3 p;\n"            // current point
        "  mat3 Minv;\n"         // M = [U|V|W]; Minv = M^-1 
        "  mat3 M_clip_space;\n" // M_clip_space = MVP * M
        "} VertexOut;\n"
        "void main(void) {\n"
        "   VertexOut.p  = vertex_in.xyz;\n"
        "   gl_Position = GLUP.modelviewprojection_matrix*vertex_in;\n"
        "   mat3 MVP33 = mat3(GLUP.modelviewprojection_matrix);\n"
        "   VertexOut.M_clip_space = MVP33*mat3(\n"
        "      color_in.xyz, tex_coord_in.xyz, normal_in.xyz\n"
        "   );\n"
        "   vec3 Up = vec3(color_in.xyz);\n"
        "   vec3 Vp = vec3(tex_coord_in.xyz);\n"
        "   vec3 Wp = vec3(normal_in.xyz);\n"
        "   VertexOut.Minv = transpose(mat3(\n"
        "      Up/dot(Up,Up), Vp/dot(Vp,Vp), Wp/dot(Wp,Wp)\n"
        "   ));\n"
        "}\n"

        // Geometry shader
        // Generates a box around each ellipsoid
        "//stage GL_GEOMETRY_SHADER\n"
        "#version 440\n"
        "#define GLUP_GEOMETRY_SHADER\n"
        "layout(points) in;\n"
        "layout(triangle_strip, max_vertices = 18) out;\n"
        "in VertexData {\n"
        "   vec3 p;\n"
        "   mat3 Minv;\n"
        "   mat3 M_clip_space;\n"
        "} VertexIn[];\n"
        "out VertexData {\n"
        "   flat vec3 p;\n"
        "   flat mat3 Minv;\n"
        "} VertexOut;\n"
        "void cube_vrtx(int i) {\n"
        "   vec3 delta = vec3(\n"
        "      float(i&1),float((i&2)>>1),float((i&4)>>2)\n"
        "   );\n"
        "   delta = vec3(-1.0, -1.0, -1.0) + 2.0*delta;\n"
        "   gl_Position = gl_in[0].gl_Position + \n"
        "      vec4(VertexIn[0].M_clip_space*delta,0.0); \n"
        "   EmitVertex();"
        "}\n"
        "void main() {\n"
        "   VertexOut.p    = VertexIn[0].p;\n"
        "   VertexOut.Minv = VertexIn[0].Minv;\n"
        "   cube_vrtx(6); cube_vrtx(7); cube_vrtx(4); cube_vrtx(5);\n"
        "   cube_vrtx(0); cube_vrtx(1); cube_vrtx(2); cube_vrtx(3);\n"
        "   cube_vrtx(6); cube_vrtx(7); \n"
        "   EndPrimitive();\n"
        "   cube_vrtx(4); cube_vrtx(0); cube_vrtx(6); cube_vrtx(2);\n"
        "   EndPrimitive();\n"
        "   cube_vrtx(1); cube_vrtx(5); cube_vrtx(3); cube_vrtx(7);\n"
        "   EndPrimitive();\n"  
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
        "   flat vec3  p;\n"
        "   flat mat3 Minv;\n"
        "} FragmentIn;\n"
        "void main() {\n"
        "   if(!gl_FrontFacing) discard; \n"
        "   if(glupIsEnabled(GLUP_CLIPPING)) {\n"
        "      if(dot(\n"
        "         vec4(FragmentIn.p,1.0),\n"
        "         GLUP.world_clip_plane\n"
        "      ) < 0.0) discard; \n"
        "   }\n"
        "   Ray R = glup_primary_ray();\n"
        "   vec3 D = FragmentIn.Minv * (R.O - FragmentIn.p); \n"
        "   vec3 v = FragmentIn.Minv * R.V; \n"
        "   float a   = dot(v,v); \n"
        "   float b_p = dot(D,v); \n"
        "   float c   = dot(D,D) - 1.0; \n"
        "   float delta_p = b_p*b_p - a*c; \n"
        "   if(delta_p < 0.0) discard; \n"
        "   float t = -(b_p+sqrt(delta_p))/a;\n"
        "   vec3 I = R.O + t*R.V; \n"
        "   glup_update_depth(I); \n"
        "   vec4 result = GLUP.front_color;\n"
        "   if(glupIsEnabled(GLUP_LIGHTING)) {\n"
        "      vec3 w = I-FragmentIn.p; \n"
        "      vec3 N = transpose(FragmentIn.Minv)*(FragmentIn.Minv*w); \n"
        "      N = normalize(GLUP.normal_matrix*N); \n"
        "      if(result.r<0.01 && result.g<0.01 && result.b<0.01) {\n"
        "         result = vec4(0.5*(N+vec3(1.0, 1.0, 1.0)),1.0);\n"
        "      }\n"
        "      result = glup_lighting(result, N);\n"
        "  }\n"
        "  glup_FragColor = result;\n"
        "}\n";
    

    /***************************************************************/

    // More accurate ellipsoid renderer, uses double precision numbers
    // (much slower on most graphic boards)
    const char* fp64_source = 
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
        "  vec3 p;\n"             // current point
        "  dmat3 Minv;\n"         // M = [U|V|W]; Minv = M^-1 
        "  mat3  M_clip_space;\n" // M_clip_space = MVP * M
        "} VertexOut;\n"
        "void main(void) {\n"
        "   VertexOut.p  = vertex_in.xyz;\n"
        "   gl_Position = GLUP.modelviewprojection_matrix*vertex_in;\n"
        "   mat3 MVP33 = mat3(GLUP.modelviewprojection_matrix);\n"
        "   VertexOut.M_clip_space = MVP33*mat3(\n"
        "      color_in.xyz, tex_coord_in.xyz, normal_in.xyz\n"
        "   );\n"
        "   dvec3 Up = dvec3(color_in.xyz);\n"
        "   dvec3 Vp = dvec3(tex_coord_in.xyz);\n"
        "   dvec3 Wp = dvec3(normal_in.xyz);\n"
        "   VertexOut.Minv = transpose(dmat3(\n"
        "      Up/dot(Up,Up), Vp/dot(Vp,Vp), Wp/dot(Wp,Wp)\n"
        "   ));\n"
        "}\n"

        // Geometry shader
        // Generates a box around each ellipsoid
        "//stage GL_GEOMETRY_SHADER\n"
        "#version 440\n"
        "#define GLUP_GEOMETRY_SHADER\n"
        "layout(points) in;\n"
        "layout(triangle_strip, max_vertices = 18) out;\n"
        "in VertexData {\n"
        "   vec3 p;\n"
        "   dmat3 Minv;\n"
        "   mat3 M_clip_space;\n"
        "} VertexIn[];\n"
        "out VertexData {\n"
        "   flat vec3 p;\n"
        "   flat dmat3 Minv;\n"
        "} VertexOut;\n"
        "void cube_vrtx(int i) {\n"
        "   vec3 delta = vec3(\n"
        "      float(i&1),float((i&2)>>1),float((i&4)>>2)\n"
        "   );\n"
        "   delta = vec3(-1.0, -1.0, -1.0) + 2.0*delta;\n"
        "   gl_Position = gl_in[0].gl_Position + \n"
        "      vec4(VertexIn[0].M_clip_space*delta,0.0); \n"
        "   EmitVertex();"
        "}\n"
        "void main() {\n"
        "   VertexOut.p    = VertexIn[0].p;\n"
        "   VertexOut.Minv = VertexIn[0].Minv;\n"
        "   cube_vrtx(6); cube_vrtx(7); cube_vrtx(4); cube_vrtx(5);\n"
        "   cube_vrtx(0); cube_vrtx(1); cube_vrtx(2); cube_vrtx(3);\n"
        "   cube_vrtx(6); cube_vrtx(7); \n"
        "   EndPrimitive();\n"
        "   cube_vrtx(4); cube_vrtx(0); cube_vrtx(6); cube_vrtx(2);\n"
        "   EndPrimitive();\n"
        "   cube_vrtx(1); cube_vrtx(5); cube_vrtx(3); cube_vrtx(7);\n"
        "   EndPrimitive();\n"  
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
        "   flat vec3  p;\n"
        "   flat dmat3 Minv;\n"
        "} FragmentIn;\n"
        "void main() {\n"
        "   if(!gl_FrontFacing) discard; \n"
        "   if(glupIsEnabled(GLUP_CLIPPING)) {\n"
        "      if(dot(\n"
        "         vec4(FragmentIn.p,1.0),\n"
        "         GLUP.world_clip_plane\n"
        "      ) < 0.0) discard; \n"
        "   }\n"
        "   Ray R = glup_primary_ray();\n"
        "   dvec3 D = FragmentIn.Minv * (dvec3(R.O - FragmentIn.p)); \n"
        "   dvec3 v = FragmentIn.Minv * dvec3(R.V); \n"
        "   double a   = dot(v,v); \n"
        "   double b_p = dot(D,v); \n"
        "   double c   = dot(D,D) - 1.0; \n"
        "   double delta_p = b_p*b_p - a*c; \n"
        "   if(delta_p < 0.0) discard; \n"
        "   double t = -(b_p+sqrt(delta_p))/a;\n"
        "   vec3 I = R.O + float(t)*R.V; \n"
        "   glup_update_depth(I); \n"
        "   vec4 result = GLUP.front_color;\n"
        "   if(glupIsEnabled(GLUP_LIGHTING)) {\n"
        "      vec3 w = I-FragmentIn.p; \n"
        "      mat3 fMinv = mat3(FragmentIn.Minv); \n"
        "      mat3 fMinvt = transpose(fMinv); \n"
        "      vec3 N = fMinvt*(fMinv*w); \n"
        "      N = normalize(GLUP.normal_matrix*N); \n"
        "      if(result.r<0.01 && result.g<0.01 && result.b<0.01) {\n"
        "         result = vec4(0.5*(N+vec3(1.0, 1.0, 1.0)),1.0);\n"
        "      }\n"
        "      result = glup_lighting(result, N);\n"
        "  }\n"
        "  glup_FragColor = result;\n"
        "}\n";
    
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
        program_ = 0;
        ellipsoids_ = true;
        fp64_ = true;
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
            if(profile != "GLUP440") {
                Logger::err("AnisoShader")
                    << "Only supported with GLUP440"
                    << std::endl;
                return;
            }
            program_ = glupCompileProgram(fp64_ ? fp64_source : fp32_source);
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
