-- ------------------------------------------------------------------------------
-- \file autogui_commands.lua
-- \brief creates dialogs for calling commands and functions
-- ------------------------------------------------------------------------------

-- \brief Handles the dialog for an object command
-- \param[in] request the request, for instance grob.I.Surface.remesh_smoooth

function autogui.command_dialog(request)
  local mmethod = request.method()
  local k = autogui.request_key(request)
  if autogui.command_state[k] == nil then
     autogui.command_state[k] = autogui.init_args(mmethod)
  end
  if autogui.in_popup then
     autogui.command_state[k].show_as_window_ = false
  end
  autogui.command_state[k].request_ = request

  if(command_gui.visible) then
     imgui.TextDisabled(autogui.remove_underscores(mmethod.name))
     autogui.tooltip(autogui.help(mmethod))
     imgui.BeginChild(
        'command_args',
        0.0,  -30*main.scaling(),
        false,
        ImGuiWindowFlags_HorizontalScrollbar
     )
  end

  -- Target selector

  local apply_to_selection = false
  if command_gui.visible and request.object().is_a(OGF.Interface) then
      apply_to_selection = request.object().grob.selected
  end

  if (
     autogui.command_state[k].show_as_window_ or
     command_gui.visible
  ) and request.object().is_a(OGF.Interface) then
     local grob = request.object().grob
     if not grob.is_a(OGF.SceneGraph) then
        if apply_to_selection then
           imgui.Text('Target:')
           imgui.SameLine()
           imgui.TextDisabled(
             tostring(scene_graph.I.Selection.nb_selected())..
             ' selected object(s)'
           )
        else
           autogui.command_state[k].target_ = grob.name
           grob_names = gom.get_environment_value(
              grob.meta_class.name..'_instances'
           )
           if autogui.combo_box(
               autogui.command_state[k], 'target_', grob_names
           ) then
               grob = scene_graph.objects[autogui.command_state[k].target_]
               request.object().grob = grob
           end
           autogui.tooltip('Change command target')
        end
     end
  else
     if imgui.MenuItem( -- The button to 'windowify' command
        imgui.font_icon('code-branch')..'  '..
        autogui.remove_underscores(mmethod.name)..'##command_apply'
     ) then
        autogui.command_state[k].show_as_window_ = true
     end
     autogui.tooltip(autogui.help(mmethod))
  end
  imgui.Separator()
  autogui.command_dialog_args(request)

  if(command_gui.visible) then
     imgui.EndChild()
  end

  imgui.Separator()
  autogui.command_dialog_apply_buttons(request)

  if autogui.command_state[k] ~= nil then
     autogui.command_state[k].width_,
     autogui.command_state[k].height_ = imgui.GetWindowSize()
     -- increase height a little bit so that resizing corner
     -- does not 'eat' too much and scrollbar does not appear.
     autogui.command_state[k].height_ = autogui.command_state[k].height_ + 5

     autogui.command_state[k].x_,
     autogui.command_state[k].y_ = imgui.GetWindowPos()
  end
end

-- \brief Handles the menu item for an object command
-- \param[in] request the request, for instance grob.I.Surface.remesh_smooth

function autogui.command_menu_item(request)
   local commands = request.object()
   local mmethod = request.method()

   -- Special case, using command docking area
   -- (except when nb_args == 0, in which case we'll invoke cmd directly)
   if command_gui.visible and mmethod.nb_args() ~= 0 then
      if imgui.MenuItem(
          autogui.remove_underscores(mmethod.name)..'##'..mmethod.name
      ) then
         command_gui.request = request
      end
      autogui.tooltip(autogui.help(mmethod))
      return
   end

   if mmethod.nb_args() == 0 then -- invoke command directly when item selected
      -- Need to add '##' suffix with slot name with underscores
      -- else ImGui generates a shortcut I think (to be understood)
      if imgui.MenuItem(
          autogui.remove_underscores(mmethod.name)..'##'..mmethod.name
      ) then
         main.exec_command(
             gom.back_resolve(commands)..'.'..mmethod.name..
             '({_invoked_from_gui=true})', false
          )
      end
      local tt = autogui.help(mmethod)
      if tt == nil then tt = '' else tt = tt..'\n  ' end
      tt = tt..'Applies immediately (no dialog box)'
      autogui.tooltip(tt)
   else
      if imgui.BeginMenu(autogui.remove_underscores(mmethod.name)) then
          autogui.in_popup = true
          autogui.command_dialog(request)
	  autogui.in_popup = false
          imgui.EndMenu()
      end
   end
end

-- \brief Programmatically open a dialog for a command
-- \param[in] target a Request, for instance grob.I.Surface.remesh_smooth
-- \param[in] args optional dictionnary with default values for the arguments

function autogui.open_command_dialog(request,args)
  local k = autogui.request_key(request)
  if autogui.command_state[k] == nil then
     autogui.command_state[k] = autogui.init_args(request.method())
     if command_gui.visible then
        autogui.command_state[k].show_as_window_ = false
        command_gui.request = request
     else
        autogui.command_state[k].show_as_window_ = true
     end
     autogui.command_state[k].width_  = 250*main.scaling()
     autogui.command_state[k].height_ = 250*main.scaling()
     autogui.command_state[k].x_ = 400
     autogui.command_state[k].y_ = 200
     if args ~= nil then
        for ak,av in pairs(args) do
	   autogui.command_state[k][ak] = av
	end
     end
  end
  autogui.command_state[k].request_ = request
end

-- \brief Closes all command dialogs

function autogui.close_command_dialogs()
   autogui.command_state = {}
   command_gui.request = nil
end

-- \brief Draws all the command dialogs that are in windowed mode.

function autogui.command_dialogs()
   for k,cmd_state in pairs(autogui.command_state) do
      -- Hide window / delete command state if target object was deleted
      if not autogui.command_is_alive(autogui.command_state[k]) then
         autogui.command_state[k] = nil
      elseif autogui.command_state[k].show_as_window_ then
      	 imgui.SetNextWindowSize(
	    cmd_state.width_  +  4.0*main.scaling(),
	    cmd_state.height_ + 40.0*main.scaling(),
	    ImGuiCond_Appearing
	 )
	 imgui.SetNextWindowPos(cmd_state.x_, cmd_state.y_, ImGuiCond_Appearing)
         _,autogui.command_state[k].show_as_window_ = imgui.Begin(
	     imgui.font_icon('code-branch')..'  '..
	     autogui.remove_underscores(
	        cmd_state.request_.method().name
	     ) .. '##dialog##' .. k,
             cmd_state.show_as_window_
         )
         autogui.command_dialog(cmd_state.request_)
         imgui.End()
      end
   end
end

-- ------------------------------------------------------------------------------
-- Implementation of command_dialog()
-- ------------------------------------------------------------------------------

-- \brief Part of the implementation of command_dialog()
-- \details Handles the gui for command arguments
-- \param[in] request the request, for instance grob.I.Surface.remesh_smooth

function autogui.command_dialog_args(request)
  local k = autogui.request_key(request)
  local mmethod = request.method()
  local has_advanced_args = false

  for i=0,mmethod.nb_args()-1 do
     if mmethod.ith_arg_has_custom_attribute(i,'advanced') and
        mmethod.ith_arg_custom_attribute_value(i,'advanced') == 'true' then
	has_advanced_args = true
     end
  end

  for i=0,mmethod.nb_args()-1 do
     if (not has_advanced_args or
         not mmethod.ith_arg_has_custom_attribute(i,'advanced') or
	 mmethod.ith_arg_custom_attribute_value(i,'advanced') == 'false'
     ) then
        autogui.slot_arg(autogui.command_state[k],mmethod,i)
     end
  end

  if has_advanced_args then
     if imgui.TreeNode('Advanced'..'##'..k..'##advanced') then
        imgui.TreePop()
	for i=0,mmethod.nb_args()-1 do
	   if (mmethod.ith_arg_has_custom_attribute(i,'advanced') and
      	       mmethod.ith_arg_custom_attribute_value(i,'advanced') == 'true'
	   ) then
	       autogui.slot_arg(autogui.command_state[k],mmethod,i)
	   end
	 end
     end
  end

end


-- \brief Part of the implementation of command_dialog()
-- \details Handles the gui for the apply buttons of a command dialog
-- \param[in] request the request, for instance grob.I.Surface.remesh_smooth

function autogui.command_dialog_apply_buttons(request)
  local k = autogui.request_key(request)
  local mmethod = request.method()

  -- Request factory settings -----------------------------------
  if imgui.Button(imgui.font_icon('cog')) then
      local bkp1 = autogui.command_state[k].show_as_window_
      autogui.command_state[k] = autogui.init_args(mmethod)
      autogui.command_state[k].show_as_window_ = bkp1
      autogui.command_state[k].request_ = request
  end
  autogui.tooltip('reset factory settings')
  imgui.SameLine()

  -- Apply command without closing ------------------------------
  local doit_recycle_button = imgui.Button(imgui.font_icon('sync-alt'))
  autogui.tooltip('apply command without closing dialog')
  imgui.SameLine()

  -- Apply command to selected objects --------------------------
  local doit_apply_to_sel_button = false
  if not command_gui.visible then
     doit_apply_to_sel_button = imgui.Button(
        imgui.font_icon('clipboard-list')
     )
     autogui.tooltip('apply command to selected objects')
     imgui.SameLine()
  end

  -- Apply command and close dialog ----------------------------
  local btn_width  = autogui.button_size
  local apply_btn_width = imgui.GetContentRegionAvail() - autogui.margin
  if command_gui.visible then
     apply_btn_width = apply_btn_width - btn_width - autogui.margin
  end
  local doit_apply_button = imgui.Button(
     imgui.font_icon('check')..'##commands',apply_btn_width,0
  )
  autogui.tooltip('apply command')

  local close_dialog = false
  if command_gui.visible then
     imgui.SameLine()
     close_dialog = imgui.Button(imgui.font_icon('window-close')..'##commands')
     autogui.tooltip('Close command')
  end

  -- Now execute command if one of the buttons was pushed
  if doit_apply_button or doit_recycle_button then
      -- Close the menu (if we were in menu mode).
      if not doit_recycle_button and
         not autogui.command_state[k].show_as_window_ and
         not command_gui.visible then
         imgui.CloseCurrentPopup()
      end

      -- If command is in menu and 'recycle' was pushed,
      -- we need to execute the command now else it closes the dialog
      local no_progress = doit_recycle_button and
                          not command_gui.visible and
                          not autogui.command_state[k].show_as_window_

      local apply_to_selection = false
      if request.object().is_a(OGF.Interface) then
         apply_to_selection = command_gui.visible and
                              request.object().grob.selected
      end

      autogui.run_command(request, no_progress, apply_to_selection)
  end

  if doit_apply_to_sel_button then   -- Apply command to selected objects
      -- If command is in menu and 'recycle' was pushed,
      -- we need to execute the command now else it closes the dialog
      local no_progress = not command_gui.visible and
                          not autogui.command_state[k].show_as_window_
      autogui.run_command(request, no_progress, true)
  end

  if (doit_apply_button and command_gui.visible) or close_dialog then
     autogui.command_state[k] = nil
     -- this one needs to be queued after the commands to be executed
     -- when running on selection else this tries to run a command
     -- from a locked commands object (but I don't know why)
     main.exec_command('command_gui.request = nil',false)
  end
end

-- \brief executes a command
-- \param[in] request the request, for instance grob.I.Surface.remesh_smooth
-- \param[in] no_progress do not show progress while command is running
--  (to be set when called directly from a menu, else this closes the menu)
-- \param[in] apply_to_selection apply the command to all selected objects
-- \param[in] k an optional command key (used internally)

function autogui.run_command(request, no_progress, apply_to_selection, k)
   if k == nil then
      k = autogui.request_key(request)
   end
   local mmethod = request.method()
   local args_string = autogui.args_to_string(mmethod,autogui.command_state[k])
   local object_as_string = gom.back_resolve(request.object())
   if apply_to_selection then
      local request_grob = request.object().grob
      for i=0,scene_graph.nb_children-1 do
         local grob = scene_graph.ith_child(i)
         if grob.selected and
            request_grob.meta_class.name == grob.meta_class.name then
             autogui.run_command(
                grob.I[request.object().meta_class.name][mmethod.name],
                no_progress, false, k
             )
         end
      end
   else
      if no_progress then
         -- If command is in menu and 'recycle' was pushed,
         -- we need to execute the command now else it closes
         -- the dialog
         main.exec_command_now(
            object_as_string..'.'..mmethod.name..'('..args_string..')', false
         )
      else
         -- We queue the LUA command, this allows the command to display
         -- progress bars
         main.exec_command(
            object_as_string..'.'..mmethod.name..'('..args_string..')', false
         )
      end
   end
end

-- ------------------------------------------------------------------------------

-- \brief Keeps the state (arg lists) of the invoked commands
-- \details Command state is indexed by 'instance_name.classname.method' strings.

autogui.command_state  = {}

-- ------------------------------------------------------------------------------
-- Utilities for managing windowed commands states
-- ------------------------------------------------------------------------------

-- \brief Gets the key used to handle dialogs for requests
-- \param[in] request a request
-- \return a unique key associated with the request

function autogui.request_key(request)
    local method_name = request.method().name
    local object = request.object()
    if object.is_a(OGF.Grob) then
       return 'rq##'..object.name..'##'..method_name
    end
    -- using meta_class.is_subclass_of rather than is_a so that
    -- it also works with command classes implemented in Lua or Python
    if object.meta_class.is_subclass_of(OGF.Interface) then
       local interface_name = object.meta_class.name
       local grob_name = object.grob.name
       return 'rq##'..interface_name..'##'..method_name
    end
    -- fallback, normally does not get there, bad to use
    print('Warning: no request_key for '..object.meta_class.name)
    return request.string_id
end


-- \brief Tests whether a command is still "alive"
-- \details A command is not "alive" if the object it refers was destroyed

function autogui.command_is_alive(cmd_state)
   if cmd_state.request_.object().is_a(OGF.Interface) then
      local grob = cmd_state.request_.object().grob
      if not grob.is_a(OGF.SceneGraph) and
         not scene_graph.is_bound(grob.name) then
         return false
      end
   end
   return true
end

-- ------------------------------------------------------------------------------
-- Low level management of arguments and arglists
-- ------------------------------------------------------------------------------

-- \brief edits a command argument
-- \param[in] args a map with the arguments
-- \param[in] mslot the meta-slot that corresponds to the command
-- \param[in] i the index of the argument

function autogui.slot_arg(args,mslot,i)
   local mtype = mslot.ith_arg_type(i)
   local handler = autogui.handler_by_meta_type(mtype)
   if mslot.ith_arg_has_custom_attribute(i,'handler') then
      local handler_name = mslot.ith_arg_custom_attribute_value(i,'handler')
      handler = autogui.handler_by_name(handler_name)
   end
   local tooltip
   if mslot.ith_arg_has_custom_attribute(i,'help') then
      tooltip = mslot.ith_arg_custom_attribute_value(i,'help')
   end
   handler(args,mslot.ith_arg_name(i),mtype,tooltip)
end

-- \brief Initializes a LUA table with the default values of the parameters
--  of a meta-method
-- \param[in] mmethod the meta-method
-- \return args a table indexed by argument names and initialized with the
--  default values.

function autogui.init_args(mmethod)
   local args = {}
   for i=0,mmethod.nb_args()-1 do
      local val
      if mmethod.ith_arg_has_default_value(i) then
         val = mmethod.ith_arg_default_value_as_string(i)
      else
         mtype_name = mmethod.ith_arg_type_name(i)
         val = autogui.default_value[mtype_name]
	 if val == nil then
	    mtype = gom.resolve_meta_type(mtype_name)
	    if mtype.is_a(OGF.MetaEnum) then
	       val = mtype.ith_name(0)
	    else
	       val = ''
	    end
	 end
      end
      args[mmethod.ith_arg_name(i)] = val
   end
   return args
end

-- \brief Default values of parameters indexed by type name, used by init_args()

autogui.default_value = {}
autogui.default_value['bool']         = 'false'
autogui.default_value['float']        = '0.0'
autogui.default_value['double']       = '0.0'
autogui.default_value['unsigned int'] = '0'
autogui.default_value['int']          = '0'
autogui.default_value['OGF::index_t'] = '0'
autogui.default_value['GEO::index_t'] = '0'

-- \brief Converts a table with name-argument pairs into a string that can
--  be sent to the LUA interpreter
-- \param[in] mmethod the meta-method
-- \param[in] args the table with the name-argument pairs
-- \return the table as a string

function autogui.args_to_string(mmethod,args)
   local result = '{'
   for i=0,mmethod.nb_args()-1 do
      local name=mmethod.ith_arg_name(i)
      local value=tostring(args[name])
      if #result > 1 then
         result = result .. ','
      end
      if autogui.ith_arg_needs_quotes(mmethod,i) then
         result = result .. name .. '=' .. '\'' .. value .. '\''
      else
         result = result .. name .. '=' .. value
      end
   end
   if #result > 1 then
       result = result .. ','
   end
   result = result .. '_invoked_from_gui=true'
   result = result .. '}'
   return result
end

-- \brief Tests whether a parameter needs quotes
-- \details This is used to make the history look better (would work with
--  quotes everywhere)

function autogui.ith_arg_needs_quotes(mmethod, i)
   return (autogui.default_value[mmethod.ith_arg_type_name(i)] == nil)
end
