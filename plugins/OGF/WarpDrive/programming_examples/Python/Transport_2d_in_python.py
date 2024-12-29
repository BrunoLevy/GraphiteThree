# Tutorial on Optimal Transport
# "by-hand" computation of Hessian and gradient (almost fully in Python)
# Version that exploit numpy array functions (much faster than naive version)

import math, ctypes, numpy as np

OGF=gom.meta_types.OGF # shortcut to OGF.MeshGrob for instance

NO_INDEX = ctypes.c_uint32(-1).value

N = 1000              # Number of points (try with 10000, 100000
shrink_points = True  # Group points in a smaller area

# Computes the area of a mesh triangle
# XY the coordinates of the mesh vertices
# T an array with the three vertices indices of the triangle
# Works also when T is an array of triangles (then it returns
# the array of triangle areas). This is why the ellipsis (...)
# is used (here it means indexing/slicing through the last dimension)
def triangle_area(XY, T):
  v1 = T[...,0]
  v2 = T[...,1]
  v3 = T[...,2]
  U = XY[v2] - XY[v1]
  V = XY[v3] - XY[v1]
  return np.abs(0.5*(U[...,0]*V[...,1] - U[...,1]*V[...,0]))

# Computes the length of a mesh edge
# XY the coordinates of the mesh vertices
# v1 , v2 the mesh extremities indices. Can be also arrays (then it
# returns the array of distances).
def distance(XY, v1, v2):
  axis = v1.ndim if type(v1) is np.ndarray else 0
  return np.linalg.norm(XY[v2]-XY[v1],axis=axis)

# Computes the linear system to be solved at each Newton step
# This is a Python implementation equivalent to the builtin (C++)
#   OT.compute_Laguerre_cells_P1_Laplacian(
#      Omega, weight, H, b, 'EULER_2D'
#   )
#  H the matrix of the linear system
#  b the right-hand side of the linear system
def compute_linear_system(H,b):
   global RVD,OT

   # Get the Laguerre diagram (RVD) as a triangulation
   OT.compute_Laguerre_diagram(Omega, weight, RVD, 'EULER_2D')
   RVD.I.Surface.triangulate()
   RVD.I.Surface.merge_vertices(1e-10)

   # vertex v's coordinates are XY[v][0], XY[v][1]
   XY = np.asarray(RVD.I.Editor.get_points())

   # Triangle t's vertices indices are T[t][0], T[t][1], T[t][2]
   T = np.asarray(RVD.I.Editor.get_triangles())

   # The indices of the triangles adjacent to triangle t are
   #    Tadj[t][0], Tadj[t][1], Tadj[t][2]
   #  WARNING: they do not follow the usual convention for triangulations
   #  (this is because GEOGRAM also supports arbitrary polygons)
   #
   # It is like that:
   #
   #        2
   #       / \
   # adj2 /   \ adj1
   #     /     \
   #    /       \
   #   0 ------- 1
   #      adj0
   #
   # (Yes, I know, this is stupid and you hate me !!)

   Tadj = np.asarray(RVD.I.Editor.get_triangle_adjacents())

   # chart[t] indicates the index of the seed that corresponds to the
   #  Laguerre cell that t belongs to
   chart = np.asarray(RVD.I.Editor.find_attribute('facets.chart'))

   # The coordinates of the seeds
   seeds_XY = np.asarray(points.I.Editor.find_attribute('vertices.point'))

   # The number of triangles in the triangulation of the Laguerre diagram
   nt = T.shape[0]

   # Compute cell areas by summing each triangle's contribution
   b[:] = 0
   np.add.at(b, chart, triangle_area(XY,T))

   # Diagonal, initialized to zero
   diag = np.zeros(N,np.float64)

   # For each triangle t, for each edge e of t ...
   # ... swap loops to exploit numpy array functions
   for e in range(3):

     # Create a qidx (quad index) array that encodes for each triangle edge e:
     #   [ i, j, v1, v2 ] where:
     #     i: seed index
     #     j: adjacent seed index (Laguerre cell on the other side of e)
     #     v1, v2: vertices of the triangle edge e
     # We assemble them in the same array so that we can remove the entries
     # that we do not want (triangle edges on the border of Omega, and triangle
     # edges that stay in the same Laguerre cell).

     qidx = np.column_stack((chart, Tadj[:,e], T[:,e], T[:,(e+1)%3]))
     #                        |         |       |           |
     # 0: seed index (i) -----'         |       +-----------'
     # 1: adj trgl accross e (for now) -'       |
     # 2,3: triangle edge (dual to Voronoi) ----'
     # (see comment on mesh indexing)

     qidx = qidx[qidx[:,1] != NO_INDEX]  # remove edges on border (adjacent = -1)
     qidx[:,1] = chart[qidx[:,1]]        # 1: adjacent seed index (j)
     qidx = qidx[qidx[:,0] != qidx[:,1]] # remove edges that stay in same cell

     I = qidx[:,0].copy()  # get arrays of i's, j's, v1's and v2's.
     J = qidx[:,1].copy()  # We need to copy I and J to have contiguous arrays
     V1 = qidx[:,2]        # (H.add_coefficients() requires that).
     V2 = qidx[:,3]

     # Now we can compute a vector of coefficient (note: V1,V2,I,J are vectors)
     coeff = -distance(XY,V1,V2) / (2.0 * distance(seeds_XY,I,J))

     # Accumulate minus the sum of extra-diagonal entries to the diagonal
     np.add.at(diag,I,-coeff)

     # Insert coefficients into matrix
     H.add_coefficients(I,J,coeff)

   # Insert diagonal into matrix
   H.add_coefficients_to_diagonal(diag)

# minimal legal area for Laguerre cells (KMT criterion #1)
# (computed at the first run, when Laguerre diagram = Voronoi diagram)

smallest_cell_threshold = -1.0

#-- ---------------------------------------------------
#-- Do one Newton step when button is pushed
#-- ---------------------------------------------------

def compute():
   global smallest_cell_threshold, smallest_cell_area, weight
   global Omega_measure, N
   print('Newton step')

   # Measure of the whole domain
   Omega_measure = OT.Omega_measure(Omega, 'EULER_2D')

   # compute L(Laplacian) and b(init. with Laguerre cells areas)
   L = NL.create_matrix(N,N) # P1 Laplacian of Laguerre cells
   b = np.ndarray(N,np.float64) # right-hand side

   compute_linear_system(L,b)

   # compute minimal legal area for Laguerre cells (KMT criterion #1)
   # (computed at the first run, when Laguerre diagram = Voronoi diagram)
   if smallest_cell_threshold == -1.0:
      smallest_cell_threshold = 0.5 * min(np.min(b), Omega_measure/N)
      print('smallest cell threshold = '+str(smallest_cell_threshold))

   # desired area for Laguerre cells (all the same here, but could be different)
   b[:] = Omega_measure/N - b # rhs = desired areas - actual areas

   g_norm = np.linalg.norm(b) # norm of gradient at curent step
                              # (used by KMT criterion #2)

   p = np.ndarray(N,np.float64)
   L.solve_symmetric(b,p)   # solve for p in Lp=b
   alpha = 1.0              # Steplength

   # Divide steplength by 2 until both KMT criteria are satisfied
   for k in range(10):
      # print('   Substep: k = '+tostring(k)+'  alpha='+alpha)
      weight2 = weight + alpha * p
      OT.compute_Laguerre_cells_measures(Omega, weight2, b, 'EULER_2D')
      smallest_cell_area = np.min(b)

      # Compute gradient norm at current substep
      g_norm_substep = np.linalg.norm(b - Omega_measure/N)

      # KMT criterion #1 (Laguerre cell area)
      KMT_1 = (smallest_cell_area > smallest_cell_threshold)

      # KMT criterion #2 (gradient norm)
      KMT_2 = (g_norm_substep <= (1.0 - 0.5*alpha) * g_norm)

      print(
         '      KMT #1 (cell area):'+str(KMT_1)+' '+
      	     str(smallest_cell_area) + '>' +
      	     str(smallest_cell_threshold)
      )

      print(
         '      KMT #2 (gradient ):'+str(KMT_2)+' '+
      	     str(g_norm_substep) + '<=' +
      	     str((1.0 - 0.5*alpha) * g_norm)
      )

      if KMT_1 and KMT_2:
         break

      alpha = alpha / 2.0

   np.copyto(weight, weight2)

   # We have already computed it, but it was triangulated,
   # We recompute it so that display is not cluttered with
   # the triangle edges (you can comment-out the next line
   # to see what it looks like if you are interested)
   OT.compute_Laguerre_diagram(Omega, weight, RVD, 'EULER_2D')
   RVD.shader.autorange()
   RVD.update()


# -------------------------------------------------
# Create domain Omega (a square)
# -------------------------------------------------
scene_graph.clear()
Omega = scene_graph.create_object(OGF.MeshGrob,'Omega')
Omega.I.Shapes.create_quad()
Omega.I.Surface.triangulate()

# Create points (random sampling of Omega)
Omega.I.Points.sample_surface(nb_points=N,Lloyd_iter=0,Newton_iter=0)
points = scene_graph.objects.points

# -------------------------------------------------
# Shrink points
# -------------------------------------------------
if shrink_points:
   coords = np.asarray(points.I.Editor.get_points())
   coords[:] = 0.125 + coords/4.0
   points.update()

# -------------------------------------------------
# Create diagram
# -------------------------------------------------
RVD = scene_graph.create_object(OGF.MeshGrob,'RVD')

# -------------------------------------------------
# Compute diagram
# -------------------------------------------------
OT = points.I.Transport
weight = np.zeros(N,np.float64)
OT.compute_Laguerre_diagram(Omega, weight, RVD, 'EULER_2D')

# -------------------------------------------------
# Change graphic attributes of diagram
# -------------------------------------------------
Omega.visible=False
RVD.shader.painting='ATTRIBUTE'
RVD.shader.attribute='facets.chart'
RVD.shader.colormap = 'plasma;false;732;false;false;;'
RVD.shader.autorange()

# ------------------------------------------
# GUI
# ------------------------------------------

# The GUI is written in Lua, and communicates
# with Python through Graphite's interop layer.
# The function for the GUI is in a big string,
# sent to the Lua interpreter.
# Calling back the Python function 'compute' from LUA is
# done as follows:
#   gom.interpreter('Python').globals.compute()

gom.interpreter("Lua").execute(command="""

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
      gom.interpreter('Python').globals.compute()
   end
end

graphite_main_window.add_module(OT_dialog)

""",save_in_history=False,log=False)
