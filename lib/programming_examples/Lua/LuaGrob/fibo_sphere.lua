-- LuaGrob (Keep this comment, it is an indication for editor's 'run' command)

-- Example for LuaGrob
-- Draws a Fibonacci sphere
-- Inspired by: https://twitter.com/DonaldM38768041

function draw()
  GLUP.Enable(GLUP.VERTEX_COLORS)
  GLUP.Begin(GLUP.SPHERES)
  
  local N = 700
  local phi = 2.618033
  local pi = 3.14159
  for n=1,N do
     z = 2*n/N - 1;
     local r = math.sqrt(1-z*z);
     local theta = (2*pi)*n/phi;
     local x = r*math.sin(theta);
     local y = r*math.cos(theta);
     x = 0.5 + x*0.5
     y = 0.5 + y*0.5
     z = 0.5 + z*0.5
     GLUP.Color(x,y,z)
     GLUP.Vertex(x,y,z, 0.05)
   end
  GLUP.End()
  GLUP.Disable(GLUP.VERTEX_COLORS)  
end


