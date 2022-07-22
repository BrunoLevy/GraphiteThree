-- Lua (Keep this comment, this is an indicator for editor's 'run' command)

-- A simple example that creates a MeshGrob using the MeshEditor


-- Create the surface if it does not already exist,
-- so that one can modify and run the program multiple
-- times.
S = scene_graph.find_or_create_object('OGF::MeshGrob','Surface')

-- Show the vertices and the mesh
S.shader.vertices_style='true; 0 1 0 1; 3'
S.shader.mesh_style='true; 0 0 0 1; 3'

-- Create a MeshEditor object
E = S.I.Editor

-- Clear the existing mesh (so that you can
-- run the program multiple times)
E.clear()

-- Create the 8 vertices of a cube
E.create_vertex(0.0, 0.0, 0.0)
E.create_vertex(0.0, 0.0, 1.0)
E.create_vertex(0.0, 1.0, 0.0)
E.create_vertex(0.0, 1.0, 1.0)
E.create_vertex(1.0, 0.0, 0.0)
E.create_vertex(1.0, 0.0, 1.0)
E.create_vertex(1.0, 1.0, 0.0)
E.create_vertex(1.0, 1.0, 1.0)

-- Create the 6 faces of the cube
E.create_quad(2,3,1,0)
E.create_quad(4,5,7,6)
E.create_quad(0,1,5,4)
E.create_quad(1,3,7,5)
E.create_quad(3,2,6,7)
E.create_quad(2,0,4,6)

-- Compute facet adjacencies
E.connect_facets()






