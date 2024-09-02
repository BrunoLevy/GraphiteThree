/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
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
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 */


#ifndef H_OGF_SCENE_GRAPH_TYPES_GROB_H
#define H_OGF_SCENE_GRAPH_TYPES_GROB_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/types/properties.h>
#include <OGF/gom/types/node.h>
#include <OGF/basic/math/geometry.h>

#include <map>

/**
 * \file OGF/scene_graph/grob/grob.h
 * \brief The base class for all 3D objects in the system.
 */

namespace OGF {

//_________________________________________________________

    class CompositeGrob;
    class SceneGraph;
    class Commands;
    class InputGraphiteFile;
    class OutputGraphiteFile;
    class Interpreter;
    // class ShaderManager;
    // class Shader;

    /**
     * \brief Base class for all 3D Graphite objects.
     */
    gom_attribute(abstract, "true")
    gom_class SCENE_GRAPH_API Grob : public Node {
    public:

        /**
         * \brief Grob constructor.
         * \param[in] parent a pointer to the CompositeGrob this
         *  Grob belongs to (i.e., the SceneGraph)
         */
        Grob(CompositeGrob* parent);

        /**
         * \brief Grob destructor.
         */
         ~Grob() override;


        /**
         * \brief Gets the name of this Grob
         * \details Each Grob has a unique name that identifies
         *  it in the SceneGraph. This corresponds to the names
         *  that appear in the GUI, in the object list.
         * \return a const reference to the name of this object
         */
        const std::string& name() const {
            return name_;
        }

        /**
         * \brief Gets the bounding box.
         * \return the bounding box
         */
        virtual Box3d bbox() const = 0;

        /**
         * \brief Gets the bounding box in world coordinates.
         * \return the bounding box in world coordinates
         * \TODO some explanations are missing there
         */
        virtual Box3d world_bbox() const;

        /**
         * \brief Tests whether this Grob can be serialized in
         *  GeoFile.
         * \retval true if this Grob is serializable
         * \retval false otherwise
         * \see serialize_read(), serialize_write()
         */
        virtual bool is_serializable() const;

        /**
         * \brief Reads this Grob from a GeoFile.
         * \param[in,out] geofile the input GeoFile
         * \retval true if the Grob could be successfully read
         * \retval false otherwise
         * \see is_serializable()
         */
        virtual bool serialize_read(InputGraphiteFile& geofile);

        /**
         * \brief Writes this Grob into a stream.
         * \param[in,out] geofile the output GeoFile
         *  The file extension determines the file format used to
         *  write the object.
         * \retval true if the Grob could be successfully written
         * \retval false otherwise
         * \see is_serializable()
         */
        virtual bool serialize_write(OutputGraphiteFile& geofile);

        /**
         * \brief Changes the current shader of this Grob
         * \param[in] value the shader user
         *  class name to be used, as a string (without the "OGF::" prefix).
         * \note This Grob needs to be the current object in
         *  its SceneGraph.
         */
        void set_shader(const std::string& value);

        /**
         * \brief Gets the attributes associated with this Grob.
         * \return a modifiable reference to the attributes
         * \details Each Grob has a set of name-value pairs that can
         *  be used to store additional information (to extend the
         *  current state of the object, if needed by some algorithms).
         */
        ArgList& attributes() {
            return grob_attributes_;
        }

        /**
         * \brief Gets the attributes associated with this Grob.
         * \return a const reference to the attributes
         * \details Each Grob has a set of name-value pairs that can
         *  be used to store additional information (to extend the
         *  current state of the object, if needed by some algorithms).
         */
        const ArgList& attributes() const {
            return grob_attributes_;
        }

        /**
         * \brief Gets the dirty flag.
         * \details The dirty flag indicates whether cached information,
         *  such as Vertex Buffer Objects, need to be reconstructed.
         * \retval true if this VoxelGrob is dirty
         * \retval false otherwise
         */
        bool dirty() const {
            return dirty_;
        }

        /**
         * \brief Tests whether this object is up to date.
         * \details An object is up to date if it is not dirty.
         * \retval true if this object is up to date
         * \retval false otherwise
         */
        void up_to_date() {
            dirty_ = false;
        }

        /**
         * \brief Tests whether this VoxelGrob is locked
         *  for graphics display.
         * \details When graphics updates are locked,
         *  redraw requests are ignored for this VoxelGrob.
         * \retval true if this VoxelGrob is locked
         * \retval false otherwise
         */
        bool graphics_are_locked() const {
            return (nb_graphics_locks_ != 0);
        }

        /**
         * \brief Locks graphics for this VoxelGrob.
         * \details When graphics updates are locked,
         *  redraw requests are ignored for this VoxelGrob.
         *  Graphics needs to be locked whenever there is a
         *  possibility that a graphics object will be requested
         *  on an object that is in a transient state. For instance,
         *  this function can be used to avoid OpenGL warnings that
         *  the buffer object has not the expected size.
         *  Multiple lock/unlock requests can be nested, the
         *  VoxelGrob keeps track of the number of active locks.
         *  The object is considered unlocked when the number
         *  of graphics locks is zero.
         */
        void lock_graphics() {
            ++nb_graphics_locks_;
            dirty_ = true;
        }

        /**
         * \brief Unlocks graphics for this VoxelGrob.
         * \details When graphics updates are locked,
         *  redraw requests are ignored for this VoxelGrob.
         *  Graphics needs to be locked whenever there is a
         *  possibility that a graphics object will be requested
         *  on an object that is in a transient state. For instance,
         *  this function can be used to avoid OpenGL warnings that
         *  the buffer object has not the expected size.
         *  Multiple lock/unlock requests can be nested, the
         *  VoxelGrob keeps track of the number of active locks.
         *  The object is considered unlocked when the number
         *  of graphics locks is zero.
         */
        void unlock_graphics() {
            --nb_graphics_locks_;
        }

        /**
         * \brief Finds a Grob by name
         * \param[in] sg a pointer to the SceneGraph
         * \param[in] name the name
         * \return a pointer to the Grob named as \p name in the
         *  SceneGraph \p sg if it exists, or nil otherwise.
         */
        static Grob* find(SceneGraph* sg, const std::string& name);


	/**
	 * \brief Gets a pointer to the main Interpreter.
	 * \return a pointer to the main Interpreter.
	 */
	virtual Interpreter* interpreter();

    gom_slots:
        /**
         * \brief Triggers update events.
         * \details Should be called whenever the object
         *  is modified, typically at the end of a Commands
         *  slot.
         */
        virtual void update();

	/**
	 * \brief Triggers update events and redraws the
	 *  scene.
	 * \details Used by commands to display the intermediary
	 *  state of the object during computations.
	 */
        virtual void redraw();

        /**
         * \brief Gets the SceneGraph.
         * \return a pointer to the SceneGraph
         */
        SceneGraph* scene_graph() const {
            return scene_graph_;
        }

        /**
         * \brief Replaces this Grob with the contents of a file.
         * \param[in] value the name of the file
	 * \retval true on success
	 * \retval false otherwise
         */
        virtual bool load(const FileName& value);

        /**
         * \brief Saves this Grob to a file.
         * \param[in] value the name of the file
	 * \retval true on success
	 * \retval false otherwise
         */
        virtual bool save(const NewFileName& value);

	/**
	 * \brief Appends the content of the specified file to
	 *  this Grob.
         * \param[in] value the name of the file
	 * \retval true on success
	 * \retval false otherwise
	 */
	virtual bool append(const FileName& value);

        /**
         * \brief Clears this Grob
         */
        virtual void clear();

        /**
         * \brief Renames this Grob
         * \param[in] value the new name. If another Grob exists with the
         *  specified name, then the name is changed (by appending a
         *  number to it).
         */
        virtual void rename(const std::string& value);

        /**
         * \brief Duplicates this Grob
         * \details The newly created Grob has the same name as the
         *  current one, with "_copy" appended.
         */
        virtual Grob* duplicate(SceneGraph* sg);

        /**
         * \brief Creates an Interface object connected to this Grob
         * \details This is used in GEL scripts, to easily invoke
         *  commands on objects by using for instance:
         *  \code
         *     mesh.query_interface(
	 *      "OGF::MeshGrobSurfaceCommands"
	 *     ).remesh_smooth(nb_pts=10000)
         *  \endcode
         * \param[in] name the class name of the Interface or Commands object as
         *  a string, with the "OGF::" prefix
         * \return a pointer to the created Interface object
         */
        virtual Object* query_interface(const std::string& name);


        /**
         * \brief gets the number of grob attributes
         * \return the number of grob attributes
         */
        index_t nb_grob_attributes() const {
            return attributes().nb_args();
        }

        /**
         * \brief gets the name of a grob attribute
         * \param[in] i the index of the grob attribute,
         *   in 0 .. nb_grob_attributes()-1
         * \return the name of the attribute
         */
        const std::string& ith_grob_attribute_name(index_t i) const {
            return attributes().ith_arg_name(i);
        }

        /**
         * \brief gets the value of a grob attribute
         * \param[in] i the index of the grob attribute,
         *   in 0 .. nb_grob_attributes()-1
         * \return the value of the attribute, as a string
         */
        std::string ith_grob_attribute_value(index_t i) const {
            return attributes().ith_arg_value(i).as_string();
        }

        /**
         * \brief sets the value of a grob attribute
         * \details if the attribute already exists it is overwritten else
         *  it is created
         * \param[in] name the name of the attribute
         * \param[in] value the new value of the attrinute as a string
         */
        void set_grob_attribute(
            const std::string& name, const std::string& value
        ) {
            attributes().set_arg(name, value);
        }

    gom_signals:

        /**
         * \brief A signal that is triggered each time the object
         *  changed.
         * \param[in] value a pointer to this Grob
         */
        virtual void value_changed(Grob* value);

    gom_properties:

        /**
         * \brief Gets the name of this Grob.
         * \return a const reference to the name
         */
        const std::string& get_name() const {
            return name_;
        }

        /**
         * \brief Sets the name of this Grob.
         * \details The system stores for each grob the
         *  filename it was read from.
         * \param[in] value a const reference to the name
         */
        void set_filename(const std::string& value) {
            filename_ = value;
        }

        /**
         * \brief Gets the filename of this Grob.
         * \details The system stores for each grob the
         *  filename it was read from.
         * \return a const reference to the filename
         */
        const std::string& get_filename() const {
            return filename_;
        }

        /**
         * \brief Tests whether this Grob is visible
         * \details In the GUI, this corresponds to the checkboxes
         *  in the object list.
         * \retval true if this Grob is visible
         * \retval false otherwise
         */
        bool get_visible() const;

        /**
         * \brief Sets the visibility flag of this Grob
         * \details In the GUI, this corresponds to the checkboxes
         *  in the object list.
         * \param[in] value the new value of the visibility flag
         */
        void set_visible(bool value);

        /**
         * \brief Gets the shader associated with this Grob.
         * \return a pointer to the Shader
         */
        Object* get_shader() const;

	/**
	 * \brief Gets the ShaderManager associated with this Grob.
	 * \return a pointer to the ShaderManager.
	 */
	Object* get_shader_manager() const {
	    // Stored as Object_var to avoid dependency to Shader
	    // return (ShaderManager*)(shader_manager_.get());
            return shader_manager_;
	}

        /**
         * \brief Sets the object to world transform.
         * \param[in] value the object to world transform, as
         *  a 4x4 homogeneous matrix
         * \note In most cases, this is the identity matrix.
         * \TODO more explanations needed here
         */
        void set_obj_to_world_transform(const mat4& value) {
            obj_to_world_ = value;
        }

        /**
         * \brief Gets the object to world transform.
         * \return a const reference to the object to
         *  world transform, as a 4x4 homogeneous matrix
         * \note In most cases, this is the identity matrix.
         * \TODO more explanations needed here
         */
        const mat4& get_obj_to_world_transform() const {
            return obj_to_world_;
        }

    protected:
        /**
         * \brief Initializes the name of this Grob.
         * \param[in] name the new name for this Grob
         * \details Tests if there is already a Grob with
         *  the same name in the SceneGraph, and generates
         *  a unique name if this was the case.
         */
        void initialize_name(const std::string& name);

        /**
         * \brief Sets the ShaderManager associated with this
         *  Grob.
         * \param[in] s a pointer to the ShaderManager
         */
        void set_shader_manager(Object* s/* ShaderManager* s */) {
            // shader_manager_ = (Object *)s;
            shader_manager_ = s;
        }

    protected:
        std::string name_;
        std::string filename_;
        bool visible_;
        SceneGraph* scene_graph_;
        mat4 obj_to_world_;
        Object_var shader_manager_;
        ArgList grob_attributes_;
        bool dirty_;
        index_t nb_graphics_locks_;

        friend class SceneGraph;
        friend class SceneGraphShaderManager;
        friend class ShaderManager;
    };

    /**
     * \brief An automatic reference-counted pointer to a Grob.
     */
    typedef SmartPointer<Grob> Grob_var;

//_________________________________________________________

}
#endif
