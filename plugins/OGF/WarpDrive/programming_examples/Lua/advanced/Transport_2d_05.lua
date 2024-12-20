-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Tutorial on Optimal Transport
-- Step 05: Transport with KMT step-length control
--   (Kitagawa Merigot Thibert)

N = 1000            -- Number of points (try with 10000)
shrink_points=true

-- ---------------------------------------------------
-- Do one Newton step when button is pushed
-- ---------------------------------------------------

smallest_cell_threshold = -1.0

function compute()
   print('Newton step')

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
      print('smallest cell threshold = '..tostring(smallest_cell_threshold))
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

   -- Divide steplength by 2 until both KMT criteria are satisfied
   for k=1,10 do
      print('   Substep: k = '..tostring(k)..'  alpha='..alpha)
      NL.blas.copy(weight, weight2)
      NL.blas.axpy(alpha, p, weight2) -- weight2 = weight + alpha * p
      OT.compute_Laguerre_cells_measures(Omega, weight2, b, 'EULER_2D')
      local smallest_cell_area = b[0]
      for i=1,N-1 do
         smallest_cell_area = math.min(smallest_cell_area, b[i])
      end

      -- Compute gradient norm at current substep
      local g_norm_substep = 0.0
      for i=0,N-1 do
         local g_i = b[i] - Omega_measure/N
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

      print(
         '      KMT #1 (cell area):'..tostring(KMT_1)..' '..
	     tostring(smallest_cell_area) .. '>' ..
	     tostring(smallest_cell_threshold)
      )

      print(
         '      KMT #2 (gradient ):'..tostring(KMT_2)..' '..
	     tostring(g_norm_substep) .. '<=' ..
	     tostring((1.0 - 0.5*alpha) * g_norm)
      )

      alpha = alpha / 2.0
   end

   NL.blas.copy(weight2, weight)

   OT.compute_Laguerre_diagram(Omega, weight, RVD, 'EULER_2D')
   RVD.shader.autorange()
   RVD.update()
end

-- -------------------------------------------------
-- Create domain Omega (a square)
-- -------------------------------------------------
scene_graph.clear()
Omega = scene_graph.create_object(OGF.MeshGrob,'Omega')
Omega.I.Shapes.create_quad()
Omega.I.Surface.triangulate()

-- Create points (random sampling of Omega)
Omega.I.Points.sample_surface(
   {nb_points=N,Lloyd_iter=0,Newton_iter=0}
)
scene_graph.current_object = 'points'
points = scene_graph.resolve('points')

-- -------------------------------------------------
-- Get access to point coords
-- -------------------------------------------------
E = points.I.Editor
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

-- -------------------------------------------------
-- Create diagram
-- -------------------------------------------------
RVD = scene_graph.create_object(OGF.MeshGrob,'RVD')

-- -------------------------------------------------
-- Compute diagram
-- -------------------------------------------------
OT = points.I.Transport
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
      compute()
   end
end

graphite_main_window.add_module(OT_dialog)
