-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Euler step 01
--   Simple Euler simulator for point masses
--    (disclaimer: super naive)

N=10 -- Number of points. Try this: N=10, N=100, N=150

-- Computes one step of Euler simulation
function Euler_step()
   local tau = 0.005 -- Timestep
   local G=9.81      -- Gravity in m/s^2 

   -- Make particles bounce on borders
   for v = 0,E.nb_vertices-1 do
      bounce_on_borders(v)
   end

   -- Update forces, speeds and positions 
   -- (Explicit Euler scheme, super simple !)
   for v = 0,E.nb_vertices-1 do
       -- Compute forces: F = - m G Z
      local Fx = 0.0
      local Fy = - mass[v] * G
      
      -- V += tau * a ; F = ma ==> V += tau * F / m
      V[2*v]   = V[2*v]   + tau * Fx / mass[v]
      V[2*v+1] = V[2*v+1] + tau * Fy / mass[v]
 
      -- position += tau * V
      point[3*v]   = point[3*v]   + tau*V[2*v]
      point[3*v+1] = point[3*v+1] + tau*V[2*v+1]
   end
   show_speeds()
   points.redraw()
end

-- \brief make the particle bounce on the borders
--  of the domain
-- \param[in] v the index of the particle

function bounce_on_borders(v)
   -- Damping of speed when there is a choc
   local damp = 0.9
   
   if point[3*v] > 1.0 then
      point[3*v] = 1.0
      V[2*v]   = -damp * V[2*v]
      V[2*v+1] =  damp * V[2*v+1]
   end
      
   if point[3*v] < 0.0 then
      point[3*v] = 0.0
      V[2*v]   = -damp * V[2*v]
      V[2*v+1] =  damp * V[2*v+1]
   end

   if point[3*v+1] > 1.0 then
      point[3*v+1] = 1.0
      V[2*v]   =  damp * V[2*v]
      V[2*v+1] = -damp * V[2*v+1]
   end
      
   if point[3*v+1] < 0.0 then
      point[3*v+1] = 0.0
      V[2*v]   =  damp * V[2*v]	  
      V[2*v+1] = -damp * V[2*v+1]
   end
end

scene_graph.clear()
Omega = scene_graph.create_object('OGF::MeshGrob','Omega')
Omega.I.Shapes.create_square()

points = scene_graph.create_object('OGF::MeshGrob','points')
scene_graph.current_object = 'points'


E = points.I.Editor

-- Low level access to point coordinates
point    = E.find_attribute('vertices.point')

-- Attributes attached to each vertex:
-- mass, speed vector and centroid of Laguerre cell
mass     = E.find_or_create_attribute('vertices.mass')
V        = E.find_or_create_attribute(
   {attribute_name='vertices.speed',dimension=2}
)

vertex_id = E.find_or_create_attribute('vertices.id')

-- Initialize Euler simulation.
-- Start with points at centroids, and initial speeds at zero.
function Euler_init()
    if N == 2 then
       E.create_vertex(1,0)
       E.create_vertex(0,0)
       mass[0] = 1
       mass[1] = 1
       V[0] = -4
       V[1] = 4
       V[2] = 4
       V[3] = 4
       vertex_id[0] = 0
       vertex_id[1] = 1
       show_speeds()
       return
    end
    for v=0,N-1 do
       local x = 0.1 + 0.8*math.random()
       local y = 0.1 + 0.8*math.random()
       E.create_vertex(x,y)
       mass[v] = 1
       V[2*v]   = 3.0 * (math.random()-0.5)
       V[2*v+1] = 3.0 * (math.random()-0.5)
       vertex_id[v] = v
    end
    points.update()
    show_speeds()
end

-- Function to display speed vectors

speeds_display=nil
function show_speeds()
   if Euler_dialog ~= nil and not Euler_dialog.show_speeds then
       if speeds_display ~= nil then
          speeds_display.visible=false
       end
       return
   end
   if speeds_display == nil then
      speeds_display = scene_graph.create_object('OGF::MeshGrob','speeds')
      E_speeds_display = speeds_display.I.Editor
      speeds_display_points = E_speeds_display.find_attribute('vertices.point')
   end
   speeds_display.clear()
   E_speeds_display.create_vertices(2*N)
   local off = 3*N
   local scale = 0.05
   if N <= 10 then
      scale = 0.075
   end
   for v=0,N-1 do
      local x = point[3*v]
      local y = point[3*v+1]
      local Vx = V[2*v]
      local Vy = V[2*v+1]
      speeds_display_points[3*v]   = x
      speeds_display_points[3*v+1] = y
      speeds_display_points[3*v  +off] = x + scale * Vx
      speeds_display_points[3*v+1+off] = y + scale * Vy
      E_speeds_display.create_edge(v, v+N)
   end
   speeds_display.visible=true
   speeds_display.update()
end

Euler_init()

-- Display points
if N <= 10 then
   radius2 = 0.01
   points.shader.vertices_style='true; 0 1 0 1; 10'
else
   points.shader.vertices_style='true; 0 1 0 1; 4'
   radius2 = 0.001
end
points.shader.painting='ATTRIBUTE'
points.shader.attribute='vertices.id'
points.shader.autorange()

function Euler_steps(n)
   for i=1,n do
      Euler_step()
      sleep(0.01) 
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
Euler_dialog.show_speeds = true
Euler_dialog.stopped = false

function Euler_dialog.draw_window()
   imgui.PushItemWidth(-1)
   imgui.Text('nb timesteps')
    _,Euler_dialog.nb_steps = 
       imgui.InputInt('##nb_steps',Euler_dialog.nb_steps)
   _,Euler_dialog.show_speeds =
       imgui.Checkbox('##speeds', Euler_dialog.show_speeds)
   imgui.SameLine()
   imgui.Text('show speeds')
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

