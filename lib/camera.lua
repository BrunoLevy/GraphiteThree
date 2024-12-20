--  Camera for graphite with Skin_imgui
----------------------------------------------------

xform = OGF.Transform3d.create()
arcball = OGF.ArcBall.create()
translation = OGF.Translation.create()

-- We bind the camera to a global variable so
-- that scene-graph can save camera position
-- and clipping config.
-- Note that xform is also accessed in the code
-- that saves global viewing parameters.

-- see OGF/skin/types/events.h for mouse button mapping

camera = main.camera()

gom.connect(
   xform.rotation_changed, arcball.set_property
).add_arg('name','value')

gom.connect(xform.value_changed, main.render_area.set_property)
   .add_arg('name', 'viewing_matrix')

gom.connect(main.render_area.mouse_down, arcball.grab)
   .if_arg('button', 3)
   .if_arg('control', true)
   .if_arg('shift', false)
   .rename_arg('point_ndc', 'value')

gom.connect(main.render_area.mouse_move, arcball.drag)
   .if_arg('button', 3)
   .if_arg('control', true)
   .if_arg('shift', false)
   .rename_arg('point_ndc', 'value')

gom.connect(main.render_area.mouse_up, arcball.release)
   .if_arg('button', 3)
   .if_arg('control',true)
   .if_arg('shift', false)
   .rename_arg('point_ndc', 'value')

gom.connect(arcball.value_changed, xform.set_property)
  .add_arg('name','rotation_matrix')

gom.connect(main.render_area.mouse_down, translation.grab)
   .if_arg('button', 1)
   .if_arg('control', true)
   .if_arg('shift', false)
   .rename_arg('point_ndc', 'value')

gom.connect(main.render_area.mouse_move, translation.drag)
   .if_arg('button', 1)
   .if_arg('control', true)
   .if_arg('shift', false)
   .rename_arg('point_ndc', 'value')

gom.connect(main.render_area.mouse_up, translation.release)
   .if_arg('button', 1)
   .if_arg('control',true)
   .if_arg('shift', false)
   .rename_arg('point_ndc', 'value')

gom.connect(translation.value_changed, xform.translate)

gom.connect(main.render_area.mouse_move, xform.zoom_in)
   .if_arg('button', 2)
   .if_arg('control', true)
   .if_arg('shift', false)
   .rename_arg('delta_y_ndc', 'value')

gom.connect(main.render_area.mouse_move, xform.zoom_in)
   .if_arg('button', 4)
   .if_arg('control', true)
   .if_arg('shift', false)
   .rename_arg('delta_y_ndc', 'value')

gom.connect(main.render_area.mouse_move, xform.zoom_in)
   .if_arg('button', 5)
   .if_arg('control', true)
   .if_arg('shift', false)
   .rename_arg('delta_y_ndc', 'value')

camera_gui = {}
camera_gui.name = 'Camera'
camera_gui.icon = '@camera'
camera_gui.x = 3*main.margin()+150*main.scaling()+5
camera_gui.y = main.margin()
camera_gui.w = 220*main.scaling()
camera_gui.h = 300*main.scaling()

function camera_gui.backgrounds()

     local size = 2*autogui.icon_size()

     if imgui.ImageButton(
       'cam_bkgnd_white',
       main.resolve_icon('backgrounds/white'),
       size,size
     ) then
	 main.set_style("Light")
     end
     imgui.SameLine()
     if imgui.ImageButton(
       'cam_bkgnd_blue-white',
       main.resolve_icon('backgrounds/blue-white'),
       size,size
     ) then
	 main.set_style("Light")
         main.render_area.background_color_1 = '0.2 0.2 1 1'
         main.render_area.background_color_2 = '1 1 1 1'
     end
     imgui.SameLine()
     if imgui.ImageButton(
       'cam_bkgnd_blue-black',
       main.resolve_icon('backgrounds/blue-black'),
       size,size
     ) then
	 main.set_style("Dark")
         main.render_area.background_color_1 = '0 0 0.5 1'
         main.render_area.background_color_2 = '0 0 0 1'
     end
     imgui.SameLine()
     if imgui.ImageButton(
       'cam_bkgnd_black',
       main.resolve_icon('backgrounds/black'),
       size,size
     ) then
	 main.set_style("Dark")
     end

     imgui.PushItemWidth(size*4)
     local sel,style=autogui.combo_box_value(
        'style',
	main.get_style(),
	gom.get_environment_value('gui_styles')
     )
     if sel then
        main.set_style(style)
     end
     imgui.PopItemWidth()

     imgui.EndMenu()
end

camera_gui.snapshot_filename = ''

function camera_gui.draw_menu()
  if imgui.Button(imgui.font_icon('home')..' Home',-1,0) then
     camera_gui.home()
  end
  imgui.Separator()
  if imgui.BeginMenu('Properties...') then
     autogui.properties_editor(main.camera())
     imgui.EndMenu()
  end
  if imgui.BeginMenu('Background...') then
     camera_gui.backgrounds()
  end
  imgui.Separator()
  if imgui.BeginMenu('File...') then
     if imgui.MenuItem('Load...##scene_graph') then
         local extensions = 'graphite;graphite_ascii'
    	 imgui.OpenFileDialog(
	     '##scene_graph##load_dlg',
	     extensions,
	     '', -- default filename
	     ImGuiExtFileDialogFlags_Load
	 )
     end
     if imgui.MenuItem('Save snapshot as...') then
       local extensions =
           gom.get_environment_value('image_write_extensions')
       extensions = string.gsub(extensions,'*.','')
       imgui.OpenFileDialog(
          '##camera##snapshot_dlg',
          extensions,
          '', -- default filename
          ImGuiExtFileDialogFlags_Save
       )
     end
     if imgui.MenuItem('Save view as...##scene_graph') then
         local extensions = 'graphite;graphite_ascii'
     	 imgui.OpenFileDialog(
	    '##scene_graph##save_view_dlg',
	    extensions,
	    '', -- default filename
	    ImGuiExtFileDialogFlags_Save
	 )
     end
     imgui.EndMenu()
  end
  imgui.Separator()
  imgui.NewLine()
  camera_gui.projection_dialog()
  imgui.NewLine()
end

function camera_gui.draw_extra()
  local sel
  sel,camera_gui.snapshot_filename = imgui.FileDialog(
     '##camera##snapshot_dlg',camera_gui.snapshot_filename
  )
  if sel then
     main.render_area.snapshot(camera_gui.snapshot_filename)
  end
end

function camera_gui.draw_window()
   -- second arg = false: called_from_inspect
   --  third arg =  true: no_windowify
   autogui.properties_editor(main.camera(),false,true)
end

autogui.handlers['OGF::ClippingConfig'] =
         function(object, property_name, mtype, tooltip)

   local value = object[property_name]
   local words={}
   for word in string.split(value,';') do
      words[#words+1] = word
   end
   local active = (words[1] == 'true')
   local axis = words[2]
   local volume_mode = words[3]
   local shift = tonumber(words[4])
   local rotation = words[5]
   local invert = (words[6] == 'true')

   local sel1,sel2,sel3,sel4,sel6,sel7

   sel1,active = imgui.Checkbox('##active##'..property_name,active)
   imgui.SameLine()
   imgui.Text(property_name)
   autogui.tooltip(tooltip)

   if active then
      imgui.PushItemWidth(main.scaling()*90)
      sel2,axis = autogui.combo_box_value(
       '##axis##'..property_name,axis,'x;y;z;d'
      )
      imgui.SameLine()
      sel3,volume_mode = autogui.combo_box_value(
          '##volume_mode##'..property_name,volume_mode,'std.;cell;strad.;slice'
      )
      imgui.PopItemWidth()
      imgui.PushItemWidth(main.scaling()*80)
      sel4, shift = imgui.SliderInt(
            '##shift##'..property_name,shift,-300,300,''
      )
      imgui.PopItemWidth()
      imgui.SameLine()
      sel6,invert = imgui.Checkbox('##invert##'..property_name,invert)
      imgui.SameLine()
      if invert then
         imgui.Image(
	    main.resolve_icon('flop',true),
	    autogui.icon_size(),autogui.icon_size()
	 )
      else
         imgui.Image(
	    main.resolve_icon('flip',true),
	    autogui.icon_size(),autogui.icon_size()
	 )
      end
      imgui.PushItemWidth(-1)
      sel7, shift = imgui.InputInt(
            '##shift2##'..property_name,shift
      )
      imgui.PopItemWidth()
   end

   if sel1 or sel2 or sel3 or sel4 or sel6 or sel7 then
       local result =
            tostring(active)..';'
	     ..axis..';'
	     ..volume_mode..';'
             ..tostring(shift)..';'
	     ..rotation..';'
	     ..tostring(invert)
       object[property_name] = result
   end
end

function camera_gui.home()
    scene_graph.scene_graph_shader_manager.update_focus()
    xform.reset()
end

function camera_gui.projection(axis)
    camera_gui.home()
    if(    axis == '+X') then
       xform.rotation_matrix = ' 0 0  1 0   0 1  0 0  -1  0  0 0   0 0 0 1'
    elseif(axis == '-X') then
       xform.rotation_matrix = ' 0 0 -1 0   0 1  0 0   1  0  0 0   0 0 0 1'
    elseif(axis == '+Y') then
       xform.rotation_matrix = ' 1 0  0 0   0 1  0 0   0  0  1 0   0 0 0 1'
    elseif(axis == '-Y') then
       xform.rotation_matrix = '-1 0  0 0   0 1  0 0   0  0 -1 0   0 0 0 1'
    elseif(axis == '+Z') then
       xform.rotation_matrix = ' 1 0  0 0   0 0  1 0   0 -1  0 0   0 0 0 1'
    elseif(axis == '-Z') then
       xform.rotation_matrix = ' 1 0  0 0   0 0 -1 0   0  1  0 0   0 0 0 1'
    end
end

function camera_gui.projection_dialog()
    local r,g,b

    if gom.get_environment_value('gui:style') == 'Dark' then
       r = 0xFF8080FF
       g = 0xFF80FF80
       b = 0xFFFF8080
    else
       r = 0xFF0000FF
       g = 0xFF00FF00
       b = 0xFFFF0000
    end

    local ImGuiStyleVar_FrameRounding = 12
    imgui.PushStyleVar(ImGuiStyleVar_FrameRounding,0)

    function axis_button(axis,color)
       imgui.PushStyleColor(ImGuiCol_Text,color)
       if imgui.Button(axis,30,0) then
          camera_gui.projection(axis)
       end
       imgui.PopStyleColor()
    end

    local offset = 45
        if(imgui.GetWindowWidth() > 20) then
       offset = offset + (imgui.GetWindowWidth() - 120)/2
    end

    imgui.Text(' ')
    imgui.SameLine(offset)
    axis_button('+Z',b)
    imgui.NewLine()

    imgui.SameLine(offset-37)
    axis_button('-X',r)
    imgui.SameLine(offset)
    axis_button('+Y',g)
    imgui.SameLine(offset+37)
    axis_button('+X',r)

    imgui.Text(' ')
    imgui.SameLine(offset)
    axis_button('-Z',b)

    imgui.Text(' ')
    imgui.SameLine(offset)
    axis_button('-Y',g)

    imgui.PopStyleVar()
end

graphite_main_window.add_module(camera_gui)