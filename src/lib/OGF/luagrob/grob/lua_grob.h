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
 

#ifndef H_OGF_GRAPHITE_LUAGROB_LUA_GROB_H
#define H_OGF_GRAPHITE_LUAGROB_LUA_GROB_H

#include <OGF/luagrob/common/common.h>
#include <OGF/scene_graph/grob/grob.h>

#ifdef GOMGEN
typedef int lua_State;
#else
extern "C" {
#include <geogram/third_party/lua/lua.h>
}
#endif

/**
 * \file OGF/luagrob/grob/lua_grob.h
 * \brief the LuaGrob class.
 */
namespace OGF {

    /**
     * \brief A Grob class for luagrob grids.
     */
    gom_class LUAGROB_API LuaGrob : public Grob {
    public:
        /**
         * \brief LuaGrob constructor.
         * \param[in] parent a pointer to the container (the scenegraph 
         *  in most cases).
         */
        LuaGrob(CompositeGrob* parent);

	/** 
	 * \brief LuaGrob destructor.
	 */
	~LuaGrob() override;

	/**
	 * \brief Tests the status of the loaded LUA program.
	 * \retval true if program is OK
	 * \retval false if error occured
	 */
	bool shader_OK() const {
	    return !lua_error_occured_;
	}
	
    gom_properties:

	/**
	 * \brief Tests whether the source should be
	 *  executed when the object is loaded.
	 * \retval true if the source should be executed
	 *   automatically.
	 * \retval false otherwise.
	 */
	bool get_autorun() const {
	    return autorun_;
	}

	/**
	 * \brief Sets whether the source should be 
	 *  executed automatically when the object is loaded.
	 * \param[in] x true if the source should be 
	 *  executed automatically, false otherwise.
	 */
	void set_autorun(bool x) {
	    autorun_ = x;
	}
	
	/**
	 * \brief Sets the source of the LUA program.
	 * \param[in] source the LUA source.
	 */
	void set_source(const std::string& source) {
	    source_ = source;
	}

	/**
	 * \brief Gets the source of the LUA program.
	 * \return the LUA source.
	 */
	const std::string& get_source() const {
	    return source_;
        }
	
	/**
	 * \brief Sets the source of the LUA shader program 
	 *   and executes it.
	 * \param[in] source the LUA source.
	 */
	void set_shader_source(const std::string& source) {
	    shader_source_ = source;
	    execute_shader_program();
	}

	/**
	 * \brief Gets the source of the LUA shader program.
	 * \return the LUA source.
	 */
	const std::string& get_shader_source() const {
	    return shader_source_;
        }

	
    gom_slots:

	/**
	 * \brief Executes the stored program.
	 * \details The program is executed in the main LUA
	 *  interpreter.
	 */
	void execute_program();
	
	/**
	 * \brief Executes a LUA command in the interpreter
	 *  used for the shader.
	 * \param[in] value the command.
	 */
	bool execute_shader_command(const std::string& value);

	/**
	 * \brief Executes the loaded LUA shader program.
	 */
	void execute_shader_program();

	/**
	 * \brief Loads the LUA program from a file.
	 * \param[in] value the filename.
	 */
	void load_program_source(const std::string& value);

	/**
	 * \brief Saves the LUA program to a file.
	 * \param[in] value the filename.
	 */
	void save_program_source(const std::string& value);
	
	/**
	 * \brief Loads the LUA shader program from a file.
	 * \param[in] value the filename.
	 */
	void load_shader_source(const std::string& value);

	/**
	 * \brief Saves the LUA shader program to a file.
	 * \param[in] value the filename.
	 */
	void save_shader_source(const std::string& value);
	
    public:
	
        /**
         * \brief Sets the bounding box
         * \param[in] box the new box
         */
        void set_box(const Box3d& box);

        /**
         * \copydoc Grob::clear()
         */
        void clear() override;

        /**
         * \copydoc Grob::duplicate()
         */
        Grob* duplicate(SceneGraph* sg) override;

        /**
         * \copydoc Grob::is_serializable()
         */
        bool is_serializable() const override;

        /**
         * \copydoc Grob::serialize_read()
         */
        bool serialize_read(InputGraphiteFile& geofile) override;

        /**
         * \copydoc Grob::serialize_write()
         */
        bool serialize_write(OutputGraphiteFile& geofile) override;

        /**
         * \copydoc Grob::update()
         */
        void update() override;

        /**
         * \copydoc Grob::bbox()
         */
        Box3d bbox() const override;

        /**
         * \brief Finds or creates a LuaGrob with the specified name
         * \param[in] sg a pointer to the SceneGraph
         * \param[in] name the name
         * \return a pointer to the LuaGrob named as \p name in the
         *  SceneGraph \p sg if it exists, or a newly created LuaGrob
         *  otherwise.
         */
        static LuaGrob* find_or_create(
            SceneGraph* sg, const std::string& name
        );

        /**
         * \brief Finds a LuaGrob by name
         * \param[in] sg a pointer to the SceneGraph
         * \param[in] name the name
         * \return a pointer to the LuaGrob named as \p name in the
         *  SceneGraph \p sg if it exists, or nil otherwise.
         */
        static LuaGrob* find(SceneGraph* sg, const std::string& name);

    private:
	lua_State* lua_shader_state_;
	bool lua_error_occured_;
	Box3d box_;
	bool autorun_;
	std::string source_;
	std::string shader_source_;
    };

    /**
     * \brief The name of an existing LuaGrob in the SceneGraph.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with all the LuaGrob objects found in the SceneGraph.
     */
    typedef Name<LuaGrob*> LuaGrobName;

    /**
     * \brief The name of an (existing or not) LuaGrob in the SceneGraph.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with all the LuaGrob objects found in the SceneGraph.
     *  In additon, the field can be edited, and the user can enter in 
     *  it a new name, not already present in the SceneGraph.     
     */
    typedef Name<LuaGrob*,true> NewLuaGrobName;
}
#endif

