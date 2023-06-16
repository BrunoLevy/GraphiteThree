-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Tutorial on Optimal Transport
-- Step 07: "by-hand" computation of Hessian and gradient (fully in Lua)

N = 1000           -- Number of points (try with 10000)
shrink_points=true

-- ---------------------------------------------------
-- Do one Newton step when button is pushed
-- ---------------------------------------------------

smallest_cell_threshold = -1.0

-- Computes the area of a mesh triangle
-- XYZ the coordinates of the mesh vertices (Z is ignored)
-- v1,v2,v3 the three vertices of the triangle
function triangle_area(XYZ, v1, v2, v3)
  local x1 = XYZ[3*v1]
  local y1 = XYZ[3*v1+1]
  local x2 = XYZ[3*v2]
  local y2 = XYZ[3*v2+1]
  local x3 = XYZ[3*v3]
  local y3 = XYZ[3*v3+1]
  return math.abs(
    0.5 *(
      (x2-x1)*(y3-y1) - (y2-y1)*(x3-x1)
    )
  )
end

-- Computes the length of a mesh edge
-- XYZ the coordinates of the mesh vertices (Z is ignored)
-- v1 , v2 the mesh extremities index
function distance(XYZ, v1, v2)
  local x1 = XYZ[3*v1]
  local y1 = XYZ[3*v1+1]
  local x2 = XYZ[3*v2]
  local y2 = XYZ[3*v2+1]
  return math.sqrt(
    (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)
  )
end

-- Computes the linear system to be solved at each Newton step
-- This is a LUA implementation equivalent to the builtin (C++)
--   OT.compute_Laguerre_cells_P1_Laplacian(
--      Omega, weight, H, b, 'EULER_2D'
--   )
-- H the matrix of the linear system
-- b the right-hand side of the linear system

function compute_linear_system(H,b)

   -- Get the Laguerre diagram (RVD) as a triangulation
   OT.compute_Laguerre_diagram(Omega, weight, RVD, 'EULER_2D')  
   RVD.I.Surface.triangulate()
   RVD.I.Surface.merge_vertices(1e-10)

   -- vertex v's coordinates are XYZ[3*v], XYZ[3*v+1], XYZ[3*v+2]
   XYZ   = RVD.I.Editor.find_attribute('vertices.point')

   -- Triangle t's vertices indices are T[3*t], T[3*t+1], T[3*t+2]
   T     = RVD.I.Editor.get_triangles()

   -- The indices of the triangles adjacent to triangle t are
   --    Tadj[3*t], Tadj[3*t+1], Tadj[3*t+2]
   --  WARNING: they do not follow the usual convention for triangulations
   --  (this is because GEOGRAM also supports arbitrary polygons)
   --
   -- It is like that:
   --
   --        2
   --       / \
   -- adj2 /   \ adj1
   --     /     \
   --    /       \
   --   0 ------- 1
   --      adj0
   --
   -- (Yes, I know, this is stupid and you hate me !!)
   
   Tadj  = RVD.I.Editor.get_triangle_adjacents()

   -- chart[t] indicates the index of the seed that corresponds to the 
   --  Laguerre cell that t belongs to
   chart = RVD.I.Editor.find_attribute('facets.chart')

   -- The coordinates of the seeds
   seeds_XYZ = points.I.Editor.find_attribute('vertices.point')

   -- The number of triangles in the triangulation of the Laguerre diagram
   local nt = #T/3

   -- For each triangle of the triangulation of the Laguerre diagram
   for t=0,nt-1 do

     -- Triangle t is in the Laguerre cell of i
     local i  = chart[t]

     -- Accumulate right-hand side (Laguerre cell areas)
     b[i] = b[i] + triangle_area(XYZ,T[3*t], T[3*t+1], T[3*t+2])

     --   For each triangle edge, determine whether the triangle edge
     -- is on a Laguerre cell boundary and accumulate its contribution
     -- to the Hessian
     
     for e=0,2 do
         -- index of adjacent triangle accross edge e
         local tneigh = Tadj[3*t+e]
	 
	 -- test if we are not on Omega boundary
	 if tneigh < nt then
	 
	    -- Triangle tneigh is in the Laguerre cell of j
	    local j = chart[tneigh]
	    
	    -- We are on a Laguerre cell boundary only if t and tneigh
	    -- belong to two different Laguerre cells
	    if not (j == i) then

	       -- The two vertices of the edge e in triangle t
	       local v1 = T[3*t+e]
	       local v2 = T[3*t+((e+1)%3)]
	       
	       local hij = distance(XYZ,v1,v2) / (2.0 * distance(seeds_XYZ, i,j))
	       H.add_coefficient(i,j,-hij)    
	       H.add_coefficient(i,i,hij)
	    end
	 end
     end
   end
end

function compute()
   print('Newton step')

   -- Measure of the whole domain
   local Omega_measure = OT.Omega_measure(Omega, 'EULER_2D')

   -- compute L(Laplacian) and b(init. with Laguerre cells areas)
   local L = NL.create_matrix(N,N) -- P1 Laplacian of Laguerre cells
   local b = NL.create_vector(N)   -- right-hand side

   compute_linear_system(L,b)

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

   -- We have already computed it, but it was triangulated,
   -- We recompute it so that display is not cluttered with
   -- the triangle edges (you can comment-out the next line
   -- to see what it looks like if you are interested)
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
Omega.I.Shapes.create_square()
Omega.I.Surface.triangulate()

-- Create points (random sampling of Omega)
Omega.I.Points.sample_surface(
   {nb_points=N,Lloyd_iter=0,Newton_iter=0}
)
scene_graph.current_object = 'points'
points = scene_graph.resolve('points')

-- -------------------------------------------------
-- Shrink points
-- -------------------------------------------------
if shrink_points then
   coords = points.I.Editor.find_attribute('vertices.point')
   for i=0,N-1 do
      coords[3*i]   = 0.125 + coords[3*i]/4.0
      coords[3*i+1] = 0.125 + coords[3*i+1]/4.0
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



