-- LuaGrob (Keep this comment, it is an indication for editor's 'run' command)
--
-- Example for LuaGrob
-- Draws an analog clock

function handle(alpha, R, r, g, b)
   GLUP.SetColor(GLUP.MESH_COLOR,r,g,b)
   GLUP.SetMeshWidth(15)
   GLUP.Begin(GLUP.LINES)
      GLUP.Vertex(0.5,0.5,0.5)
      GLUP.Vertex(0.5+R*math.sin(alpha), 0.5+R*math.cos(alpha),0.5)
   GLUP.End()
end

function balls()
   GLUP.SetColor(GLUP.FRONT_AND_BACK_COLOR,0.0,0.0,1.0)
   GLUP.Begin(GLUP.SPHERES)
   for i=1,60 do
      local alpha = i*6.28/60
      local R = 0.01
      GLUP.Vertex(0.5+0.4*math.sin(alpha),0.5+0.4*math.cos(alpha),0.45,R)
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
      GLUP.Vertex(0.5+0.4*math.sin(alpha),0.5+0.4*math.cos(alpha),0.45,R)
   end
   GLUP.Vertex(0.5, 0.5, 0.5, 0.025)
   GLUP.End()
   GLUP.SetColor(GLUP.FRONT_AND_BACK_COLOR,0.0,0.0,0.0)
end

function draw()
   local now = os.date("*t")
   handle(now.sec * 6.28 / 60.0, 0.4, 0.5, 0.5, 0.5)
   handle(now.min * 6.28 / 60.0, 0.35, 0.5, 0.5, 0.5)
   handle(
       (now.hour + now.min / 60.0) * 6.28 / 12.0, 0.2, 1.0, 0.0, 0.0
   )
   balls()
   GLUP.Update()
end
