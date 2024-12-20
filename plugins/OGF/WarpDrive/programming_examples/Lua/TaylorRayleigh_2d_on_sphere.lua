-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Simulating 2D Taylor-Rayleigh instability with
-- Gallouet-Merigot scheme.

N = 5000 -- Number of points
scene_graph.clear()
Omega = scene_graph.create_object(OGF.MeshGrob,'Omega')
Omega.I.Shapes.create_sphere()  
Omega.I.Points.sample_surface({nb_points=N})
points = scene_graph.resolve('points')
scene_graph.current_object = 'points'

E = points.I.Editor

point = E.find_attribute('vertices.point')
mass  = E.find_or_create_attribute('vertices.mass')

for v = 0,E.nb_vertices-1 do
   local x = point[3*v]
   local y = point[3*v+1]
   local z = point[3*v+2]
   local f = 0.7*math.sin(x*5)*math.sin(y*5)
   if z > f then
      mass[v] = 3
   else
      mass[v] = 1
   end 
end

points.shader.painting='ATTRIBUTE'
points.shader.attribute='vertices.mass'
points.shader.colormap = 'blue_red;true;0;false;false;;'
points.shader.autorange()

points.I.TransportCommands.init_Euler(
  {omega='Omega', mode='EULER_ON_SURFACE'}
)

-- Open the dialog that launches the simulation
autogui.open_command_dialog_for_current_object(
   'OGF::MeshGrobTransportCommands',
   'Euler_on_surface',
   {nb_iter=100}
)




