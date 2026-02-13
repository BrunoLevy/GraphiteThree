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


#ifndef H_OGF_SCENE_GRAPH_TYPES_SCENE_GRAPH_H
#define H_OGF_SCENE_GRAPH_TYPES_SCENE_GRAPH_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/grob/composite_grob.h>
#include <OGF/gom/types/node.h>

/**
 * \file OGF/scene_graph/types/scene_graph.h
 * \brief the class that represents the scene graph.
 */
namespace OGF {

    class Grob;
    class RenderingContext;

/*************************************************************************/

    /**
     * \brief Represents the list of objects loaded in Graphite.
     * \details A SceneGraph stores a list of Grob and
     *  manages the associated events. It has a current object,
     *  that is the target of commands invocation.
     */
    gom_class SCENE_GRAPH_API SceneGraph : public CompositeGrob {
    public:

        /**
         * \brief SceneGraph constructor.
	 * \param[in] interpreter a pointer to the main interpreter, or
         *   nullptr. If interpreter is set to nullptr, then the default
         *   interpreter is used.
         */
        SceneGraph(
	    Interpreter* interpreter = nullptr,
	    bool transfer_ownership_to_scenegraph_library=false
	);

        /**
         * \brief SceneGraph destructor.
         */
         ~SceneGraph() override;


    gom_slots:

        /**
         * \brief Gets the current object.
         * \return a pointer to the current object.
         */
        Grob* current();

        /**
         * \brief Sets the current object.
         * \param grob the object to be set as current or its name.
         */
	void set_current(const GrobName& grob);

	/**
	 * \brief Removes an object from this SceneGraph and deletes it
	 * \param[in] grob the object to be deleted or its name
	 */
	void delete_object(const GrobName& grob);

	/**
	 * \brief Removes the current object from this SceneGraph and deletes it
	 */
	void delete_current();

	/**
	 * \brief Removes and deletes all the selected objects in this SceneGraph
	 */
	void delete_selected();

	/**
	 * \brief Deletes and removes all the objects in this SceneGraph
	 */
	void delete_all();

	/**
	 * \brief Makes an object visible.
	 * \details Ignored in terminal applications.
	 * \param[in] grob the object to be shown or its name
	 */
	void show_object(const GrobName& grob);

	/**
	 * \brief Makes all selected objects visible.
	 * \details Ignored in terminal applications.
	 */
	void show_selected();

	/**
	 * \brief Makes all objects visible.
	 * \details Ignored in terminal applications.
	 */
	void show_all();

	/**
	 * \brief Makes an object invisible.
	 * \details Ignored in terminal applications.
	 * \param[in] grob the object to be hidden or its name
	 */
	void hide_object(const GrobName& grob);

	/**
	 * \brief Makes all selected objects invisible.
	 * \details Ignored in terminal applications.
	 */
	void hide_selected();

	/**
	 * \brief Makes all objects invisible.
	 * \details Ignored in terminal applications.
	 */
	void hide_all();

	/**
	 * \brief Moves an object up in the SceneGraph
	 * \param[in] grob the object to be moved up or its name
	 */
	void move_object_up(const GrobName& grob);

	/**
	 * \brief Moves an object down in the SceneGraph
	 * \param[in] grob the object to be moved down or its name
	 */
	void move_object_down(const GrobName& grob);

	/**
	 * \brief Renames an object
	 * \param[in] grob the object to be renamed or its name
	 * \param[in] new_name the desired new name for the object
	 */
	void rename_object(
	    const GrobName& grob, const std::string& new_name
	);

        /**
         * \brief Duplicates an object object.
         * \details The name of the created object will be the same as
         *  current object name with "_copy" appended.
	 * \param[in] grob the grob to be copied or its name
	 * \return the created object or nullptr if there was no current
	 *  object.
         */
	Grob* duplicate_object(const GrobName& grob);

	/**
	 * \brief Copies graphic properties of an object to all objects
	 * \details Ignored in terminal applications
	 * \param[in] grob the object or its name
	 */
	void copy_object_properties_to_all(const GrobName& grob);

	/**
	 * \brief Copies graphic properties of an object to all visible objects
	 * \details Ignored in terminal applications
	 * \param[in] grob the object or its name
	 */
	void copy_object_properties_to_visible(const GrobName& grob);

	/**
	 * \brief Copies graphic properties of an object to all selected objects
	 * \details Ignored in terminal applications
	 * \param[in] grob the object or its name
	 */
	void copy_object_properties_to_selected(const GrobName& grob);


	/**
	 * \brief Adds an object to the selection
	 * \param[in] grob the object or its name
	 */
	void select_object(const GrobName& grob);

	/**
	 * \brief Removes an object from the selection
	 * \param[in] grob the object or its name
	 */
	void unselect_object(const GrobName& grob);

	/**
	 * \brief Toggles the selection flag for an object
	 * \param[in] grob the object or its name
	 */
	void toggle_selection(const GrobName& grob);

	/**
	 * \brief Adds all objects to selection.
	 */
	void select_all();

	/**
	 * \brief Removes all objects from selection.
	 */
	void clear_selection();

	/*************************************/

        /**
         * \brief Clears this SceneGraph.
         * \details Deletes all the objects of this SceneGraph.
         */
        void clear() override;

        /**
         * \brief Deletes the current object.
         */
        void delete_current_object();

        /**
         * \brief Duplicates the current object.
         * \details The name of the created object will be the same as
         *  current object name with "_copy" appended.
	 * \return the created object or nullptr if there was no current
	 *  object.
         */
        Grob* duplicate_current();

        /**
         * \brief Swaps the current object with the previous one
         * \details Does nothing if there is no current object or
         *  if current object is the first one
         */
        void move_current_up();

        /**
         * \brief Swaps the current object with the next one
         * \details Does nothing if there is no current object or
         *  if current object is the last one
         */
        void move_current_down();

        /**
         * \brief Loads an object from a file, and stores it in this
         *  SceneGraph.
         * \param[in] value the name of the file
         * \param[in] type the class name that should be used to
         *  create the object, or "default" (then it is deduced from
         *  the file extension)
         * \param[in] change_cwd set if set, change current working directory
	 *  to the directory that contains the file
         * \return a pointer to the loaded object, or nullptr if no object
         *  could be loaded
         */
        Grob* load_object(
            const FileName& value, const std::string& type="default",
            bool change_cwd=false
        );

        /**
         * \brief Loads objects from a list of files, and stores them in this
         *  SceneGraph.
         * \param[in] value the list of file names, separated with ';'
         * \param[in] type the class name that should be used to
         *  create the objects, or "default" (then it is deduced from
         *  the file extensions)
         * \param[in] change_cwd if set, change current working directory to the
	 *  directory that contains the file
         */
        void load_objects(
            const std::string& value, const std::string& type="default",
            bool change_cwd=false
        );

        /**
         * \brief Saves the current object to a file.
         * \param[in] value the name of the file
         */
        bool save_current_object(const NewFileName& value);

	/**
	 * \brief Saves viewer properties to a file.
	 * \details This saves current viewpoint, clipping and background
	 *  color to a .graphite file.
	 * \param[in] value a .graphite file name.
	 * \retval true if viewer properties could be successfully saved to
	 *  the file.
	 * \retval false otherwise.
	 */
        bool save_viewer_properties(const std::string& value);

        /**
         * \brief Creates an object.
         * \param[in] classname the class name of the object to create,
         *  as a string, with the "OGF::" prefix
         * \param[in] name the name of the object in the SceneGraph. If no
	 *  name is specified, then the created object will be
         *  given a default name.
         * \return a pointer to the created object
         */
        Grob* create_object(
	    const GrobClassName& classname="OGF::MeshGrob",
	    const std::string& name = "new_object"
	);

        /**
         * \brief Creates a mesh.
         * \param[in] name the name of the mesh in the SceneGraph. If no
	 *  name is specified, then the created object will be
         *  given a default name.
         * \return a pointer to the created mesh
         */
        Grob* create_mesh(const std::string& name = "") {
            return create_object("OGF::MeshGrob",name);
        }

	/**
	 * \brief Creates a new object or retreives an existing one.
	 * \param[in] classname the class name of the object to create,
         *  as a string, with the "OGF::" prefix.
	 * \param[in] name the name of the object
         * \return a pointer to the newly created or already existing object,
	 *  or nullptr if there was already an object with the same name
	 *  but with a different classname.
	 */
	Grob* find_or_create_object(
	    const GrobClassName& classname, const std::string& name
	);


        /**
         * \brief Associates a Commands class to a Grob class.
         * \details It is used with Commands class created in a script.
         * \param[in] grob_class the MetaClass of the grob
         * \param[in] commands_class the MetaClass of the commands
         */
        void register_grob_commands(
            MetaClass* grob_class, MetaClass* commands_class
        );

    gom_properties:
        /**
         * \brief Sets the current object by name.
         * \param[in] value the name of the object that should be made current
	 * NOTE: deprecated, use set_current(Grob*) instead
         */
        void set_current_object(const GrobName& value);

        /**
         * \brief Gets the name of the current object.
         * \return the name of the current object
	 * NOTE: deprecated, use current()->Grob* instead
         */
        const GrobName& get_current_object() const;

        /**
         * \brief Gets the names of all the objects in this SceneGraph.
         * \return the ';'-separated list of object names
         */
        std::string get_values() const;

	/**
	 * \brief Sets the render area.
	 * \param[in] rdra a pointer to the render area.
	 */
	void set_render_area(Object* rdra) {
	    render_area_ = rdra;
	}

	/**
	 * \brief Gets the render area.
	 * \return a pointer to the render area, or nullptr if
	 *  no render area was specified.
	 */
	Object* get_render_area() const {
	    return render_area_;
	}

	/**
	 * \brief Sets the application.
	 * \param[in] app a pointer to the application.
	 */
	void set_application(Object* app) {
	    application_ = app;
	}

	/**
	 * \brief Gets the application.
	 * \return a pointer to the application, or nullptr if
	 *  no application was specified.
	 */
	Object* get_application() const {
	    return application_;
	}

	/**
	 * \brief sets the scene graph shader manager.
	 * \param[in] sgshdmgr a pointer to the
	 *   scene graph shader manager.
	 */
	void set_scene_graph_shader_manager(Object* sgshdmgr);

	/**
	 * \brief gets the scene graph shader manager.
	 * \return a pointer to the scene graph shader manager
	 *  or nullptr if none was specified.
	 */
	Object* get_scene_graph_shader_manager() const;


    public:
        /**
         * \brief Triggers the value_changed() signal.
         */
        void update() override;

        /**
         * \brief Triggers the values_changed(), visibilities_changed(),
         *  types_changed() and value_changed() signals.
         */
        void update_values();

        /**
         * \copydoc Grob::is_serializable()
         */
         bool is_serializable() const override;

        /**
         * \copydoc Grob::serialize_read()
         */
	 bool serialize_read(InputGraphiteFile& in) override;

        /**
         * \copydoc Grob::serialize_write()
         */
	 bool serialize_write(OutputGraphiteFile& out) override;

	/**
	 * \copydoc Grob::interpreter()
	 */
	Interpreter* interpreter() override;

    gom_signals:
        /**
         * \brief a signal that is triggered whenever the list of objects
         *  changes.
         * \param[in] value the ';'-separated list of object names
         */
        virtual void values_changed(const std::string& value);

        /**
         * \brief a signal that is triggered whenever this SceneGraph
         *  changes.
         * \param[in] value a pointer to this SceneGraph
         */
        void value_changed(Grob* value) override;

        /**
         * \brief a signal that is triggered whenever the current object
         *  changes.
         * \param[in] value the name of the new current object
         */
        virtual void current_object_changed(const std::string& value);

    public:
        /**
         * \copydoc Object::invoke_method
         * \details Overload of the invokation mechanism,
         *  that adds history recording.
         */
        bool invoke_method(
            const std::string& method_name,
            const ArgList& args, Any& ret_val
        ) override;

    protected:
        /**
         * \brief Loads alignment data for pointsets.
         * \details The loaded transform are stored in the
         *  object to world transforms of all objects.
         * \param[in] filename name of the alignment data file
         * \param[in] sg a pointer to the SceneGraph
         * \retval true if alignment data could be read
         * \retval false otherwise
         * \see Grob::set_obj_to_world_transform(),
         *  Grob::get_obj_to_world_transform()
         */
        static bool load_aln(const std::string& filename, SceneGraph* sg);

        /**
         * \brief Writes the preamble of a gsg file to a stream.
         * \param[in,out] out the stream
         * \param[in] all_scene if true, the gsg file is meant to record
         *  the whole scene, else it is for a single object
         */
        void begin_graphite_file(
            OutputGraphiteFile& out, bool all_scene
        );

        /**
         * \brief Writes the trailer of a gsg file.
         * \param[in,out] out the stream
         */
        void end_graphite_file(OutputGraphiteFile& out);

        /**
         * \brief Writes an object to a geogram file.
         * \param[in] grob a pointer to the object
         * \param[in,out] out the stream where the object should be written
         */
        void serialize_grob_write(
            Grob* grob, OutputGraphiteFile& out
        );

        /**
         * \brief Reads an object from a geogram file.
         * \param[in,out] in the stream
         * \return a pointer to the read object
         */
        Grob* serialize_grob_read(
            InputGraphiteFile& in
        );

	void get_grob_shader(
	    Grob* grob, std::string& classname, ArgList& properties
	);

	void set_grob_shader(
	    Grob* grob, const std::string& classname, const ArgList& properties
	);

	/**
	 * \brief Copies an objet property to an arglist
	 * \param[in] obj_prop_name a string in the form "objectid.propname",
	 *   where objectid is the name of a gom object and propname a name
	 *   of a property of the object.
	 * \param[in,out] args the ArgList where the value of the property
	 *   should be copied. Its name will be "objectid.propname".
	 */
	void copy_property_to_arglist(
	    const std::string& obj_prop_name, ArgList& args
	);

	/**
	 * \brief Copies all the object properties previously recorded by
	 *   copy_property_to_arglist() from an ArgList to the gom objects.
	 * \param[in] args an ArgList, with recorded object properties, with
	 *   names "objectid.propname", where objectid is the name of a gom
	 *   object and propname a name of a property of the object. Args
	 *   with names that do not match this pattern are ignored.
	 */
	void copy_arglist_to_properties(const ArgList& args);

    private:
        GrobName current_object_name_;
	Interpreter* interpreter_;
	Object* render_area_;
	Object* application_;
	Object_var scene_graph_shader_manager_;
    };

/*************************************************************************/

}
#endif
