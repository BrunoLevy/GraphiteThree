-- A Graphite module for displaying histograms.

Stats = {}
Stats.visible = true
Stats.name = 'Statistics'
Stats.icon = '@chart-bar'
Stats.help = 'Properties statistics and histogram'
Stats.x = 100
Stats.y = 400
Stats.w = 300
Stats.h = 600
Stats.bins = 500

Stats.histo = {}
Stats.color = true
Stats.dirty = false

function Stats.reset()
   Stats.histo = {}
   Stats.histo[0]     = 0
   Stats.histo[1]     = 0
   Stats.histo_min    = 0
   Stats.histo_max    = 1
   Stats.histo_max_display = nil
   Stats.object_name  = ''
   Stats.attrib_name  = ''
   Stats.attrib       = nil
   Stats.attrib_index = 0
   Stats.attrib_min   = 0
   Stats.attrib_max   = 1
   Stats.initialized  = false
   Stats.dragging     = false
   Stats.sel1 = 0
   Stats.sel2 = 0
   Stats.dirty = false
   Stats.mean = 0
end

function Stats.update_histo()
   if not Stats.visible then
      Stats.dirty = true
      return
   end
   if Stats.attrib == nil then
      Stats.reset()
      return
   end
   if #Stats.attrib <= 0 then
      Stats.reset()
      return
   end
   Stats.histo = {}
   for i = 1,Stats.bins do
      Stats.histo[i] = 0
   end
   Stats.mean = 0
   if Stats.attrib.element_meta_type.name == 'double' then
     for i = Stats.attrib_index,#Stats.attrib-1,Stats.attrib.dimension do
       local val = Stats.attrib[i]
       Stats.mean = Stats.mean + val
       val = (val - Stats.attrib_min) /
	     (Stats.attrib_max - Stats.attrib_min)
       val = math.floor((#Stats.histo)*val)
       if val >= 0 and val < #Stats.histo then
          Stats.histo[val+1] = Stats.histo[val+1]+1
       end
     end
   end
   Stats.mean = Stats.mean / (#Stats.attrib*Stats.attrib.dimension)
   Stats.histo_min = Stats.histo[1]
   Stats.histo_max = Stats.histo[1]
   for i = 2,#Stats.histo do
      Stats.histo_min = math.min(Stats.histo_min, Stats.histo[i])
      Stats.histo_max = math.max(Stats.histo_max, Stats.histo[i])
   end
   Stats.histo_min = math.max(Stats.histo_min, 0.0)
   if Stats.histo_max_display == nil then
      Stats.histo_max_display = Stats.histo_max
   end
   Stats.initialized = true
   Stats.dirty = false
end

function Stats.update()

   local changed = false
   local object = scene_graph.current()

   if object == nil then
      Stats.reset()
      return
   end

   if object.name ~= Stats.object_name then
      Stats.object_name = object.name
      Stats.histo_max_display = nil
      changed = true
   end

   local shd = object.shader

   if shd.meta_class.name ~= 'OGF::PlainMeshGrobShader' or
      shd.painting        ~= 'ATTRIBUTE'
   then
       Stats.reset()
       return
   end

   local attrib_name  = shd.attribute
   local attrib_index = 0
   local container_attrib_name
   local pos = attrib_name:find('%[')
   if pos ~= nil then
      attrib_index = attrib_name:sub(pos+1,-2)
      attrib_index = math.floor(tonumber(attrib_index))
      container_attrib_name  = attrib_name:sub(1,pos-1)
   else
      container_attrib_name  = attrib_name
   end
   if attrib_name  ~= Stats.attrib_name then
      Stats.histo_max_display = nil
      Stats.attrib_name = attrib_name
      Stats.attrib_index = attrib_index
      changed = true
   end

   Stats.attrib = object.I.Editor.find_attribute(container_attrib_name)

   local f = tostring(shd.colormap):split(';')
   local colormap_name = f()
   local smooth  = f()
   local mult    = math.max(tonumber(f()),1.0)
   local show    = f()
   local flipped = (f() == 'true')
   local colormap = main.resolve_icon(
       'colormaps/'..colormap_name
   )
   if colormap ~= Stats.colormap then
      Stats.colormap = colormap
      changed = true
   end
   if flipped ~= Stats.colormap_flipped then
      Stats.colormap_flipped = flipped
      changed = true
   end
   if mult ~= Stats.colormap_mult then
      Stats.colormap_mult = mult
      changed = true
   end

   if shd.attribute_min ~= Stats.attrib_min or
      shd.attribute_max ~= Stats.attrib_max then
      Stats.attrib_min = shd.attribute_min
      Stats.attrib_max = shd.attribute_max
      changed = true
   end

   if changed or Stats.dirty then
      Stats.update_histo()
   end
end

-- Transforms an (index, value) pair to screen space
-- \param[in] i the index, in 0 .. #Stats.histo + 1
-- \param[in] val the value (in general, Stats.histo[i])
-- \return x,y,t where (x,y) are the screen coordinates and
--                t the texture coordinate.
function Stats.transform(i,val)
    local d  = (val - Stats.histo_min) /
              (Stats.histo_max_display - Stats.histo_min)
    local dx = Stats.left_prop *
              (Stats.size_x - Stats.left_margin - Stats.right_margin) * d
    local t  = i / (#Stats.histo+1)
    local dy = (Stats.size_y - Stats.top_margin - Stats.bottom_margin) *
               (1.0 - t)
    if Stats.colormap_flipped then
       t = 1.0 - t
    end
    if Stats.colormap_mult > 1 then
       t = (math.floor(10000.0*t * Stats.colormap_mult) % 10000) / 9999.0
    end
    return
        Stats.pos_x+Stats.left_margin+dx,
	Stats.pos_y+Stats.top_margin+dy,
	t
end

function Stats.index_to_val(i)
    return Stats.attrib_min + (i-1)/#Stats.histo *
             (Stats.attrib_max - Stats.attrib_min)
end

function Stats.draw_marker(i,val)
   local draw_list      = imgui.GetWindowDrawList()
   local frame_color = math.floor(imgui.GetColorU32(ImGuiCol_Separator,1.0))
   local text_color  = math.floor(imgui.GetColorU32(ImGuiCol_Text,1.0))

   local s = string.format('%0.3f',val)
   local tx,ty = imgui.CalcTextSize(s)

   local x1,y1 = Stats.transform(i, Stats.histo_min)
   local x2 = x1 + Stats.size_x
           - Stats.left_margin - Stats.right_margin - tx - ty
   imgui.AddLine(draw_list, x1, y1, x2, y1, text_color, 2)
   imgui.AddText(draw_list, x2+ty, y1-0.5*ty,  text_color, s)
end

function Stats.selection_handler()
   local dragging = false
   local mi,y1,y2
   _,y1 = Stats.transform(0, 0.0)
   _,y2 = Stats.transform(#Stats.histo+1, 0.0)
   _,mi = imgui.GetMousePos()
   mi = math.floor((#Stats.histo+1)*(mi-y1)/(y2-y1))
   local hovering_histo = (mi >= 1 and mi <= #Stats.histo)
   mi = math.max(mi,1)
   mi = math.min(mi,#Stats.histo)
   if Stats.dragging then
      Stats.sel2 = mi
      if not imgui.IsMouseDown(0) then
         Stats.dragging = false
      end
   end
   if imgui.IsItemHovered() then
      if not Stats.dragging and imgui.IsMouseClicked(0) then
         Stats.sel1 = mi
         Stats.sel2 = mi
	 Stats.dragging = true
      end
      if hovering_histo then
         if imgui.BeginTooltip() then
            imgui.Text(
	       string.format('value: %0.3f',Stats.index_to_val(mi)) ..
	       '\nnb samples: ' ..
	       tostring(Stats.histo[mi])
	    )
            imgui.EndTooltip()
         end
      end
   end
   if Stats.sel1 ~= Stats.sel2 then
      _,y1 = Stats.transform(math.max(Stats.sel1, Stats.sel2),0.0)
      _,y2 = Stats.transform(math.min(Stats.sel1, Stats.sel2),0.0)
      imgui.AddRectFilled(
         imgui.GetWindowDrawList(),
         Stats.pos_x + Stats.left_margin, y1,
	 Stats.pos_x + Stats.size_x
	    - Stats.left_margin - 0.5*Stats.right_margin, y2,
	 math.floor(imgui.GetColorU32(ImGuiCol_DockingPreview,1.0)),
	 main.scaling()*10.0, (2+8)
      )
   end
end

function Stats.draw_window()
   if scene_graph.current() == nil then
      return
   end
   Stats.update()
   if scene_graph.current().shader.meta_class.name ==
      'OGF::PlainMeshGrobShader' and
      scene_graph.current().shader.painting == 'ATTRIBUTE'
   then
      if imgui.Button(imgui.font_icon('angle-double-up')) then
          scene_graph.current().shader.painting = 'SOLID_COLOR'
      end
   else
      if imgui.Button(imgui.font_icon('angle-double-down')) then
          scene_graph.current().shader.painting = 'ATTRIBUTE'
      end
   end
   autogui.tooltip('Display histogram for current object and attribute')
   imgui.SameLine()
   _,Stats.color = imgui.Checkbox(imgui.font_icon('paint-brush'),Stats.color)
   imgui.SameLine()
   if imgui.Button(imgui.font_icon('expand')) then
      scene_graph.current().shader.autorange()
      Stats.sel1 = 0
      Stats.sel2 = 0
      Stats.histo_max_display = nil
      Stats.update_histo()
   end
   autogui.tooltip('Autofit range')
   imgui.SameLine()
   if imgui.Button(imgui.font_icon('search')) then
      if Stats.sel1 ~= Stats.sel2 then
         local val1 = Stats.index_to_val(Stats.sel1)
         local val2 = Stats.index_to_val(Stats.sel2)
         scene_graph.current().shader.attribute_min = math.min(val1,val2)
         scene_graph.current().shader.attribute_max = math.max(val1,val2)
         Stats.sel1 = 0
         Stats.sel2 = 0
      end
   end
   autogui.tooltip('Set selection as range')

   if not Stats.initialized then
      return
   end

   local sel
   sel,Stats.bins = imgui.SliderInt(
     'bins', Stats.bins, 3, 1000, '%d'
   )
   if sel then
       Stats.bins = math.max(Stats.bins,3)
       Stats.update_histo()
   end

   _,Stats.histo_max_display = imgui.SliderInt(
     'max', Stats.histo_max_display, Stats.histo_min+1, Stats.histo_max, '%d'
   )


   autogui.combo_box(
      scene_graph.current().shader,
      'attribute',
      scene_graph.current().scalar_attributes,
      'displayed attribute'
   )

   -- math.floor needed because ImGui lua bindings
   -- do not do the right thing (to be fixed)
   local graph_color = math.floor(imgui.GetColorU32(ImGuiCol_PlotHistogram,1.0))
   local lines_color = math.floor(imgui.GetColorU32(ImGuiCol_PlotLines,1.0))
   local frame_color = math.floor(imgui.GetColorU32(ImGuiCol_Separator,1.0))
   local text_color  = math.floor(imgui.GetColorU32(ImGuiCol_Text,1.0))

   Stats.pos_x,   Stats.pos_y = imgui.GetCursorScreenPos()
   Stats.size_x, Stats.size_y = imgui.GetContentRegionAvail()
   Stats.left_margin   = 15.0 * main.scaling()
   Stats.right_margin  = 15.0 * main.scaling()
   Stats.top_margin    = 15.0 * main.scaling()
   Stats.bottom_margin = 15.0 * main.scaling()
   Stats.left_prop     = 0.8

   local draw_list = imgui.GetWindowDrawList()

   -- Draw the frame around the widget
   imgui.AddRect(
       draw_list,
       Stats.pos_x, Stats.pos_y,
       Stats.pos_x + Stats.size_x, Stats.pos_y + Stats.size_y,
       frame_color, 0.0, 0, 1.0
   )
   imgui.InvisibleButton('canvas', Stats.size_x, Stats.size_y)
   imgui.PushClipRect(
       draw_list,
       Stats.pos_x, Stats.pos_y,
       Stats.pos_x + Stats.size_x, Stats.pos_y + Stats.size_y,
       true
   )

   local half_line_width =
       0.5*(Stats.size_y - Stats.top_margin - Stats.bottom_margin) /
       (#Stats.histo+1)
   half_line_width = math.max(half_line_width,1)

   local x1,y1,x2,y2,t

   -- Draw the histogram
   for i=1,#Stats.histo do
       x1,y1   = Stats.transform(i,Stats.histo_min)
       x2,y2,t = Stats.transform(i,Stats.histo[i])
       if Stats.color and Stats.colormap then
         imgui.AddImage(
	        draw_list, Stats.colormap,
	        x1, y1-half_line_width,
	        x2, y2+half_line_width,
	        t, 0.0,  t, 1.0,
	        0xffffffff
	     )
       else
        imgui.AddRectFilled(
	        draw_list,
	        x1, y1-half_line_width,
		x2, y2+half_line_width,
 	        graph_color,
                0.0, 0
	     )
       end
   end

   -- Draw the frame around the histogram
   x1,y1 = Stats.transform(0,             Stats.histo_min)
   x2,y2 = Stats.transform(#Stats.histo+1,Stats.histo_min)
   imgui.AddLine(draw_list,x1,y1,x2,y2,text_color,2)
   Stats.draw_marker(0, Stats.attrib_min)
   Stats.draw_marker(#Stats.histo+1, Stats.attrib_max)

   Stats.selection_handler()

   imgui.PopClipRect(draw_list)

   imgui.Text('mean:'..tostring(Stats.mean))
end

Stats.reset()
graphite_main_window.add_module(Stats)
gom.connect(scene_graph.value_changed, Stats.update_histo)
