-- Lua (Keep this comment, this is an indication for editor's 'run' command)
-- An example of how to modify the coordinates of a mesh in Lua
-- Try it on a sphere (shapes->create sphere)

-- If there is nothing in the scene graph, create a sphere
-- (to have something to play with), else we play with the
-- current object.
if scene_graph.nb_children == 0 then
   S = scene_graph.create_object(OGF.MeshGrob,'S')
   S.I.Shapes.create_sphere()
end

S = scene_graph.current()
E = S.I.Editor
point  = E.find_attribute('vertices.point')

-- Iterate on all the vertices
for v=0,E.nb_vertices-1 do
   -- get coordinates of current vertex
   -- (point is a 'vector attribute', with 3
   --  elements per item).
   local x = point[{v,0}]
   local y = point[{v,1}]
   local z = point[{v,2}]


   x = x + (math.random() - 0.5) * 0.02
   y = y + (math.random() - 0.5) * 0.02
   z = z + (math.random() - 0.5) * 0.02

   point[{v,0}] = x
   point[{v,1}] = y
   point[{v,2}] = z

end
