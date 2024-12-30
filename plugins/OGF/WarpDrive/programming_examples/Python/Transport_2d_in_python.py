# Tutorial on Optimal Transport
# "by-hand" computation of Hessian and gradient (almost fully in Python)
# Version that exploit numpy array functions (much faster than naive version)

import math, ctypes, numpy as np

OGF=gom.meta_types.OGF # shortcut to OGF.MeshGrob for instance

N = 1000 # number of points, try 10000, 100000 (be ready to wait a bit)

class Transport:

  def __init__(self, N: int, shrink_points: bool):
    """
    @param[in] N number of points
    @param[in] shrink_points if set, group points in a small zone
    """
    self.verbose = False
    self.N = N

    # Create domain Omega (a square)
    scene_graph.clear()
    self.Omega = scene_graph.create_object(OGF.MeshGrob,'Omega')
    self.Omega.I.Shapes.create_quad()
    # self.Omega.I.Shapes.create_ngon(nb_edges=300) # try this instead of square
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
    self.compute_Laguerre_diagram(self.weight)

    # Measure of the whole domain
    self.Omega_measure = self.OT.Omega_measure(self.Omega, 'EULER_2D')

    # Desired area for each cell
    self.nu_i = self.Omega_measure / self.N

    # Variables for Newton iteration
    self.b = np.ndarray(self.N, np.float64) # right-hand side
    self.p = np.ndarray(self.N, np.float64) # Newton step

    # Minimal legal area for Laguerre cells (KMT criterion #1)
    self.compute_Laguerre_cells_measures(self.b)
    self.smallest_cell_threshold = 0.5 * min(np.min(self.b), self.nu_i)


    # Change graphic attributes of diagram
    self.Omega.visible=False

    self.show()

  def compute(self):
    """
    @brief Computes the optimal transport
    @details Calls one_iteration() until measure error of worst cell
      is smaller than 1%
    """
    threshold = self.nu_i * 0.01
    while(self.one_iteration() > threshold):
      pass

  def one_iteration(self):
    """
    @brief One iteration of Newton
    @return the measure error of the worst cell
    """
    if self.verbose:
      print('Newton step')

    # compute Hessian and b(init. with Laguerre cells areas)
    H = self.compute_Hessian()

    # rhs (- grad of Kantorovich dual) = desired areas - actual areas
    self.compute_Laguerre_cells_measures(self.b)
    self.b[:] = self.nu_i - self.b

    g_norm = np.linalg.norm(self.b) # norm of gradient at current step
                                    # (used by KMT criterion #2)

    H.solve_symmetric(self.b,self.p) # solve for p in Lp=b
    alpha = 1.0                     # Steplength
    self.weight += self.p           # Start with Newton step

    # Divide steplength by 2 until both KMT criteria are satisfied
    for k in range(10):
      if self.verbose:
        print('   Substep: k = '+str(k)+'  alpha='+str(alpha))

      # rhs (- grad of Kantorovich dual) = actual measures - desired measures
      self.compute_Laguerre_diagram(self.weight)
      self.compute_Laguerre_cells_measures(self.b)

      smallest_cell_area = np.min(self.b)
      self.b -= self.nu_i
      g_norm_substep = np.linalg.norm(self.b)

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
      self.weight -= alpha * self.p

    # Return measure error of the worst cell
    worst_cell_measure_error = np.max(np.abs(self.b))
    worst_percent = 100.0 * worst_cell_measure_error / self.nu_i
    if self.verbose:
      print(
        'Worst cell measure error = ' + str(worst_cell_measure_error) + \
        '(' + str(worst_percent) + '%)'
      )
    return worst_cell_measure_error

  def compute_Laguerre_diagram(self, weights: np.ndarray):
    """
    @brief Computes a Laguerre diagram from a weight vector
    @param[in] weights the weights vector
    """
    self.OT.compute_Laguerre_diagram(self.Omega, weights, self.RVD, 'EULER_2D')
    self.RVD.update()
    self.RVD.I.Surface.triangulate()
    self.RVD.I.Surface.merge_vertices(1e-10)
    self.RVD.shader.painting='ATTRIBUTE'
    self.RVD.shader.attribute='facets.chart'
    self.RVD.shader.colormap = 'plasma;false;732;false;false;;'
    self.RVD.shader.mesh_style = 'false;0 0 0 1;1' # Try this: comment this line
    self.RVD.shader.autorange()

  def compute_Hessian(self):
    """
    @brief Computes the matrix of the linear system to be solved
      at each Newton step
    @details Uses the current Laguerre diagram (in self.RVD).
    This is a Python implementation equivalent to the builtin (C++)
     OT.compute_Laguerre_cells_P1_Laplacian(
        Omega, weight, H, b, 'EULER_2D'
     )
    @return the Hessian matrix of the Kantorovich dual
    """

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

    # Diagonal, initialized to zero
    diag = np.zeros(N,np.float64)

    # special value for Tadj that indicates edge on border
    NO_INDEX = ctypes.c_uint32(-1).value

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

      qidx = np.column_stack((trgl_seed, Tadj[:,e], T[:,e], T[:,(e+1)%3]))
      #                           |         |       |           |
      #    0: seed index (i) -----'         |       +-----------'
      #    1: adj trgl accross e (for now) -'       |
      #    2,3: triangle edge (dual to Voronoi) ----'
      #    (see comment on mesh indexing)

      qidx = qidx[qidx[:,1] != NO_INDEX]  # remove edges on border
      qidx[:,1] = trgl_seed[qidx[:,1]]    # 1: adjacent seed index (j)
      qidx = qidx[qidx[:,0] != qidx[:,1]] # remove edges that stay in same cell

      I = qidx[:,0].copy()  # get arrays of i's, j's, v1's and v2's.
      J = qidx[:,1].copy()  # We need to copy I and J to have contiguous arrays
      V1 = qidx[:,2]        # (H.add_coefficients() requires that).
      V2 = qidx[:,3]

      # Now we can compute a vector of coefficient (note: V1,V2,I,J are vectors)
      coeff = -self.distance(XY,V1,V2) / (2.0 * self.distance(seeds_XY,I,J))

      # Accumulate minus the sum of extra-diagonal entries to the diagonal
      np.add.at(diag,I,-coeff)

      # Insert coefficients into matrix
      H.add_coefficients(I,J,coeff)

    # Insert diagonal into matrix
    H.add_coefficients_to_diagonal(diag)

    return H

  def compute_Laguerre_cells_measures(self, measures: np.ndarray):
    """
    @brief Computes the measures of the Laguerre cells
    @out measures: the vector of Laguerre cells measures
    @details Uses the current Laguerre diagram (in self.RVD).
     This is a Python implementation equivalent to the builtin (C++)
      OT.compute_Laguerre_cells_measures(
         Omega, weights, b, 'EULER_2D'
      )
    @return an array with the cells measures
    """
    # See comments about XY,T,trgl_seed,nt in compute_Hessian()
    XY = np.asarray(self.RVD.I.Editor.get_points())
    T = np.asarray(self.RVD.I.Editor.get_triangles())
    trgl_seed = np.asarray(self.RVD.I.Editor.find_attribute('facets.chart'))
    measures[:] = 0
    np.add.at(measures, trgl_seed, self.triangle_area(XY,T))

  def triangle_area(self, XY, T):
    """
    @brief Computes the area of a mesh triangle
    @param[in] XY the coordinates of the mesh vertices
    @param[in] T an array with the three vertices indices of the triangle
    @details Works also when T is an array of triangles (then it returns
     the array of triangle areas). This is why the ellipsis (...)
     is used (here it means indexing/slicing through the last dimension)
    """
    v1 = T[...,0]
    v2 = T[...,1]
    v3 = T[...,2]
    U = XY[v2] - XY[v1]
    V = XY[v3] - XY[v1]
    return np.abs(0.5*(U[...,0]*V[...,1] - U[...,1]*V[...,0]))

  def distance(self, XY, v1, v2):
    """
    @brief Computes the length of a mesh edge
    @param[in] XY the coordinates of the mesh vertices
    @param[in] v1 , v2 the mesh extremities indices.
    @details v1 and v2 can be also arrays (then returns the array of distances).
    """
    axis = v1.ndim if type(v1) is np.ndarray else 0
    return np.linalg.norm(XY[v2]-XY[v1],axis=axis)

  def show(self):
    """
    @brief Prepares the RVD for display
    @details Unglues the charts so that we better see the Laguerre cells
    """
    self.RVD.I.Surface.unglue_charts()

  def unshow(self):
    """
    @brief To be called before each computation
    @details Re-glues the charts so that the Hessian can be computed
    """
    self.RVD.I.Surface.merge_vertices(1e-10)


transport = Transport(N,True)
# transport.verbose = True # uncomment to display Newton convergence

# We need these two global functions to plug the GUI
def compute():
  transport.unshow()
  transport.compute()
  transport.show()

def one_iteration():
  transport.unshow()
  transport.one_iteration()
  transport.show()

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
   if imgui.Button('Compute transport',-1,70) then
      gom.interpreter('Python').globals.compute()
   end
   if imgui.Button('One iteration',-1,70) then
      gom.interpreter('Python').globals.one_iteration()
   end
end

graphite_main_window.add_module(OT_dialog)

""",save_in_history=False,log=False)
