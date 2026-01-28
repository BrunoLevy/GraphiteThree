-- Lua
-- Script to create a ZIP archive for a Windows version of Graphite
-- from a compiled source-tree.

if FileSystem.os_name() ~= 'Windows' then
    gom.err('make_windows_dist: current OS is not Windows')

else

-- The version of Graphite, used in the name of the created
-- ZIP archive.
GRAPHITE_VERSION=gom.get_environment_value("version")

-- Where the current Graphite distribution is located
SOURCE_PROJECT_ROOT = gom.get_environment_value("PROJECT_ROOT")

-- The directory that will contain the copy of the distribution and
--   the generated ZIP file.
TARGET_DIR = FileSystem.normalized_path(
    SOURCE_PROJECT_ROOT .. "/../../GRAPHITEDIST"
)

-- The copy of the distribution.
TARGET_PROJECT_ROOT = TARGET_DIR .. "/GraphiteThree"

-- ****************************************************************************

-- Gets the value of a variable from the CMake cache.
-- name: name of the variable
-- returns the value of the variable
--
function get_cmake_cache_variable(name)
   cmakecache=gom.get_environment_value("PROJECT_ROOT") ..
       "/build/Windows/CMakeCache.txt"
   gom.out("Opening CMakeCache: " .. cmakecache)
   for line in io.lines(cmakecache) do
      if line[1] ~= "#" then
          col=string.find(line,":")
          equ=string.find(line,"=")
	  if col ~= nil and equ ~= nil then
	      local var_name = string.sub(line,1,col-1)
              if var_name == name then
    	           local var_val  = string.sub(line,equ+1)
                   gom.out(var_name .. " = " .. var_val)
                   return var_val
	      end
	  end
       end
   end
   return ""    
end

-- *****************************************************************************

-- Deletes a directory and all the files and subdirectories that it contains.
-- dir: the path to the directory to be deleted
--
function delete_tree(dir)
   -- print("Delete directory:" .. dir)
   for i, path in pairs(FileSystem.get_files(dir)) do
      -- print("Delete file: " .. path)
      FileSystem.delete_file(path)
   end
   for i, path in pairs(FileSystem.get_subdirectories(dir)) do
      delete_tree(path)
   end
   FileSystem.delete_directory(dir)   
end


-- Copies a directory and all the files and subdirectories that it contains.
-- from: the path to the directory to be copied
-- to: the destination directory, including the toplevel directory
-- exclude_list: an optional array of LUA regular expressionq indicating
--   the files that should not be copied.
--
function copy_tree(from, to, exclude_list)
   -- gom.out("Copy directory:" .. from .. "->" .. to)

   FileSystem.create_directory(to)
   
   for i,path in pairs(FileSystem.get_files(from)) do
       local exclude_file = false
       local file = FileSystem.base_name(path,false)
       if(exclude_list ~= nil) then
          for j,pattern in pairs(exclude_list) do
	     if string.match(file, pattern) ~= nil then
	        exclude_file = true
		break
	     end
	  end
       end
       if not exclude_file then
          -- gom.out("Copying " .. file)
	      FileSystem.copy_file(from .. "/" .. file, to .. "/" .. file)
       else
          -- gom.out("Skipping " .. file .. " (excluded)")
       end
   end

   for i,path in pairs(FileSystem.get_subdirectories(from)) do
       local directory = FileSystem.base_name(path,false)
       copy_tree(path, to .. "/" .. directory, exclude_list)
   end
end

-- Creates a ZIP archive using standard Windows tools.
--
function make_archive(archive_name, root_dir, base_dir)
   local command = string.gsub(SOURCE_PROJECT_ROOT,"/","\\")..
          "\\lib\\zipjs.bat zipItem -source "..root_dir.."\\"..base_dir..
		  " -destination "..archive_name
   os.execute(command)
end

-- *****************************************************************************

-- Creates a binary distribution of Graphite
--
function create_binary_distrib()

   gom.out(
      "Preparing Graphite source tree compiled under windows " ..
      "for binary distribution, version= " .. GRAPHITE_VERSION
   )

   gom.out("*** Creating Graphite file hierarchy...")

   gom.out("TARGET_DIR="..TARGET_DIR)

   if FileSystem.is_directory(TARGET_DIR) then
      delete_tree(TARGET_DIR)
   end

   FileSystem.create_directory(TARGET_DIR)
   FileSystem.create_directory(TARGET_PROJECT_ROOT)

   gom.out("*** Copying Graphite binaries...")

   SOURCE_BIN_DIR=get_cmake_cache_variable("Graphite_BINARY_DIR") ..
      "/bin/Release"
   TARGET_BIN_DIR=TARGET_PROJECT_ROOT.."/bin/win64"

   copy_tree(
	SOURCE_BIN_DIR,
        TARGET_BIN_DIR,
	{"Qt.*d%.dll","gomgen%.exe","Qt.?XmlPatterns%.dll"}
   )

   gom.out("*** Copying Graphite runtime...")
   copy_tree(
       SOURCE_PROJECT_ROOT .. "/lib",
       TARGET_PROJECT_ROOT .. "/lib",
	   {"graphite_ini%.gml"}
   )

   gom.out("*** Copying doc and launcher bat script...")
   FileSystem.copy_file(
      SOURCE_PROJECT_ROOT.."/doc/binary_win_dist/README.txt",
      TARGET_PROJECT_ROOT.."/README.txt"
   )
   FileSystem.copy_file(
      SOURCE_PROJECT_ROOT.."/doc/binary_win_dist/graphite.bat",
      TARGET_PROJECT_ROOT.."/graphite.bat"
   )

   gom.out("*** Copying plugin configuration files and scripts...")

   FileSystem.create_directory(TARGET_PROJECT_ROOT.."/plugins/OGF")
   FileSystem.copy_file(
      SOURCE_PROJECT_ROOT.."/plugins/OGF/Plugins.txt",
      TARGET_PROJECT_ROOT.."/plugins/OGF/Plugins.txt"
   )

   for plugin in string.split(
      gom.get_environment_value('loaded_dynamic_modules'),';'
   ) do
      gom.out('Plugin configuration for: ' .. plugin)
      if FileSystem.is_directory(
         SOURCE_PROJECT_ROOT.."/plugins/OGF/"..plugin.."/programming_examples"
      ) then
         copy_tree(
	 SOURCE_PROJECT_ROOT.."/plugins/OGF/"..plugin.."/programming_examples",
	 TARGET_PROJECT_ROOT.."/lib/"..plugin.."/programming_examples"
	 )
      end
   end

   if FileSystem.is_file('C:/Windows/system32/vcruntime140_1.dll') then
      gom.out("*** Copying MSVC runtime")   
      FileSystem.copy_file(
         "C:/Windows/system32/vcruntime140_1.dll",
	 TARGET_BIN_DIR.."/vcruntime140_1.dll"
      )
   end

   gom.out("*** Now creating archive...")

   ARCHIVE_SHORTNAME = 'graphite' .. GRAPHITE_VERSION .. '-win64.zip'
   ARCHIVE_NAME = TARGET_DIR .. '/' .. ARCHIVE_SHORTNAME

   gom.out("Archive name = " .. ARCHIVE_NAME)
   make_archive(ARCHIVE_NAME, TARGET_DIR, "GraphiteThree")

   F = io.open(TARGET_DIR..'/index.html','w')
   F:write('<H1> Graphite for Windows Release </H1>\n')
   F:write('<ul>\n')
   F:write('<li> <a href="' .. ARCHIVE_SHORTNAME .. '">' .. ARCHIVE_SHORTNAME .. '</li>')
   F:write('<ul>\n')   
   io.close(F)

   gom.out('*** DONE ! ***')

end


-- *****************************************************************************

if graphite_main_window ~= nil then

   make_windows_dist_module = {}
   make_windows_dist_module.visible = false
   make_windows_dist_module.name = 'WinPacker'
   make_windows_dist_module.icon = '@box-open'

   function make_windows_dist_module.draw_window()
      SOURCE_PROJECT_ROOT = gom.get_environment_value("PROJECT_ROOT")
      TARGET_PROJECT_ROOT = SOURCE_PROJECT_ROOT ..
         "/../../GRAPHITEDIST/GraphiteThree"

      imgui.Text(
          'Preparing Graphite source tree\n'..
          'compiled under windows\n' ..
          'for binary distribution...'
      )
      imgui.Separator()
      imgui.Text('Version:')
      imgui.SameLine()
      _,GRAPHITE_VERSION = imgui.TextInput('##version', GRAPHITE_VERSION)
      imgui.Separator()
      imgui.Text('Are you sure ?')
      imgui.Text('This will erase all files in:')
      imgui.Text(TARGET_DIR)
      imgui.Separator()
      if imgui.Button('OK##make_windows_dist') then
         make_windows_dist_module.visible = false
         console_gui.visible = true      				
         create_binary_distrib()
      end
      imgui.SameLine()
      if imgui.Button('Cancel##make_windows_dist') then
         make_windows_dist_module.visible = false      
      end
   end

   graphite_main_window.add_module(make_windows_dist_module)

else
   -- if running in shell mode, create the distribution
   create_binary_distrib()
   os.exit()
end

end -- this one is for "if FileSystem.os_name() == 'Windows'"
