--  GUI for the scene-graph, for use with Skin_imgui
----------------------------------------------------

-- Needed to enable navigation in object list with
-- arrow keys (declared in imgui_internal.h)
ImGuiSelectableFlags_SelectOnNav = (1 << 21)

-- All types and functions related with the scene graph.
scene_graph_gui = {}
scene_graph_gui.name = 'Scene'
scene_graph_gui.icon = '@cubes'

scene_graph_gui.grob_icon = {}
scene_graph_gui.grob_icon['OGF::SceneGraph']  = 'cubes'
scene_graph_gui.grob_icon['OGF::MeshGrob']    = 'cube'
scene_graph_gui.grob_icon['OGF::LuaGrob']     = 'code'
scene_graph_gui.grob_icon['OGF::VoxelGrob']   = 'th'

-- \brief MenuMap Handles object commands menus
-- \details With the 'menu' gom_attribute, the user
--   can redesign the menu hierarchy, thus generating
--   it needs a twopass algorithm. MenuMap stores the
--   menu hierarchy associated with a Grob class.
scene_graph_gui.menu_map = {}

-- \brief Creates a new node
-- \return an empty new node
function scene_graph_gui.menu_map.new_node()
   local result = {}
   result.by_name = {}
   result.by_index = {}
   return result
end

-- \brief Creates a menu item associated with a command
-- \param[in] node the node in which the item should be created
-- \param[in] mslot the MetaSlot associated with the command
function scene_graph_gui.menu_map.create_menu_item(node, mslot)
   local item_name = autogui.remove_underscores(mslot.name)
   node.by_name[item_name] = mslot
   table.insert(node.by_index, mslot)
end

-- \brief Finds or create a submenu in a node
-- \param[in] node the node
-- \param[in] submenu_name the name of the submenu
-- return the node associated with the submenu. It is
--  created if it did not exist already.
function scene_graph_gui.menu_map.find_or_create_submenu(node, submenu_name)
   local submenu = node.by_name[submenu_name]
   if submenu == nil then
      submenu = scene_graph_gui.menu_map.new_node()
      submenu.name = submenu_name
      node.by_name[submenu_name] = submenu
      table.insert(node.by_index,submenu)
   end
   return submenu
end

-- \brief Inserts a command in the menu hierarchy
-- \param[in] node the root node of the hierarchy
-- \param[in] menu_path the path in the menu, with
--  submenus separated by '/'.
function scene_graph_gui.menu_map.insert(node, menu_path, mslot)
   if menu_path == nil then
       scene_graph_gui.menu_map.create_menu_item(node,mslot)
   else
       local head,tail = scene_graph_gui.menu_map.split_menu_path(menu_path)
       local submenu = scene_graph_gui.menu_map.find_or_create_submenu(
           node, head
       )
       scene_graph_gui.menu_map.insert(submenu,tail,mslot)
   end
end

-- \brief Splits a menu path into its head component
--  and the rest of the path.
-- \return the first component of the path
--  and the rest or nil if the past had a single
--  component
function scene_graph_gui.menu_map.split_menu_path(menu_path)
   local pos = menu_path:find('/')
   if pos == nil then
      return menu_path,nil
   else
      return menu_path:sub(1,pos-1),menu_path:sub(pos+1)
   end
end

-- \brief Gets the MenuMap associated with a grob.
-- \return the MenuMap.
function scene_graph_gui.menu_map.get(grob)
   -- We could cache it if need be, but LUA is so fast
   -- that it is probably not worth it...
   local menus = scene_graph_gui.menu_map.new_node()
   local grob_class_name = grob.meta_class.name
   local k = grob_class_name..'_commands'
   local commands_str = gom.get_environment_value(k)
   for command_class_name in string.split(commands_str,';') do
      -- We skip OGF::SceneGraphSceneCommands because they
      -- are now implemented in the context menu of each object
      -- (rename, delete etc...)
      if command_class_name ~= 'OGF::SceneGraphSceneCommands' then
         local default_menu_name = command_class_name
	 local command_mclass = gom.resolve_meta_type(command_class_name)
	 -- Command may be associated with a base class, so we find
	 -- the name of this base class in the 'grob_class_name' attribute
	 -- of the Command and strip it to generate the menu name.
         default_menu_name = string.strip_prefix(
	    default_menu_name,
	    command_mclass.custom_attribute_value('grob_class_name')
	 )
         default_menu_name = string.strip_suffix(default_menu_name,'Commands')
         local mclass = gom.resolve_meta_type(command_class_name)
         for i=0,mclass.nb_slots()-1 do
            local mslot = mclass.ith_slot(i)
	    local menu_name = default_menu_name
	    if mslot.has_custom_attribute('menu') then
   	       local submenu_name = mslot.custom_attribute_value('menu')
	       -- Remove trailing '/' if present	
	       if submenu_name:sub(-1) == '/' then
		   submenu_name = submenu_name:sub(1,-2)
	       end
               -- If submenu starts with '/' it is an absolute path
	       if submenu_name:sub(1,1) == '/' then
                   menu_name = submenu_name:sub(2)
                   -- Particular case: SceneGraph commands starting with '/',
                   -- to be rooted in the menu bar, are stored in the '/menubar'
                   -- menumap (and handled with
                   -- specific code in graphite_gui.draw_menu_bar())
                   if(grob.meta_class.name == 'OGF::SceneGraph') then
                      menu_name = 'menubar/'..menu_name
                   end
	       else 
	          menu_name = menu_name .. '/' .. submenu_name
	       end
	    end
            if autogui.member_is_visible(nil, mslot) then
	       scene_graph_gui.menu_map.insert(menus, menu_name, mslot)
	    end   
         end
      end
   end
   return menus
end

-- \brief Draws the command menus for a grob.
-- \param[in] grob the grob
-- \param[in] node the submenu to draw, or nil
--   if nil, the whole menu hierarchy is drawn.
function scene_graph_gui.menu_map.draw(grob, node)
   if node == nil then
     node = scene_graph_gui.menu_map.get(grob)
   end
   for i,v in pairs(node.by_index) do
      if v.meta_class ~= nil then
         local command_class = v.container_meta_class()
	 local command_class_name = command_class.name
	 local object_as_string
	 if grob == scene_graph then
	    object_as_string = 'scene_graph'
	 else
            object_as_string = 'scene_graph.resolve(\'' .. grob.name .. '\')'
	 end

         object_as_string = object_as_string ..
            '.query_interface(\''..command_class_name..'\')'
	    
         autogui.command_menu_item(grob, v, object_as_string)	    
      else
         if v.name ~= 'menubar' then
            if imgui.BeginMenu(v.name) then
               scene_graph_gui.menu_map.draw(grob, v)
	       imgui.EndMenu()
	    end
         end
      end
   end
end

--------------------------------------------------------------

-- \brief Opens a 'save as...' dialog for a given grob
-- \param[in] grob the grob to be saved
function scene_graph_gui.save_grob_as(grob)
   save_grob = grob
   local grob_class_name = save_grob.meta_class.name
   local extensions = gom.get_environment_value(
       grob_class_name..'_write_extensions'
   )
   extensions = string.gsub(extensions,'*.','')
   imgui.OpenFileDialog(
      '##object##save_dlg',
      extensions,
      '', -- default filename
      ImGuiExtFileDialogFlags_Save
   )
end

--------------------------------------------------------------

-- \brief Handles a grob operations menu
-- \param[in] grob the grob
-- \param[in] main_menu if true, the grob operations are drawn
--   from the main menu.
-- \retval nil if the object is deleted
-- \retval the object if it still exists
function scene_graph_gui.grob_ops(grob, main_menu)
   local name = grob.name

   if not main_menu then
      imgui.MenuItem(name,nil,false,false)
      imgui.Separator()
      if imgui.BeginMenu(imgui.font_icon('edit')..'  properties') then
         autogui.in_popup = true
         autogui.properties_editor(grob.shader)
	 autogui.in_popup = false
         imgui.EndMenu()
      end
      imgui.Separator()
   end

   scene_graph_gui.menu_map.draw(grob)

   if not main_menu then
      imgui.Separator()

      if imgui.MenuItem(imgui.font_icon('edit')..'  rename') then
         autogui.rename_old = name
         autogui.rename_new = name
      end

      if imgui.MenuItem(imgui.font_icon('clone')..'  duplicate') then
         main.save_state()
         scene_graph.current_object = name
         local dup = scene_graph.duplicate_current()
         autogui.rename_old = dup.name
         autogui.rename_new = dup.name
         scene_graph.current_object = dup.name
      end
      
      if imgui.MenuItem(imgui.font_icon('window-close')..'  delete') then
         main.save_state()
         main.picked_grob = nil
         scene_graph.current_object = name
         scene_graph.delete_current_object()
         return nil
      end

      imgui.Separator()

      if imgui.BeginMenu(imgui.font_icon('file')..'  File...') then
        if imgui.MenuItem(imgui.font_icon('save')..'  Save as...') then
	   scene_graph_gui.save_grob_as(grob)
        end
        imgui.EndMenu()
     end
   end  
   return grob
end

function scene_graph_gui.about_window()
   if not scene_graph_gui.about_visible then
      return
   end
   _,scene_graph_gui.about_visible = imgui.Begin(
        imgui.font_icon('info')..'  About Graphite...',
	scene_graph_gui.about_visible,
	 ImGuiWindowFlags_NoDocking	
   )
   if scene_graph_gui.about_visible then
      imgui.Image(
         main.resolve_icon('logos/small-geogram-logo',true),
	 main.scaling()*128, main.scaling()*128
      )
      imgui.SameLine()
      imgui.Text( '\n'..
          'Graphite version: '..gom.get_environment_value('version')..'\n\n'..
	  'Released: '..gom.get_environment_value('release_date')..'\n\n'..
	  'Running on: '..gom.get_environment_value('nb_cores')..' cores'
      )
      imgui.Separator()
      imgui.Text('Websites: ');
      imgui.Text('   https://github.com/BrunoLevy/GraphiteThree/');      
      imgui.Text('   https://github.com/BrunoLevy/geogram');
      imgui.Separator()
      imgui.Text('\n')
      imgui.Text('   ')
      imgui.SameLine()
      local iconname = 'logos/small-erc-logo-dark'
      if gom.get_environment_value('gfx:style') == 'Light' then
         iconname = 'logos/small-erc-logo'	  
      end
      imgui.Image(
         main.resolve_icon(iconname,true),
	 main.scaling()*64, main.scaling()*64
      )
      imgui.SameLine()
      imgui.Text(
          '   With algorithms from:\n'..
          '      ERC-StG-205693 GoodShape\n'..
          '      ERC-PoC-334829 Vorpaline\n'..
	  '   ... as well as new ones.'
      )
      imgui.Text('\n')
      imgui.Separator()
      imgui.Text('\n')
      imgui.Text('          ')
      imgui.SameLine()
      local iconname = 'logos/small-inria'
      local frame = math.floor(80.0*os.clock()) % 100
      if frame == 1 or frame == 3 or framme == 5 or frame == 7 then
        iconname = iconname..'-2'
      elseif frame == 2 or frame == 6 then
        iconname = iconname..'-3'
      end
      imgui.Image(
         main.resolve_icon(iconname,true),
	 main.scaling()*74, main.scaling()*22
      )
      imgui.SameLine()
      imgui.Text('  ')
      imgui.Text('\n')
   end
   imgui.End()
end


function scene_graph_gui.file_menu()
       if imgui.BeginMenu('File...') then
       
          if imgui.MenuItem(
	      imgui.font_icon('folder-open')..'  Load...##scene_graph'
	  ) then
             local extensions =
	         gom.get_environment_value('grob_read_extensions')
	     extensions = string.gsub(extensions,'*.','')
       	     imgui.OpenFileDialog(
	        '##scene_graph##load_dlg',
	        extensions,
	        '', -- default filename
	        ImGuiExtFileDialogFlags_Load
	     )
          end

          if imgui.MenuItem(
	     imgui.font_icon('save')..
	     '  Save current object as...##scene_graph'
	  ) then
	     if scene_graph.current() == nil then
	        gom.err(
		   {tag='SceneGraph',message='There is no selected object.'}
		)
	     else 
	        scene_graph_gui.save_grob_as(scene_graph.current())
             end		
          end

          if imgui.MenuItem(
	     imgui.font_icon('box-open')..'  Save scene as...##scene_graph'
	  ) then
             local extensions = 'graphite;graphite_ascii'
       	     imgui.OpenFileDialog(
	        '##scene_graph##save_dlg',
	        extensions,
	        '', -- default filename
	        ImGuiExtFileDialogFlags_Save
	     )
          end

          if imgui.MenuItem(
	     imgui.font_icon('camera')..'  Save view as...##scene_graph'
	  ) then
             local extensions = 'graphite;graphite_ascii'
       	     imgui.OpenFileDialog(
	        '##scene_graph##save_view_dlg',
	        extensions,
	        '', -- default filename
	        ImGuiExtFileDialogFlags_Save
	     )
          end

          imgui.Separator()

	  if imgui.MenuItem(imgui.font_icon('cogs')..'  preferences...') then
	     preferences_window.show()
	  end

	  if imgui.MenuItem(imgui.font_icon('info')..'  about...') then
	     scene_graph_gui.about_visible = true
	  end

	  imgui.Separator()

          if imgui.MenuItem(imgui.font_icon('door-open')..'  quit') then
             main.stop()
          end

          imgui.EndMenu()
       end
end

function scene_graph_gui.scene_graph_menu(with_file_menu)
    local commands_meta_class =
           gom.resolve_meta_type('OGF::SceneGraphSceneCommands')
    local commands_as_string =
           'scene_graph.query_interface(\'OGF::SceneGraphSceneCommands\')'
    if imgui.MenuItem('show all') then
       for i=0,scene_graph.nb_children-1 do
          scene_graph.ith_child(i).visible = true
       end
    end
    if imgui.MenuItem('hide all') then
       for i=0,scene_graph.nb_children-1 do
          scene_graph.ith_child(i).visible = false
       end
    end
    imgui.Separator()
    autogui.command_menu_item(
       scene_graph,
       commands_meta_class.find_method('create_object'),
       commands_as_string
    )
    autogui.command_menu_item(
       scene_graph,
       commands_meta_class.find_method('delete_all'),
       commands_as_string
    )
    imgui.Separator()
    scene_graph_gui.menu_map.draw(scene_graph, nil)
    if with_file_menu then
       imgui.Separator()
       scene_graph_gui.file_menu()
    end
    if gom.get_environment_value('gui:undo') == 'true' then
        imgui.Separator()
        if imgui.MenuItem(imgui.font_icon('undo')..' undo') then
           main.undo()
        end
        if imgui.MenuItem(imgui.font_icon('redo')..' redo') then
           main.redo();
        end
    end
end

-- \brief Handles the scene-graph operations menu
function scene_graph_gui.scene_graph_ops()

  local name='Scene'
  local sel,visible = imgui.Checkbox(
       '##box##'..name,
       scene_graph.visible
  )
  if(sel) then
     scene_graph.visible = visible
  end
  imgui.SameLine()
  imgui.Text(imgui.font_icon('cubes'))
  imgui.SameLine()
  imgui.Selectable(name,false)

  if imgui.BeginPopupContextItem(name..'##ops') then
       scene_graph_gui.scene_graph_menu(true)       
       imgui.EndPopup()
  end

  local filename=''
  local sel
  
  sel,filename = imgui.FileDialog('##scene_graph##load_dlg',filename)
  if sel then
     scene_graph.load_object({value=filename,invoked_from_gui=true})
  end

  sel,filename = imgui.FileDialog('##scene_graph##save_dlg',filename)
  if sel then
     scene_graph.save(filename)
  end

  sel,filename = imgui.FileDialog('##scene_graph##save_view_dlg',filename)
  if sel then
     scene_graph.save_viewer_properties(filename)
  end

end


-- \brief Draws the scene-graph and object list
-- \details Handles context-menus 

function scene_graph_gui.draw()
   local current_name=scene_graph.current_object
   scene_graph_gui.scene_graph_ops()

   imgui.Indent()
   for i=0,scene_graph.nb_children-1 do
       -- Need to verify in case an object was not deleted 
       -- during iteration.
       if i < scene_graph.nb_children then
          local grob = scene_graph.ith_child(i)
          local name = grob.name
	  local sel,val = imgui.Checkbox(
	       '##box##'..tostring(i),
	       grob.visible
	  )
	  if sel then
	     grob.visible=val
             scene_graph.current_object = grob.name
	  end
	  imgui.SameLine()
	  local icon = scene_graph_gui.grob_icon[grob.meta_class.name]
	  if icon == nil then
	     icon = 'cube'
	  end
          imgui.Text(imgui.font_icon(icon))
	  autogui.tooltip(grob.meta_class.name)
	  imgui.SameLine()
	  if name == autogui.rename_old then
	     if autogui.rename_old == autogui.rename_new then
	        imgui.SetKeyboardFocusHere()
	     end
             local sel
	     sel,autogui.rename_new = imgui.TextInput(
	          '##renames##'..name,
	          autogui.rename_new,
                  ImGuiInputTextFlags_EnterReturnsTrue |
		  ImGuiInputTextFlags_AutoSelectAll
             )
	     if sel then
                main.save_state()             
		scene_graph.current_object = name
		local o = scene_graph.current()
		o.rename(autogui.rename_new)
		scene_graph.current_object = o.name
	        autogui.rename_old = nil
		autogui.rename_new = nil
	     end
	  elseif imgui.Selectable(
	       name, name == current_name,
	       ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnNav
	  ) then
	      if imgui.IsMouseDoubleClicked(0) then
                  for i = 0,scene_graph.nb_children-1 do
		     scene_graph.ith_child(i).visible = false
		  end
		  grob.visible=true
	      end
	      scene_graph.current_object = grob.name
          end
	  grob = nil
          if i < scene_graph.nb_children then
	     grob = scene_graph.ith_child(i)
	     name = grob.name
             if imgui.BeginPopupContextItem(name..'##ops') then	  
                 grob = scene_graph_gui.grob_ops(grob)
	         imgui.EndPopup()	      
	     end
	  end   
	  if grob == nil then
	     i = scene_graph.nb_children
	  end
      end
   end

  local filename=''
  local sel
  sel,filename = imgui.FileDialog('##object##save_dlg',filename)
  if sel then
     save_grob.save(filename)
     save_grob = nil
  end

  -- The context menu when clicking on a 3D object
  imgui.Unindent()
  if main.picked_grob ~= nil then
    if imgui.BeginPopupContextVoid() then
       scene_graph_gui.grob_ops(main.picked_grob)
       imgui.EndPopup()
    end
  end

  scene_graph_gui.about_window()

end

graphite_main_window.add_module(scene_graph_gui)
