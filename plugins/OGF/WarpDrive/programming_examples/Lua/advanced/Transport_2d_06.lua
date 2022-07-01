-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Tutorial on Optimal Transport
-- Step 06: Transport with KMT step-length control
--   (Kitagawa Merigot Thibert) and varying density

N = 2500            -- Number of points (try with 10000)
shrink_points=false -- Try this: set to true

-- ---------------------------------------------------
-- Computes transport when button is pushed
-- ---------------------------------------------------

function compute()
   for k=1,500 do
      local err = one_Newton_step()
      RVD.redraw()
      -- We consider convergence is reached when largest error
      -- is smaller than 1% of cell measure prescription
      if err < 1.0 / 100.0 then
         break
      end
   end
end

smallest_cell_threshold = -1.0

-- \brief Does one step of Newton optimization for transport
-- \return the largest relative cell area error

function one_Newton_step()

   -- Measure of the whole domain
   local Omega_measure = OT.Omega_measure(Omega, 'EULER_2D')

   -- compute L(Laplacian) and b(init. with Laguerre cells areas)
   local L = NL.create_matrix(N,N) -- P1 Laplacian of Laguerre cells
   local b = NL.create_vector(N)   -- right-hand side
   OT.compute_Laguerre_cells_P1_Laplacian(
      Omega, weight, L, b, 'EULER_2D'
   )

   -- compute minimal legal area for Laguerre cells (KMT criterion #1)
   -- (computed at the first run, when Laguerre diagram = Voronoi diagram)
   if smallest_cell_threshold == -1.0 then
      local smallest_cell_area = b[0]
      for i=1,N-1 do
         smallest_cell_area = math.min(smallest_cell_area, b[i])
      end
      smallest_cell_threshold =
         0.5 * math.min(smallest_cell_area, Omega_measure/N)
   end

   for i=0,N-1 do
     local nu_i = Omega_measure/N  -- desired area for Laguerre cell i
     b[i] = nu_i - b[i]            -- rhs = desired area - actual area
   end

   local g_norm = NL.blas.nrm2(b)  -- norm of gradient at curent step
                                   -- (used by KMT criterion #2)

   local p = NL.create_vector(N)   -- Newton step vector
   L.solve_symmetric(b,p)          -- solve for p in Lp=b
   local alpha = 1.0               -- Steplength
   local weight2 = NL.create_vector(N)

   local largest_cell_error = 0.0

   -- Divide steplength by 2 until both KMT criteria are satisfied
   for k=1,20 do
      NL.blas.copy(weight, weight2)
      NL.blas.axpy(alpha, p, weight2) -- weight2 = weight + alpha * p
      OT.compute_Laguerre_cells_measures(Omega, weight2, b, 'EULER_2D')
      local smallest_cell_area = b[0]
      for i=1,N-1 do
         smallest_cell_area = math.min(smallest_cell_area, b[i])
      end

      -- Compute gradient norm at current substep
      local g_norm_substep = 0.0
      largest_cell_error = 0.0
      for i=0,N-1 do
         local g_i = b[i] - Omega_measure/N
	 largest_cell_error = math.max(largest_cell_error, math.abs(g_i))
         g_norm_substep = g_norm_substep + g_i * g_i
      end
      g_norm_substep = math.sqrt(g_norm_substep)

      -- KMT criterion #1 (Laguerre cell area)
      local KMT_1 = (smallest_cell_area > smallest_cell_threshold)
      
      -- KMT criterion #2 (gradient norm)      
      local KMT_2 = (g_norm_substep <= (1.0 - 0.5*alpha) * g_norm)

      if KMT_1 and KMT_2 then
         break
      end
      
      alpha = alpha / 2.0
   end

   NL.blas.copy(weight2, weight)

   OT.compute_Laguerre_diagram(Omega, weight, RVD, 'EULER_2D')   
   RVD.shader.autorange()
   RVD.update()

   return largest_cell_error / (Omega_measure / N)
end

-- -------------------------------------------------
-- Create domain Omega (a square)
-- -------------------------------------------------
scene_graph.clear()
Omega = scene_graph.create_object('OGF::MeshGrob')
Omega.rename('Omega')
Omega.query_interface('OGF::MeshGrobShapesCommands').create_square()
Omega.query_interface('OGF::MeshGrobSurfaceCommands').split_quads(5)
Omega.query_interface('OGF::MeshGrobSurfaceCommands').triangulate()


-- Create points (random sampling of Omega)
Omega.query_interface('OGF::MeshGrobPointsCommands').sample_surface(
   {nb_points=N,Lloyd_iter=0,Newton_iter=0}
)
scene_graph.current_object = 'points'
points = scene_graph.resolve('points')


-- Create 'weight' attribute in Omega (piecewise linear source measure)
E = Omega.query_interface('OGF::MeshGrobEditor')
coords = E.find_attribute('vertices.point')
density = E.create_attribute('vertices.weight')
for i=0,#density-1 do
   local x = coords[3*i]
   local y = coords[3*i+1]
   local s = 0.5 * (1.0 + math.sin(x*10) * math.sin(y*10))
   density[i] = 0.001 + s*s
end
Omega.shader.painting='ATTRIBUTE'
Omega.shader.attribute='vertices.weight'
Omega.shader.colormap = 'plasma;false;1;false;false;;'
Omega.shader.autorange() 



-- -------------------------------------------------
-- Get access to point coords
-- -------------------------------------------------
E = points.query_interface('OGF::MeshGrobEditor')
coords = E.find_attribute('vertices.point')

-- -------------------------------------------------
-- Shrink points
-- -------------------------------------------------
if shrink_points then
   for i=0,N-1 do
      coords[3*i]   = 0.125 + coords[3*i]/4.0
      coords[3*i+1] = 0.125 + coords[3*i+1]/4.0
   end
end

points.visible=false

-- -------------------------------------------------
-- Create diagram
-- -------------------------------------------------
RVD = scene_graph.create_object('OGF::MeshGrob')
RVD.rename('RVD')

-- -------------------------------------------------
-- Compute diagram
-- -------------------------------------------------
OT = points.query_interface('OGF::MeshGrobTransport')
weight   = NL.create_vector(N)
OT.compute_Laguerre_diagram(Omega, weight, RVD, 'EULER_2D')   

-- -------------------------------------------------
-- Change graphic attributes of diagram
-- -------------------------------------------------
RVD.shader.painting='ATTRIBUTE'
RVD.shader.attribute='facets.chart'
RVD.shader.colormap = 'plasma;false;732;false;false;;'
RVD.shader.autorange() 

if shrink_points then
   Omega.visible = false
end

-- ------------------------------------------
-- GUI
-- ------------------------------------------
 
OT_dialog = {} 
OT_dialog.visible = true
OT_dialog.name = 'Transport' 
OT_dialog.x = 100
OT_dialog.y = 400
OT_dialog.w = 150
OT_dialog.h = 200
OT_dialog.width = 400

function OT_dialog.draw_window()
   if imgui.Button('Compute',-1,-1) then
      main.exec_command('compute()')
   end
end

graphite_main_window.add_module(OT_dialog)





