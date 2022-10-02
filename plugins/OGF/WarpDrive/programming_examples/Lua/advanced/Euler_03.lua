-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Euler step 03
--   Simple Euler simulator for point masses
-- with chocs with search grid (disclaimer: super naive)

N=500 -- Number of points. Try this: N=500, N=1000

-- Computes one step of Euler simulation
function Euler_step()
   local tau = 0.005 -- Timestep
   local G=0.0       -- Gravity in m/s^2 (no gravity)

   -- Make particles bounce on borders
   for v = 0,E.nb_vertices-1 do
      bounce_on_borders(v)
   end

   -- Handle particle-particle collisions
   -- (with search grid)

   search_grid.init()
   NL.blas.copy(V, V_new)
   if(Euler_dialog.collisions) then
      local nb_tests = 0.0
      for v1=0,E.nb_vertices-1 do
          local U1,V1 = search_grid.v_to_UV(v1)
          for U=U1-1,U1+1 do
            for V=V1-1,V1+1 do
               local v2 = search_grid.first_by_UV(U,V)
               while v2 ~= -1 do
	              nb_tests = nb_tests+1
                  bounce(v1,v2)
                  v2 = next[v2]
               end
            end
          end
       end
    end
--  print('Avg nb tests per vertex '..tostring(nb_tests/N))
   
   NL.blas.copy(V_new,V)


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
   local damp = 1.0 -- No damping
   
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

-- \brief Detects and simulates chocs with other
--  particles
-- \details On entry, both V and V_new contain the old speeds
--          On exit, V_new contains the new speeds
-- \param[in] v1 , v2 indices of the two particle

function bounce(v1,v2)
   -- Damping of speed when there is a choc
   local damp = 1.0 -- No damping

   if not choc(v1,v2) then
      return
   end
 
   local m1 = mass[v1]
   local m2 = mass[v2]

   local Vgx = (m1*V[2*v1]   + m2*V[2*v2]  ) / (m1+m2)
   local Vgy = (m1*V[2*v1+1] + m2*V[2*v2+1]) / (m1+m2)

   local Vgnorm = math.sqrt(Vgx*Vgx+Vgy*Vgy)

   local Nx=-Vgy/Vgnorm
   local Ny= Vgx/Vgnorm

   local dot = V[2*v1]*Nx + V[2*v1+1]*Ny
   V_new[2*v1  ] = damp * (V[2*v1]   - 2*dot*Nx)
   V_new[2*v1+1] = damp * (V[2*v1+1] - 2*dot*Ny)

   dot = V[2*v2]*Nx + V[2*v2+1]*Ny
   V_new[2*v2  ] = damp * (V[2*v2]   - 2*dot*Nx)
   V_new[2*v2+1] = damp * (V[2*v2+1] - 2*dot*Ny)

end

search_grid = {}
search_grid.nU = math.floor(math.sqrt(N))*5
search_grid.nV = search_grid.nU
search_grid.first = gom.create('OGF::NL::Vector')

search_grid.first.resize({
   new_size=search_grid.nU * search_grid.nV,
   new_dim=1,
   element_meta_type=gom.resolve_meta_type('int')}
)

print('search grid cell size: '..tostring(search_grid.nU))

function search_grid.uv(x,y)
   local U = math.min(math.floor(x*search_grid.nU),search_grid.nU-1)
   local V = math.min(math.floor(x*search_grid.nV),search_grid.nV-1)
   return U,V   
end 

function search_grid.linear_index(U,V)
   return V*search_grid.nU + U
end

function search_grid.insert(U,V,v)
   local i = search_grid.linear_index(U,V)
   next[v] = search_grid.first[i]
   search_grid.first[i] = v
end

function search_grid.init()
   for i=0,search_grid.nU*search_grid.nV-1 do
      search_grid.first[i] = -1
   end
   for v=0,N-1 do
      local x = point[3*v]
      local y = point[3*v+1]
      local U,V = search_grid.uv(x,y)
      search_grid.insert(U,V,v)
   end
end

function search_grid.first_by_UV(U,V)
   if U < 0 or V < 0 or
      U >= search_grid.nU or
      V >= search_grid.nV
   then
      return -1
   end
   local i = search_grid.linear_index(U,V)
   return search_grid.first[i]
end

function search_grid.v_to_UV(v)
  local x = point[3*v]
  local y = point[3*v+1]
  local U,V = search_grid.uv(x,y)
  return U,V
end


-- \brief Detects whether there is a choc between two particles
-- \detail There is a choc if particles are nearer to eachother than
--   particle radius, and if one particle moves towards the other one
-- \param[in] v1 , v2 the indices of the particle
-- \retval true if there is a choc between v1 and v2
-- \retval false otherwise

function choc(v1,v2)
    if v1==v2 then
       return false
    end
    local x1 = point[3*v1]
    local y1 = point[3*v1+1]
    local x2 = point[3*v2]
    local y2 = point[3*v2+1]
    local dx = x2-x1
    local dy = y2-y1
    local dist = dx*dx+dy*dy
    if dist < radius2 then
         local vx1 = V[2*v1]
         local vy1 = V[2*v1+1]
         local vx2 = V[2*v1]
         local vy2 = V[2*v1+1]
         if(
             dx*vx1+dy*vy1 > 0.0 or
             dx*vx2+dy*vy2 < 0.0
         ) then
	         return true
         end
    end
    return false
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
V_new    = E.find_or_create_attribute(
   {attribute_name='vertices.speed',dimension=2}
)
next     = E.find_or_create_attribute(
   {attribute_name='vertices.next',dimension=1,type=gom.resolve_meta_type('int')}
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
   local scale = 0.03
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
points.shader.vertices_style='true; 0 1 0 1; 3'
radius2 = 0.001
points.shader.painting='ATTRIBUTE'
points.shader.attribute='vertices.id'
points.shader.autorange()

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
Euler_dialog.show_speeds = true
Euler_dialog.collisions = true
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
   _,Euler_dialog.collisions =
       imgui.Checkbox('##collisions', Euler_dialog.collisions)
   imgui.SameLine()
   imgui.Text('collisions')
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




