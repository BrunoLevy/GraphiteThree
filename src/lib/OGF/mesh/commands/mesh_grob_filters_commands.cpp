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
 * As an exception to the GPL, Graphite can be linked
 *  with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */


#include <OGF/mesh/commands/mesh_grob_filters_commands.h>
#include <OGF/mesh/commands/filter.h>

namespace OGF {

    /*********************************************************/


    MeshGrobFiltersCommands::MeshGrobFiltersCommands() {
    }

    MeshGrobFiltersCommands::~MeshGrobFiltersCommands() {
    }


    void MeshGrobFiltersCommands::set_filter(
        const std::string& where, const std::string& filter_string,
        bool propagate
    ) {
        apply_filter_op(FILTER_SET, where, filter_string, propagate);
    }


    void MeshGrobFiltersCommands::add_to_filter(
        const std::string& where, const std::string& filter_string,
        bool propagate
    ) {
        apply_filter_op(FILTER_ADD, where, filter_string, propagate);
    }

    void MeshGrobFiltersCommands::remove_from_filter(
        const std::string& where, const std::string& filter_string,
        bool propagate
    ) {
        apply_filter_op(FILTER_REMOVE, where, filter_string, propagate);
    }

    void MeshGrobFiltersCommands::set_filter_from_attribute(
        const std::string& full_attribute_name,
        const std::string& filter_string, bool propagate
    ) {
        apply_filter_op_attribute(
            FILTER_SET, full_attribute_name, filter_string, propagate
        );
    }

    void MeshGrobFiltersCommands::add_to_filter_attribute(
        const std::string& full_attribute_name,
        const std::string& filter_string, bool propagate
    ) {
        apply_filter_op_attribute(
            FILTER_ADD, full_attribute_name, filter_string, propagate
        );
    }

    void MeshGrobFiltersCommands::remove_from_filter_attribute(
        const std::string& full_attribute_name,
        const std::string& filter_string, bool propagate
    ) {
        apply_filter_op_attribute(
            FILTER_REMOVE, full_attribute_name, filter_string, propagate
        );
    }


    void MeshGrobFiltersCommands::propagate_filter(
        const std::string& where
    ) {
        MeshElementsFlags where_id = Mesh::name_to_subelements_type(where);
        if(where_id == MESH_NONE) {
            Logger::err("Attributes")
                << where << ": invalid attribute localization"
                << std::endl;
            return;
        }

        propagate_filter(mesh_grob(), where_id);
        mesh_grob()->update();
    }

    void MeshGrobFiltersCommands::delete_filters(const std::string& where) {
        if(where == "all") {
            delete_filters("vertices");
            delete_filters("facets");
            delete_filters("cells");
            return;
        }
        MeshElementsFlags where_id = Mesh::name_to_subelements_type(where);
        if(where_id == MESH_NONE) {
            Logger::err("Attributes")
                << where << ": invalid attribute localization"
                << std::endl;
            return;
        }
        AttributesManager& attributes =
            mesh_grob()->get_subelements_by_type(where_id).attributes();
        if(attributes.is_defined("filter")) {
            attributes.delete_attribute_store("filter");
        }

        Object* shd = mesh_grob()->get_shader();
        if(shd != nullptr) {
            if(shd->has_property(where + "_filter")) {
                shd->set_property(where + "_filter", "false");
            }
        }

        mesh_grob()->update();
    }

    void MeshGrobFiltersCommands::copy_filter_to_selection(
        const std::string& filter
    ) {
        MeshElementsFlags where;
        std::string filter_name;
        index_t component;

        if(!Mesh::parse_attribute_name(
               filter, where, filter_name, component)
        ) {
            Logger::err("Attributes") << "Error in filter name: "
                                      << filter
                                      << std::endl;
            return;
        }

        Attribute<Numeric::uint8> filter_attr;
        filter_attr.bind_if_is_defined(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            filter_name
        );

        if(!filter_attr.is_bound()) {
            Logger::err("Attributes") << filter
                                      << ": no such attribute"
                                      << std::endl;
            return;
        }

        Attribute<Numeric::uint8> selection_attr(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );

        for(index_t i=0; i<filter_attr.size(); ++i) {
            selection_attr[i] = filter_attr[i];
        }

        mesh_grob()->update();
    }

    void MeshGrobFiltersCommands::copy_selection_to_filter(
        const std::string& selection, bool propagate
    ) {
        MeshElementsFlags where;
        std::string selection_name;
        index_t component;

        if(!Mesh::parse_attribute_name(
               selection, where, selection_name, component)
        ) {
            Logger::err("Attributes") << "Error in selection name: "
                                      << selection
                                      << std::endl;
            return;
        }

        Attribute<Numeric::uint8> selection_attr;
        selection_attr.bind_if_is_defined(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            selection_name
        );

        if(!selection_attr.is_bound()) {
            Logger::err("Attributes") << selection
                                      << ": no such attribute"
                                      << std::endl;
            return;
        }

        Attribute<Numeric::uint8> filter_attr(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "filter"
        );

        for(index_t i=0; i<selection_attr.size(); ++i) {
            filter_attr[i] = selection_attr[i];
        }

        Object* shd = mesh_grob()->get_shader();
        if(shd != nullptr) {
            std::string prop_name =
                Mesh::subelements_type_to_name(where) + "_filter";
            if(shd->has_property(prop_name)) {
                shd->set_property(prop_name, "true");
            }
        }

        if(propagate) {
            propagate_filter(mesh_grob(), where);
        }

        mesh_grob()->update();
    }

    /*********************************************************/


    void MeshGrobFiltersCommands::apply_filter_op(
        FilterOp op,
        const std::string& where, const std::string& filter_string,
        bool propagate
    ) {
        MeshElementsFlags where_id = Mesh::name_to_subelements_type(where);
        if(where_id == MESH_NONE) {
            Logger::err("Attributes")
                << where << ": invalid attribute localization"
                << std::endl;
            return;
        }

        Attribute<Numeric::uint8> attribute(
            mesh_grob()->get_subelements_by_type(where_id).attributes(),
            "filter"
        );

        try {
            Filter filter(attribute.size(), filter_string);
            for(index_t i=0; i<attribute.size(); ++i) {
                switch(op) {
                case FILTER_SET:
                    attribute[i] = Numeric::uint8(
                        filter.test(i)
                    );
                    break;
                case FILTER_ADD: {
                    attribute[i] = Numeric::uint8(
                        attribute[i] || filter.test(i)
                    );
                } break;
                case FILTER_REMOVE:
                    attribute[i] = Numeric::uint8(
                        attribute[i] && !filter.test(i)
                    );
                    break;
                }
            }
        } catch(...) {
            Logger::err("Attributes") << "Invalid filter specification"
                                      << std::endl;
        }

        Object* shd = mesh_grob()->get_shader();
        if(shd != nullptr) {
            if(shd->has_property(where + "_filter")) {
                shd->set_property(where + "_filter", "true");
            }
        }

        if(propagate) {
            propagate_filter(mesh_grob(), where_id);
        }

        mesh_grob()->update();
    }

    void MeshGrobFiltersCommands::apply_filter_op_attribute(
        FilterOp op,
        const std::string& full_attribute_name,
        const std::string& filter_string,
        bool propagate
    ) {
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;

        if(!Mesh::parse_attribute_name(
               full_attribute_name, where, attribute_name, component)
          ) {
            Logger::err("Attributes") << "Error in attribute name: "
                                      << full_attribute_name
                                      << std::endl;
            return;
        }

        ReadOnlyScalarAttributeAdapter attribute(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            attribute_name + "[" + String::to_string(component) + "]"
        );

        if(!attribute.is_bound()) {
            Logger::err("Attributes") << full_attribute_name
                                      << " : no such attribute"
                                      << std::endl;
            return;
        }

        Attribute<Numeric::uint8> filter_attribute(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "filter"
        );

        try {
            Filter filter(attribute.size(), filter_string, true);
            for(index_t i=0; i<attribute.size(); ++i) {
                switch(op) {
                case FILTER_SET: {
                    filter_attribute[i] =
                        Numeric::uint8(filter.test(attribute[i]));
                } break;
                case FILTER_ADD: {
                    filter_attribute[i] =
                        Numeric::uint8(
                            filter_attribute[i] || filter.test(attribute[i])
                        );
                } break;
                case FILTER_REMOVE: {
                    filter_attribute[i] =
                        Numeric::uint8(
                            filter_attribute[i] && !filter.test(attribute[i])
                        );
                } break;
                }
            }
        } catch(...) {
            Logger::err("Attributes") << "Invalid filter specification"
                                      << std::endl;
        }

        if(propagate) {
            propagate_filter(mesh_grob(), where);
        }

        mesh_grob()->update();
    }

    void MeshGrobFiltersCommands::propagate_filter(
        MeshGrob* mesh, MeshElementsFlags from
    ) {
        Attribute<Numeric::uint8> vertices_filter(
            mesh->vertices.attributes(), "filter"
        );
        Attribute<Numeric::uint8> facets_filter(
            mesh->facets.attributes(), "filter"
        );
        Attribute<Numeric::uint8> cells_filter(
            mesh->cells.attributes(), "filter"
        );
        switch(from) {
        case MESH_VERTICES: {
            for(index_t f: mesh->facets) {
                facets_filter[f] = 1;
                for(
                    index_t lv=0;
                    lv<mesh->facets.nb_vertices(f); ++lv
                ) {
                    index_t v = mesh->facets.vertex(f,lv);
                    if(vertices_filter[v] == 0) {
                        facets_filter[f] = 0;
                        break;
                    }
                }
            }
            for(index_t c: mesh->cells) {
                cells_filter[c] = 1;
                for(
                    index_t lv=0;
                    lv<mesh->cells.nb_vertices(c); ++lv
                ) {
                    index_t v = mesh->cells.vertex(c,lv);
                    if(vertices_filter[v] == 0) {
                        cells_filter[c] = 0;
                        break;
                    }
                }
            }
        } break;
        case MESH_FACETS: {
            for(index_t v: mesh->vertices) {
                vertices_filter[v] = 0;
            }
            for(index_t f: mesh->facets) {
                if(facets_filter[f] != 0) {
                    for(
                        index_t lv=0;
                        lv<mesh->facets.nb_vertices(f); ++lv
                    ) {
                        index_t v = mesh->facets.vertex(f,lv);
                        vertices_filter[v] = 1;
                    }
                }
            }
            for(index_t c: mesh->cells) {
                cells_filter[c] = 1;
                for(
                    index_t lv=0;
                    lv<mesh->cells.nb_vertices(c); ++lv
                ) {
                    index_t v = mesh->cells.vertex(c,lv);
                    if(vertices_filter[v] == 0) {
                        cells_filter[c] = 0;
                        break;
                    }
                }
            }
        } break;
        case MESH_CELLS: {
            for(index_t v: mesh->vertices) {
                vertices_filter[v] = 0;
            }
            for(index_t c: mesh->cells) {
                if(cells_filter[c] != 0) {
                    for(
                        index_t lv=0;
                        lv<mesh->cells.nb_vertices(c); ++lv
                    ) {
                        index_t v = mesh->cells.vertex(c,lv);
                        vertices_filter[v] = 1;
                    }
                }
            }
            for(index_t f: mesh->facets) {
                facets_filter[f] = 1;
                for(
                    index_t lv=0;
                    lv<mesh->facets.nb_vertices(f); ++lv
                ) {
                    index_t v = mesh->facets.vertex(f,lv);
                    if(vertices_filter[v] == 0) {
                        facets_filter[f] = 0;
                        break;
                    }
                }
            }
        } break;
	case MESH_NONE:
	case MESH_EDGES:
	case MESH_ALL_ELEMENTS:
	case MESH_FACET_CORNERS:
	case MESH_CELL_CORNERS:
	case MESH_CELL_FACETS:
	case MESH_ALL_SUBELEMENTS: {
	    Logger::err("Filter") << "Invalid localisation"
				     << std::endl;
	} break;
        }
        Object* shd = mesh->get_shader();
        if(shd != nullptr) {
            std::string oldval;
            if(shd->has_property("vertices_filter")) {
                shd->get_property("vertices_filter", oldval);
                if(oldval != "true") {
                    shd->set_property("vertices_filter", "true");
                }
            }
            if(shd->has_property("facets_filter")) {
                shd->get_property("facets_filter", oldval);
                if(oldval != "true") {
                    shd->set_property("facets_filter", "true");
                }
            }
            if(shd->has_property("cells_filter")) {
                shd->get_property("cells_filter", oldval);
                if(oldval != "true") {
                    shd->set_property("cells_filter", "true");
                }
            }
        }
    }


}
