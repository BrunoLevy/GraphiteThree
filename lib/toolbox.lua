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
      graphite_gui.right_pane_width = imgui.GetWindowWidth()
      if scene_graph.current() ~= nil then

            -- Lighten background of buttons for tools selection else
	    -- we cannot see them.
	    local pushed_color = false
	    if gom.get_environment_value('gui:style') == 'Dark' then
   	       imgui.PushStyleColor_2(ImGuiCol_Button, 0.5, 0.5, 0.75, 1.0)
	       pushed_color = true
	    end

	    local size = 23 * main.scaling()
	    imgui.Text('current:')
	    imgui.SameLine()
	    local mclass = tools.current().meta_class
	    local icon_name = 'tool'
	    if mclass.has_custom_attribute('icon') then
  	       icon_name = mclass.custom_attribute_value('icon')
            end

            -- Tool display needs a light background. If the dark
	    -- style is used, use a button instead of an image.
	    if gom.get_environment_value('gui:style') == 'Dark' then
	       imgui.ImageButton(
	          'toolbox_showtool',
	          main.resolve_icon('tools/'..icon_name,true),
	          size,size
	       )
	    else
	       imgui.Image(
	          main.resolve_icon('tools/'..icon_name,true),
	          size,size
	       )
	    end

	    imgui.Spacing()
	    imgui.Separator()
	    imgui.Spacing()

	    local grob_class_name = scene_graph.current().meta_class.name
	    local tool_class_names = gom.get_environment_value(
	       grob_class_name .. "_tools"
	    )

            for tool_class_name in string.split(tool_class_names,";") do
               local mclass = gom.resolve_meta_type(tool_class_name)

	       local icon_name = 'tool'
	       if mclass.has_custom_attribute('icon') then
	          icon_name = mclass.custom_attribute_value('icon')
               end

	       if imgui.ImageButton(
	          'toolbox_'..icon_name,
	          main.resolve_icon('tools/'..icon_name, true),
		  size,size
	       ) then
	          tools.tool(tool_class_name)
	       end
	       autogui.tooltip(autogui.help(mclass))

	       if mclass.nb_properties() > OGF.Object.nb_properties() then
		  if imgui.BeginPopupContextItem(tool_class_name.."##ops") then
	             tools.tool(tool_class_name)
		     autogui.properties_editor(tools.current())
	             imgui.EndPopup()
	          end
	       end

	       imgui.SameLine()
	       if imgui.GetContentRegionAvail() < size then
	          imgui.NewLine()
		  imgui.Spacing()
	       end

	    end

	    if pushed_color then
               imgui.PopStyleColor(1)
	    end

	 end
end

graphite_main_window.add_module(toolbox_gui)

--------------------------------------------------------

tools = OGF.SceneGraphToolsManager.create(main.render_area)

ray_pick=OGF.RayPicker.create()

gom.connect(
   main.render_area.mouse_down, ray_pick.grab
).if_arg('control', 'false').if_arg('shift', 'false')

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
