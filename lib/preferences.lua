--  GUI for the preferences window of Graphite, for use with Skin_imgui
-----------------------------------------------------------------------

preferences_window = {}
preferences_window.visible = false
preferences_window.selected = {}
preferences_window.edit = {}

-- \brief Edits a list of values
-- \param[in] label an ImGui label
-- \param[in] vals_string a ';'-separated list of values
-- \return sel,new_vals_string, where
--    sel is true if the list changed
--    new_vals_string is the new ';'-separated list of values
--    extension optional file extension

function preferences_window.edit_list(label, vals_string, extension)
   local result = vals_string
   imgui.PushID(label)
   local vals = {}
   for s in string.split(vals_string, ';') do
      table.insert(vals,s)
   end
   imgui.PushItemWidth(-1)
   imgui.BeginListBox(label, #vals)
   for i = 1,#vals do
      if imgui.Selectable(
            vals[i],
	    preferences_window.selected[label] == vals[i]
      ) then
         preferences_window.selected[label] = vals[i]
      end
   end
   imgui.EndListBox()
   imgui.PopItemWidth()
   local w = imgui.GetWindowWidth()/2 - main.scaling()*10
   local rem_selected_lbl = 'Remove selected'
   local clear_lbl = 'Clear'
   if w/main.scaling() < 100 then
      rem_selected_lbl = 'Remove\nselected'
      clear_lbl = 'Clear\n '
   end
   if imgui.Button(rem_selected_lbl,w,0) then
       local selected = preferences_window.selected[label]
       if selected ~= nil then
          result = string.gsub(result, ';'..selected..';', ';')
          result = string.gsub(result, '^'..selected..';', '')
          result = string.gsub(result, ';'..selected..'$', '')
	  if result == selected then
	     result = ''
	  end
       end
       preferences_window.selected[label] = nil
   end
   imgui.SameLine()
   if imgui.Button(clear_lbl,w,0) then
       result = ''
       preferences_window.selected[label] = nil
   end
   if imgui.Button('Add') then
      if preferences_window.edit[label] ~= nil then
         if result ~= '' then
	    result = result..';'
	 end
	 result = result..preferences_window.edit[label]
	 preferences_window.edit[label] = ''
      end
   end
   if preferences_window.edit[label] == nil then
       preferences_window.edit[label] = ''
   end
   if extension ~= nil then
      imgui.SameLine()
      if imgui.Button(
         imgui.font_icon('folder-open')
      ) then
    	imgui.OpenFileDialog(
	    '##edit_file_dlg',
	    'lua',
            preferences_window.edit[label],
	    ImGuiExtFileDialogFlags_Load
	)
      end
   end
   imgui.SameLine()

   _,preferences_window.edit[label] =
       imgui.FileDialog('##edit_file_dlg',preferences_window.edit[label])

   imgui.PushItemWidth(-1)
   _,preferences_window.edit[label] =
       imgui.TextInput('##edit',preferences_window.edit[label]
   )
   imgui.PopItemWidth()

   imgui.PopID(label)
   return result ~= vals_string, result
end

-- \brief Edits a boolean preference
-- \param[in] label the label to be displayed
-- \param[in] name the name fo the preferences variable

function preferences_window.edit_preference_boolean(label, name)
   local sel
   local value = gom.get_environment_value(name)
   value = (value == 'true')
   sel,value = imgui.Checkbox(label,value)
   gom.set_environment_value(name,tostring(value))
   return sel
end


-- \brief Loads preferences from a file
-- \param[in] name the name of the file
-- \details Loads preferenes from lib/'name'.ini

function preferences_window.load_preferences(name)
   if name == 'user' then
      main.load_preferences(
         gom.get_environment_value('HOME_DIRECTORY')..'/graphite.ini'
      )
   else
      main.load_preferences(
         gom.get_environment_value('PROJECT_ROOT')..'/lib/'..name..'.ini'
      )
   end
   for _,module in ipairs(graphite_main_window.modules_by_index) do
      module.visible = (
         gom.get_environment_value(
           'gui:module_'..module.name..'_visible'
         ) == 'true'
      )
   end
end

-- \brief Activates the display of the preferences window

function preferences_window.show()
   preferences_window.visible = true
end

-- \brief Draws the 'Plugins' pane

function preferences_window.draw_Plugins()
   imgui.Text('\n')
   local sel
   local modules = gom.get_environment_value('modules')
   sel,modules = preferences_window.edit_list("##modules",modules)
   if sel then
      gom.set_environment_value('modules',modules)
   end
end

-- \brief Draws the 'AppearanceRendering' pane

preferences_window['draw_Appearance and rendering'] = function()
   imgui.Text('\n')
   imgui.Separator()
   imgui.Text('   Appearance')
   local sel
   local style = gom.get_environment_value('gui:style')
   imgui.Text('Style')
   imgui.SameLine()
   imgui.PushItemWidth(-1)
   sel,style = autogui.combo_box_value(
	       '##style',
	        style,
		gom.get_environment_value('gui_styles')
	     )
   imgui.PopItemWidth()
   if sel then
      gom.set_environment_value('gui:style',style)
      main.set_style(style)
   end

   imgui.Text('Font size')
   imgui.SameLine()
   local font_size = gom.get_environment_value('gui:font_size')
   imgui.PushItemWidth(-1)
   sel,font_size = imgui.InputInt('##font_size',font_size)
   imgui.PopItemWidth()
   if sel then
      main.set_font_size(font_size)
   end
   preferences_window.edit_preference_boolean(
       'Tooltips', 'gui:tooltips'
   )
   imgui.SameLine()
   preferences_window.edit_preference_boolean(
       'Shaders selector', 'gui:shaders_selector'
   )
   preferences_window.edit_preference_boolean(
       'Maximize window at startup', 'gfx:full_screen'
   )
   if imgui.Button('Restore default layout',-1,0) then
      preferences_window.load_preferences('default_layout')
   end

   imgui.Text('\n')
   imgui.Separator()
   imgui.Text('   Rendering')
   imgui.Text('Default effect')
   imgui.SameLine()
   local effect = gom.get_environment_value('gfx:default_full_screen_effect')
   imgui.PushItemWidth(-1)
   sel,effect = autogui.combo_box_value(
                     '##effect',
		     effect,
		     gom.get_environment_value('full_screen_effects')
                )
   imgui.PopItemWidth()
   if sel then
      main.camera().effect = effect
   end
   gom.set_environment_value('gfx:default_full_screen_effect', effect)

   preferences_window.edit_preference_boolean(
       'Enable undo (saves state before each command)', 'gui:undo'
   )

   if gom.get_environment_value('gui:undo') == 'true' then
      local undo_depth = tonumber(gom.get_environment_value('gui:undo_depth'))
      if undo_depth == nil then
         undo_depth = 4
      end
      imgui.Text('Undo depth')
      imgui.SameLine()
      local sel
      imgui.PushItemWidth(-1)
      sel,undo_depth = imgui.InputInt('##undo_depth',undo_depth)
      imgui.PopItemWidth()
      if sel then
         gom.set_environment_value('gui:undo_depth',undo_depth)
      end
   end

end

-- \brief Draws the 'Startup' pane

function preferences_window.draw_Startup()
   imgui.Text('\n')
   imgui.Text('User Lua files to load at startup.')
   local sel
   local gel_startup_files = gom.get_environment_value('gel:startup_files')
   sel,gel_startup_files =
          preferences_window.edit_list("##startup",gel_startup_files,'lua')
   if sel then
      gom.set_environment_value('gel:startup_files',gel_startup_files)
   end
end

-- \brief Draws the 'Logger' pane

function preferences_window.draw_Logger()
   local footer_size = 80
   local w = imgui.GetWindowWidth() * 0.5 - 12*main.scaling()
   local sel,include,exclude
   imgui.PushID('##Logger_prefs')
   imgui.BeginChild(
      '##include',
      w, -footer_size, true
   )
   imgui.Text('features to log')
   include = gom.get_environment_value('log:features')
   sel,include = preferences_window.edit_list("##include",include)
   if sel then
      gom.set_environment_value('log:features',include)
   end
   imgui.EndChild()
   imgui.SameLine()
   imgui.BeginChild(
      '##exclude',
      w, -footer_size, true
   )
   imgui.Text('features to exclude')
   exclude = gom.get_environment_value('log:features_exclude')
   sel,exclude = preferences_window.edit_list("##exclude",exclude)
   if sel then
      gom.set_environment_value('log:features_exclude',exclude)
   end
   imgui.EndChild()
   imgui.PopID()
end

-- \brief Draws the 'FnKeys' pane

function preferences_window.draw_FnKeys()
   for i=1,10 do
      local var_name = "gel:F" .. tostring(i) .. "_script"
      imgui.PushID(var_name)
      preferences_window.edit[var_name] = gom.get_environment_value(var_name)
      if i < 10 then
         imgui.Text('F'..tostring(i)..'  ')
      else
         imgui.Text('F'..tostring(i))
      end
      imgui.SameLine()
      if imgui.Button(
         imgui.font_icon('folder-open')
      ) then
    	imgui.OpenFileDialog(
	    '##'..var_name..'##edit_file_dlg',
	    'lua',
            preferences_window.edit[var_name],
	    ImGuiExtFileDialogFlags_Load
	)
      end
      imgui.SameLine()
      _,preferences_window.edit[var_name] =
         imgui.FileDialog(
	    '##'..var_name..'##edit_file_dlg',
	    preferences_window.edit[var_name]
	 )
      imgui.PushItemWidth(-1)
      _,preferences_window.edit[var_name] =
         imgui.TextInput('##'..var_name,preferences_window.edit[var_name]
      )
      imgui.PopItemWidth()
      gom.set_environment_value(var_name,preferences_window.edit[var_name])
      imgui.PopID()
   end
end

-- \brief Draws the 'Advanced' pane

function preferences_window.draw_Advanced()
   imgui.Text('\n')
   imgui.Separator()
   imgui.Text('   Advanced GUI parameters')
   preferences_window.edit_preference_boolean(
       'Keyboard navigation', 'gui:keyboard_nav'
   )
   preferences_window.edit_preference_boolean(
       'Dockable dialogs', 'gui:viewports'
   )
   imgui.Text('\n')
   imgui.Separator()
   imgui.Text('   Debugging and performance tests')
   preferences_window.edit_preference_boolean(
       'Floating point exceptions', 'sys:FPE'
   )
   preferences_window.edit_preference_boolean(
       'OpenGL exceptions', 'gfx:GL_debug'
   )

   local nb_cores = tonumber(gom.get_environment_value('nb_cores'))

   if preferences_window.edit_preference_boolean(
       'Multithreading', 'sys:multithread'
   ) then
      if gom.get_environment_value('sys:multithread') == 'true' then
         gom.set_environment_value('sys:max_threads', nb_cores)
      end
   end

   local max_threads = tonumber(gom.get_environment_value('sys:max_threads'))
   if max_threads == nil then
      max_threads = tonumber(gom.get_environment_value('nb_cores'))
   end
   imgui.Text('Max threads')
   imgui.SameLine()
   local sel
   imgui.PushItemWidth(-1)
   sel,max_threads = imgui.InputInt('##max_threads',max_threads)
   imgui.PopItemWidth()
   if sel then
      gom.set_environment_value('sys:max_threads',max_threads)
   end
   if FileSystem.os_name() == 'Windows' then
      preferences_window.edit_preference_boolean(
          'Win32 console', 'sys:show_win32_console'
      )
   end
   imgui.Text('\n')
   imgui.Separator()
   imgui.Text('   OpenGL fine tuning')

   --[[
   imgui.Text('OpenGL profile')
   local sel
   local GL_profile = gom.get_environment_value('gfx:GL_profile')
   imgui.SameLine()
   sel,GL_profile = autogui.combo_box_value(
	'##GL_profile',
	GL_profile,
	'core;ES'
   )
   if sel then
      gom.set_environment_value('gfx:GL_profile', GL_profile)
   end
   --]]

   imgui.Text('GLUP profile')
   local GLUP_profile = gom.get_environment_value('gfx:GLUP_profile')
   imgui.SameLine()
   imgui.PushItemWidth(-1)
   sel,GLUP_profile = autogui.combo_box_value(
	'##GLUP_profile',
	GLUP_profile,
	'GLUPES2;GLUP150;GLUP440;auto'
   )
   imgui.PopItemWidth()
   if sel then
      gom.set_environment_value('gfx:GLUP_profile', GLUP_profile)
   end

   imgui.Text('Adapter')
   local gfx_adapter = gom.get_environment_value('gfx:adapter')
   imgui.SameLine()
   imgui.PushItemWidth(-1)
   sel,gfx_adapter = autogui.combo_box_value(
	'##gfx_adapter',
	gfx_adapter,
	'auto;nvidia;intel'
   )
   imgui.PopItemWidth()
   if sel then
      gom.set_environment_value('gfx:adapter', gfx_adapter)
   end

   preferences_window.edit_preference_boolean(
       'polygon offset', 'gfx:polygon_offset'
   )

end

function preferences_window.draw()
   if not main.preferences_loaded() then
      preferences_window.load_preferences('default_layout')
   end
   imgui.PushID('preferences')
   if preferences_window.visible then

      imgui.SetNextWindowSize(
         350*main.scaling(),
         400*main.scaling(),
         ImGuiCond_FirstUseEver
      )

      _,preferences_window.visible = imgui.Begin(
         imgui.font_icon('cogs')..'  Preferences',
	 preferences_window.visible,
	 ImGuiWindowFlags_NoDocking
      )

      if preferences_window.visible then

         imgui.BeginTabBar('##tabs')
	 for i in string.split(
	    'Appearance and rendering;Plugins;Startup;Logger;FnKeys;Advanced',
	    ';'
	 ) do
	    if imgui.BeginTabItem(i) then
	        preferences_window['draw_'..i]()
		imgui.EndTabItem()
	    end
	 end
	 imgui.EndTabBar()


	 local h
	 _,h = imgui.GetContentRegionAvail()
	 imgui.Dummy(-1,h-80*main.scaling())
	 imgui.Separator()
	 imgui.Text('To take all changes into account,')
	 imgui.Text('Save configuration and restart Graphite')
	 if imgui.Button(
	    imgui.font_icon('save')..' Save config.',
	    -imgui.GetContentRegionAvail()/2,-1
	 ) then
	    main.save_preferences()
	 end
	 imgui.SameLine()
	 if imgui.Button(
	    imgui.font_icon('sync-alt')..
	    '  Reload config.',-1,-1
	 ) then
	    preferences_window.load_preferences('user')
	 end
      end
      imgui.End()
   end
   imgui.PopID()
end

-- \brief The callback called whenever a function key is pressed.
-- \param[in] i the index of the key Fi that was pressed.

function preferences_window.function_key(i)
   local script_name = "gel:F" .. i .. "_script"
   local script = gom.get_environment_value(script_name)
   if script ~= "" then
      gom.execute_file(script)
   end
end

-- \brief Binds the function keys to the callback

function preferences_window.bind_function_keys()
   for i = 1,10,1 do
      gom.connect(main.render_area.key_down,gom.execute)
        .if_arg("value","F" .. i)
        .add_arg("command","preferences_window.function_key(" .. i .. ")")
	.add_arg("save_in_history","false")
	.add_arg("log","false")
   end
end
