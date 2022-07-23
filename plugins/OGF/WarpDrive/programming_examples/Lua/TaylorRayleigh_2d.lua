-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Simulating 2D Taylor-Rayleigh instability with
-- Gallouet-Merigot scheme.

N = 3000 -- Number of points

scene_graph.clear()
Omega = scene_graph.create_object('OGF::MeshGrob')
Omega.rename('Omega')
Omega.I.Shapes.create_square()
Omega.I.Surface.triangulate()
Omega.I.Points.sample_surface({nb_points=N})
points = scene_graph.objects.points

E = points.I.Editor

-- Low level access to point coordinates
point    = E.find_attribute('vertices.point')

-- Attributes attached to each vertex:
-- mass, speed vector and centroid of Laguerre cell
mass     = E.find_or_create_attribute('vertices.mass')

-- Initialize masses with nice sine wave, 
-- and heavy fluid on top.
for v = 0,E.nb_vertices-1 do
   local x = point[3*v]
   local y = point[3*v+1]
   local f =0.1*math.sin(x*10) 
   if (y-0.5) > f then
      mass[v] = 3
   else
      mass[v] = 1
   end
end

-- Display mass attribute.
points.shader.painting='ATTRIBUTE'
points.shader.attribute='vertices.mass'
points.shader.colormap = 'blue_red;true;0;false;false;;'
points.shader.autorange()

-- Initialize Euler simulation.
points.I.TransportCommands.init_Euler(
  {omega='Omega', mode='EULER_2D'}
)

-- Open the dialog that launches the simulation
scene_graph.current_object = 'points'
autogui.open_command_dialog_for_current_object(
   'OGF::MeshGrobTransportCommands',
   'Euler2d',
   {nb_iter=500}
)


