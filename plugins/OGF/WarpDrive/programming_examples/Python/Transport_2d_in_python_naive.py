# Tutorial on Optimal Transport
# "by-hand" computation of Hessian and gradient (almost fully in Python)
# Naive version, with for loops (see Transport_2d_in_python for a more
# efficient version)

import math, numpy as np

OGF=gom.meta_types.OGF # shortcut to OGF.MeshGrob for instance

N = 1000 # number of points, try 10000

class Transport:

  # \param[in] N number of points
  # \param[in] shrink_points if set, group points in a small zone
  def __init__(self, N, shrink_points):
    self.verbose = False
    self.N = N

    # Create domain Omega (a square)
    scene_graph.clear()
    self.Omega = scene_graph.create_object(OGF.MeshGrob,'Omega')
    self.Omega.I.Shapes.create_quad()
    self.Omega.I.Surface.triangulate()

    # Create points (random sampling of Omega)
    self.Omega.I.Points.sample_surface(nb_points=N,Lloyd_iter=0,Newton_iter=0)
    self.points = scene_graph.objects.points

    # Shrink points
    if shrink_points:
      coords = np.asarray(self.points.I.Editor.get_points())
      coords[:] = 0.125 + coords/4.0
      self.points.update()

    # Compute Laguerre diagram
    self.RVD = scene_graph.create_object(OGF.MeshGrob,'RVD')
    self.OT = self.points.I.Transport
    self.weight = np.zeros(self.N,np.float64)
    self.OT.compute_Laguerre_diagram(
      self.Omega, self.weight, self.RVD, 'EULER_2D'
    )

    # Measure of the whole domain
    self.Omega_measure = self.OT.Omega_measure(self.Omega, 'EULER_2D')
    self.nu_i = self.Omega_measure / self.N

    # minimal legal area for Laguerre cells (KMT criterion #1)
    # (computed at the first run, when Laguerre diagram = Voronoi diagram)
    self.smallest_cell_threshold = -1.0

    # Change graphic attributes of diagram
    self.Omega.visible=False
    self.RVD.shader.painting='ATTRIBUTE'
    self.RVD.shader.attribute='facets.chart'
    self.RVD.shader.colormap = 'plasma;false;732;false;false;;'
    self.RVD.shader.autorange()

  # \brief Computes the optimal transport
  def compute(self):
    threshold = self.nu_i * 0.01
    while(self.one_iteration() > threshold):
      pass

  # \brief One iteration of Newton
  # \return the measure error of the worst cell
  def one_iteration(self):
    if self.verbose:
      print('Newton step')

    # Get the Laguerre diagram (RVD) as a triangulation
    #  used by compute_Hessian() and compute_Laguerre_cells_measures()
    self.OT.compute_Laguerre_diagram(
      self.Omega, self.weight, self.RVD, 'EULER_2D'
    )
    self.RVD.I.Surface.triangulate()
    self.RVD.I.Surface.merge_vertices(1e-10)

    # compute Hessian and b(init. with Laguerre cells areas)
    H = self.compute_Hessian()

    # rhs (- grad of Kantorovich dual) = actual measures - desired measures
    b = self.compute_Laguerre_cells_measures()

    # compute minimal legal area for Laguerre cells (KMT criterion #1)
    # (computed at the first run, when Laguerre diagram = Voronoi diagram)
    if self.smallest_cell_threshold == -1.0:
      self.smallest_cell_threshold = 0.5 * min(np.min(b), self.nu_i)
      if self.verbose:
        print('smallest cell threshold = '+str(self.smallest_cell_threshold))

    # desired area for Laguerre cells (same for all pt, but could be different)
    b[:] = self.nu_i - b # rhs = desired areas - actual areas

    g_norm = np.linalg.norm(b) # norm of gradient at curent step
                               # (used by KMT criterion #2)

    p = np.ndarray(self.N,np.float64) # Newton step
    H.solve_symmetric(b,p)       # solve for p in Lp=b
    alpha = 1.0                  # Steplength

    # Divide steplength by 2 until both KMT criteria are satisfied
    for k in range(10):
      if self.verbose:
        print('   Substep: k = '+str(k)+'  alpha='+str(alpha))
      weight2 = self.weight + alpha * p

      # rhs (- grad of Kantorovich dual) = actual measures - desired measures
      self.OT.compute_Laguerre_cells_measures(self.Omega, weight2, b, 'EULER_2D')
      smallest_cell_area = np.min(b)
      b -= self.nu_i
      g_norm_substep = np.linalg.norm(b)

      # KMT criterion #1 (Laguerre cell area)
      KMT_1 = (smallest_cell_area > self.smallest_cell_threshold)

      # KMT criterion #2 (gradient norm)
      KMT_2 = (g_norm_substep <= (1.0 - 0.5*alpha) * g_norm)

      if self.verbose:
        print(
          '      KMT #1 (cell area):'+str(KMT_1)+' '+
      	  str(smallest_cell_area) + '>' +
      	  str(self.smallest_cell_threshold)
        )

        print(
          '      KMT #2 (gradient ):'+str(KMT_2)+' '+
      	  str(g_norm_substep) + '<=' +
      	  str((1.0 - 0.5*alpha) * g_norm)
        )

      if KMT_1 and KMT_2:
         break

      alpha = alpha / 2.0

    np.copyto(self.weight, weight2)

    # We have already computed it, but it was triangulated,
    # We recompute it so that display is not cluttered with
    # the triangle edges (you can comment-out the next line
    # to see what it looks like if you are interested)
    self.OT.compute_Laguerre_diagram(
      self.Omega, self.weight, self.RVD, 'EULER_2D'
    )
    self.RVD.shader.autorange()
    self.RVD.update()

    # Return measure error of the worst cell
    worst_cell_measure_error = np.max(np.abs(b))
    worst_percent = 100.0 * worst_cell_measure_error / self.nu_i
    if self.verbose:
      print(
        'Worst cell measure error = ' + str(worst_cell_measure_error) + \
        '(' + str(worst_percent) + '%)'
      )
    return worst_cell_measure_error

  # \brief Computes the matrix of the linear system to be solved
  #  at each Newton step
  # \details Uses the current Laguerre diagram (in self.RVD).
  #  This is a Python implementation equivalent to the builtin (C++)
  #   OT.compute_Laguerre_cells_P1_Laplacian(
  #      Omega, weight, H, b, 'EULER_2D'
  #   )
  # \return the Hessian matrix of the Kantorovich dual

  def compute_Hessian(self):
    # Creates a sparse matrix using OpenNL
    H = NL.create_matrix(self.N,self.N)

    # vertex v's coordinates are XY[v][0], XY[v][1]
    XY = np.asarray(self.RVD.I.Editor.get_points())

    # Triangle t's vertices indices are T[t][0], T[t][1], T[t][2]
    T = np.asarray(self.RVD.I.Editor.get_triangles())

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

    Tadj = np.asarray(self.RVD.I.Editor.get_triangle_adjacents())

    # trgl_seed[t] indicates the index of the seed that corresponds to the
    #  Laguerre cell that contains t
    trgl_seed = np.asarray(self.RVD.I.Editor.find_attribute('facets.chart'))

    # The coordinates of the seeds
    seeds_XY = np.asarray(self.points.I.Editor.find_attribute('vertices.point'))

    # The number of triangles in the triangulation of the Laguerre diagram
    nt = T.shape[0]

    # For each triangle of the triangulation of the Laguerre diagram
    for t in range(nt):

      # Triangle t is in the Laguerre cell of i
      i = trgl_seed.item(t) # item() instead of trgl_seed[t] that is a 1x1 mtx

      #   For each triangle edge, determine whether the triangle edge
      # is on a Laguerre cell boundary and accumulate its contribution
      # to the Hessian
      for e in range(3):
        # index of adjacent triangle accross edge e
        tneigh = Tadj[t,e]

	# test if we are inside mesh (not on Omega boundary)
        if tneigh < nt:

          # Triangle tneigh is in the Laguerre cell of j
          j = trgl_seed[tneigh]

	  # We are on a Laguerre cell boundary only if t and tneigh
	  # belong to two different Laguerre cells
          if j != i:

            # The two vertices of the edge e in triangle t
            v1 = T[t,e]
            v2 = T[t,((e+1)%3)]

            hij = self.distance(XY,v1,v2) / (2.0 * self.distance(seeds_XY, i,j))
            H.add_coefficient(i,j,-hij)
            H.add_coefficient(i,i,hij)
    return H

  # \brief Computes the measures of the Laguerre cells
  # \details Uses the current Laguerre diagram (in self.RVD).
  #  This is a Python implementation equivalent to the builtin (C++)
  #   OT.compute_Laguerre_cells_measures(
  #      Omega, weights, b, 'EULER_2D'
  #   )
  # \return an array with the cells measures
  def compute_Laguerre_cells_measures(self):
    b = np.ndarray(self.N,np.float64)
    # See comments about XY,T,trgl_seed,nt in compute_Hessian()
    XY = np.asarray(self.RVD.I.Editor.get_points())
    T = np.asarray(self.RVD.I.Editor.get_triangles())
    trgl_seed = np.asarray(self.RVD.I.Editor.find_attribute('facets.chart'))
    nt = T.shape[0]
    b[:] = 0
    for t in range(nt):
      i = trgl_seed.item(t) # item() instead of trgl_seed[t] that is a 1x1 mtx
      b[i] += self.triangle_area(XY, T[t])

    return b


  # \brief Computes the area of a mesh triangle
  # \param[in] XY the coordinates of the mesh vertices
  # \param[in] T an array with the three vertices indices of the triangle
  # \return the area of the triangle
  def triangle_area(self, XY, T):
    v1 = T[0]
    v2 = T[1]
    v3 = T[2]
    U = XY[v2] - XY[v1]
    V = XY[v3] - XY[v1]
    return abs(0.5*(U[0]*V[1] - U[1]*V[0]))

  # \brief Computes the length of a mesh edge
  # \param[in] XY the coordinates of the mesh vertices
  # \param[in] v1 , v2 the mesh extremities index
  # \return the distance between the two vertices
  def distance(self, XY, v1, v2):
    return np.linalg.norm(XY[v2]-XY[v1])

transport = Transport(N,True)
# transport.verbose = True # uncomment to display Newton convergence

def compute():
  transport.compute()

def one_iteration():
  transport.one_iteration()


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
   imgui.PushItemWidth(-1)
   if imgui.Button('Compute transport') then
      gom.interpreter('Python').globals.compute()
   end
   if imgui.Button('One iteration') then
      gom.interpreter('Python').globals.one_iteration()
   end
   imgui.PopItemWidth()
end

graphite_main_window.add_module(OT_dialog)

""",save_in_history=False,log=False)
