--  GUI for the scene-graph, for use with Skin_imgui
----------------------------------------------------

-- All types and functions related with the scene graph.
scene_graph_gui = {}
scene_graph_gui.name = 'Scene'
scene_graph_gui.icon = '@cubes'
scene_graph_gui.edit_list = false -- true when up/down/delete btn are visible
scene_graph_gui.picked_grob = nil

gom.execute_file("scene_graph_menu_map.lua")

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

   local btn_width = autogui.button_size
   if not main_menu then
      imgui.Separator()
      imgui.PushStyleVar_2(ImGuiStyleVar_ItemSpacing, 0.0, 4.0)
      imgui.TextDisabled('Edit...   ')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('edit')..'##  rename') then
         scene_graph_gui.rename_old = name
         scene_graph_gui.rename_new = name
      end
      autogui.tooltip('rename')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('clone')..'##  duplicate') then
         local dup = scene_graph.duplicate_object{
             grob=grob,_invoked_from_gui=true
         }
         scene_graph_gui.rename_old = dup.name
         scene_graph_gui.rename_new = dup.name
         scene_graph_gui.set_current(dup)
      end
      autogui.tooltip('duplicate')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('window-close')..'##  delete') then
         main.picked_grob = nil
         scene_graph.delete_object{grob=grob,_invoked_from_gui=true}
      end
      autogui.tooltip('delete')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('sort-up')..'##move to top') then
         scene_graph.move_object_to_top{grob=grob,_invoked_from_gui=true}
      end
      autogui.tooltip('move to top')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('arrow-up')..'##move up') then
         scene_graph.move_object_up{grob=grob,_invoked_from_gui=true}
      end
      autogui.tooltip('move up')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('arrow-down')..'##move down') then
         scene_graph.move_object_down{grob=grob,_invoked_from_gui=true}
      end
      autogui.tooltip('move down')
      imgui.SameLine()
      if imgui.SimpleButton(imgui.font_icon('sort-down')..'##move to bot') then
         scene_graph.move_object_to_bottom{grob=grob,_invoked_from_gui=true}
      end
      autogui.tooltip('move to bottom')
      imgui.SameLine()
      imgui.PopStyleVar()

      imgui.Separator()

      imgui.PushStyleVar_2(ImGuiStyleVar_ItemSpacing, 0.0, 4.0)
      imgui.TextDisabled('Gfx...      ')
      imgui.SameLine()
      if imgui.SimpleButton(
         imgui.font_icon('eye-slash')..'## show/hide'
      ) then
         if grob.visible then
            scene_graph.I.Graphics.hide_object{
               grob=grob,_invoked_from_gui=true
            }
         else
            scene_graph.I.Graphics.show_object{
               grob=grob,_invoked_from_gui=true
            }
         end
      end
      if camera.draw_selected_only then
         autogui.tooltip('show/hide disabled in selected only mode')
      else
         autogui.tooltip('show/hide')
      end
      imgui.SameLine()
      if imgui.SimpleButton(
         imgui.font_icon('cubes')..'## copy properties to all'
      ) then
         scene_graph.I.Graphics.copy_object_properties_to_all{
            grob=grob,_invoked_from_gui=true
         }
      end
      autogui.tooltip('copy graphic properties to all')
      imgui.SameLine()
      if imgui.SimpleButton(
         imgui.font_icon('eye')..'## copy properties to visible'
      ) then
         scene_graph.I.Graphics.copy_object_properties_to_visible{
            grob=grob,_invoked_from_gui=true
         }
      end
      autogui.tooltip('copy graphic properties to visible objects')
      imgui.SameLine()
      if imgui.SimpleButton(
         imgui.font_icon('clipboard-list')..'## copy properties to selected'
      ) then
         scene_graph.I.Graphics.copy_object_properties_to_selected{
            grob=grob,_invoked_from_gui=true
         }
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
    autogui.command_menu_item(scene_graph.I.Graphics.show_all)
    autogui.command_menu_item(scene_graph.I.Graphics.hide_all)
    autogui.command_menu_item(scene_graph.I.Graphics.show_selected)
    autogui.command_menu_item(scene_graph.I.Graphics.hide_selected)
    imgui.Separator()
    autogui.command_menu_item(scene_graph.create_object)
    imgui.Separator()
    autogui.command_menu_item(scene_graph.delete_all)
    autogui.command_menu_item(scene_graph.delete_current)
    autogui.command_menu_item(scene_graph.delete_selected)
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
     scene_graph.load_object{
        filename=filename, change_cwd=true, _invoked_from_gui=true
     }
  end

  sel,filename = imgui.FileDialog('##scene_graph##save_dlg',filename)
  if sel then
     scene_graph.save{filename=filename,_invoked_from_gui=true}
  end

  sel,filename = imgui.FileDialog('##scene_graph##save_view_dlg',filename)
  if sel then
     scene_graph.save_viewer_properties{filename=filename,_invoked_from_gui=true}
  end
end


-- \brief Draws the scene-graph and object list
-- \details Handles context-menus
function scene_graph_gui.draw_object_list()
   local selection_op=nil
   local edit_op=nil
   local picked_grob=nil

   -- Detect if object was just picked (to scroll there if list is looooonnng)
   if main.picked_grob ~= scene_graph_gui.picked_grob then
       scene_graph_gui.picked_grob = main.picked_grob
       picked_grob = main.picked_grob
   end

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
       if grob == picked_grob then
          imgui.SetScrollHereY()
       end
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
   if grob.selected then
      autogui.tooltip('delete selected')
   else
      autogui.tooltip('delete this object')
   end
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
   local btn_width = autogui.button_size
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
            scene_graph.rename_object{
                grob=grob, new_name=scene_graph_gui.rename_new,
                _invoked_from_gui=true
            }
         end
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
            scene_graph.I.Graphics.show_only{grob=grob,_invoked_from_gui=true}
         end
         scene_graph_gui.set_current(grob)
      end
      if imgui.IO_KeyShift_pressed() and
         imgui.IsItemFocused() and not grob.selected then
           scene_graph.I.Selection.select_object{
              grob=grob,_invoked_from_gui=true
           }
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
   local btn_width = autogui.button_size
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
   if camera.draw_selected_only then
      autogui.tooltip('show/hide disabled in selected only mode')
   else
      if grob.selected then
         autogui.tooltip('show/hide selected')
      else
         autogui.tooltip('show/hide')
      end
   end
   imgui.PopStyleVar()
   if visible_changed and grob ~= nil then
      if visible then
         if grob.selected then
            scene_graph.I.Graphics.show_selected{_invoked_from_gui=true}
         else
            scene_graph.I.Graphics.show_object{
               grob=grob,_invoked_from_gui=true
            }
         end
      else
         if grob.selected then
            scene_graph.I.Graphics.hide_selected{_invoked_from_gui=true}
         else
            scene_graph.I.Graphics.hide_object{
               grob=grob,_invoked_from_gui=true
            }
         end
      end
   end
end

-- -----------------------------------------------------------------------
-- Some wrappers around SceneGraph functions

function scene_graph_gui.delete_grob(grob)
   if grob ~= nil then
      if grob.selected then
         scene_graph.delete_selected{_invoked_from_gui=true}
      else
         scene_graph.delete_object{grob=grob,_invoked_from_gui=true}
      end
   end
end

function scene_graph_gui.move_grob_up(grob)
   if grob ~= nil then
      scene_graph.move_object_up{grob=grob,_invoked_from_gui=true}
   end
end

function scene_graph_gui.move_grob_down(grob)
   if grob ~= nil then
      scene_graph.move_object_down{grob=grob,_invoked_from_gui=true}
   end
end

-- \brief sets the current object in the scene graph
-- \param[in] grob the object to be set as current
function scene_graph_gui.set_current(grob)
   if grob ~= nil then
      if grob.name ~= scene_graph.current_object then
         scene_graph.set_current{grob=grob,_invoked_from_gui=true}
      end
   end
end

-- \brief toggles the selection flag for a given object
-- \param grob the object
function scene_graph_gui.toggle_selection(grob)
   if grob ~= nil then
      scene_graph.I.Selection.toggle_selection{
          grob=grob,_invoked_from_gui=true
      }
   end
end

-- \brief expand selection from current object to a given object
-- \param grob the object towards which the selection should be expanded
-- \details on exit, all objects between the current one and \p grob are
--   selected. The selection flags of the other objects are left unchanged.
function scene_graph_gui.extend_selection(grob)
   if grob ~= nil then
      scene_graph.I.Selection.extend_selection{
         grob=grob, _invoked_from_gui=true
      }
   end
end

-- \brief Resets the selection flag for all object of the SceneGraph
function scene_graph_gui.clear_selection()
   -- test to avoid littering history
   if scene_graph.I.Selection.nb_selected() ~= 0 then
      scene_graph.I.Selection.clear_selection{_invoked_from_gui=true}
   end
end

-- \brief Deletes all the selected objects of the SceneGraph
function scene_graph_gui.delete_selected()
   scene_graph.delete_selected{_invoked_from_gui=true}
end

----------------------------------------------------------------------------
