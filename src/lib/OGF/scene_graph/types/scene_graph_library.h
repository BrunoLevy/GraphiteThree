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
 
 
#ifndef H_OGF_SCENE_GRAPH_TYPES_SCENE_GRAPH_LIBRARY_H
#define H_OGF_SCENE_GRAPH_TYPES_SCENE_GRAPH_LIBRARY_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/gom/reflection/meta.h>

#include <geogram/basic/environment.h>

#include <map>

/**
 * \file OGF/scene_graph/types/scene_graph_library.h
 * \brief global management of grob class names, tools and shaders.
 */

namespace OGF {

    class SceneGraph;
    class SceneGraphShaderManager;
    class SceneGraphToolsManager;
    
    /**
     * \brief Provides functions to 
     *  dynamically declare new Grob classes,
     *  commands, shaders and tools. 
     * \note SceneGraphLibrary member functions 
     *  register_xxx use typenames passed as strings. It is 
     *  preferred to use typesafe template wrappers
     *  ogf_register_xxx<>().
     */
    class SCENE_GRAPH_API SceneGraphLibrary : public Environment {
    public:

        /**
         * \brief SceneGraphLibrary constructor.
         */
        SceneGraphLibrary();

        /**
         * \brief SceneGraphLibrary destructor.
         */
         ~SceneGraphLibrary() override;

        /**
         * \brief Gets the instance.
         * \return a pointer to the SceneGraphLibrary instance
         */
        static SceneGraphLibrary* instance();

        /**
         * \brief Initializes the SceneGraphLibrary instance.
         * \details This function is called at Graphite startup. Client
         *  code should not call it.
         */
        static void initialize();

        /**
         * \brief Terminates the SceneGraphLibrary instance.
         * \details This function is called at Graphite shutdown. Client
         *  code should not call it.
         */
        static void terminate();


        /**
         * \brief Gets the SceneGraph.
         * \return a pointer to the SceneGraph
         */
        SceneGraph* scene_graph() {
            return scene_graph_;
        }

        /**
         * \brief Gets the SceneGraph.
         * \return a const pointer to the SceneGraph
         */
        const SceneGraph* scene_graph() const {
            return scene_graph_;
        }
        
        /**
         * \brief Gets the SceneGraphToolsManager.
         * \return a pointer to the SceneGraphToolsManager
         */
        SceneGraphToolsManager* scene_graph_tools_manager() {
            return scene_graph_tools_manager_;
        }
        
        /**
         * \brief Registers a new Grob type.
         * \param[in] grob_class_name the object class name as a string,
         *  with the "OGF::" prefix
	 * \param[in] abstract true if the object class name is abstract
         * \note it is prefered to use the type-safe 
         *  ogf_register_grob_type instead.
         */
        void register_grob_type(
            const std::string& grob_class_name, bool abstract=false
        );

        /**
         * \brief Registers a new file extension associated with a Grob type
         *  for reading.
         * \param[in] grob_class_name the object class name as a string,
         *  with the "OGF::" prefix
         * \param[in] extension the extension, without the "."
         * \note it is preferred to use the type-safe 
         *  ogf_register_grob_read_file_extension instead.
         */
        void register_grob_read_file_extension(
            const std::string& grob_class_name, const std::string& extension
        );

        /**
         * \brief Registers a new file extension associated with a Grob type
         *  for writing.
         * \param[in] grob_class_name the object class name as a string,
         *  with the "OGF::" prefix
         * \param[in] extension the extension, without the "."
         * \note it is preferred to use the type-safe 
         *  ogf_register_grob_write_file_extension instead.
         */
        void register_grob_write_file_extension(
            const std::string& grob_class_name, const std::string& extension
        );

        /**
         * \brief Registers a new Shader class associated with a Grob class.
         * \param[in] grob_class_name the object class name as a string,
         *  with the "OGF::" prefix
         * \param[in] shader_class_name the shader class name as a string,
         *  with the "OGF::" prefix
         * \param[in] shader_user_name an optional user name for the shader,
         *  that will be used in the GUI
         * \note it is preferred to use the type-safe 
         *  ogf_register_grob_shader instead.
         */
        void register_grob_shader(
            const std::string& grob_class_name,
            const std::string& shader_class_name,
            const std::string& shader_user_name=""
        );

        /**
         * \brief Registers a new Tool class associated with a Grob class.
         * \param[in] grob_class_name the object class name as a string,
         *  with the "OGF::" prefix
         * \param[in] tool_class_name the tool class name as a string,
         *  with the "OGF::" prefix
         * \note it is preferred to use the type-safe
         *  ogf_register_grob_tool instead.
         */
        void register_grob_tool(
            const std::string& grob_class_name,
            const std::string& tool_class_name
        );


        /**
         * \brief Registers a new Interface class associated with a Grob class.
         * \param[in] grob_class_name the object class name as a string,
         *  with the "OGF::" prefix
         * \param[in] interface_class_name the tool class name as a string,
         *  with the "OGF::" prefix
         * \note it is preferred to use the type-safe
         *  ogf_register_grob_interface instead.
         */
        void register_grob_interface(
            const std::string& grob_class_name,
            const std::string& interface_class_name
        );
	
        /**
         * \brief Registers a new Commands class associated with a Grob class.
         * \param[in] grob_class_name the object class name as a string,
         *  with the "OGF::" prefix
         * \param[in] commands_class_name the tool class name as a string,
         *  with the "OGF::" prefix
         * \note it is preferred to use the type-safe
         *  ogf_register_grob_commands instead.
         */
        void register_grob_commands(
            const std::string& grob_class_name,
            const std::string& commands_class_name
        );

        /**
         * \brief Registers a new full screen effect.
         * \param[in] full_screen_effect_class_name the class name of the
         *  full screen effect, with the "OGF::" prefix
         * \note it is preferred to use the type-safe
         *  ogf_register_full_screen_effect instead.
         * \param[in] user_name an optional user name for the full screen
         *  effect that will be used in the GUI
         */
        void register_full_screen_effect(
            const std::string& full_screen_effect_class_name,
            const std::string& user_name=""
        );

        /**
         * \brief Finds the object class names associated with a file 
         *  extension for reading.
         * \param[in] extension the extension, without the "."
         * \return the ';'-separated list of object class names that 
         *  can read the extension
         */
        std::string file_extension_to_grob(const std::string& extension) const;

        /**
         * \copydoc Environment::get_local_value()
         * \details Provides the following environment variables:
         *  - grob_types
         *  - grob_read_extensions
         *  - grob_write_extensions
         *  - SceneGraph_commands
         *  - xxx_read_extensions
         *  - xxx_write_extensions
         *  - xxx_shaders
         *  - xxx_tools
         *  - xxx_commands
         *  - full_screen_effects
         */
	bool get_local_value(
            const std::string& name, std::string& value
        ) const override;

        /**
         * \copydoc Environment::set_local_value()
         */
	bool set_local_value(
            const std::string& name, const std::string& value
        ) override;

        /**
         * \brief Gets the default file extension associated with
         *  a Grob class for reading.
         * \param[in] grob_class_name the Grob class name as a string,
         *  with the "OGF::" prefix
         * \return the default file extension associated with the Grob
         *  class name for reading
         */
        std::string default_grob_read_extension(
            const std::string& grob_class_name
        ) const;

        /**
         * \brief Gets the default file extension associated with
         *  a Grob class for writing.
         * \param[in] grob_class_name the Grob class name as a string,
         *  with the "OGF::" prefix
         * \return the default file extension associated with the Grob
         *  class name for writing
         */
        std::string default_grob_write_extension(
            const std::string& grob_class_name
        ) const;

        /**
         * \brief Converts a shader user name to the associated internal
         *  class name.
         * \param[in] grob_class_name the Grob class name the shader is
         *  associated with, as a string with the "OGF::" prefix
         * \param[in] shader_user_name the shader user name
         * \return the internal shader class name as a string, with the
         *  "OGF::" prefix
         */
        const std::string& shader_user_to_classname(
            const std::string& grob_class_name,
            const std::string& shader_user_name
        ) const;

        /**
         * \brief Converts a shader class name to the associated user
         *  shader name.
         * \param[in] grob_class_name the Grob class name the shader is
         *  associated with, as a string with the "OGF::" prefix
         * \param[in] shader_class_name the Shader class name as a string,
         *  with the "OGF::" prefix
         * \return the user shader name
         */
        const std::string& shader_classname_to_user(
            const std::string& grob_class_name,
            const std::string& shader_class_name
        ) const;

        /**
         * \brief Converts a full screen effect user name to the associated
         *  internal full screen effect class name.
         * \param[in] full_screen_effect_user_name the user name of the
         *  full screen effect
         * \return the internal class name of the full screen effect as
         *  a string, with the "OGF::" prefix
         */
	std::string full_screen_effect_user_to_classname(
            const std::string& full_screen_effect_user_name
        ) const;

        /**
         * \brief Converts a full screen effect class name to the associated
         *  user name.
         * \param[in] full_screen_effect_classname the class name of the
         *  full screen effect as a string, with the "OGF::" prefix
         * \return the user name of the full screen effect
         */
        const std::string& full_screen_effect_classname_to_user(
            const std::string& full_screen_effect_classname
        ) const;

    protected:
        friend class SceneGraph;
        friend class SceneGraphCommandsManager;
        friend class SceneGraphShaderManager;
        friend class SceneGraphToolsManager;


        /**
         * \brief Notifies all listeners of scene graph
         *  environment variables.
         */
        void scene_graph_values_changed_notify_environment();
        
        /**
         * \brief Sets the SceneGraph.
         * \param[in] scene_graph a pointer to the SceneGraph
         * \pre scene_graph() == nullptr
         */
        void set_scene_graph(SceneGraph* scene_graph) {
            ogf_assert(scene_graph_ == nullptr);
            scene_graph_ = scene_graph;
        }

        /**
         * \brief Sets the SceneGraphShaderManager.
         * \param[in] scene_graph_shader_manager a pointer to the 
         *  SceneGraphShaderManager
         * \pre scene_graph_shader_manager() == nullptr
         */
        void set_scene_graph_shader_manager(
            SceneGraphShaderManager* scene_graph_shader_manager
        ) {
            ogf_assert(scene_graph_shader_manager_ == nullptr);
            scene_graph_shader_manager_ = scene_graph_shader_manager;
        }

        /**
         * \brief Sets the SceneGraphToolsManager.
         * \param[in] scene_graph_tools_manager a pointer 
         *  to the SceneGraphToolsManager
         * \pre scene_graph_tools_manager() == nullptr
         */
        void set_scene_graph_tools_manager(
            SceneGraphToolsManager* scene_graph_tools_manager
        ) {
            ogf_assert(scene_graph_tools_manager_ == nullptr);
            scene_graph_tools_manager_ = scene_graph_tools_manager;
        }
        
    private:
        struct GrobInfo {
	    GrobInfo(bool abstract_in=false) : abstract(abstract_in) {
	    }
            std::vector<std::string> read_file_extensions;
            std::vector<std::string> write_file_extensions;
            std::vector<std::string> shaders;
            std::vector<std::string> shaders_user_names;
            std::vector<std::string> tools;
            std::vector<std::string> commands;
	    std::vector<std::string> interfaces;
	    bool abstract;
            std::string read_file_extensions_string() const;
            std::string write_file_extensions_string() const;
            std::string shaders_user_names_string() const;
            std::string tools_string() const;
            std::string commands_string() const;
            std::string interfaces_string() const;	    
            bool has_read_extension(const std::string& ext) const;
        };
        std::map<std::string, GrobInfo> grob_infos_;
        std::vector<std::string> full_screen_effects_;
        std::vector<std::string> full_screen_effects_user_names_;
        SceneGraph* scene_graph_;
        SceneGraphShaderManager* scene_graph_shader_manager_;
        SceneGraphToolsManager* scene_graph_tools_manager_;
        static SceneGraphLibrary* instance_;
    };

//______________________________________________________________________________


    /**
     * \brief Helper class to register a new Grob class.
     * \tparam T the Grob class to be registered
     */
    template <class T> class ogf_register_grob_type {
    public:
        /**
         * \brief Registers a new Grob class. 
         * \details Example:
         * \code
         *    ogf_register_grob_type<Mesh>();
         * \endcode
         */
        ogf_register_grob_type() {
            SceneGraphLibrary::instance()->register_grob_type(
                ogf_meta<T>::type()->name()
            );            
        }
    };


    /**
     * \brief Helper class to register a new abstract Grob class.
     * \details Abstract grobs cannot be created.
     * \tparam T the Grob class to be registered.
     */
    template <class T> class ogf_register_abstract_grob_type {
    public:
        /**
         * \brief Registers a new Grob class. 
         * \details Example:
         * \code
         *    ogf_register_grob_type<Mesh>();
         * \endcode
         */
        ogf_register_abstract_grob_type() {
            SceneGraphLibrary::instance()->register_grob_type(
                ogf_meta<T>::type()->name(), true
            );            
        }
    };
    
    /**
     * \brief Helper class to register a new file extension for reading
     * \tparam T the Grob class
     */
    template <class T> class ogf_register_grob_read_file_extension {
    public:
        /**
         * \brief Registers a new file extension for reading.
         * \details Example:
         * \code
         *    ogf_register_grob_read_file_extension<Mesh>("mesh");
         * \endcode
         */
        ogf_register_grob_read_file_extension(const std::string& extension) {
            SceneGraphLibrary::instance()->register_grob_read_file_extension(
                ogf_meta<T>::type()->name(), extension
            );            
        }
    };

    /**
     * \brief Helper class to register a new file extension for writing
     * \tparam T the Grob class
     */
    template <class T> class ogf_register_grob_write_file_extension {
    public:
        /**
         * \brief Registers a new file extension for writing.
         * \details Example:
         * \code
         *    ogf_register_grob_write_file_extension<Mesh>("mesh");
         * \endcode
         */
        ogf_register_grob_write_file_extension(const std::string& extension) {
            SceneGraphLibrary::instance()->register_grob_write_file_extension(
                ogf_meta<T>::type()->name(), extension
            );            
        }
    };

    /**
     * \brief Helper class to register a new Shader associated with a Grob.
     * \tparam T1 the Grob class
     * \tparam T2 the Shader class
     */
    template <class T1, class T2> class ogf_register_grob_shader {
    public:

        /**
         * \brief Registers a new Shader associated with a Grob.
         * \param[in] shader_user_name an optional shader name to be
         *  exposed to the user instead of the Shader class name.
         * \details Example:
         * \code
         *    ogf_register_grob_shader<Mesh, PlainMeshShader>("Plain");
         * \endcode
         */
        ogf_register_grob_shader(const std::string& shader_user_name="") {
            SceneGraphLibrary::instance()->register_grob_shader(
                ogf_meta<T1>::type()->name(),
                ogf_meta<T2>::type()->name(),
                shader_user_name
            );            
        }
    };

    /**
     * \brief Helper class to register a new Commands associated with a Grob.
     * \tparam T1 the Grob class
     * \tparam T2 the Commands class
     */
    template <class T1, class T2> class ogf_register_grob_interface {
    public:
        
        /**
         * \brief Registers a new Interface associated with a Grob.
         * \details Example:
         * \code
         *    ogf_register_grob_commands<Mesh, MeshEditorInterface>();
         * \endcode
         */
        ogf_register_grob_interface() {
            SceneGraphLibrary::instance()->register_grob_interface(
                ogf_meta<T1>::type()->name(), ogf_meta<T2>::type()->name()
            );            
        }
    };
    
    /**
     * \brief Helper class to register a new Commands associated with a Grob.
     * \tparam T1 the Grob class
     * \tparam T2 the Commands class
     */
    template <class T1, class T2> class ogf_register_grob_commands {
    public:
        
        /**
         * \brief Registers a new Commands associated with a Grob.
         * \details Example:
         * \code
         *    ogf_register_grob_commands<Mesh, MeshSurfaceCommands>();
         * \endcode
         */
        ogf_register_grob_commands() {
            SceneGraphLibrary::instance()->register_grob_commands(
                ogf_meta<T1>::type()->name(), ogf_meta<T2>::type()->name()
            );            
        }
    };

    /**
     * \brief Helper class to register a new Tool associated with a Grob.
     * \tparam T1 the Grob class
     * \tparam T2 the Tool class
     */
    template <class T1, class T2> class ogf_register_grob_tool {
    public:

        /**
         * \brief Registers a new Tool associated with a Grob.
         * \details Example:
         * \code
         *    ogf_register_grob_tool<Mesh, MeshGrobTransformComponent>();
         * \endcode
         */
        ogf_register_grob_tool() {
            SceneGraphLibrary::instance()->register_grob_tool(
                ogf_meta<T1>::type()->name(), ogf_meta<T2>::type()->name()
            );            
        }
    };

    /**
     * \brief Helper class to register a new FullScreenEffect.
     * \tparam T the FullScreenEffect class.
     */    
    template <class T> class ogf_register_full_screen_effect {
    public:

        /**
         * \brief Registers a new FullScreenEffect.
         * \param[in] user_name an optional user name for the full screen
         *  effect, that will be used in the GUI
         * \details Example:
         * \code
         *    ogf_register_full_screen_effect<AmbientOcclusion, "SSAO">();
         * \endcode
         */
        ogf_register_full_screen_effect(const std::string& user_name="") {
            SceneGraphLibrary::instance()->register_full_screen_effect(
                ogf_meta<T>::type()->name(), user_name
            );            
        }
    };

}

#endif
