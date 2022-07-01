-- Lua (Keep this comment, this is an indication for editor's 'run' command)

N = 20000 -- number of points

scene_graph.clear()
Omega = scene_graph.create_object('OGF::MeshGrob')
Omega.rename('Omega')
Omega.query_interface('OGF::MeshGrobShapesCommands').create_cube()
Omega.query_interface('OGF::MeshGrobSurfaceCommands').triangulate()
Omega.shader.surface_style = 'true; 0.5 0.5 0.5 0.0'
Omega.shader.volume_style = 'false; 1 1 0 1'
Omega.shader.mesh_style = 'true; 0 0 0 1; 1'
Omega.query_interface('OGF::MeshGrobPointsCommands').sample_volume(
  {nb_points=N}
)
scene_graph.current_object = 'points'

points = scene_graph.resolve('points')

E = points.query_interface('OGF::MeshGrobEditor')

point = E.find_attribute('vertices.point')
mass  = E.find_or_create_attribute('vertices.mass')


for v = 0,E.nb_vertices-1 do
   local x = point[3*v]
   local y = point[3*v+1]
   local z = point[3*v+2]
   local f = -0.1*math.cos((x-0.5)*10)*math.cos((y-0.5)*10)
   if z-0.5 > f then
      mass[v] = 3
   else
      mass[v] = 1
   end 
end
 
points.shader.painting='ATTRIBUTE'
points.shader.attribute='vertices.mass'
points.shader.colormap = 'transparent;true;0;false;false;;'
points.shader.autorange()

points.query_interface('OGF::MeshGrobTransportCommands').init_Euler(
   {omega='Omega', mode='EULER_3D'}
)

-- Open the dialog that launches the simulation
autogui.open_command_dialog_for_current_object(
   'OGF::MeshGrobTransportCommands',
   'Euler3d',
   {nb_iter=100}
)

