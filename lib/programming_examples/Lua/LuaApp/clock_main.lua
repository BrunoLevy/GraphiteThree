-- LuaMain (Keep this comment, it is an indication for editor's 'run' command)
--
-- Example for main LUA Graphite application
-- Draws an analog clock
--
-- Usage: start from command line:
--  graphite main=clock_main.lua

if main ~= nil then
    gom.err('Graphite application already exists')
    gom.err('You need to use: graphite main=clock_main.lua')
    gom.err('(from the command line)')
    error('Graphite application already exists')
end

function handle(alpha, R, r, g, b)
   GLUP.SetColor(GLUP.FRONT_AND_BACK_COLOR,r,g,b)
   GLUP.Begin(GLUP.LINES)
      GLUP.Vertex(0,0,0.5)
      GLUP.Vertex(R*math.sin(alpha), R*math.cos(alpha),0.5)
   GLUP.End() 
end

function balls()
   GLUP.SetColor(GLUP.FRONT_AND_BACK_COLOR,0.0,0.0,1.0)
   GLUP.Begin(GLUP.SPHERES)
   for i=1,60 do
      local alpha = i*6.28/60
      local R = 0.01
      GLUP.Vertex(0.8*math.sin(alpha),0.8*math.cos(alpha),0.45,R)
   end
   GLUP.End()
   GLUP.SetColor(GLUP.FRONT_AND_BACK_COLOR,0.0,1.0,1.0)
   GLUP.Begin(GLUP.SPHERES)
   for i=1,12 do
      local alpha = i*6.28/12
      local R = 0.025
      if i == 12 then
         R = 0.04
      end
      GLUP.Vertex(0.8*math.sin(alpha),0.8*math.cos(alpha),0.45,R)
   end
   GLUP.Vertex(0, 0, 0.5, 0.025)
   GLUP.End()
   GLUP.SetColor(GLUP.FRONT_AND_BACK_COLOR,0.0,0.0,0.0)
end

function draw()
   local now = os.date("*t")
   handle(now.sec * 6.28 / 60.0, 0.8, 0.5, 0.5, 0.5)
   handle(now.min * 6.28 / 60.0, 0.7, 0.5, 0.5, 0.5)
   handle(
       (now.hour + now.min / 60.0) * 6.28 / 12.0, 0.4, 1.0, 0.0, 0.0
   )
   balls()
   win.update()
end

function draw_gui()
    if imgui.BeginMainMenuBar() then
	if imgui.BeginMenu('File...') then
	    if imgui.MenuItem('quit') then
		main.stop()
	    end
	    imgui.EndMenu()
	end
	imgui.EndMainMenuBar()
    end
end    

main = gom.create({classname='OGF::Application',interpreter=gom})
win = main.render_area()
gom.connect(win.redraw_request, draw)
gom.connect(main.redraw_request,draw_gui)
main.start()

