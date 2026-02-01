--  GUI for the main window of Graphite, for use with Skin_imgui
----------------------------------------------------------------

-- A table of functions to be called when
--  handling the GUI for Graphite objects.
graphite_main_window = {}
graphite_main_window.modules_by_index = {}
graphite_main_window.modules_by_name = {}

-- \brief Adds a new graphite module
-- \param[in] module the graphite module, a table with the
--   following fields:
--   name: the name of the module
--   visible: initial visibility of the module
--   (optional) icon: name of the icon
--   (optional) menubar: if true, draw a menubar
--   (optional) draw_window: the function to draw the interior of the
--     window
--   (optional) draw_menu: the function to draw the context menu (and
--     the menus in the menubar if menubar was set to true)
--   (optional) draw: the function to draw the entry in the main
--     graphite window. If unspecified, uses the default one.
--   (optional) draw_extra: a function to draw additional stuff,
--     e.g. file dialogs
--   (optional) x,y,w,h: initial position and size of the window.
--   (optional) window_flags: additional window flags passed to imgui.Begin()
--
-- \details One can overwrite a previously added object

function graphite_main_window.add_module(module)
   local name = module.name
   local oldfunc = graphite_main_window.modules_by_name[name]
   if oldfunc ~= nil then
      for i,ioldfunc in ipairs(graphite_main_window.modules_by_index) do
         if ioldfunc == oldfunc then
	    graphite_main_window.modules_by_index[i] = module
	 end
      end
   else
      table.insert(graphite_main_window.modules_by_index, module)
   end
   graphite_main_window.modules_by_name[name] = module
   local module_visibility = false
   if module.visible ~= nil then
      module_visibility = module.visible
   end
   main.declare_preference_variable(
      'gui:module_'..name..'_visible',
      tostring(module_visibility),
      'module '..name..' visibility'
   )
   module.visible = (
      gom.get_environment_value('gui:module_'..name..'_visible') == 'true'
   )
end

-- \brief Removes a module
-- \param name the name of the module

function graphite_main_window.remove_module(name)
   local module = graphite_main_window.modules_by_name[name]
   if module == nil then
      print(name .. ": no such graphite module")
      return
   end
   graphite_main_window.modules_by_name[name] = nil
   for i,module2 in ipairs(graphite_main_window.modules_by_index) do
      if module2 == module then
         table.remove(graphite_main_window.modules_by_index,i)
	 return
      end
   end
end


-- \brief Handles the GUI for a module toggle
-- \param[in] module the module
-- \param[in] in_menu true if drawing toggles in menu
-- \return true if the name was clicked

function graphite_main_window.draw_module_name_and_toggle(module, in_menu)

   local draw_props = false

   if not in_menu and module.draw_properties ~= nil then
      draw_props = imgui.TreeNodeEx(
         '##'..module.name..'##props',ImGuiTreeNodeFlags_DrawLinesFull
      )
      imgui.SameLine()
   end

   if module.no_toggle == nil then
      _,module.visible = imgui.Checkbox(
         "##module_box##"..module.name,
         module.visible
      )
   else
      if in_menu then
         return nil
      end
   end

   gom.set_environment_value(
      'gui:module_'..module.name..'_visible',
      tostring(module.visible)
   )
   if module.icon ~= nil then
      imgui.SameLine()
      if module.icon:starts_with('@') then
         imgui.Text(imgui.font_icon(module.icon:sub(2)))
      else
         imgui.Image(
          main.resolve_icon(module.icon,true),
          autogui.icon_size(), autogui.icon_size()
         )
      end
   end
   imgui.SameLine()
   local sel = imgui.Selectable(module.name,false)

   if in_menu then
      return sel
   end

   if module.draw_menu ~= nil then
      if imgui.BeginPopupContextItem(module.name..'##ops') then
          module.draw_menu()
          imgui.EndPopup()
      end
   end

   if draw_props then
      autogui.in_tree = true
      module.draw_properties()
      autogui.in_tree = false
      imgui.TreePop()
   end

   return sel
end

-- \brief Handles the GUI for a module
-- \param[in] module the module

function graphite_main_window.draw_module(module)
     if module.visible and module.draw_window ~= nil then
         if module.x ~= nil and module.y ~= nil then
	    imgui.SetNextWindowPos(module.x, module.y, ImGuiCond_FirstUseEver)
	 end
	 if module.w ~= nil and module.h ~= nil then
	    imgui.SetNextWindowSize(module.w, module.h, ImGuiCond_FirstUseEver)
	 end
	 local flags = 0
	 if module.menubar and module.draw_menu ~= nil then
	    flags=ImGuiWindowFlags_MenuBar
	 end
	 local with_menubar = (flags ~= 0)
	 if module.window_flags ~= nil then
	    flags = flags | module.window_flags
 	 end
	 local winname = module.name..'##module'
	 if module.icon ~= nil and module.icon:starts_with('@') then
	    winname = imgui.font_icon(module.icon:sub(2))..'  '..winname
	 end
	 _,module.visible = imgui.Begin(winname, module.visible, flags)
	 if module.visible then
	    if with_menubar then
	       if imgui.BeginMenuBar() then
	          module.draw_menu()
	          imgui.EndMenuBar()
	       end
	    end
            module.draw_window()
	 end
	 imgui.End()
     end
     if module.draw_extra ~= nil then
        module.draw_extra()
     end
end

function graphite_main_window.draw_modules_menu()
   for index,module in ipairs(graphite_main_window.modules_by_index) do
      if graphite_main_window.draw_module_name_and_toggle(module,true) then
         module.visible = not module.visible
         gom.set_environment_value(
            'gui:module_'..module.name..'_visible',
            tostring(module.visible)
         )
      end
   end
end

function graphite_main_window.draw_contents()
  local btn_width  = 25 * main.scaling()
  if scene_graph_gui ~= nil and graphite_gui.presentation_mode() then
     if imgui.BeginMenuBar() then
        scene_graph_gui.file_menu()
        imgui.EndMenuBar()
     end
  end
  toolbox_gui.draw_tools(true)
  imgui.NewLine()
  imgui.Spacing()
  imgui.Separator()
  if gom.get_environment_value('gui:undo') == 'true' then
     local ImGuiStyleVar_Alpha = 0
     if(not main.can_undo) then
        imgui.PushStyleVar(ImGuiStyleVar_Alpha, 0.3);
     end
     if imgui.Button(imgui.font_icon('undo')..' undo') then
        main.undo()
     end
     if(not main.can_undo) then
        imgui.PopStyleVar()
     end
     imgui.SameLine()
     if(not main.can_redo) then
        imgui.PushStyleVar(ImGuiStyleVar_Alpha, 0.3);
     end
     if imgui.Button(imgui.font_icon('redo')..' redo') then
        main.redo();
     end
     if(not main.can_redo) then
        imgui.PopStyleVar()
     end
     imgui.Separator()
  end
  if imgui.TreeNodeEx(
        imgui.font_icon('cog')..'Modules',
        ImGuiTreeNodeFlags_DrawLinesFull
  ) then
     for index,module in ipairs(graphite_main_window.modules_by_index) do
        graphite_main_window.draw_module_name_and_toggle(module)
     end
     imgui.TreePop()
  end
  if imgui.BeginPopupContextItem('Modules##ShowHide') then
      graphite_main_window.draw_modules_menu()
      imgui.EndPopup()
  end

  imgui.PushItemWidth(-100)
  local sg_open = imgui.TreeNodeEx(
       imgui.font_icon('cubes')..'Scene ',
          ImGuiTreeNodeFlags_DrawLinesFull |
          ImGuiTreeNodeFlags_DefaultOpen  |
          ImGuiTreeNodeFlags_AllowOverlap
  )
  imgui.PopItemWidth()
  scene_graph_gui.scene_graph_ops()
  imgui.SameLine()
  imgui.Dummy(imgui.GetContentRegionAvail()-btn_width,2)
  imgui.SameLine()
  if scene_graph_gui.edit_list then
    if imgui.SimpleButton(imgui.font_icon('check')) then
       scene_graph_gui.edit_list = false
    end
    autogui.tooltip('done editing')
  else
    if imgui.SimpleButton(imgui.font_icon('ellipsis-h')) then
       scene_graph_gui.edit_list = true
    end
    autogui.tooltip('edit object list')
  end

  if sg_open then
     scene_graph_gui.draw_object_list()
     imgui.TreePop()
  end
  for index,module in ipairs(graphite_main_window.modules_by_index) do
     graphite_main_window.draw_module(module)
  end
end

-- \brief Draws the main window of Graphite, with all the modules in it.

function graphite_main_window.draw()
  graphite_gui.right_pane_width = 0
  imgui.SetNextWindowPos(
      3*main.margin(), main.margin(), ImGuiCond_FirstUseEver
  )
  imgui.SetNextWindowSize(
      150*main.scaling(),
      300*main.scaling(),
      ImGuiCond_FirstUseEver
  )
  local flags = 0;
  if graphite_gui.presentation_mode() then
     flags = ImGuiWindowFlags_MenuBar
  end
  imgui.Begin('Graphite',true,flags)
  graphite_main_window.draw_contents()
  graphite_gui.left_pane_width = imgui.GetWindowWidth()
  imgui.End()
end
