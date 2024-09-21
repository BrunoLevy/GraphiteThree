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
 * As an exception to the GPL, Graphite can be linked with the
 *  following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */


#include <OGF/voxel/grob/voxel_grob.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph/types/geofile.h>
#include <OGF/gom/reflection/meta_class.h>


namespace OGF {

    VoxelGrob::VoxelGrob(CompositeGrob* parent) : Grob(parent) {
        initialize_name("voxel");
        nu_ = 0;
        nv_ = 0;
        nw_ = 0;
        origin_ = vec3(0.0, 0.0, 0.0);
        U_ = vec3(1.0, 0.0, 0.0);
        V_ = vec3(0.0, 1.0, 0.0);
        W_ = vec3(0.0, 0.0, 1.0);
    }

    VoxelGrob::VoxelGrob() : Grob() {
        initialize_name("voxel");
        nu_ = 0;
        nv_ = 0;
        nw_ = 0;
        origin_ = vec3(0.0, 0.0, 0.0);
        U_ = vec3(1.0, 0.0, 0.0);
        V_ = vec3(0.0, 1.0, 0.0);
        W_ = vec3(0.0, 0.0, 1.0);
    }

    void VoxelGrob::update() {
        Grob::update();
    }

    void VoxelGrob::clear() {
        nu_ = 0;
        nv_ = 0;
        nw_ = 0;
        origin_ = vec3(0.0, 0.0, 0.0);
        U_ = vec3(1.0, 0.0, 0.0);
        V_ = vec3(0.0, 1.0, 0.0);
        W_ = vec3(0.0, 0.0, 1.0);
        attributes_.clear(false, false);
        update();
    }



    Grob* VoxelGrob::duplicate(SceneGraph* sg) {
        VoxelGrob* result = dynamic_cast<VoxelGrob*>(Grob::duplicate(sg));
        ogf_assert(result != nullptr);

        result->nu_ = nu_;
        result->nv_ = nv_;
        result->nw_ = nw_;
        result->origin_ = origin_;
        result->U_ = U_;
        result->V_ = V_;
        result->W_ = W_;
        result->attributes_.copy(attributes_);
        result->update();
        return result;
    }

    Box3d VoxelGrob::bbox() const {
        Box3d result;
        result.add_point(origin_);
        result.add_point(origin_ + U_);
        result.add_point(origin_ + V_);
        result.add_point(origin_ + W_);
        result.add_point(origin_ + U_ + V_);
        result.add_point(origin_ + V_ + W_);
        result.add_point(origin_ + W_ + U_);
        result.add_point(origin_ + U_ + V_ + W_);
        return result;
    }

    void VoxelGrob::resize(index_t nu, index_t nv, index_t nw) {
        attributes_.clear(false, false);
        nu_ = nu;
        nv_ = nv;
        nw_ = nw;
        attributes_.resize(nu_*nv_*nw_);
    }

    void VoxelGrob::set_box(
        const vec3& origin, const vec3& U, const vec3& V, const vec3& W
    ) {
        origin_ = origin;
        U_ = U;
        V_ = V;
        W_ = W;
    }

    VoxelGrob* VoxelGrob::find_or_create(
        SceneGraph* sg, const std::string& name
    ) {
        VoxelGrob* result = find(sg, name);
        if(result == nullptr) {
            std::string cur_grob_bkp = sg->get_current_object();
            result = dynamic_cast<VoxelGrob*>(
                sg->create_object("OGF::VoxelGrob")
            );
            ogf_assert(result != nullptr);
            result->rename(name);
            sg->set_current_object(result->name());
            sg->set_current_object(cur_grob_bkp);
        }
        return result;
    }

    VoxelGrob* VoxelGrob::find(SceneGraph* sg, const std::string& name) {
        VoxelGrob* result = nullptr;
        if(sg->is_bound(name)) {
            result = dynamic_cast<VoxelGrob*>(sg->resolve(name));
        }
        return result;
    }

    std::string VoxelGrob::get_displayable_attributes() const {
        std::string result;
        vector<std::string> attribute_names;
        attributes_.list_attribute_names(attribute_names);
        for(index_t i=0; i<attribute_names.size(); ++i) {
            const AttributeStore* store = attributes_.
                find_attribute_store(attribute_names[i]);
            if(ReadOnlyScalarAttributeAdapter::can_be_bound_to(store)) {
                if(
                    store->dimension() == 1 ||
                    store->dimension() == 3 ||
                    store->dimension() == 4
                ) {
                    if(result != "") {
                        result += ";";
                    }
                    result += attribute_names[i];
                }
                if(store->dimension() > 1) {
                    for(index_t j=0; j<store->dimension(); ++j) {
                        if(result != "") {
                            result += ";";
                        }
                        result +=
                            attribute_names[i] +
                            "[" + String::to_string(j) + "]";
                    }
                }
            }
        }
        return result;
    }

    bool VoxelGrob::is_serializable() const {
        return true;
    }

    bool VoxelGrob::serialize_read(InputGraphiteFile& in) {
        for(
            std::string chunk_class = in.next_chunk();
            chunk_class != "EOFL" && chunk_class != "SPTR";
            chunk_class = in.next_chunk()
        ) {
            if(chunk_class == "VOXH") {
                ArgList args;
                in.read_arg_list(args);
                nu_ = args.get_arg<index_t>("nu");
                nv_ = args.get_arg<index_t>("nv");
                nw_ = args.get_arg<index_t>("nw");
                origin_ = args.get_arg<vec3>("origin");
                U_ = args.get_arg<vec3>("U");
                V_ = args.get_arg<vec3>("V");
                W_ = args.get_arg<vec3>("W");
            } else if(chunk_class == "ATTS") {
                const std::string& name =
                    in.current_attribute_set().name;
                index_t nb_items = in.current_attribute_set().nb_items;
                if(name != "GEO::VoxelGrob::cells") {
                    Logger::err("VoxelGrob")
                        << "Invalid attribute set"
                        << name
                        << std::endl;
                }
                if(nb_items != nu_*nv_*nw_) {
                    Logger::err("VoxelGrob")
                        << "Invalid number of cells in attribute: "
                        << nb_items
                        << " expected "
                        << nu_*nv_*nw_
                        << std::endl;
                }
                attributes_.resize(nu_*nv_*nw_);
            } else if(chunk_class == "ATTR") {
                if(
                    !AttributeStore::element_type_name_is_known(
                        in.current_attribute().element_type
                    )
                ) {
                    Logger::warn("I/O") << "Skipping attribute "
                                        << in.current_attribute().name
                                        << ":"
                                        << in.current_attribute().element_type
                                        << " (unknown type)"
                                        << std::endl;
                } else {
                    AttributeStore* store =
                    AttributeStore::create_attribute_store_by_element_type_name(
                        in.current_attribute().element_type,
                        in.current_attribute().dimension
                    );
                    attributes_.bind_attribute_store(
                        in.current_attribute().name,store
                    );
                    in.read_attribute(store->data());
                }
            }
        }
        update();
        return true;
    }

    bool VoxelGrob::serialize_write(OutputGraphiteFile& out) {
        ArgList args;

        args.create_arg("nu", nu_);
        args.create_arg("nv", nv_);
        args.create_arg("nw", nw_);
        args.create_arg("origin", origin_);
        args.create_arg("U", U_);
        args.create_arg("V", V_);
        args.create_arg("W", W_);

        out.write_chunk_header("VOXH", out.arg_list_size(args));
        out.write_arg_list(args);
        out.check_chunk_size();


        std::string attribute_set_name = "GEO::VoxelGrob::cells";
        out.write_attribute_set(attribute_set_name, nu_*nv_*nw_);

        vector<std::string> attribute_names;
        attributes_.list_attribute_names(attribute_names);
        for(index_t i=0; i<attribute_names.size(); ++i) {
            AttributeStore* store = attributes_.find_attribute_store(
                attribute_names[i]
            );
            if(
                AttributeStore::element_typeid_name_is_known(
                    store->element_typeid_name()
                )
            ) {
                std::string element_type =
                    AttributeStore::element_type_name_by_element_typeid_name(
                        store->element_typeid_name()
                    );

                out.write_attribute(
                    attribute_set_name,
                    attribute_names[i],
                    element_type,
                    store->element_size(),
                    store->dimension(),
                    store->data()
                );
            } else {
                Logger::warn("I/O")
                    << "Skipping attribute: "
                    << attribute_names[i]
                    << " on "
                    << attribute_set_name
                    << std::endl;
                Logger::warn("I/O")
                    << "Typeid "
                    << store->element_typeid_name()
                    << " unknown"
                    << std::endl;
            }
        }
        return true;
    }

}
