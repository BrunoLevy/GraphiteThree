-- Lua (Keep this comment, this is an indicator for editor's 'run' command)

-- Custom rendering in ImGui

Draw_dialog = {} 
Draw_dialog.visible = true
Draw_dialog.name = 'CustomRendering'
Draw_dialog.icon = '@pen' 
Draw_dialog.x = 100
Draw_dialog.y = 400
Draw_dialog.w = 600
Draw_dialog.h = 350

Draw_dialog.sz = 36.0
Draw_dialog.thickness = 3.0

Draw_dialog.draw_bg = true
Draw_dialog.draw_fg = true

Draw_dialog.points = {}
Draw_dialog.adding_line = false

function Draw_dialog.draw_window()
  imgui.BeginTabBar('##TabBar') 
  if imgui.BeginTabItem('Primitives') then

     _,Draw_dialog.sz = imgui.DragFloat(
       'Size', Draw_dialog.sz, 0.2, 2.0, 72.0, '%.0f'
     )
     _,Draw_dialog.thickness = imgui.DragFloat(
       'Thickness', Draw_dialog.thickness, 0.05, 1.0, 8.0, '%.02f'
     )
     local draw_list = imgui.GetWindowDrawList()
     local px,py = imgui.GetCursorScreenPos()
     local col = 0xffaaffff
     local x = px + 4.0
     local y = py + 4.0
     local spacing = 10.0
     local corners_none = 0
     local corners_all = ImDrawCornerFlags_All
     local corners_tl_br = ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight
     local sz = Draw_dialog.sz
     for n=0,1 do
        -- First line uses a thickness of 1.0f, second line uses the configurable thickness
	local th = 1.0
	if n == 1 then
	   th = Draw_dialog.thickness
	end
	imgui.AddCircle(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col,  6, th)        x = x + sz + spacing
	imgui.AddCircle(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col, 20, th)        x = x + sz + spacing
	imgui.AddRect(draw_list, x, y, x + sz, y + sz, col, 0.0, corners_none, th)     x = x + sz + spacing
 	imgui.AddRect(draw_list, x, y, x + sz, y + sz, col, 10.0, corners_all, th)     x = x + sz + spacing
	imgui.AddRect(draw_list, x, y, x + sz, y + sz, col, 10.0, corners_tl_br, th)   x = x + sz + spacing
	imgui.AddTriangle(draw_list, x+sz*0.5,y, x+sz, y+sz-0.5, x, y+sz-0.5, col, th) x =  x + sz + spacing;
	imgui.AddTriangle(draw_list, x+sz*0.2,y, x, y+sz-0.5, x+sz*0.4, y+sz-0.5, col, th)  x = x + sz*0.4 + spacing
	imgui.AddLine(draw_list, x, y, x + sz, y, col, th)                                  x = x + sz + spacing
	imgui.AddLine(draw_list, x, y, x, y + sz, col, th)                                  x = x + spacing
	imgui.AddLine(draw_list, x, y, x + sz, y + sz, col, th)                             x = x +  sz + spacing
	imgui.AddBezierCurve(draw_list, x, y, x + sz*1.3, y + sz*0.3, x + sz - sz*1.3, y + sz - sz*0.3, x + sz, y + sz, col, th,0)
	x = px + 4
        y = y + sz + spacing
     end
     imgui.AddCircleFilled(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col, 6)             x = x + sz + spacing
     imgui.AddCircleFilled(draw_list, x + sz*0.5, y + sz*0.5, sz*0.5, col, 32)            x = x + sz + spacing
     imgui.AddRectFilled(draw_list, x, y, x + sz, y + sz, col, 0.0, corners_none)         x = x + sz + spacing
     imgui.AddRectFilled(draw_list, x, y, x + sz, y + sz, col, 10.0, corners_all)         x = x + sz + spacing
     imgui.AddRectFilled(draw_list, x, y, x + sz, y + sz, col, 10.0, corners_tl_br)       x = x + sz + spacing
     imgui.AddTriangleFilled(draw_list, x+sz*0.5,y, x+sz, y+sz-0.5, x, y+sz-0.5, col)     x =  x + sz + spacing;
     imgui.AddTriangleFilled(draw_list, x+sz*0.2,y, x, y+sz-0.5, x+sz*0.4, y+sz-0.5, col) x = x + sz*0.4 + spacing
     imgui.AddRectFilled(draw_list, x, y, x + sz, y + Draw_dialog.thickness, col, 0.0, corners_none)         x = x + sz + spacing
     imgui.AddRectFilled(draw_list, x, y, x + Draw_dialog.thickness, y + sz, col, 0.0, corners_none)         x = x + spacing*2.0
     imgui.AddRectFilled(draw_list, x, y, x + 1, y + 1, col, 0.0, corners_none)                              x = x + sz
     imgui.AddRectFilledMultiColor(draw_list, x, y, x + sz, y + sz, 0xff000000, 0xff0000ff, 0xff00ffff, 0xffff00ff)
     imgui.EndTabItem()
  end
     
  if imgui.BeginTabItem('Canvas') then
     if imgui.Button('Clear') then
        Draw_dialog.points = {}
     end
     if #Draw_dialog.points >= 4 then
        imgui.SameLine()
        if imgui.Button('Undo') then
	   table.remove(Draw_dialog.points)
	   table.remove(Draw_dialog.points)
	   table.remove(Draw_dialog.points)
	   table.remove(Draw_dialog.points)
        end
     end
     local pos_x, pos_y = imgui.GetCursorScreenPos()
     local size_x, size_y = imgui.GetContentRegionAvail()
     local draw_list = imgui.GetWindowDrawList()
     imgui.AddRect(
         draw_list, pos_x, pos_y, pos_x + size_x, pos_y + size_y, 0xffffffff, 0.0, 0, 1.0
     )
     local adding_preview = false
     imgui.InvisibleButton('canvas', size_x, size_y)
     local mx,my = imgui.GetMousePos()
     mx = mx - pos_x
     my = my - pos_y
     if Draw_dialog.adding_line then
        adding_preview = true
	table.insert(Draw_dialog.points, mx)
	table.insert(Draw_dialog.points, my)
	if not imgui.IsMouseDown(0) then
            Draw_dialog.adding_line = false
	    adding_preview = false
        end		    
     end
     if imgui.IsItemHovered() then
        if not Draw_dialog.adding_line and imgui.IsMouseClicked(0) then
  	   table.insert(Draw_dialog.points, mx)
	   table.insert(Draw_dialog.points, my)
	   Draw_dialog.adding_line = true
	end
	if imgui.IsMouseClicked(1) and #Draw_dialog.points ~= 0 then
     	   Draw_dialog.adding_line = false
	   adding_preview = false
	   table.remove(Draw_dialog.points)
	   table.remove(Draw_dialog.points)
	   table.remove(Draw_dialog.points)
	   table.remove(Draw_dialog.points)
	end
     end
     imgui.PushClipRect(draw_list, pos_x, pos_y, pos_x + size_x, pos_y + size_y, true)

     for i = 0, #Draw_dialog.points-4, 4 do
        imgui.AddLine(
	  draw_list,
	  pos_x + Draw_dialog.points[1+i],
	  pos_y + Draw_dialog.points[1+i+1],
	  pos_x + Draw_dialog.points[1+i+2],
	  pos_y + Draw_dialog.points[1+i+3],
	  0xff00ffff, 2.0
        )
     end
     imgui.PopClipRect(draw_list)
     if adding_preview then
	table.remove(Draw_dialog.points)
	table.remove(Draw_dialog.points)	   
     end
     imgui.EndTabItem()     
  end

  if imgui.BeginTabItem('BG / FG Draw Lists') then
     _,Draw_dialog.draw_bg = imgui.Checkbox('Draw in Background draw list', Draw_dialog.draw_bg);
     _,Draw_dialog.draw_fg = imgui.Checkbox('Draw in Foreground draw list', Draw_dialog.draw_fg);     

     local px,py = imgui.GetWindowPos()
     local sx,sy = imgui.GetWindowSize()
     local cx,cy = px+sx/2, py+sy/2
     local bh = imgui.GetBackgroundDrawList()
     local fh = imgui.GetForegroundDrawList()
     if Draw_dialog.draw_bg then
         imgui.AddCircle(bh, cx, cy, sx*0.6, 0xff0000ff, 48, 10+4)
     end
     if Draw_dialog.draw_fg then
         imgui.AddCircle(fh, cx, cy, sy*0.6, 0xff00ff00, 48, 10)
     end	 
     imgui.EndTabItem()     
  end

  imgui.EndTabBar()

end

graphite_main_window.add_module(Draw_dialog)
