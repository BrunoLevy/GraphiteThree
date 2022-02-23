-- Lua (Keep this comment, this is an indication for editor's 'run' command)
--
-- Examples for mesh attributes manipulation in Graphite
-- Modify the source, use 'Run' to take changes into account.
--
-- Note: Attribute access in Lua (through Graphite Object Model) is not very fast,
-- while OK for small-sized meshes, it may be too slow to do serious stuff on large
-- meshes.

-- Create a mesh with a sphere if not already there
S = scene_graph.find_or_create_object('OGF::MeshGrob', 'Sphere')
S.clear()
S.query_interface('OGF::MeshGrobShapesCommands').create_sphere()
 
-- Create a MeshGrobEditor object, that has low-level access to the 
-- mesh and its attribute.
E = S.query_interface('OGF::MeshGrobEditor')

-- Create a vertex attribute attached to the vertices of the mesh
-- and named 'foobar'
foobar = E.find_or_create_attribute('vertices.foobar')
 
-- Access the vertices geometry and the new attribute
point  = E.find_attribute('vertices.point')

-- Iterate on all the vertices
for v=0,E.nb_vertices-1 do
   -- get coordinates of current vertex
   -- (point is a 'vector attribute', with 3
   --  elements per item).
   local x = point[3*v]
   local y = point[3*v+1]
   local z = point[3*v+2]

   -- set the new attribute 
   foobar[v] = math.sin(5*x)*math.sin(5*y)*math.sin(5*z)
end

-- Display the result
S.shader.painting  = 'ATTRIBUTE'
S.shader.attribute = 'vertices.foobar'
S.shader.autorange()

-- Create an attribute of type 'bool', named 'vertices.selection'
-- It corresponds to the vertices selection in the Toolbox
selection = E.find_or_create_attribute('vertices.selection',1,gom.resolve_meta_type('bool'))
 
-- Iterate on all the vertices and select
-- some vertices based on attribute value.
for v=0,E.nb_vertices-1 do
   selection[v] = (foobar[v] > 0.4)
end

-- Try this:
--S.query_interface('OGF::MeshGrobSelectionCommands').delete_selected_vertices()

