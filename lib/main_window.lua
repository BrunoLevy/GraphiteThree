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

-- \brief Handles the GUI for a module
-- \param[in] module the module

function graphite_main_window.draw_module(module)
   if module.draw ~= nil then
      module.draw()
   else
      _,module.visible = imgui.Checkbox(
          "##module_box##"..module.name,
          module.visible
      )
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
      imgui.Selectable(module.name,false)
      if module.draw_menu ~= nil then
        if imgui.BeginPopupContextItem(module.name..'##ops') then
	   module.draw_menu()
           imgui.EndPopup()
	end
      end
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
end

function graphite_main_window.draw_contents()
  if scene_graph_gui ~= nil and graphite_gui.presentation_mode() then
     if imgui.BeginMenuBar() then
        scene_graph_gui.file_menu()
        imgui.EndMenuBar()
     end
  end
  if imgui.Button(imgui.font_icon('home')..' Home',-40,0) then
     camera_gui.home()
  end
  imgui.SameLine()
  if imgui.BeginMenu(imgui.font_icon('ruler-combined')) then
     camera_gui.projection_dialog()
     imgui.EndMenu()
  end
  
  imgui.Separator()
  for index,module in ipairs(graphite_main_window.modules_by_index) do
     graphite_main_window.draw_module(module)
  end
end

-- \brief Draws the main window of Graphite, with all the modules in it.

function graphite_main_window.draw()
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
  imgui.End()  
end

