-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Tutorial on Optimal Transport
-- Step 02: Change the weight of a point
--  in a Laguerre diagram

N = 100 -- Number of points

-- -------------------------------------------------------
-- This function is called each time the 'Compute' button
--   is pushed.
-- It changes the weights of a point in the Laguerre diagram
-- ------------------------------------------------------

function compute()
   weight[0] = weight[0] + 0.01
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



