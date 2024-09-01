--  GUI for the text editor, for use with Skin_imgui
------------------------------------------------

text_editor_gui = {}
text_editor_gui.visible = false
text_editor = gom.create({classname='OGF::TextEditor',interpreter=gom})
text_editor.text = '-- Lua (Keep this comment, this is an indication for editor\'s \'run\' command)\n\n'
text_editor_gui.name = 'Programs'
text_editor_gui.icon = '@code'
text_editor_gui.menubar = true
text_editor_gui.x = 4*main.margin() + 150*main.scaling()
text_editor_gui.y = main.margin()
text_editor_gui.w = main.width - 5*main.margin() - 150*main.scaling()
text_editor_gui.h = 200*main.scaling()
text_editor_gui.window_flags = 131072 -- ImGuiWindowFlags_ResizeFromAnySide
text_editor_gui.filename = nil
text_editor_gui.find_visible = false
text_editor_gui.find_word = ''
text_editor_gui.find_focus = false

function text_editor_gui.error(msg)
   gom.err(msg)
end

function text_editor_gui.set_language(lang)
   text_editor.language = lang
-- text_editor_gui.icon = lang
end

function text_editor_gui.load(file)
   text_editor_gui.set_language(FileSystem.extension(file))
   text_editor.load(file)
end

function text_editor_gui.reload()
   if text_editor_gui.filename ~= nil then
      text_editor_gui.load(text_editor_gui.filename)
   end
end

function text_editor_gui.save(filename)
   if filename == nil then
      filename = text_editor_gui.filename
   end
   if filename == nil then
	 imgui.OpenFileDialog(
	    '##text_editor##save_dlg',
	    text_editor.language,
	    '', -- default filename
	    ImGuiExtFileDialogFlags_Save
        )
   else
      text_editor_gui.set_language(FileSystem.extension(filename))
      text_editor.save(filename)
      print('Saved '..filename)
   end
end

-- Generates menus and menu entries to load files
-- in a directory
function text_editor_gui.browse(directory)
   subdirs = FileSystem.get_subdirectories(directory)
   table.sort(subdirs)
   for _,subdir in pairs(subdirs) do
      subdir_basename = FileSystem.base_name(subdir,false)
      if not string.starts_with(subdir_basename,'.') then
         if imgui.BeginMenu(subdir_basename) then
            text_editor_gui.browse(subdir)
	    imgui.EndMenu()
	 end
      end
   end
   local files = FileSystem.get_files(directory)
   table.sort(files)
   for _,file in pairs(files) do
      ext = FileSystem.extension(file)
      if imgui.MenuItem(FileSystem.base_name(file,false)) then
         text_editor_gui.load(file)
         text_editor_gui.filename = nil
      end
   end
end

-- Generates menus and menu entries to load files
-- in a directory. First level contains one
-- subdirectory per language. They are browsed only
-- if the associated interpreter is loaded.
function text_editor_gui.browse_by_language(directory)
   subdirs = FileSystem.get_subdirectories(directory)
   table.sort(subdirs)
   for _,subdir in pairs(subdirs) do
      subdir_basename = FileSystem.base_name(subdir,false)
      if not string.starts_with(subdir_basename,'.') then
         if subdir_basename == "GLSL" or
	    gom.interpreter(subdir_basename) ~= nil then
            if imgui.BeginMenu(subdir_basename) then
               text_editor_gui.browse(subdir)
	       imgui.EndMenu()
	    end
	 end
      end
   end
end

-- \brief Parses the latest error and installs error markers
--  in the editor.
function text_editor_gui.parse_errors()
   local error_lines = main.latest_error
   if error_lines == '' then
      return
   end
   main.reset_latest_error()
   if text_editor.language == 'glsl' then
      -- If the error message has multiple lines, segment it.
      for line in string.gmatch(error_lines,'[^\r\n]+') do
         -- Now try to parse GLSL error message.
	 -- Unfortunately it is not normalized...
	 local lineno, message
	 if string.starts_with(line,'ERROR') then
            -- This works for Apple
	    lineno, message = string.match(line, 'ERROR:.*:(%d+): (.*)')
         else
            -- This works for both Intel and NVidia GLSL.
	    local errorkind
            lineno, errorkind, message =
                string.match(line, '%d+.(%d+).*:(.*):(.*)')
            if errorkind == nil or string.find(errorkind,'error') == nil then
                message = nil
		lineno = nil
            end
	 end
	 if lineno ~= nil and message ~= nil then
	     text_editor.add_error_marker(tonumber(lineno), message)
	 end
      end
   elseif text_editor.language == 'lua' then
      -- If the error message has multiple lines, segment it.
      for line in string.gmatch(error_lines,'[^\r\n]+') do
	 local lineno, message = string.match(line, 'Error: (%d+): (.*)')
	 if lineno ~= nil and message ~= nil then
	     text_editor.add_error_marker(tonumber(lineno), message)
	 end
      end
   end
end

function text_editor_gui.run()
   -- Hide histogram window, else it will keep updating
   -- it even when changing a single vertex position of
   -- attribute (sloooowwww). TODO: find cleaner way.
   -- (there is an update loop somewhere, that stacks
   -- multiple calls *after* this functions returns).
   Stats.visible = false
   if text_editor.selection ~= '' then
      main.exec_command('text_editor_gui.run_selection()')
      return
   end
   text_editor.clear_error_markers()
   main.reset_latest_error()
   if text_editor.language == 'lua' then
      if not string.starts_with(text_editor.text, '-- Lua') then
         gom.err('Missing -- Lua or -- LuaGrob comment at beginning of source')
	 gom.err('Without it, editor does not know how file should be run')
	 text_editor_gui.error('Cannot run file')
      end
      if string.starts_with(text_editor.text, '-- LuaGrob') then
         local grob = scene_graph.resolve('Lua_program')
         if grob == nil then
            grob = scene_graph.create_object('OGF::LuaGrob')
	    grob.rename('Lua_program')
	    scene_graph.current_object = 'Lua_program'
         end
         grob.shader_source = text_editor.text
      else
         main.exec_command(text_editor.text)
      end
   elseif text_editor.language == 'glsl' then
       local grob = scene_graph.resolve('GLSL_program')
       if grob == nil then
          grob = scene_graph.create_object('OGF::MeshGrob')
          grob.rename('GLSL_program')
	  grob.query_interface('OGF::MeshGrobShapesCommands').create_square()
          scene_graph.current_object = 'GLSL_program'
       end
       if not string.starts_with(text_editor.text, '//stage') then
          gom.err('Missing //stage declaration')
	  gom.err('If copy-pasted from www.shadertoy.com, then')
	  gom.err('Use Edit...Adapt ShaderToy code')
	  text_editor_gui.error('Cannot run file')
       end
       grob.shader.glsl_source = text_editor.text
   else
       main.exec_command('text_editor_gui.run_other_language()')
   end
end

function text_editor_gui.run_other_language()
   local lang = text_editor.language
   if lang == 'py' then
      lang = 'Python'
   end
   local interp = gom.interpreter(lang)
   -- Copy references to main Graphite objects
   -- from Lua to the other language
   interp.bind_object('scene_graph', scene_graph)
   interp.bind_object('main', main)
   interp.bind_object('NL', NL)
   interp.execute(text_editor.text, false, false)
end

function text_editor_gui.run_selection()
   text_editor.clear_error_markers()
   main.reset_latest_error()
   local lang = text_editor.language
   if lang == 'GLSL' then
      text_editor_gui.error('Run selection does not work in GLSL mode')
      return
   end
   if lang == 'py' then
      lang = 'Python'
   elseif lang == 'lua' then
      lang = 'Lua'
   end
   local interp = gom.interpreter(lang)
   interp.execute(text_editor.selection, false, false)
end

function text_editor_gui.stop()
   grob = scene_graph.resolve('Lua_program')
   if grob ~= nil then
      scene_graph.current_object = 'Lua_program'
      scene_graph.delete_current_object()
   end
   grob = scene_graph.resolve('GLSL_program')
   if grob ~= nil then
      scene_graph.current_object = 'GLSL_program'
      scene_graph.delete_current_object()
   end
end

function text_editor_gui.draw_menu()
   if imgui.BeginMenu('File...') then
      if imgui.MenuItem('Run <F5>') then
         text_editor_gui.run()
      end
      if imgui.MenuItem('Stop <Ctrl><C>') then
         text_editor_gui.stop()
      end
      imgui.Separator()
      if imgui.BeginMenu('New...') then
         local templates_path =
	    gom.get_environment_value('PROJECT_ROOT') ..
  	    '/lib/templates'
         if imgui.MenuItem('Lua program') then
	    text_editor_gui.load(templates_path .. '/lua_template.lua')
	    text_editor_gui.filename = nil
         end
         if imgui.MenuItem('LuaGrob shader') then
	    text_editor_gui.load(templates_path .. '/luagrob_template.lua')
	    text_editor_gui.filename = nil
         end
         if imgui.MenuItem('GLSL ShaderToy') then
	    text_editor_gui.load(templates_path .. '/ShaderToy_template.glsl')
	    text_editor_gui.filename = nil
         end
	if gom.interpreter('Python') ~= nil then
	   if imgui.MenuItem('Python') then
  	      text_editor_gui.load(templates_path .. '/Python_template.py')
	      text_editor_gui.filename = nil
	   end
	end
         imgui.EndMenu()
      end
      imgui.Separator()
      if imgui.MenuItem('Load...') then
         local extensions = 'lua;glsl'
	 if gom.interpreter('Python') ~= nil then
	    extensions = extensions .. ';py'
	 end
         imgui.OpenFileDialog(
            '##text_editor##load_dlg',
            extensions,
            '', -- default filename
            ImGuiExtFileDialogFlags_Load
         )
	 end
      if imgui.MenuItem('Reload') then
         text_editor_gui.reload()
      end
      if imgui.MenuItem('Load history') then
         text_editor.text = gom.history
	 text_editor_gui.set_language('lua')
      end
      autogui.tooltip('Loads the Lua history\nin the text editor')
      if imgui.MenuItem('Save <F2>') then
	    text_editor_gui.save()
      end
      if imgui.MenuItem('Save as...') then
	 imgui.OpenFileDialog(
	    '##text_editor##save_dlg',
	    text_editor.language,
	    '', -- default filename
	    ImGuiExtFileDialogFlags_Save
        )
      end
      imgui.Separator()
      if imgui.MenuItem('Hide editor') then
         text_editor_gui.visible = false
      end
      imgui.EndMenu()
   end

   if imgui.BeginMenu('Edit...') then
      if imgui.MenuItem('Clear##edit') then
         text_editor.clear()
	 text_editor_gui.filename = nil
      end
      if imgui.MenuItem('Find...##edit') then
         text_editor_gui.show_find()
      end
      if imgui.MenuItem('Adapt ShaderToy code') then
         text_editor_gui.set_language('glsl')
	 if text_editor.text:starts_with('//stage') then
	    text_editor_gui.error(
                'Has //stage specifier, seems to be already a GLUP shader'
            )
	 end
         local filename=
            gom.get_environment_value('PROJECT_ROOT') ..
	       '/lib/templates/ShaderToy_preamble.glsl'
         local preamble = string.load_file(filename)
	 text_editor.text = preamble .. text_editor.text
	 text_editor_gui.set_language('glsl')
      end
      autogui.tooltip(
          'Copy-paste code from www.shadertoy.com\n'..
	  'then use this function to insert the two lines\n'..
	  'required by Graphite GLSL runtime.')
      imgui.Separator()
      if imgui.BeginMenu('Language') then
        if imgui.MenuItem('Lua') then
	   text_editor_gui.set_language('lua')
	end
	if imgui.MenuItem('GLSL') then
	   text_editor_gui.set_language('glsl')
	end
	if gom.interpreter('Python') ~= nil then
	   if imgui.MenuItem('Python') then
	      text_editor_gui.set_language('py')
	   end
	end
	imgui.EndMenu()
      end
      imgui.EndMenu()
   end

   if imgui.BeginMenu('Objects...') then
    if imgui.BeginMenu('LuaGrob') then
      if imgui.BeginMenu('New Lua object') then
         local sel
         local name = ''
         imgui.Text('Name')
         imgui.PushItemWidth(main.scaling()*100)
         sel,name = imgui.TextInput(
            '##NewLuaGrob',
   	    name,
	    ImGuiInputTextFlags_EnterReturnsTrue
         )
	 imgui.PopItemWidth()
	 if sel then
	   local obj=scene_graph.create_object('OGF::LuaGrob')
	   obj.rename(name)
	 end
	 imgui.EndMenu()
      end
      if imgui.BeginMenu('Get Lua program from...') then
         lua_grobs = gom.get_environment_value('OGF::LuaGrob_instances')
         for lua_grob in string.split(lua_grobs,';') do
            if imgui.MenuItem(lua_grob) then
               text_editor.text = scene_graph.resolve(lua_grob).source
            end
         end
         imgui.EndMenu()
      end
      if imgui.BeginMenu('Send Lua program to...') then
         lua_grobs = gom.get_environment_value('OGF::LuaGrob_instances')
         for lua_grob in string.split(lua_grobs,';') do
            if imgui.MenuItem(lua_grob) then
               scene_graph.resolve(lua_grob).source = text_editor.text
            end
         end
         imgui.EndMenu()
      end
      if imgui.BeginMenu('Get Lua shader from...') then
         lua_grobs = gom.get_environment_value('OGF::LuaGrob_instances')
         for lua_grob in string.split(lua_grobs,';') do
            if imgui.MenuItem(lua_grob) then
               text_editor.text = scene_graph.resolve(lua_grob).shader_source
            end
         end
         imgui.EndMenu()
      end
      if imgui.BeginMenu('Send Lua shader to...') then
         lua_grobs = gom.get_environment_value('OGF::LuaGrob_instances')
         for lua_grob in string.split(lua_grobs,';') do
            if imgui.MenuItem(lua_grob) then
               scene_graph.resolve(lua_grob).shader_source = text_editor.text
            end
         end
         imgui.EndMenu()
      end
      imgui.EndMenu()
    end

    if imgui.BeginMenu('MeshGrob') then
      if imgui.BeginMenu('New Mesh object') then
         local sel
         local name = ''
         imgui.Text('Name')
         imgui.PushItemWidth(main.scaling()*100)
         sel,name = imgui.TextInput(
            '##NewMeshGrob',
   	    name,
	    ImGuiInputTextFlags_EnterReturnsTrue
         )
	 imgui.PopItemWidth()
	 if sel then
	   local obj=scene_graph.create_object('OGF::MeshGrob')
	   obj.query_interface('OGF::MeshGrobShapesCommands').create_square()
	   obj.rename(name)
	 end
	 imgui.EndMenu()
      end
      if imgui.BeginMenu('Get GLSL from...') then
         mesh_grobs = gom.get_environment_value('OGF::MeshGrob_instances')
         for mesh_grob in string.split(mesh_grobs,';') do
            if imgui.MenuItem(mesh_grob) then
               text_editor.text =
	           scene_graph.resolve(mesh_grob).shader.glsl_source
            end
         end
         imgui.EndMenu()
      end
      if imgui.BeginMenu('Send GLSL to...') then
         mesh_grobs = gom.get_environment_value('OGF::MeshGrob_instances')
         for mesh_grob in string.split(mesh_grobs,';') do
            if imgui.MenuItem(mesh_grob) then
               scene_graph.resolve(mesh_grob).shader.glsl_source
	          = text_editor.text
            end
         end
         imgui.EndMenu()
      end
      imgui.EndMenu()
    end
    imgui.EndMenu()
   end
   if imgui.BeginMenu('Examples...') then
      local path =
          gom.get_environment_value('PROJECT_ROOT')..
	     '/lib/programming_examples'
      text_editor_gui.browse_by_language(path)
      for plugin in string.split(gom.get_environment_value(
         'loaded_dynamic_modules'),';'
      ) do
         local path1 = gom.get_environment_value('PROJECT_ROOT')..
	    '/plugins/OGF/'..plugin..'/programming_examples'
         local path2 = gom.get_environment_value('PROJECT_ROOT')..
	    '/lib/'..plugin..'/programming_examples'
	 if FileSystem.is_directory(path1) then
            if imgui.BeginMenu(plugin) then
     	       text_editor_gui.browse_by_language(path1)
	       imgui.EndMenu()
	    end
	 elseif FileSystem.is_directory(path2) then
            if imgui.BeginMenu(plugin) then
     	       text_editor_gui.browse_by_language(path2)
	       imgui.EndMenu()
	    end
         end
      end
      imgui.EndMenu()
   end
   local w = imgui.GetContentRegionAvail()-20*main.scaling()
   if w > 0 then
      imgui.Dummy(w,1.0)
      if object_properties_gui.visible then
         if imgui.SimpleButton(imgui.font_icon('angle-double-right')) then
             camera_gui.visible=false
             object_properties_gui.visible=false
             toolbox_gui.visible=false
         end
	 autogui.tooltip('hide right pane')
      else
         if imgui.SimpleButton(imgui.font_icon('angle-double-left')) then
             camera_gui.visible=true
             object_properties_gui.visible=true
             toolbox_gui.visible=true
         end
	 autogui.tooltip('show right pane')
      end
   end
end

function text_editor_gui.find_next()
   text_editor.cursor_forward()
   text_editor.find(text_editor_gui.find_word)
end

function text_editor_gui.show_find()
   if text_editor_gui.find_visible then
      text_editor_gui.find_next()
   else
      text_editor_gui.find_word = ''
      text_editor_gui.find_visible = true
      text_editor_gui.find_focus = true
   end
end

function text_editor_gui.hide_find()
   text_editor.clear_breakpoints()
   text_editor_gui.find_visible = false
end

function text_editor_gui.draw_window()
   text_editor_gui.parse_errors()

   if imgui.SimpleButton(
      imgui.font_icon('play-circle')
   ) then
      text_editor_gui.run()
   end
   autogui.tooltip('Run program <F5>')
   imgui.SameLine()
   if imgui.SimpleButton(
      imgui.font_icon('stop-circle')
   ) then
      text_editor_gui.stop()
   end
   autogui.tooltip('Stop program <Ctrl><C>')
   imgui.SameLine()
   if imgui.SimpleButton(
     imgui.font_icon('sync-alt')
   ) then
      text_editor_gui.reload()
   end
   autogui.tooltip('Reload file')
   imgui.SameLine()
   imgui.Text('   ')
   imgui.SameLine()
   if text_editor_gui.filename == nil then
      imgui.Text('<newfile>'..'.'..text_editor.language)
   else
      imgui.Text(FileSystem.base_name(text_editor_gui.filename,false))
      autogui.tooltip(text_editor_gui.filename)
   end
   if text_editor_gui.find_visible then
     imgui.SameLine()
     imgui.Text('   '..imgui.font_icon('search'))
     imgui.SameLine()
     local sel
     imgui.PushItemWidth(-70.0*main.scaling())
     if text_editor_gui.find_focus then
	imgui.SetKeyboardFocusHere()
	text_editor_gui.find_focus = false
     end
     sel,text_editor_gui.find_word = imgui.TextInput(
	'##TextEditorFind',
	text_editor_gui.find_word,
        0
     )
     imgui.PopItemWidth()
     if(sel) then
        text_editor.find(text_editor_gui.find_word)
     end
     imgui.SameLine()
     if imgui.Button('Next') then
        text_editor_gui.find_next()
     end
     imgui.SameLine()
     if imgui.Button(
        imgui.font_icon('window-close')
     ) then
        text_editor_gui.hide_find()
     end
   end
   imgui.Separator()
   text_editor.draw('Lua Editor##area')
end

function text_editor_gui.draw_extra()
  local sel
  local filename=''

  sel,filename = imgui.FileDialog('##text_editor##load_dlg',filename)
  if sel then
     text_editor_gui.filename = filename
     text_editor_gui.set_language(FileSystem.extension(filename))
     text_editor_gui.load(filename)
  end

  local sel,filename = imgui.FileDialog('##text_editor##save_dlg',filename)
  if sel then
     text_editor_gui.filename = filename
     text_editor_gui.save(filename)
  end
end

function text_editor_gui.Text_title(str)
   imgui.PushFont(2)
   if(main.get_style() == 'Light') then
      imgui.PushStyleColor_2(0, 0.0, 0.0, 0.0, 1.0)
   else
      imgui.PushStyleColor_2(0, 1.0, 1.0, 1.0, 1.0)
   end
   imgui.Text(str)
   imgui.PopStyleColor()
   imgui.PopFont()
end

function text_editor_gui.Text_keyword(str)
   imgui.PushFont(0)
   if(main.get_style() == 'Light') then
      imgui.PushStyleColor_2(0, 0.0, 0.0, 0.0, 1.0)
   else
      imgui.PushStyleColor_2(0, 1.0, 1.0, 1.0, 1.0)
   end
   imgui.Text(str)
   imgui.PopStyleColor()
   imgui.PopFont()
end

function text_editor_gui.Text_section(str)
   imgui.PushFont(2)
   if(main.get_style() == 'Light') then
       imgui.PushStyleColor_2(0, 0.5, 0.5, 0.5, 1.0)
   else
       imgui.PushStyleColor_2(0, 0.7, 0.7, 0.7, 1.0)
   end
   imgui.Text(str)
   imgui.PopStyleColor()
   imgui.PopFont()
end

function text_editor_gui.Text_comment(str)
   imgui.PushFont(0)
   if(main.get_style() == 'Light') then
       imgui.PushStyleColor_2(0, 0.4, 0.4, 0.4, 1.0)
   else
       imgui.PushStyleColor_2(0, 0.6, 0.6, 0.6, 1.0)
   end
   imgui.Text(str)
   imgui.PopStyleColor()
   imgui.PopFont()
end

function text_editor_gui.Text(str)
   imgui.PushFont(0)
   if(main.get_style() == 'Light') then
      imgui.PushStyleColor_2(0, 0.0, 0.0, 0.5, 1.0)
   else
      imgui.PushStyleColor_2(0, 0.85, 0.85, 1.0, 1.0)
   end
   imgui.Text(str)
   imgui.PopStyleColor()
   imgui.PopFont()
end

function text_editor_gui.show_help(minfo)
  if minfo.has_custom_attribute('help') then
     local str = minfo.custom_attribute_value('help')
     text_editor_gui.Text_comment('  '..str)
  end
end

function text_editor_gui.tooltip(args)
  local interp = gom.interpreter_by_file_extension(
     text_editor.language
  )
  if interp == nil then
     return
  end
  -- Note: gom.connect-ed functions receive their
  -- arguments in a table (thus args.context to
  -- get the context)
  local obj = interp.eval_object(args.context,true)
  if obj == nil then
    local strval = interp.eval_string(args.context)
    if strval ~= '' then
       if imgui.BeginTooltip() then
          if strval ~= args.context and
             strval ~= '\''..args.context..'\'' then
	        text_editor_gui.Text(args.context .. '=' .. strval)
          else
	        text_editor_gui.Text(strval)
          end
          imgui.EndTooltip()
       end
    end
    return
  end
  local mclass = obj.meta_class
  if imgui.BeginTooltip() then
     if mclass.name == 'OGF::Request' then
        local rq_obj = obj.object()
        local mmethod = obj.method()
        text_editor_gui.Text_title(
          rq_obj.meta_class.name .. '::' .. mmethod.name .. '()'
        )
        text_editor_gui.show_help(mmethod)
        if mmethod.nb_args() ~= 0 then
           imgui.Separator()
        end
        for i=0,mmethod.nb_args()-1 do
           text_editor_gui.Text('  '..mmethod.ith_arg_name(i))
	   imgui.SameLine()
	   text_editor_gui.Text_keyword(' : ' .. mmethod.ith_arg_type_name(i))
	   if mmethod.ith_arg_has_custom_attribute(i, 'help') then
              text_editor_gui.Text_comment(
   	         '    '..mmethod.ith_arg_custom_attribute_value(i,'help')
	      )
	   end
        end
     elseif obj.is_a(gom.meta_types.OGF.Callable) then
        imgui.Text('builtin or function')
     else
        text_editor_gui.Text_title(mclass.name)
        text_editor_gui.show_help(mclass)
        imgui.Separator()
        imgui.BeginGroup()
        text_editor_gui.Text_section('gom_slots')
        for i=0,mclass.nb_slots()-1 do
           text_editor_gui.Text('  '..mclass.ith_slot(i).name..'()')
        end
        imgui.EndGroup()
        imgui.SameLine()
        imgui.Text('  ')
        imgui.SameLine()
        imgui.BeginGroup()
        text_editor_gui.Text_section('gom_properties')
        for i=0,mclass.nb_properties()-1 do
           local propname = mclass.ith_property(i).name
	   local proptype = mclass.ith_property(i).type_name()
	   local propval
	   if proptype:sub(-1) == '*' then
	      propval = '<pointer>'
	   elseif proptype == 'std::string' then
	      propval = tostring(obj[propname])
	      if propval == '' then
                 propval = '<emptystring>'
	      else
                 propval = '\''..propval..'\''
	      end
	   else
	      propval = tostring(obj[propname])
	   end
           text_editor_gui.Text('  '..propname)
	   imgui.SameLine()
	   text_editor_gui.Text_keyword(' : '..proptype)
	   imgui.SameLine()
	   text_editor_gui.Text_comment(' = '..propval)
        end
        imgui.EndGroup()
     end
     imgui.EndTooltip()
  end
end

graphite_main_window.add_module(text_editor_gui)

gom.connect(text_editor.run_request,  text_editor_gui.run)
gom.connect(text_editor.save_request, text_editor_gui.save)
gom.connect(text_editor.stop_request, text_editor_gui.stop)
gom.connect(text_editor.find_request, text_editor_gui.show_find)
gom.connect(text_editor.tooltip_request, text_editor_gui.tooltip)
