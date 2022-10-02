-- =============================================================================
-- Utilities
-- =============================================================================

-- \brief Tests whether a string starts with a prefix
-- \param[in] str the string
-- \param[in] prefix the prefix
-- \return true if \p str starts with \p prefix
-- \return false otherwise

function string.starts_with(str,prefix)
   return str:sub(1,prefix:len())==prefix
end

-- \brief Tests whether a string ends with a suffix
-- \param[in] str the string
-- \param[in] suffix the suffix
-- \return true if \p str ends with \p suffix
-- \return false otherwise

function string.ends_with(str,suffix)
   return suffix=='' or str:sub(-suffix:len())==suffix
end

-- \brief Splits a string
-- \param[in] str the string
-- \param[in] sep a separator
-- \return an iterator on the words contained in the string

function string.split(str,sep)
   return str:gmatch("([^"..sep.."]+)")
end

-- \brief Removes the beginning of a string
-- \param[in] str a string
-- \param[in] prefix the prefix to remove
-- \return str with prefix removed if it matched

function string.strip_prefix(str,prefix)
   local result = str
   if string.starts_with(str,prefix) then
      str = str:sub(prefix:len()-str:len())
   end
   return str
end

-- \brief Removes the end of a string
-- \param[in] str a string
-- \param[in] suffix the suffix to remove
-- \return str with suffix removed if it matched

function string.strip_suffix(str,suffix)
   local result = str
   if string.ends_with(str,suffix) then
      str = str:sub(1,str:len()-suffix:len())
   end
   return str
end

-- \brief Reads a file in a string
-- \param[in] filename the name of the file
-- \return a string with the contents of the file

function string.load_file(filename)
    local f = assert(io.open(filename, 'rb'))
    local content = f:read('*all')
    f:close()
    return content
end

-- \brief Sleeps during a certain time
-- \param a the time to sleep in seconds
-- (floating point number)

function sleep(a) 
    local sec = tonumber(os.clock() + a); 
    while (os.clock() < sec) do 
    end 
end

-- Auto load plugins declared in the sources

function auto_load_plugins()
   local plugins_config_file =
       gom.get_environment_value('PROJECT_ROOT') ..
       '/plugins/OGF/Plugins.txt'
   if FileSystem.is_file(plugins_config_file) then
      gom.out("Loading plugins declared in " .. plugins_config_file)
      for line in io.lines(plugins_config_file) do
          line = line:gsub('add_subdirectory','')
	  if line:sub(1,1) == "(" and line:sub(-1) == ")" then
	     local module = line:sub(2,-2)
	     gom.load_module(module)
	  end
      end
   end
end

-- Post-init mechanism
-- (loads scripts and objects specified on the command line 
--  after the GUI is created and ready to start)

function post_init()

    if scene_graph.scene_graph_shader_manager ~= nil then
	scene_graph.scene_graph_shader_manager.full_screen_effect(
	   gom.get_environment_value('gfx:default_full_screen_effect')
	)
    end	   

    auto_load_plugins()

    local arglist = gom.get_environment_value("command_line")
    for arg in arglist:split("!") do
      if arg:starts_with("-") or arg:find("=") then
          gom.out("Flag on command line: " .. arg)
      else
         if arg:ends_with(".lua") then
	    if text_editor_gui ~= nil then
	       text_editor_gui.load(arg)
	       text_editor_gui.filename = arg
	       text_editor_gui.visible = true
	    end
	    if main ~= nil then
	       main.exec_command('gom.execute_file(\''..arg..'\')')
	    else
	       gom.execute_file(arg)	   
	    end
	 else
            scene_graph.load_object(arg)
	 end
      end
   end
   collectgarbage()
end

-- Numerical library, helpers.

NL = gom.create({classname = 'OGF::NL::Library'})


