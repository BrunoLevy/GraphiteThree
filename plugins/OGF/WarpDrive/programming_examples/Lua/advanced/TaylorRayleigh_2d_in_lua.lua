-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- This program implements a simple incompressible fluid simulator in Lua,
-- it is there for educational purposes. More efficient implementations of the
-- fluid simulator are available in the Transport->Compute->Eulerxx commands
-- (the latter is used in TaylorRayleigh_2d.lua, TaylorRayleigh_3d.lua etc...)

N=1000 -- Number of points.

-- Computes one step of Euler simulation
function Euler_step()
   local OT = points.I.Transport
 
   -- Timestep
   local tau = 0.001

   -- Stiffness of the 'spring' pressure force that pulls the 
   -- points towards the centroids
   local epsilon = 0.004

   -- Gravity on earth in m/s^2
   local G = 9.81

   local inveps2 = 1.0/(epsilon*epsilon)

   -- Compute the centroids of the unique Laguerre diagram defined
   -- from the points that has constant areas.
   OT.compute_optimal_Laguerre_cells_centroids(
      {Omega=Omega,centroids=centroid,weights=weight,mode='EULER_2D'}
   )

   -- Update forces, speeds and positions (Explicit Euler scheme, super simple !)
   for v = 0,E.nb_vertices-1 do
      -- Compute forces: F = spring_force(point, centroid) - m G Z
      local Fx = inveps2 * (centroid[2*v]   - point[3*v]  )
      local Fy = inveps2 * (centroid[2*v+1] - point[3*v+1]) - mass[v] * G
      -- V += tau * a ; F = ma ==> V += tau * F / m
      V[2*v]   = V[2*v]   + tau * Fx / mass[v]
      V[2*v+1] = V[2*v+1] + tau * Fy / mass[v]
      -- position += tau * V
      point[3*v]   = point[3*v]   + tau*V[2*v]
      point[3*v+1] = point[3*v+1] + tau*V[2*v+1]
   end
   if Euler_dialog.show_Laguerre then
      RVD.visible = true
      if RVD.shader.attribute == 'facets.mass' then
         RVD.shader.colormap = 'blue_red;false;0;false;false;;'
      else
         RVD.shader.colormap = 'plasma;false;0;true;false;;'
      end
      OT.compute_Laguerre_diagram({
          Omega=Omega,
          weights=weight,
          RVD=RVD,
          mode='EULER_2D'
      })
      for f=0,#RVD_mass-1 do
          local v = RVD_chart[f]
          RVD_mass[f] = mass[v]
	      RVD_V[2*f] = V[2*v]
	      RVD_V[2*f+1] = V[2*v+1]	  
      end
      RVD.shader.autorange()      
   end
   points.redraw()
end


scene_graph.clear()
Omega = scene_graph.create_object('OGF::MeshGrob')
Omega.rename('Omega')
Omega.I.Shapes.create_quad()
Omega.I.Surface.triangulate()
Omega.I.Points.sample_surface({nb_points=N})
scene_graph.current_object = 'points'
points = scene_graph.resolve('points')

RVD = scene_graph.create_object('OGF::MeshGrob')
RVD.rename('RVD')
RVD_E = RVD.I.Editor
RVD_chart = RVD_E.find_or_create_attribute(
    'facets.chart',1,gom.resolve_meta_type('OGF::index_t')
)
RVD_mass  = RVD_E.find_or_create_attribute('facets.mass')
RVD_V     = RVD_E.find_or_create_attribute(
    {attribute_name='facets.speed',dimension=2}
)


E = points.I.Editor

-- Low level access to point coordinates
point    = E.find_attribute('vertices.point')

-- Attributes attached to each vertex:
-- mass, speed vector and centroid of Laguerre cell
mass     = E.find_or_create_attribute('vertices.mass')
V        = E.find_or_create_attribute(
   {attribute_name='vertices.speed',dimension=2}
)
centroid = NL.create_vector()
weight   = NL.create_vector()

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
-- Start with points at centroids, and initial speeds at zero.
function Euler_init()
   local OT = points.I.Transport
   OT.compute_optimal_Laguerre_cells_centroids(
      {Omega=Omega,centroids=centroid,weights=weight,mode='EULER_2D'}
   )
   for v = 0,E.nb_vertices-1 do
      point[3*v]   = centroid[2*v]
      point[3*v+1] = centroid[2*v+1]
      V[2*v]   = 0.0
      V[2*v+1] = 0.0 
   end
   points.update()
end

Euler_init()

function Euler_steps(n)
   for i=1,n do
      Euler_step()
      if Euler_dialog.stopped then
         break
      end
   end
   Euler_dialog.stopped = false
end

-- ------------------------------------------
-- GUI
-- ------------------------------------------
 
Euler_dialog = {} 
Euler_dialog.visible = true
Euler_dialog.name = 'Euler' 
Euler_dialog.x = 100
Euler_dialog.y = 400
Euler_dialog.w = 150
Euler_dialog.h = 250
Euler_dialog.width = 400
Euler_dialog.nb_steps = 1000
Euler_dialog.show_Laguerre = false
Euler_dialog.stopped = false

function Euler_dialog.draw_window()
   imgui.PushItemWidth(-1)
   imgui.Text('nb timesteps')
    _,Euler_dialog.nb_steps = 
       imgui.InputInt('##nb_steps',Euler_dialog.nb_steps)
   imgui.Text('Laguerre')
   imgui.SameLine()
   local sel
   sel,Euler_dialog.show_Laguerre = 
       imgui.Checkbox('##show_Laguerre',Euler_dialog.show_Laguerre)
   if sel then
      if Euler_dialog.show_Laguerre then
         points.visible=false
         Omega.visible=false
	     RVD.shader.painting = 'ATTRIBUTE'
         if RVD.shader.attribute == 'vertices.point[0]' then
   	        RVD.shader.attribute = 'facets.mass'
         end
	     RVD.shader.autorange()
      else
         points.visible=true
         Omega.visible=true
         RVD.visible=false
      end
   end
   if Euler_dialog.show_Laguerre then
      _,RVD.shader.attribute=imgui.Combo(
         '##RVD_attribute',
         RVD.shader.attribute,RVD.shader.scalar_attributes
      )
   end
   imgui.Separator()
   if imgui.Button('run Euler',-1,0) then
       -- we need to 'exec_command' rather than directly calling
       -- the function, this is to ensure that graphic updates will
       -- be possible during the simulation loop.
       main.exec_command('Euler_steps(Euler_dialog.nb_steps)')
   end
   if imgui.Button('Stop',-1,0) then
      Euler_dialog.stopped = true 
   end
   imgui.PopItemWidth()
end

graphite_main_window.add_module(Euler_dialog)





