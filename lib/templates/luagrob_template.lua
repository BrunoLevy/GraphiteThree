-- LuaGrob   (keep this comment, it is an indication for editor's 'run' command)

function draw()
   GLUP.SetColor(GLUP.FRONT_AND_BACK_COLOR, 1.0, 1.0, 0.0)
   GLUP.Begin(GLUP.SPHERES)
   GLUP.Vertex(0.5, 0.5, 0.5, 0.2)
   GLUP.End()
   GLUP.SetColor(GLUP.FRONT_AND_BACK_COLOR, 0.0, 0.0, 0.0)   
end
 
