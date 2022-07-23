-- Lua (Keep this comment, this is an indication for editor's 'run' command)
-- An example of how to modify the coordinates of a mesh in Lua
-- Try it on a sphere (shapes->create sphere)

S = scene_graph.current()
E = S.I.Editor
point  = E.find_attribute('vertices.point')

-- Iterate on all the vertices
for v=0,E.nb_vertices-1 do
   -- get coordinates of current vertex
   -- (point is a 'vector attribute', with 3
   --  elements per item).
   local x = point[3*v]
   local y = point[3*v+1]
   local z = point[3*v+2]


   x = x + (math.random() - 0.5) * 0.02
   y = y + (math.random() - 0.5) * 0.02
   z = z + (math.random() - 0.5) * 0.02

   point[3*v]   = x
   point[3*v+1] = y
   point[3*v+2] = z

end

 

