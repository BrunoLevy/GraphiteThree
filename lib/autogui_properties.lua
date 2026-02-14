-- ----------------------------------------------------------------------------
-- \file autogui.properties.lua
-- \brief creates dialogs for editing object properties
-- ----------------------------------------------------------------------------

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
         if not v.inspect_ then
   	    imgui.SetNextWindowSize(
	        v.width_  + 4.0*main.scaling(),
	        v.height_ + 2.0*main.scaling(),
	        ImGuiCond_Appearing
	    )
	    imgui.SetNextWindowPos(v.x_, v.y_, ImGuiCond_Appearing)
	 end
         _,v.show_as_window_ = imgui.Begin(
	     imgui.font_icon('edit')..'  '.. v.name_ .. " properties",
             v.show_as_window_
         )
         autogui.properties_editor(v.object_)
         imgui.End()
      end
   end
end

-- \brief Creates a graphical object inspector for a given Graphite object.
-- \details Can be called from Graphite console. Note: use with care,
--   you are on your own (if the object is destroyed in the meanwhile,
--   it will crash !

function autogui.inspect(object)
   if object.meta_class == nil then
      print('Object '..tostring(object)..' is not a Graphite object')
   end
   autogui.properties_editor(object, true)
end

-- ------------------------------------------------------------------------------

-- \brief Keeps the window state of the properties editors
-- \details Property editor state is indexed by 'instance_name.classname'
--  strings.

autogui.property_editor_state  = {}

-- ------------------------------------------------------------------------------

-- \brief Draws the properties in the property editor
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

-- \brief Generates pushbuttons for slots with no arguments.
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

-- ---------------------------------------------------------------------------

-- \brief Edits an object property
-- \param[in] object the object
-- \param[in] mproperty the meta-property of the property beeing edited

function autogui.property(object, mproperty)
   local bkp = autogui.input_text_flags
   autogui.input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue
   gom.record_set_property = true
   local tooltip = autogui.help(mproperty)
   local handler = autogui.handler_by_meta_type(mproperty.type())
   if mproperty.has_custom_attribute('handler') then
       local handler_name = mproperty.custom_attribute_value('handler')
       handler = autogui.handler_by_name(handler_name)
   end
   handler(object,mproperty.name,mproperty.type(),tooltip)
   gom.record_set_property = false
   autogui.input_text_flags = bkp
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

-- ---------------------------------------------------------------------------

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
        scene_graph.set_object_shader{
                grob=grob, user_name=xxx.shader,
                _invoked_from_gui = true
        }
    end
end
