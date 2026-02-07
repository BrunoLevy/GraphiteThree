--  GOM/ImGUI/LUA coupling, for use with Skin_imgui
----------------------------------------------------

autogui = {}

-- \brief User API function for editing object properties
-- \param[in] object the object
-- \param[in] mproperty the meta-property of the property beeing edited

function autogui.property(object, mproperty)
   local bkp = autogui.input_text_flags
   autogui.input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue
   gom.record_set_property = true

   local tooltip = autogui.help(mproperty)
   if mproperty.type().is_a(OGF.MetaEnum) then
        autogui.enum(object, mproperty.name, mproperty.type(), tooltip)
   else
        local handler = autogui.handler_by_meta_type(mproperty.type())
	if mproperty.has_custom_attribute('handler') then
	   local handler_name = mproperty.custom_attribute_value('handler')
	   handler = autogui.handler_by_name(handler_name)
	end
        handler(object,mproperty.name,mproperty.type(),tooltip)
   end
   gom.record_set_property = false
   autogui.input_text_flags = bkp
end

-- \brief User API function for editing command arguments
-- \param[in] args a map with the arguments
-- \param[in] mslot the meta-slot that corresponds to the command
-- \param[in] i the index of the argument

function autogui.slot_arg(args,mslot,i)
   local handler = autogui.handler_by_meta_type(mslot.ith_arg_type(i))
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

-- \brief Keeps the state (arg lists) of the invoked commands
-- \details Command state is indexed by
--  'instance_name.classname.method' strings.

autogui.command_state  = {}

-- \brief Default values of parameters indexed by type name

autogui.default_value = {}
autogui.default_value['bool']         = 'false'
autogui.default_value['float']        = '0.0'
autogui.default_value['double']       = '0.0'
autogui.default_value['unsigned int'] = '0'
autogui.default_value['int']          = '0'
autogui.default_value['OGF::index_t'] = '0'
autogui.default_value['GEO::index_t'] = '0'

-- \brief Tests whether a parameter needs quotes
-- \details This is used to make the history look better (would work with
--  quotes everywhere)

function autogui.ith_arg_needs_quotes(mmethod, i)
   return (autogui.default_value[mmethod.ith_arg_type_name(i)] == nil)
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


-- \brief Gets the key used to handle dialogs for requests
-- \param[in] request a request
-- \return a unique key associated with the request

function autogui.request_key(request)
    local method_name = request.method().name
    local object = request.object()
    if object.is_a(OGF.Grob) then
       return 'rq##'..object.name..'##'..method_name
    end
    if object.is_a(OGF.Interface) then
       local interface_name = object.meta_class.name
       local grob_name = object.grob.name
       return 'rq##'..interface_name..'##'..method_name
    end
    return request.string_id --fallback, normally does not get there, bad to use
end

-- \brief Programmatically open a dialog for a command
-- \param[in] target a Request
-- \param[in] args optional default values for the arguments

function autogui.open_command_dialog(request,args)
  local k = autogui.request_key(request)
  if autogui.command_state[k] == nil then
     autogui.command_state[k] = autogui.init_args(request.method())
     autogui.command_state[k].show_as_window_ = true
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

-- \brief Programmatically open a dialog for a command that applies to
--  the current object.
-- \param[in] cmdclass the classname of the commands ('OGF::MeshGrobxxxCommands')
-- \param[in] cmdname the function name of the command
-- \param[in] args optional default values for the arguments

function autogui.open_command_dialog_for_current_object(cmdclass, cmdname, args)
   local o = scene_graph.current()
   local mclass = gom.resolve_meta_type(cmdclass)
   local commands = mclass.create()
   commands.grob = scene_graph.current()
   autogui.open_command_dialog(commands[cmdname],args)
end

-- \brief Handles the dialog for an object command
-- \param[in] request the request

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
  if autogui.command_state[k].show_as_window_ and
     request.object().is_a(OGF.Interface) then
     local grob = request.object().grob
     if not grob.is_a(OGF.SceneGraph) then
        autogui.command_state[k].Command_target_ = grob.name
        grob_names = gom.get_environment_value(
            grob.meta_class.name..'_instances'
        )
        if autogui.combo_box(
            autogui.command_state[k], 'Command_target_', grob_names
        ) then
            local commands_name = request.object().meta_class.name
            local command_name = request.method().name
            grob = scene_graph.objects[autogui.command_state[k].Command_target_]
            autogui.command_state[k].request_ =
                grob.I[commands_name][command_name]
        end
     end
  elseif imgui.MenuItem(
     imgui.font_icon('code-branch')..'  '..
     autogui.remove_underscores(mmethod.name)..'##command_apply'
  ) then
     autogui.command_state[k].show_as_window_ = true
  end

  autogui.tooltip(autogui.help(mmethod))
  imgui.Separator()

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

  imgui.Separator()
  if imgui.Button(
     imgui.font_icon('cog')
  ) then
      local bkp1 = autogui.command_state[k].show_as_window_
      autogui.command_state[k] = autogui.init_args(mmethod)
      autogui.command_state[k].show_as_window_ = bkp1
      autogui.command_state[k].request_ = request
  end
  autogui.tooltip('reset factory settings')
  imgui.SameLine()

  local doit_recycle_button = false
  if not autogui.command_state[k].show_as_window_ then
     doit_recycle_button = imgui.Button(imgui.font_icon('sync-alt'))
     autogui.tooltip('apply command without closing menu')
     imgui.SameLine()
  end

  local doit_apply_to_sel_button = imgui.Button(
     imgui.font_icon('clipboard-list')
  )
  autogui.tooltip('apply command to selected objects')
  imgui.SameLine()

  local doit_apply_button = imgui.Button(
     imgui.font_icon('check')..'##commands',-autogui.margin,0
  )
  autogui.tooltip('apply command')

  autogui.command_state[k].width_,
  autogui.command_state[k].height_ = imgui.GetWindowSize()

  --   Increase height a little bit so that resizing corner
  -- does not 'eat' too much and scrollbar does not appear.
  autogui.command_state[k].height_ = autogui.command_state[k].height_ + 5

  autogui.command_state[k].x_,
  autogui.command_state[k].y_ = imgui.GetWindowPos()

  if doit_apply_button or doit_recycle_button then
      local args_string = autogui.args_to_string(
         mmethod,autogui.command_state[k]
      )
      local object_as_string = gom.back_resolve(request.object())

      -- If the 'apply do not close' button was pushed, or if the command
      -- is displayed in a separate window, we directly call the LUA function
      -- (this does not close the menu)
      if doit_recycle_button then
         main.exec_command_now(
   	    object_as_string..'.'..mmethod.name..'('..args_string..')', false
         )
      -- else the command is in the menu and the apply button was pushed,
      -- and we queue the LUA command, this allows the command to display
      -- progress bars
      else
         main.exec_command(
   	    object_as_string..'.'..mmethod.name..'('..args_string..')', false
         )
	 -- Close the menu (if we were in menu mode).
	 if not autogui.command_state[k].show_as_window_ then
  	    imgui.CloseCurrentPopup()
	 end
      end
  end
  -- Apply command to selected objects
  if doit_apply_to_sel_button then
      local args_string = autogui.args_to_string(
         mmethod,autogui.command_state[k]
      )
      for i=0,scene_graph.nb_children-1 do
         local grob = scene_graph.ith_child(i)
         if grob.selected then
            local cmds = grob.I[request.object().meta_class.name]
            local cmd_str = gom.back_resolve(cmds)..'.'..mmethod.name
            main.exec_command_now(
               cmd_str..'('..args_string..')', false
            )
         end
      end
  end
end

-- Tests whether a command is still "alive"
-- A command is not "alive" if the object it refers was destroyed

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

-- Draws all the command dialogs that
-- have been switched to window mode.

function autogui.command_dialogs()

   for k,cmd_state in pairs(autogui.command_state) do

      -- Hide window / delete command state if target object was
      -- deleted in the meanwhile.

      if not autogui.command_is_alive(autogui.command_state[k]) then
         autogui.command_state[k] = nil
      elseif autogui.command_state[k].show_as_window_ then
         local sel

	 imgui.SetNextWindowSize(
	    cmd_state.width_  +  4.0*main.scaling(),
	    cmd_state.height_ + 40.0*main.scaling(),
	    ImGuiCond_Appearing
	 )

	 imgui.SetNextWindowPos(
	    cmd_state.x_,
	    cmd_state.y_,
	    ImGuiCond_Appearing
	 )

         sel,autogui.command_state[k].show_as_window_ = imgui.Begin(
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

-- \brief Handles the menu item for an object command
-- \param[in] request the request

function autogui.command_menu_item(request)
   local commands = request.object()
   local mmethod = request.method()

   if mmethod.nb_args() == 0 then
         -- Need to add '##' suffix with slot name with underscores
         -- else ImGui generates a shortcut I think (to be understood)
      if imgui.MenuItem(
          autogui.remove_underscores(mmethod.name)..'##'..
          mmethod.name
      ) then
         main.exec_command(
             gom.back_resolve(commands)..'.'..mmethod.name..
             '({_invoked_from_gui=true})', false
          )
      end
      autogui.tooltip(autogui.help(mmethod))
   else
      if imgui.BeginMenu(
         autogui.remove_underscores(mmethod.name)
      ) then
          autogui.in_popup = true
          autogui.command_dialog(request)
	  autogui.in_popup = false
          imgui.EndMenu()
      end
   end
end

-- \brief Tests whether an object member should be displayed in the GUI
-- \param[in] object the object
-- \param[in] mmeber the meta-member
-- \retval true if the member should be displayed
-- \retval false otherwise

function autogui.member_is_visible(object, mmember)
  -- Ignore members of the base classes
  if (
      mmember.container_meta_class().name == 'OGF::Object' or
      mmember.container_meta_class().name == 'OGF::Node'
  ) then
    return false
  end

  -- Ignore readonly properties
  if (mmember.is_a(OGF.MetaProperty) and mmember.read_only()) then
    return false
  end

  -- Take 'visible_if' custom attribute into account
  if object ~= nil then
     for i=0,mmember.nb_custom_attributes()-1 do
        -- Evaluate 'visible_if' custom attribute using object
	-- as context (Lua is so cool !!)
        if(
            mmember.ith_custom_attribute_name(i) == 'visible_if' and
            not load(
	       'return '..mmember.ith_custom_attribute_value(i),
	       'gom_attribute: visible_if', 'bt', object)()
        ) then
           return false
        end
        if(
            mmember.ith_custom_attribute_name(i) == 'visible' and
	    mmember.ith_custom_attribute_value(i) == 'false'
        ) then
           return false
        end
     end
  end

  return true
end

-- \brief The properties editor
-- \param[in,out] object the object to be edited

function autogui.properties_editor_properties(object)
   local mclass = object.meta_class
   for i=0,mclass.nb_properties()-1 do
      local mproperty = mclass.ith_property(i)
      -- If property is an Object and has the 'aggregate_properties'
      -- custom attribute, then edit its properties as well (recursive
      -- call).
      if mproperty.has_custom_attribute('aggregate_properties') and
         object[mproperty.name] ~= nil then
         subobject = object[mproperty.name]
         if autogui.in_tree then
            if subobject.meta_class.nb_properties() >
               gom.meta_types.OGF.Object.nb_properties() then
               if imgui.TreeNodeEx(
                   autogui.to_display_string(mproperty.name)..
                     ' properties'..
                     '##'..subobject.string_id,
                   ImGuiTreeNodeFlags_DrawLinesFull
               ) then
                   autogui.properties_editor_properties(subobject)
                   imgui.TreePop()
               end
            end
         else
            autogui.properties_editor_properties(subobject)
         end
      elseif autogui.member_is_visible(object, mproperty) then
         autogui.property(object,mproperty)
      end
   end
end

-- \brief Generates pushbuttons for slots with no
--  arguments.
-- \details Used for instance by properties 'autorange'

function autogui.properties_editor_functions(object)
   local mclass = object.meta_class
   for i=0,mclass.nb_slots()-1 do
      local mslot = mclass.ith_slot(i)
      if (
          autogui.member_is_visible(object, mslot) and
          mslot.nb_args() == 0 and
	  mslot.return_type_name() == 'void'
      ) then
         if imgui.Button(autogui.remove_underscores(mslot.name), -1, 0) then
            object[mslot.name]()
	 end
	 autogui.tooltip(autogui.help(mslot))
      end
   end
end


-- \brief Translates a shader user name to a shader type name
-- \param[in] grob_classname the classname of the Grob the shader
--  is attached to
-- \param[in] user_shader_name the user shader name
-- \return the classname of the shader

function shader_user_to_internal(grob_classname, user_shader_name)
   return 'OGF::'..user_shader_name..string.
      remove_prefix(grob_classname,'OGF::')..'Shader'
end

-- \brief Translates a shader classname to a user shader name
-- \param[in] grob_classname the classname of the Grob the shader
--  is attached to
-- \param[in] shader_classname the class name of the shader
-- \return the user name of the shader

function shader_internal_to_user(grob_classname, shader_classname)
   local result = string.remove_suffix(
      shader_classname,
      string.remove_prefix(grob_classname, 'OGF::')..'Shader'
   )
   result = string.remove_prefix(result,'OGF::')
   return result
end

-- \brief Handles the GUI for selecting a shader of an object
-- \param[in] grob the object

function shader_selector(grob)
    local xxx = {} -- we need a []-able object
    xxx.shader = shader_internal_to_user(
         grob.meta_class.name,
         grob.shader.meta_class.name
    )
    xxx.old_shader = xxx.shader
    autogui.combo_box(
      xxx,
      'shader',
      gom.get_environment_value(
           grob.meta_class.name .. "_shaders"
      ),
      tooltip
    )
    if xxx.shader ~= xxx.old_shader then
       local bkp = scene_graph.current_object
       scene_graph.current_object = grob.name
       scene_graph.scene_graph_shader_manager.shader(xxx.shader)
       scene_graph.current_object = bkp
    end
end

-- \brief Keeps the window state of the properties editors
-- \details Property editor state is indexed by 'instance_name.classname'
--  strings.

autogui.property_editor_state  = {}

-- \brief Handles the GUI for an object's property editor
-- \param[in] object the object to be edited. For Graphite objects,
--  pass the associated shader here.
-- \param[in] called_from_inspect true if called from autogui.inspect()
-- \param[in] no_windowify if set, do not show the button to windowify the
--  dialog.
-- \details This function is used to handle both menus and dialogs. It needs
--  to be called inside an opened menu or dialog. It memorizes the menu or
--  dialog state. In menu mode, it generates the button to switch to dialog
--  mode.

function autogui.properties_editor(object,called_from_inspect,no_windowify)
   local name = string.remove_prefix(object.meta_class.name,'OGF::')
   local is_shader = not called_from_inspect and
                     object.is_a(gom.meta_types.OGF.Shader)
   local k = object.string_id
   if is_shader then
      k = 'SHADER' .. k
      name = object.grob.name
   end

   if autogui.property_editor_state[k] == nil then
      autogui.property_editor_state[k] = {}
      autogui.property_editor_state[k].object_ = object
      autogui.property_editor_state[k].name_ = name
      autogui.property_editor_state[k].show_as_window_ = called_from_inspect
      autogui.property_editor_state[k].object_id_ = object.string_id
   end

   if called_from_inspect then
      autogui.property_editor_state[k].inspect_ = true
      autogui.property_editor_state[k].show_as_window_ = true
      return
   end

   imgui.PushID(k)

   if not no_windowify and
      not autogui.property_editor_state[k].show_as_window_ then
      if imgui.MenuItem(
         imgui.font_icon('edit')..'  ' .. name .. ' properties'
      ) then
         autogui.property_editor_state[k].show_as_window_ = true
      end
      imgui.Separator()
   end

   if is_shader then
       local grob = object.grob
       if gom.get_environment_value('gui:shaders_selector') == 'true' then
          shader_selector(grob)
       end
       object = grob.shader
       autogui.property_editor_state[k].object = object
       autogui.properties_editor_functions(object)
       imgui.Separator()
   end
   autogui.properties_editor_properties(object)
   if is_shader and not autogui.in_tree then
      imgui.Separator()
      imgui.Text(imgui.font_icon('sync-alt'))
      imgui.SameLine()
      local w = imgui.GetContentRegionAvail()/3-5
      if imgui.Button(
        imgui.font_icon('cubes')..
        '##ops##'..name, w,autogui.icon_size()+6
      ) then
         local current_bkp = scene_graph.current_object
         scene_graph.current_object = name
         scene_graph.scene_graph_shader_manager.apply_to_scene_graph()
         scene_graph.current_object = current_bkp
      end
      autogui.tooltip('Copy graphic properties to all objects')
      imgui.SameLine()
      if imgui.Button(
         imgui.font_icon('eye').."##ops##"..name,w,autogui.icon_size()+6
      ) then
         local current_bkp = scene_graph.current_object
         scene_graph.current_object = name
         scene_graph.scene_graph_shader_manager.apply_to_scene_graph(true)
         scene_graph.current_object = current_bkp
      end
      autogui.tooltip('Copy graphic properties to visible objects')
      imgui.SameLine()
      if imgui.Button(
         imgui.font_icon('clipboard-list').."##ops##"..name,
         -1,autogui.icon_size()+6
      ) then
         local current_bkp = scene_graph.current_object
         scene_graph.current_object = name
         scene_graph.scene_graph_shader_manager.apply_to_scene_graph(false,true)
         scene_graph.current_object = current_bkp
      end
      autogui.tooltip('Copy graphic properties to selected objects')
   end

   autogui.property_editor_state[k].width_,
   autogui.property_editor_state[k].height_ = imgui.GetWindowSize()

   autogui.property_editor_state[k].x_,
   autogui.property_editor_state[k].y_ = imgui.GetWindowPos()

   imgui.PopID(k)
end

-- \brief Draws all the windowed property editors.
-- \details This function needs to be called after the menu
--  handler, to draw all the property editors that were switched
--  to dialog mode by the user.

function autogui.property_editors()

   for k,v in pairs(autogui.property_editor_state) do

      -- Hide window / delete command state if target object was
      -- deleted meanwhile. Special version for shaders

      if not v.inspect_ and
         v.object_.is_a(gom.meta_types.OGF.Shader) and
         not scene_graph.is_bound(v.name_)
      then
         autogui.property_editor_state[k] = nil
         return
      end

      -- Hide window / delete command state if target object was
      -- deleted meanwhile. General version (but it does not work,
      -- to be understood). I need to debug garbage collection of
      -- LUA/GOM objects...

      if gom.resolve_object(
         autogui.property_editor_state[k].object_id_,true
      ) == nil then
         autogui.property_editor_state[k] = nil
	 return
      end

      if v.show_as_window_ then
         local sel

         if not v.inspect_ then
   	    imgui.SetNextWindowSize(
	        v.width_  + 4.0*main.scaling(),
	        v.height_ + 2.0*main.scaling(),
	        ImGuiCond_Appearing
	    )

	    imgui.SetNextWindowPos(
	        v.x_,
	        v.y_,
	        ImGuiCond_Appearing
	    )
	 end

         sel,v.show_as_window_ = imgui.Begin(
	     imgui.font_icon('edit')..'  '..
	     v.name_ .. " properties",
             v.show_as_window_
         )
         autogui.properties_editor(
	   v.object_
	 )
         imgui.End()
      end
   end
end

-- Creates a graphical object inspector for
--   a given Graphite object.
-- Note: use with care, you are on your own
--  (if the object is destroyed in the meanwhile,
--   it crashes !)
function autogui.inspect(object)
   if object.meta_class == nil then
      print('Object '..tostring(object)..' is not a Graphite object')
   end
   autogui.properties_editor(object, true)
end
