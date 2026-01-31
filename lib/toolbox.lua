--  Tools for graphite with Skin_imgui
--------------------------------------

toolbox_gui = {}
toolbox_gui.visible = false
toolbox_gui.name = 'Toolbox'
toolbox_gui.icon = '@wrench'
toolbox_gui.x = main.width-3*main.margin()-150*main.scaling()
toolbox_gui.y = main.margin()
toolbox_gui.w = 165*main.scaling()
toolbox_gui.h = 180*main.scaling()
toolbox_gui.resizable = true

function toolbox_gui.draw_window()
    toolbox_gui.draw_tools(false)
end

function toolbox_gui.draw_tools(viewer_tools)
      local size = 23 * main.scaling()

      -- Make buttons not too much packed
      imgui.PushStyleVar_2(ImGuiStyleVar_FramePadding, 4.0, 3.0)

      if scene_graph.current() ~= nil then

            -- Lighten background of buttons for tools selection else
	    -- we cannot see them.
	    local pushed_color = false
	    if gom.get_environment_value('gui:style') == 'Dark' then
   	       imgui.PushStyleColor_2(ImGuiCol_Button, 0.5, 0.5, 0.75, 1.0)
	       pushed_color = true
	    end

            -- Display currently selected tool
            if viewer_tools then
               -- Change color a bit as compared to tool buttons
               if gom.get_environment_value('gui:style') == 'Dark' then
   	          imgui.PushStyleColor_2(ImGuiCol_Button, 0.25, 0.25, 0.30, 1.0)
               else
   	          imgui.PushStyleColor_2(ImGuiCol_Button, 0.9, 0.9, 0.9, 1.0)
               end
	       local mclass = tools.current().meta_class
	       local icon_name = 'tool'
	       if mclass.has_custom_attribute('icon') then
  	          icon_name = mclass.custom_attribute_value('icon')
               end
               toolbox_gui.tool_button('toolbox_showtool', icon_name)
               imgui.SameLine()
               imgui.Dummy(size/3.0,size)
               imgui.SameLine()
               imgui.PopStyleColor()
            end

	    local grob_class_name = scene_graph.current().meta_class.name
	    local tool_class_names = gom.get_environment_value(
	       grob_class_name .. "_tools"
	    )

            -- toolbox_gui.tool_button('test','@home')

            for tool_class_name in string.split(tool_class_names,";") do
               local mclass = gom.resolve_meta_type(tool_class_name)

	       local icon_name = 'tool'
	       if mclass.has_custom_attribute('icon') then
	          icon_name = mclass.custom_attribute_value('icon')
               end

               local category = ''
               if mclass.has_custom_attribute('category') then
                  category = mclass.custom_attribute_value('category')
               end

               if (viewer_tools and category == 'viewer') or
                  (not viewer_tools and category ~= 'viewer') then

	          if imgui.GetContentRegionAvail() < size then
	             imgui.NewLine()
		     imgui.Spacing()
	          end

                  if toolbox_gui.tool_button(
                         'toolbox_'..icon_name, icon_name
                  ) then
                     tools.tool(tool_class_name)
                  end
	          autogui.tooltip(autogui.help(mclass))

	          if mclass.nb_properties() > OGF.Tool.nb_properties() then
		     if imgui.BeginPopupContextItem(
                        tool_class_name.."##ops") then
	                tools.tool(tool_class_name)
		        autogui.properties_editor(tools.current())
	                imgui.EndPopup()
	             end
	          end
   	          imgui.SameLine()
               end
	    end

	    if pushed_color then
               imgui.PopStyleColor(1)
	    end

	 end
         imgui.PopStyleVar()
end

function toolbox_gui.tool_button(id, icon)
    local size = 23 * main.scaling()
    if icon:starts_with('@') then
       return imgui.Button(
          imgui.font_icon(icon:sub(2))..'##'..id,
          size+7, size+3
       )
    else
       return imgui.ImageButton(
          id, main.resolve_icon('tools/'..icon,true), size, size
       )
    end
end

graphite_main_window.add_module(toolbox_gui)

--------------------------------------------------------

tools = OGF.SceneGraphToolsManager.create(main.render_area)

ray_pick=OGF.RayPicker.create()

gom.connect(
   main.render_area.mouse_down, ray_pick.grab
).if_arg('control', 'false').if_arg('shift', 'false')


-- When grob selection tool is active, and when using
-- <ctrl> to move the camera, ensure that when releasing
-- right mouse one does not get the pulldown context menu
gom.connect(
   main.render_area.mouse_down,
   function()
      main.picked_grob = nil
   end
).if_arg('control', 'true')

gom.connect(
   main.render_area.mouse_move, ray_pick.drag
).if_arg('control', 'false').if_arg('shift', 'false')

gom.connect(
   main.render_area.mouse_up, ray_pick.release
).if_arg('control', 'false').if_arg('shift', 'false')

gom.connect(ray_pick.ray_grab,   tools.grab   )
gom.connect(ray_pick.ray_drag,   tools.drag   )
gom.connect(ray_pick.ray_release,tools.release)

gom.connect(
   scene_graph.scene_graph_shader_manager.focus_changed,
   tools.focus
)

----------------------------------------------------------
