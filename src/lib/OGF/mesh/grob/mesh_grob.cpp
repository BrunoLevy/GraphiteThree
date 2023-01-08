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
 * *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with the 
 *  following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#include <OGF/mesh/grob/mesh_grob.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph/types/geofile.h>
#include <OGF/gom/reflection/meta_class.h>

#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/basic/file_system.h>

namespace OGF {
    
    MeshGrob::MeshGrob(CompositeGrob* parent) :
        Grob(parent)
    {
        initialize_name("mesh");
    }

    MeshGrob::~MeshGrob() {
    }
    
    void MeshGrob::update() {
        Grob::update();
    }
    
    bool MeshGrob::load(const FileName& value) {
        MeshIOFlags flags;
	flags.set_attributes(MESH_ALL_ATTRIBUTES);
        bool result = GEO::mesh_load(value, *this, flags);
	if(result) {
	  if(vertices.single_precision()) {
	     vertices.set_double_precision();
	  }
	  if(vertices.dimension() == 2) {
	    vertices.set_dimension(3);
	  }
	}
        update();

        // If the mesh only has points,
        // then activate points display.
        Shader* shader = get_shader();
        if(shader != nullptr && 
           vertices.nb() != 0 &&
           edges.nb() == 0 &&
           facets.nb() == 0 &&
           cells.nb() == 0
        ) {
            shader->set_property("vertices_style", "false;0 1 0 1;2");
        }
        return result;
    }
    
    bool MeshGrob::append(const FileName& value) {
        Logger::warn("MeshGrob") << "append() not implemented"
                                 << std::endl;
        bool result = GEO::mesh_load(value, *this);
        update();
        return result;
    }
    
    bool MeshGrob::save(const NewFileName& value) {
	if(FileSystem::extension(value) == "graphite") {
	    return Grob::save(value);
	}
        return GEO::mesh_save(*this, value);
    }
    
    void MeshGrob::clear() {
        GEO::Mesh::clear();
        update();
    }

    Grob* MeshGrob::duplicate(SceneGraph* sg) {
        MeshGrob* result = dynamic_cast<MeshGrob*>(Grob::duplicate(sg));
        ogf_assert(result != nullptr);
        MeshElementsFlags what = MeshElementsFlags(
            MESH_VERTICES | MESH_EDGES | MESH_FACETS | MESH_CELLS
        );
        result->copy(*this, true, what);
        result->update();
        return result;
    }
    
    Box3d MeshGrob::bbox() const {
        Box3d result;

        // If there is a vertex filter, apply it.
        Shader* shader = get_shader();
        if(shader != nullptr) {
            if(shader->has_property("vertices_filter")) {
                std::string prop;
                shader->get_property("vertices_filter", prop);
                if(prop == "true") {
                    Attribute<Numeric::uint8> filter;
                    filter.bind_if_is_defined(this->vertices.attributes(),"filter");
                    if(filter.is_bound()) {
                        for(index_t v: vertices) {
                            if(filter[v] != 0) {
                                result.add_point(vec3(vertices.point_ptr(v)));
                            }
                        }
                        return result;
                    }
                }
            }
        }
        
        if(vertices.nb() != 0) {
            double xyzmin[3];
            double xyzmax[3];
            GEO::get_bbox(*this, xyzmin, xyzmax);
            result.add_point(vec3(xyzmin));
            result.add_point(vec3(xyzmax));
        }
        
        return result;
    }
    
    MeshGrob* MeshGrob::find_or_create(
        SceneGraph* sg, const std::string& name
    ) {
        MeshGrob* result = find(sg, name);
        if(result == nullptr) {
            std::string cur_grob_bkp = sg->get_current_object();
            result = dynamic_cast<MeshGrob*>(
                sg->create_object("OGF::MeshGrob")
            );
            ogf_assert(result != nullptr);
            result->rename(name);
            sg->set_current_object(result->name());
	    if(sg->is_bound(cur_grob_bkp)) { 
		sg->set_current_object(cur_grob_bkp);
	    }
        }
        return result;
    }
    
    MeshGrob* MeshGrob::find(SceneGraph* sg, const std::string& name) {
        MeshGrob* result = nullptr;
        if(sg->is_bound(name)) {
            result = dynamic_cast<MeshGrob*>(sg->resolve(name));
        }
        return result;
    }


    void MeshGrob::register_geogram_file_extensions() {

        std::vector<std::string> geogram_extensions;
        GEO::MeshIOHandlerFactory::list_creators(
            geogram_extensions
        );

        for(index_t i=0; i<geogram_extensions.size(); ++i) {
            const std::string& extension = geogram_extensions[i];
            SceneGraphLibrary::instance()->
                register_grob_read_file_extension(
                    "OGF::MeshGrob", extension
                );
            SceneGraphLibrary::instance()->
                register_grob_write_file_extension(
                    "OGF::MeshGrob", extension
                );
        }
    }


    std::string MeshGrob::get_attributes() const {
	return Mesh::get_attributes();
    }
    
    std::string MeshGrob::get_scalar_attributes() const {
        return Mesh::get_scalar_attributes();
    }

    std::string MeshGrob::get_selections() const {
        std::string result = "";
        static MeshElementsFlags elements[] = {
            MESH_VERTICES, MESH_FACETS, MESH_CELLS
        };
        for(index_t i=0; i<sizeof(elements)/sizeof(MeshElementsFlags); ++i) {
            if(
                get_subelements_by_type(elements[i]).attributes().is_defined(
                    "selection"
                )
            ) {
                if(result != "") {
                    result += ";";
                }
                result += subelements_type_to_name(elements[i]) + ".selection";
            }
        }
        return result;
    }

    std::string MeshGrob::get_filters() const {
        std::string result = "";
        static MeshElementsFlags elements[] = {
            MESH_VERTICES, MESH_FACETS, MESH_CELLS
        };
        for(index_t i=0; i<sizeof(elements)/sizeof(MeshElementsFlags); ++i) {
            if(
                get_subelements_by_type(elements[i]).attributes().is_defined(
                    "filter"
                )
            ) {
                if(result != "") {
                    result += ";";
                }
                result += subelements_type_to_name(elements[i]) + ".filter";
            }
        }
        return result;
    }
    
    bool MeshGrob::is_serializable() const {
        return true;
    }
    
    bool MeshGrob::serialize_read(InputGraphiteFile& geofile) {
        bool result = mesh_load(geofile, *this);
        update();
        return result;
    }
    
    bool MeshGrob::serialize_write(OutputGraphiteFile& geofile) {
        return mesh_save(*this, geofile); 
    }
    
}

