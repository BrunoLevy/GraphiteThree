-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Tutorial on Optimal Transport
-- Step 04: Simple transport (no step-length control)

N = 100             -- Number of points
shrink_points=false -- Try this: set to true 

-- ---------------------------------------------------
-- Do one Newton step when button is pushed
-- ---------------------------------------------------

function compute()
   -- Measure of the whole domain
   local Omega_measure = OT.Omega_measure(Omega, 'EULER_2D')

   -- compute L(Laplacian) and b(init. with Laguerre cells areas)
   local L = NL.create_matrix(N,N) -- P1 Laplacian of Laguerre cells
   local b = NL.create_vector(N)   -- right-hand side
   OT.compute_Laguerre_cells_P1_Laplacian(
      Omega, weight, L, b, 'EULER_2D'
   )
   
   for i=0,N-1 do
     local nu_i = Omega_measure/N  -- desired area for Laguerre cell i
     b[i] = nu_i - b[i]            -- rhs = desired area - actual area
   end

   local p = NL.create_vector(N)   -- Newton step vector
   L.solve_symmetric(b,p)          -- solve for p in Lp=b
   local alpha = 1.0/8.0           -- Steplength (constant for now)
   NL.blas.axpy(alpha, p, weight)  -- weight = weight + alpha * p

   OT.compute_Laguerre_diagram(Omega, weight, RVD, 'EULER_2D')   
   RVD.shader.autorange()
   RVD.update()
end

-- -------------------------------------------------
-- Create domain Omega (a square)
-- -------------------------------------------------
scene_graph.clear()
Omega = scene_graph.create_object('OGF::MeshGrob')
Omega.rename('Omega')
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
      coords[3*i]   = 0.25 + coords[3*i]/2.0
      coords[3*i+1] = 0.25 + coords[3*i+1]/2.0
   end
end

-- -------------------------------------------------
-- Create diagram
-- -------------------------------------------------
RVD = scene_graph.create_object('OGF::MeshGrob')
RVD.rename('RVD')

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



