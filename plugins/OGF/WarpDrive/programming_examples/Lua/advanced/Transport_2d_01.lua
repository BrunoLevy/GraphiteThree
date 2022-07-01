-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Tutorial on Optimal Transport
-- Step 01: compute the Laguerre diagram of a
--  set of points and visualize it

N = 100 -- Number of points

-- -------------------------------------------------
-- Create domain Omega (a square)
-- -------------------------------------------------
scene_graph.clear()
Omega = scene_graph.create_object('OGF::MeshGrob')
Omega.rename('Omega')
Omega.query_interface('OGF::MeshGrobShapesCommands').create_square()
Omega.query_interface('OGF::MeshGrobSurfaceCommands').triangulate()

-- Create points (random sampling of Omega)
Omega.query_interface('OGF::MeshGrobPointsCommands').sample_surface(
   {nb_points=N,Lloyd_iter=0,Newton_iter=0}
)
scene_graph.current_object = 'points'
points = scene_graph.resolve('points')

-- -------------------------------------------------
-- Create diagram
-- -------------------------------------------------
RVD = scene_graph.create_object('OGF::MeshGrob')
RVD.rename('RVD')

-- -------------------------------------------------
-- Compute diagram
-- -------------------------------------------------
OT = points.query_interface('OGF::MeshGrobTransport')
Omega.visible=false
weight   = NL.create_vector(N)
OT.compute_Laguerre_diagram(Omega, weight, RVD, 'EULER_2D')

-- -------------------------------------------------
-- Change graphic attributes of diagram
-- -------------------------------------------------
RVD.shader.painting='ATTRIBUTE'
RVD.shader.attribute='facets.chart'
RVD.shader.colormap = 'plasma;false;732;false;false;;'
RVD.shader.autorange() 








