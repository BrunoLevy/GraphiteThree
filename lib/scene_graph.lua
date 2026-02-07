--  GUI for the scene-graph, for use with Skin_imgui
----------------------------------------------------

-- All types and functions related with the scene graph.
scene_graph_gui = {}
scene_graph_gui.name = 'Scene'
scene_graph_gui.icon = '@cubes'
scene_graph_gui.edit_list = false -- true when up/down/delete btn are visible

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
         default_menu_name = string.remove_prefix(
	    default_menu_name,
	    command_mclass.custom_attribute_value('grob_class_name')
	 )
         default_menu_name = string.remove_suffix(default_menu_name,'Commands')
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
         local request = grob.I[command_class.name][v.name]
         autogui.command_menu_item(request)
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

   local btn_width  = 25 * main.scaling()
   if not main_menu then
      imgui.Separator()
      imgui.PushStyleVar_2(ImGuiStyleVar_ItemSpacing, 0.0, 4.0)
      imgui.TextDisabled('Edit...')
      imgui.SameLine()
      imgui.Dummy(imgui.GetContentRegionAvail()-btn_width*5,1) -- here
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('edit')..'##  rename') then
         scene_graph_gui.rename_old = name
         scene_graph_gui.rename_new = name
      end
      autogui.tooltip('rename')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('clone')..'##  duplicate') then
         main.save_state()
         scene_graph.current_object = name
         local dup = scene_graph.duplicate_current()
         scene_graph_gui.rename_old = dup.name
         scene_graph_gui.rename_new = dup.name
         scene_graph.current_object = dup.name
      end
      autogui.tooltip('duplicate')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('window-close')..'##  delete') then
         main.save_state()
         main.picked_grob = nil
         scene_graph.current_object = name
         scene_graph.delete_current_object()
      end
      autogui.tooltip('delete')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('arrow-up')..'##move up') then
         main.save_state()
         scene_graph.current_object = name
         scene_graph.move_current_up()
      end
      autogui.tooltip('move up')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('arrow-down')..'##move down') then
         main.save_state()
         scene_graph.current_object = name
         scene_graph.move_current_down()
      end
      autogui.tooltip('move down')
      imgui.PopStyleVar()

      imgui.Separator()

      imgui.PushStyleVar_2(ImGuiStyleVar_ItemSpacing, 0.0, 4.0)
      imgui.TextDisabled('Gfx...')
      imgui.SameLine()
      imgui.Dummy(imgui.GetContentRegionAvail()-btn_width*4,1)
      imgui.SameLine()
      if imgui.SimpleButton(
         imgui.font_icon('eye-slash')..'## show/hide'
      ) then
         grob.visible = not grob.visible
      end
      autogui.tooltip('show/hide')
      imgui.SameLine()
      if imgui.SimpleButton(
         imgui.font_icon('cubes')..'## copy properties to all'
      ) then
         scene_graph.scene_graph_shader_manager.apply_to_scene_graph()
      end
      autogui.tooltip('copy graphic properties to all')
      imgui.SameLine()
      if imgui.SimpleButton(
         imgui.font_icon('eye')..'## copy properties to visible'
      ) then
         scene_graph.scene_graph_shader_manager.apply_to_scene_graph(true)
      end
      autogui.tooltip('copy graphic properties to visible objects')
      imgui.SameLine()
      if imgui.SimpleButton(
         imgui.font_icon('clipboard-list')..'## copy properties to selected'
      ) then
         scene_graph.scene_graph_shader_manager.apply_to_scene_graph(
            false,true
         )
      end
      autogui.tooltip('copy graphic properties to selected objects')
      imgui.PopStyleVar()

      imgui.Separator()

      if imgui.MenuItem(
          imgui.font_icon('save')..'  Save current object as...'
      ) then
          scene_graph_gui.save_grob_as(grob)
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
    if imgui.MenuItem('show selected') then
       for i=0,scene_graph.nb_children-1 do
          grob = scene_graph.ith_child(i)
          if grob.selected then
              grob.visible = true
          end
       end
       scene_graph_gui.clear_selection()
    end
    if imgui.MenuItem('hide selected') then
       for i=0,scene_graph.nb_children-1 do
          grob = scene_graph.ith_child(i)
          if grob.selected then
              grob.visible = false
          end
       end
       scene_graph_gui.clear_selection()
    end
    imgui.Separator()
    autogui.command_menu_item(
       scene_graph.I.Scene.create_object
    )
    imgui.Separator()
    if imgui.MenuItem('delete all') then
       scene_graph.I.Scene.delete_all()
    end
    if imgui.MenuItem('delete current') then
       scene_graph.I.Scene.delete_current()
    end
    if imgui.MenuItem('delete selected') then
       scene_graph_gui.delete_selected()
    end
    imgui.Separator()
    scene_graph_gui.menu_map.draw(scene_graph, nil)
    if with_file_menu then
       imgui.Separator()
       scene_graph_gui.file_menu()
    end
end

-- \brief Handles the scene-graph operations menu
function scene_graph_gui.scene_graph_ops()
  local name='Scene'
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
function scene_graph_gui.draw_object_list()
   local selection_op=nil
   local edit_op=nil

   for i=0,scene_graph.nb_children-1 do

       if i >= scene_graph.nb_children then
          break -- exit loop when deleting an object
       end

       local grob = scene_graph.ith_child(i)
       local flags = ImGuiTreeNodeFlags_DrawLinesFull |
                     ImGuiTreeNodeFlags_AllowOverlap
       if grob.selected then
          flags = flags | ImGuiTreeNodeFlags_Selected
       end
       draw_props = imgui.TreeNodeEx('##'..grob.name..'##props', flags)
       edit_op = scene_graph_gui.draw_grob_edit_list_buttons(grob)
       local selection_op = scene_graph_gui.draw_grob_name(grob)
       if imgui.BeginPopupContextItem(grob.name..'##ops') then
          grob = scene_graph_gui.grob_ops(grob)
	  imgui.EndPopup()
       end
       if grob == nil then
          break
       end
       scene_graph_gui.draw_grob_eye(grob)
       if selection_op ~= nil then
          selection_op(grob)
          selection_op = nil
       end
       if edit_op ~= nil then
          edit_op(grob)
          edit_op = nil
       end
       if grob == nil then
          break
       end
       if draw_props then
          autogui.in_popup = true
          autogui.in_tree = true
          autogui.properties_editor(grob.shader, false, true)
	  autogui.in_popup = false
          autogui.in_tree = false
          imgui.TreePop()
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
   if main.picked_grob ~= nil then
     if imgui.BeginPopupContextVoid() then
        scene_graph_gui.grob_ops(main.picked_grob)
        imgui.EndPopup()
     end
   end

   scene_graph_gui.about_window()
end

-- ----------------------------------------------------------------------------

-- \brief Draws the optional buttons to edit the list (move up/down and delete)
-- \param[in] grob one of the objects in the list
-- \return edit_op, the function to be applied on grob or nil
function scene_graph_gui.draw_grob_edit_list_buttons(grob)
   local edit_op = nil
   if not scene_graph_gui.edit_list then
      return edit_op
   end
   imgui.PushStyleVar_2(ImGuiStyleVar_ItemSpacing, 0.0, 4.0)
   imgui.SameLine()
   if imgui.SimpleButton(imgui.font_icon('window-close')..'##'..grob.name) then
       edit_op = scene_graph_gui.delete_grob
   end
   autogui.tooltip('delete')
   imgui.SameLine()
   if imgui.SimpleButton(imgui.font_icon('arrow-up')..'##'..grob.name) then
       edit_op = scene_graph_gui.move_grob_up
   end
   autogui.tooltip('move up')
   imgui.SameLine()
   if imgui.SimpleButton(imgui.font_icon('arrow-down')..'##'..grob.name) then
       edit_op = scene_graph_gui.move_grob_down
   end
   autogui.tooltip('move down')
   imgui.PopStyleVar()
   return edit_op
end

-- \brief Draws the name of a grob, handles the rename command
-- \param[in] grob one of the objects in the list
-- \return selection_op to be performed on grob or nil
function scene_graph_gui.draw_grob_name(grob)
   local btn_width  = 25 * main.scaling()
   local selection_op = nil
   imgui.SameLine()
   if grob.name == scene_graph_gui.rename_old then
      if scene_graph_gui.rename_old == scene_graph_gui.rename_new then
         imgui.SetKeyboardFocusHere()
      end
      local renamed
      imgui.PushItemWidth(-1)
      renamed,scene_graph_gui.rename_new = imgui.TextInput(
	 '##renames##'..grob.name,
	 scene_graph_gui.rename_new,
         ImGuiInputTextFlags_EnterReturnsTrue |
         ImGuiInputTextFlags_AutoSelectAll
      )
      imgui.PopItemWidth()
      if renamed then
         if scene_graph_gui.rename_new ~= '' then
            main.save_state()
            scene_graph.current_object = grob.name
            gom.add_to_history(
               gom.back_resolve(grob)..'.rename('..
                 scene_graph_gui.rename_new ..')'
            )
            grob.rename(scene_graph_gui.rename_new)
         end
	 scene_graph.current_object = grob.name
         scene_graph_gui.rename_old = nil
	 scene_graph_gui.rename_new = nil
      end
   else
      imgui.SetNextItemAllowOverlap()
      label = grob.name
      cropped = false
      szx,szy = imgui.CalcTextSize(label)
      availx = imgui.GetContentRegionAvail()
      while szx + 1.3*btn_width > availx and label ~= '' do
          label = label:sub(1,#label-1)
          cropped=true
          szx,szy = imgui.CalcTextSize(label)
      end
      if cropped then
          label = label..'...'..'##'..grob.name
      end
      if imgui.Selectable(
	 label, grob.name == scene_graph.current_object,
	 ImGuiSelectableFlags_AllowDoubleClick |
         ImGuiSelectableFlags_SelectOnNav
      ) then
	 if imgui.IsMouseDoubleClicked(0) then
              for i = 0,scene_graph.nb_children-1 do
	         scene_graph.ith_child(i).visible = false
	      end
	      grob.visible=true
	 end
	 scene_graph.current_object = grob.name
      end
      if imgui.IO_KeyShift_pressed() and imgui.IsItemFocused() then
         grob.selected=true
      end
      if cropped then
         autogui.tooltip(grob.name)
      end
   end
   if imgui.IsItemActivated() and not imgui.IsItemToggledOpen() then
       if imgui.IO_KeyCtrl_pressed() then
         selection_op = scene_graph_gui.toggle_selection
       elseif imgui.IO_KeyShift_pressed() then
         selection_op = scene_graph_gui.extend_selection
       else
         selection_op = scene_graph_gui.clear_selection
       end
   end
   return selection_op
end

-- \brief Draws the visibility toggle button on the right
-- \param[in] grob one of the objects in the list
function scene_graph_gui.draw_grob_eye(grob)
   if grob.name == scene_graph_gui.rename_old then
      return
   end
   local btn_width  = 25 * main.scaling()
   local selected_only = main.camera().draw_selected_only
   local visible = false
   if grob ~= nil then
      visible = grob.visible
      if selected_only then
          visible = (grob.name == scene_graph.current_object)
      end
   end
   local visible_changed = false
   imgui.SameLine()
   imgui.Dummy(imgui.GetContentRegionAvail()-btn_width,2)
   imgui.SameLine()
   imgui.PushStyleVar_2(ImGuiStyleVar_ItemSpacing, 0.0, 4.0)
   if visible then
       if imgui.SimpleButton(
           imgui.font_icon('eye')..'##'..grob.name
       ) and not selected_only then
           visible_changed = true
           visible = false
       end
   else
       if imgui.SimpleButton(
           imgui.font_icon('eye-slash')..'##'..grob.name
       ) and not selected_only then
           visible_changed = true
           visible = true
       end
   end
   autogui.tooltip('show/hide')
   imgui.PopStyleVar()
   if visible_changed and grob ~= nil then
      grob.visible=visible
      scene_graph.current_object = grob.name
   end
end

-- -----------------------------------------------------------------------

function scene_graph_gui.delete_grob(grob)
   if grob ~= nil then
      scene_graph.current_object = grob.name
      scene_graph.delete_current_object()
   end
end

function scene_graph_gui.move_grob_up(grob)
   if grob ~= nil then
      scene_graph.current_object = grob.name
      scene_graph.move_current_up()
   end
end

function scene_graph_gui.move_grob_down(grob)
   if grob ~= nil then
      scene_graph.current_object = grob.name
      scene_graph.move_current_down()
   end
end

-- -----------------------------------------------------------------------

-- \brief toggles the selection flag for a given object
-- \param grob the object
function scene_graph_gui.toggle_selection(grob)
   if grob ~= nil then
      grob.selected = not grob.selected
   end
end

-- \brief expand selection from current object to a given object
-- \param grob the object towards which the selection should be expanded
-- \details on exit, all objects between the current one and \p grob are
--   selected. The selection flags of the other objects are left unchanged.
function scene_graph_gui.extend_selection(grob)
   if grob == nil then
      return
   end
   cur_index = -1
   grob_index = -1
   for i=0,scene_graph.nb_children-1 do
       cur_name = scene_graph.ith_child(i).name
       if cur_name == scene_graph.current_object then
          cur_index = i
       end
       if cur_name == grob.name then
          grob_index = i
       end
   end
   if cur_index ~= -1 and grob_index ~= -1 then
      for i = math.min(cur_index,grob_index),math.max(cur_index,grob_index) do
         scene_graph.ith_child(i).selected = true
      end
   end
end

-- \brief Resets the selection flag for all object of the SceneGraph
function scene_graph_gui.clear_selection()
   for i=0,scene_graph.nb_children-1 do
      scene_graph.ith_child(i).selected = false
   end
end

-- \brief Deletes all the selected objects of the SceneGraph
function scene_graph_gui.delete_selected()
  main.save_state()
  main.picked_grob = nil
  local changed = true
  while changed do
     changed = false
     for i=0,scene_graph.nb_children-1 do
        grob = scene_graph.ith_child(i)
        if grob.selected then
            main.save_state()
            main.picked_grob = nil
            scene_graph.current_object = grob.name
            scene_graph.delete_current_object()
            changed = true
            break
        end
     end
  end
end
