-- LuaGrob (Keep this comment, it is an indication for editor's 'run' command)

-- Example for LuaGrob
-- Draws an array of colored spheres

function draw()
  GLUP.Enable(GLUP.VERTEX_COLORS)
  GLUP.Begin(GLUP.SPHERES)
  for x=0.0,1.0,0.1 do
     for y=0.0,1.0,0.1 do
         for z=0.0,1.0,0.1 do
            GLUP.Color(x,y,z)
            GLUP.Vertex(x,y,z, 0.05)
         end
     end
  end
  GLUP.End()
  GLUP.Disable(GLUP.VERTEX_COLORS)  
end
