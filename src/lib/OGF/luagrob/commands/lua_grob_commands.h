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


#ifndef H_OGF_LUAGROB_COMMANDS_LUAGROB_COMMANDS_H
#define H_OGF_LUAGROB_COMMANDS_LUAGROB_COMMANDS_H

#include <OGF/luagrob/common/common.h>
#include <OGF/luagrob/grob/lua_grob.h>
#include <OGF/scene_graph/commands/commands.h>

/**
 * \file OGF/luagrob/commands/lua_grob_commands.h
 * \brief Base class for Commands related with a LuaGrob object.
 */
namespace OGF {

    /**
     * \brief Base class for Commands related with a LuaGrob object.
     */
    gom_attribute(abstract,"true")
    gom_class LUAGROB_API LuaGrobCommands : public Commands {
    public:

        /**
         * \brief LuaGrobCommands constructor.
         */
        LuaGrobCommands();

        /**
         * \brief LuaGrobCommands destructor.
         */
        ~LuaGrobCommands() override;

        /**
         * \brief Gets the LuaGrob
         * \return a pointer to the LuaGrob these Commands are
         *  associated with
         */
        LuaGrob* lua_grob() const {
            return dynamic_cast<LuaGrob*>(grob());
        }
    };

    /**
     * \brief Just a place-holder to define a type for
     *  LUA file names.
     */
    class LuaFile {
    };

    /**
     * \brief The name of an existing LUA (.lua) file.
     */
    typedef Name<LuaFile> LuaFileName;

    /**
     * \brief The name of an existing or not LUA (.lua) file.
     */
    typedef Name<LuaFile,true> NewLuaFileName;

    /**
     * \brief Commands to load and save the LUA program in a LuaGrob.
     */
    gom_class LUAGROB_API LuaGrobProgramCommands: public LuaGrobCommands {
      public:
	/**
	 * \brief LuaGrobProgramCommands constructor.
	 */
	LuaGrobProgramCommands();

	/**
	 * \brief LuaGrobShaderCommands destructor.
	 */
	~LuaGrobProgramCommands() override;

      gom_slots:
	void run();
	void set_autorun(bool autorun);
	void load_program(const LuaFileName& file_name);
	void save_program(const NewLuaFileName& file_name);
	void clear();
    };

    /**
     * \brief Commands to load and save the LUA shader program in a LuaGrob.
     */
    gom_class LUAGROB_API LuaGrobShaderCommands: public LuaGrobCommands {
      public:
	/**
	 * \brief LuaGrobShaderCommands constructor.
	 */
	LuaGrobShaderCommands();

	/**
	 * \brief LuaGrobShaderCommands destructor.
	 */
	~LuaGrobShaderCommands() override;

      gom_slots:
	void load_shader(const LuaFileName& file_name);
	void save_shader(const NewLuaFileName& file_name);
	void clear();
    };
}
#endif
