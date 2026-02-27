--
-- The 'GraphitePoint' system for including a PDF presentation in Graphite.
--=============================================================================

-- This test to disable presentation when Graphite is running in batch mode.
if(graphite_main_window ~= nil) then

--=============================================================================
--Initializing cairo and poppler

-- ... Debian 10 'Buster' has lgi in 5.3. The LUA bundled with geogram does
-- not find it (so I added the path)
-- (Note: the lgi for LUA 5.2 also works with the LUA 5.3 bundled with geogram,
--  it is slightly modified to make it possible)

package.path=
        package.path..';/usr/share/lua/5.3/?.lua;/usr/share/lua/5.3/?/init.lua'
package.cpath=
        package.cpath..';/usr/lib/x86_64-linux-gnu/lua/5.3/?.so'

lgi = require('lgi')
if lgi ~= nil then
   cairo = lgi.cairo
   Poppler = lgi.Poppler
end

ScriptManager = {}
ScriptManagerMetaTable = {}

function ScriptManager.init()
  setmetatable(ScriptManager,nil)
  ScriptManager.scripts_map = {}
  ScriptManager.presentation_map = {}
  ScriptManager.presentation_map_inv = {}
  ScriptManager.presentation_key = ''
end

function ScriptManager.attach_script(slide, script_name)
   ScriptManager.scripts_map[slide] = script_name
end

function ScriptManager.attach_key(slide, key_name)
   ScriptManager.presentation_map[key_name] = slide
   ScriptManager.presentation_map_inv[slide] = key_name
end

function ScriptManager.presentation_keys()
   local keys = {}
   for k in pairs(ScriptManager.presentation_map) do
      table.insert(keys, k)
   end
   table.sort(keys)

   local result = ''
   for k,v in pairs(keys) do
      if result ~= '' then
         result = result .. ';'
      end
      result = result .. v
   end
   return result
end

function ScriptManager.key_to_slide(k)
   return ScriptManager.presentation_map[k] or 1
end

function ScriptManager.run_script(slide_num, slice_script)

   script_name = ScriptManager.scripts_map[slide_num]

   if script_name == nil then
      return false
   end

   script_name = script_name .. '_' .. slice_script
   script_func = ScriptManager[script_name]

   if script_func == nil then
      return false
   end

   -- Protected call, to catch errors in present. scripts.
   local OK,error = pcall(script_func)
   if not OK then
       gom.err(tostring(error))
   end

   return true
end

--=============================================================================

GraphitePoint = {}

function GraphitePoint.run_script(slide_num, script_num)
   if not GraphitePoint.scripts_enabled then
      return false
   end
   return ScriptManager.run_script(slide_num, script_num)
end

function GraphitePoint.clear3d()
   scene_graph.clear()
   scene_graph.scene_graph_shader_manager.update_focus()
   xform.reset()
   scene_graph.scene_graph_shader_manager.update_focus()
   text_editor_gui.visible=false
end

function GraphitePoint.reset_tool()
   -- To change tool, we need to have at least one object
   scene_graph.create_object('OGF::MeshGrob')
   tools.tool('OGF::GrobPan')
   tools.tool('OGF::GrobPan')
end

function GraphitePoint.set_background(file)
   main.render_area.background_image = GraphitePoint.basedir .. '/' .. file
   main.render_area.update()
end

function GraphitePoint.clear_background()
   GraphitePoint.set_background(GraphitePoint.slide_template)
end

function GraphitePoint.reset()
   if GraphitePoint.locked then
      return
   end
   GraphitePoint.clear3d()
   GraphitePoint.reset_tool()
   GraphitePoint.cur = 1
   GraphitePoint.cur_script = 1
   GraphitePoint.update()
end

function GraphitePoint.goto_end()
  if GraphitePoint.pdf_file ~= nil then
     GraphitePoint.goto_slide(GraphitePoint.pdf_file:get_n_pages())
  end
end

function GraphitePoint.next()
  if GraphitePoint.locked then
     return
  end
  GraphitePoint.locked = true
  if GraphitePoint.run_script(GraphitePoint.cur, GraphitePoint.cur_script) then
     GraphitePoint.cur_script = GraphitePoint.cur_script + 1
  else
     GraphitePoint.clear3d()
     GraphitePoint.cur = GraphitePoint.cur + 1
     GraphitePoint.cur_script = 1
     GraphitePoint.update()
     GraphitePoint.run_script(GraphitePoint.cur, 0)
  end
  GraphitePoint.locked = false
end

function GraphitePoint.prev()
  if GraphitePoint.locked then
     return
  end
  GraphitePoint.clear3d()
  if GraphitePoint.scripts_enabled then
     GraphitePoint.reset_tool()
  end
  GraphitePoint.cur_script = 1
  GraphitePoint.cur = GraphitePoint.cur - 1
  GraphitePoint.update()
end

function GraphitePoint.draw_pdf_slide(slidenum)
   local page = GraphitePoint.pdf_file:get_page(slidenum)
   local pw,ph = page:get_size()
   local iw = GraphitePoint.cairo_surface:get_width()
   local ih = GraphitePoint.cairo_surface:get_height()
   local scale = math.min(iw/pw, ih/ph)
   GraphitePoint.cairo_context:scale(scale,scale)
   GraphitePoint.cairo_context:set_source_rgb(1,1,1)
   GraphitePoint.cairo_context:rectangle(0,0,pw,ph)
   GraphitePoint.cairo_context:fill()
   page:render(GraphitePoint.cairo_context)
   GraphitePoint.cairo_context:scale(1./scale,1./scale)
end

function GraphitePoint.save_pdf_slide(filename)
  if GraphitePoint.cairo_OK then
     GraphitePoint.cairo_surface:write_to_png(filename)
  else
     command = 'cp /tmp/snapshot.png ' .. filename
     os.execute(command)
  end
end

function GraphitePoint.update()
   if GraphitePoint.cairo_OK then
       if GraphitePoint.cur < 1 or
          GraphitePoint.cur > GraphitePoint.pdf_file:get_n_pages()
       then
          main.render_area.background_image = ''
          main.render_area.update()
          return
       end
       GraphitePoint.draw_pdf_slide(GraphitePoint.cur-1)

       -- Directly update the background from cairo context raw data.
       -- Disable here if this causes some problems with cairo/lgi versions
       -- (activated by default, much much faster !!)
       local can_update_background = true
       if can_update_background then
          main.render_area.update_background_image_from_data(
              GraphitePoint.cairo_surface:get_data(),
   	      'RGBA', 'BYTE',
	      GraphitePoint.image_width,
	      GraphitePoint.image_height,
	      GraphitePoint.filtering_mode
          )
       else
          -- slower version that uses a file
          GraphitePoint.save_pdf_slide(GraphitePoint.slide_snapshot)
          main.render_area.background_image = GraphitePoint.slide_snapshot
       end
       main.render_area.update()
   else
      -- even slower version that uses an external program (pdftocairo)
      command = 'pdftocairo -png -r 90 -f ' .. GraphitePoint.cur ..
                  ' -l ' .. GraphitePoint.cur .. ' -singlefile ' ..
                   GraphitePoint.pdf_file_name .. ' ' .. '/tmp/snapshot'
      os.execute(command)
      main.render_area.background_image = '/tmp/snapshot.png'
      main.render_area.update()
   end
end

function GraphitePoint.goto_slide(value)
   print('GraphitePoint goto slide '..value)
   if not GraphitePoint.ready then
      return
   end
   GraphitePoint.clear3d()
   GraphitePoint.cur = tonumber(value)
   GraphitePoint.cur_script = 1
   GraphitePoint.update()
end

function GraphitePoint.load_presentation(value)
   gom.out('load presentation: ' .. value)
   ScriptManager.init()
   GraphitePoint.pdf_file = nil
   GraphitePoint.pdf_file_name = ''
   GraphitePoint.basedir=FileSystem.dir_name(value)
   GraphitePoint.presentation_name = FileSystem.base_name(
       GraphitePoint.basedir,true
   )
   local extension = FileSystem.extension(value)
   if extension == 'pdf' or extension == 'PDF' then
      GraphitePoint.pdf_file_name = value
      if GraphitePoint.cairo_OK then
         GraphitePoint.pdf_file = Poppler.Document.new_from_file(
            'file://' .. GraphitePoint.pdf_file_name, nil
         )
      end
   end
   local script_manager_file = GraphitePoint.basedir..'/scripts.lua'
   if FileSystem.is_file(script_manager_file) then
      gom.out('load scripts: ' .. script_manager_file)
      gom.execute_file(script_manager_file)
   end
   GraphitePoint.slide_template = GraphitePoint.basedir .. '/template.png'
   keys = GraphitePoint.script_manager.presentation_keys()
   GraphitePoint.reset()
   GraphitePoint.ready = true
end

function GraphitePoint.close_presentation()
   GraphitePoint.ready = false
   main.render_area.background_image = ''
   GraphitePoint.clear3d()
   GraphitePoint.pdf_file_name = ''
   GraphitePoint.presentation_name = ''
   ScriptManager.init()
   GraphitePoint.init()
   graphite_gui.escape()
end

function GraphitePoint.hide_graphite()
   GraphitePoint.was_full_screen = main.full_screen
   main.full_screen = false
   if main.full_screen then
       main.iconify()
   end
end

function GraphitePoint.show_graphite()
   if GraphitePoint.was_full_screen then
       main.restore()
       main.full_screen = true
   end
end

function GraphitePoint.run_geex(cmd_in)
   cmd = 'cd ' .. GraphitePoint.geex_dir .. ' ; '
   cmd = cmd .. 'binaries/bin/' .. cmd_in
   gom.out('command = ' .. cmd)
   GraphitePoint.hide_graphite()
   os.execute(cmd)
   GraphitePoint.show_graphite()
end

function GraphitePoint.run_geex_over_slide(cmd_in)
   GraphitePoint.save_pdf_slide(GraphitePoint.slide_snapshot)
   cmd = 'cd ' .. GraphitePoint.geex_dir .. ' ; '
   cmd = cmd .. 'binaries/bin/' .. cmd_in
   cmd = cmd .. ' -fs -bkg ' .. GraphitePoint.slide_snapshot
   gom.out('command = ' .. cmd)
   GraphitePoint.hide_graphite()
   os.execute(cmd)
   GraphitePoint.show_graphite()
end

function GraphitePoint.run_geex_with_template(cmd_in)
   GraphitePoint.save_pdf_slide(GraphitePoint.slide_snapshot)
   cmd = 'cd ' .. GraphitePoint.geex_dir + ' ; '
   cmd = cmd .. 'binaries/bin/' .. cmd_in
   cmd = cmd .. ' -fs -bkg ' .. GraphitePoint.slide_template
   gom.out('command = ' .. cmd)
   GraphitePoint.hide_graphite()
   os.execute(cmd)
   GraphitePoint.show_graphite()
end

function GraphitePoint.run_vorpaview(args)
   cmd = GraphitePoint.vorpaview_dir ..
      '/vorpaview full_screen=true ' ..
      GraphitePoint.data_path() .. '/' .. args
   gom.out(cmd)
   GraphitePoint.hide_graphite()
   os.execute(cmd)
   GraphitePoint.show_graphite()
end

function GraphitePoint.data_path()
   return GraphitePoint.basedir .. '/Data'
end

function GraphitePoint.load_object(filename)
   scene_graph.load_object(GraphitePoint.data_path() .. '/' .. filename)
end

function GraphitePoint.play_video(filename)
    GraphitePoint.hide_graphite()
    os.execute('mplayer -fs ' .. GraphitePoint.data_path() .. '/' .. filename)
    GraphitePoint.show_graphite()
end

function GraphitePoint.scripts(value)
   if value == 'true' then
      GraphitePoint.scripts_enabled = true
      gom.out('Enable scripts')
   else
      GraphitePoint.scripts_enabled = false
      gom.out('Disable scripts')
   end
end


function GraphitePoint.filtering(value)
   if value == 'true' then
       GraphitePoint.filtering_mode = 'MIPMAP'
   else
       GraphitePoint.filtering_mode = 'NO_FILTERING'
   end
   GraphitePoint.update()
end

function GraphitePoint.goto_key(k)
   GraphitePoint.goto_slide(GraphitePoint.script_manager.key_to_slide(k))
end

function GraphitePoint.presentation_mode()
   main.full_screen = true
   preferences_window.load_preferences('presentation_layout');
end

function GraphitePoint.reload()
   gom.out('reload')
   current = GraphitePoint.cur
   GraphitePoint.load_presentation(GraphitePoint.pdf_file_name)
   GraphitePoint.goto_slide(current)
end

function GraphitePoint.key_event(k)
   if not GraphitePoint.ready then
      return
   end
   if k == 'page_down' or k == 'down' or k == ' ' then
      GraphitePoint.next()
      return
   end
   if k == 'page_up' or k == 'up' then
      GraphitePoint.prev()
      return
   end
   if k == 'home' then
      GraphitePoint.reset()
      return
   end
   if k == 'end' then
      GraphitePoint.goto_end()
      return
   end
end

GraphitePoint.name = 'GraphitePoint'
GraphitePoint.icon = '@person-chalkboard'
GraphitePoint.x = 4*main.margin() + 250*main.scaling()
GraphitePoint.y = main.margin()
GraphitePoint.w = 300*main.scaling()
GraphitePoint.h = 300*main.scaling()

function GraphitePoint.draw_window()
	imgui.Text('Current: '..GraphitePoint.presentation_name)

	imgui.Separator()

	if imgui.SimpleButton(
	   imgui.font_icon('folder-open')
	) then
       	   imgui.OpenFileDialog(
	     '##GraphitePoint##load_dlg',
	     'pdf',
	     '', -- default filename
	     ImGuiExtFileDialogFlags_Load
	   )
	end
	autogui.tooltip('Open presentation')

        imgui.SameLine()

	if imgui.SimpleButton(
	   imgui.font_icon('home')
	) then
	   GraphitePoint.reset()
	end
	autogui.tooltip('Reset slideshow')

	imgui.SameLine()

	if imgui.SimpleButton(
	   imgui.font_icon('tv')
	) then
	   GraphitePoint.presentation_mode()
	end
	autogui.tooltip('Presentation mode\nPress ESC for normal mode')

	imgui.SameLine()

	if imgui.SimpleButton(
	   imgui.font_icon('sync-alt')
	) then
	   GraphitePoint.reload()
	end
	autogui.tooltip('Reload presentation')

	imgui.SameLine()

	if imgui.SimpleButton(
	   imgui.font_icon('window-close')
	) then
	  GraphitePoint.close_presentation()
	end
	autogui.tooltip('Close presentation')

	imgui.Separator()

	_,GraphitePoint.mode = autogui.combo_box_value(
	    '##beamer##mode', GraphitePoint.mode,
	    '640x480;800x600;1024x768;1280x1024;1680x1050;1920x1080'
        )
	imgui.SameLine()
	if imgui.Button('Apply##beamer') then
	    local cmd = gom.get_environment_value('PROJECT_ROOT')..'/lib/beamer.sh '..GraphitePoint.mode
	    os.execute(cmd)
	end
	autogui.tooltip('(tentatively) set both screen resolution\n and beamer plugged on the HDMI')

	imgui.Separator()

	_,GraphitePoint.scripts_enabled = imgui.Checkbox(
	    'scripts##GraphitePoint',
	    GraphitePoint.scripts_enabled
	)

	local sel
	sel,GraphitePoint.cur = imgui.InputInt(
	   'slide##GraphitePoint',
   	   GraphitePoint.cur
	)
	if sel then
	   GraphitePoint.goto_slide(GraphitePoint.cur)
	end


	local progress_fraction=0
	local progress_str = ''

	if GraphitePoint.pdf_file ~= nil then
           progress_fraction = GraphitePoint.cur / GraphitePoint.pdf_file:get_n_pages()
	   progress_str = 'slide ' ..
	                  tostring(math.floor(GraphitePoint.cur)) .. ' / ' ..
	                  tostring(math.floor(GraphitePoint.pdf_file:get_n_pages()))
        end

	imgui.ProgressBar(progress_fraction, progress_str)

	imgui.Separator()

	local keys = ScriptManager.presentation_keys()
	if keys == '' then
	   imgui.Text('\n')
	else
	   local key = ScriptManager.presentation_map_inv[GraphitePoint.cur]
	   if key ~= nil then
	      ScriptManager.presentation_key = key
           else
	      key = ScriptManager.presentation_key
	   end
	   sel,key = autogui.combo_box_value('key',key,keys)
           if sel and key ~= nil then
	      GraphitePoint.goto_key(key)
	   end
        end


  local sel
  sel,GraphitePoint.pdf_file_name = imgui.FileDialog(
     '##GraphitePoint##load_dlg',GraphitePoint.pdf_file_name
  )
  if sel then
      GraphitePoint.load_presentation(GraphitePoint.pdf_file_name)
  end

end

function GraphitePoint.draw_menu()
  if imgui.BeginMenu('File...') then
     if imgui.MenuItem('Load presentation') then
        GraphitePoint.visible = true
   	imgui.OpenFileDialog(
	  '##GraphitePoint##load_dlg',
	  'pdf',
	  '', -- default filename
	  ImGuiExtFileDialogFlags_Load
	)
     end
     imgui.EndMenu()
  end
end

function GraphitePoint.init()
   GraphitePoint.visible=false
   GraphitePoint.basedir=''
   GraphitePoint.basename='Diapositive'
   GraphitePoint.geex_dir='/home/levy/Programming/geex/Geex/build/Linux-Release'
   GraphitePoint.vorpaview_dir=
   '/home/levy/Programming/GraphiteThree/build/Linux64-gcc-dynamic-Release/bin/'
   GraphitePoint.extension='PNG'

   GraphitePoint.filtering_mode = 'MIPMAP'

   GraphitePoint.cur = 0
   GraphitePoint.cur_script = 1
   GraphitePoint.scripts_enabled = true
   GraphitePoint.presentation_name = FileSystem.base_name(
      GraphitePoint.basedir,true
   )
   GraphitePoint.locked = false
   GraphitePoint.pdf_file_name = ''
   GraphitePoint.script_manager = ScriptManager
   GraphitePoint.slide_template = '/tmp/template.png'
   GraphitePoint.slide_snapshot = '/tmp/snapshot.png'
   GraphitePoint.ready          = false
   GraphitePoint.image_width    = 1024
   GraphitePoint.image_height   = 768
   GraphitePoint.mode = '1024x768'
   GraphitePoint.cairo_OK = (cairo ~= nil and Poppler ~= nil)
   if GraphitePoint.cairo_OK then
      GraphitePoint.cairo_surface = cairo.ImageSurface.create(
         'RGB24', GraphitePoint.image_width, GraphitePoint.image_height
      )
      GraphitePoint.cairo_context =
          cairo.Context.create(GraphitePoint.cairo_surface)
      GraphitePoint.cairo_OK = (
           GraphitePoint.cairo_surface ~= nil and
	   GraphitePoint.cairo_context ~= nil
      )
   end
   if GraphitePoint.cairo_OK then
      gom.out('GraphitePoint: Cairo context created.')
   else
      gom.out(
     'GraphitePoint: Could not create Cairo context, using pdftocairo fallback.'
      )
   end
end

function GraphitePoint.set_presentation(pres)
   ScriptManagerMetaTable.__index = pres
   setmetatable(ScriptManager, ScriptManagerMetaTable)
end

ScriptManager.init()
GraphitePoint.init()
gom.connect(main.render_area.key_down, GraphitePoint.key_event)

graphite_main_window.add_module(GraphitePoint)

--============================================================================

end
