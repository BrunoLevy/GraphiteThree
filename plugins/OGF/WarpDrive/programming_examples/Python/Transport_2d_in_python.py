# Tutorial on Optimal Transport
# "by-hand" computation of Hessian and gradient (almost fully in Python)
# Version that exploit numpy array functions (much faster than naive version)

import math, ctypes, datetime
import numpy as np

OGF=gom.meta_types.OGF # shortcut to OGF.MeshGrob for instance

N = 1000 # number of points, try 10000, 100000 (be ready to wait a bit)

class Transport:

  def __init__(self, N: int, shrink_points: bool):
    """
    @brief Transport constructor
    @param[in] N number of points
    @param[in] shrink_points if set, group points in a small zone
    """
    self.verbose = True
    self.N = N

    scene_graph.clear() # Delete all Graphite objects

    # Create domain Omega (a square)
    self.Omega = scene_graph.create_object(OGF.MeshGrob,'Omega')
    self.Omega.I.Shapes.create_quad()
    # self.Omega.I.Shapes.create_ngon(nb_edges=300) # try this instead of square
    self.Omega.I.Surface.triangulate()

    # Create points (random sampling of Omega)
    self.Omega.I.Points.sample_surface(nb_points=N,Lloyd_iter=0,Newton_iter=0)
    self.seeds = scene_graph.objects.points

    # To demonstrate more interesting transport, cluster points in a zone
    if shrink_points:
      coords = np.asarray(self.seeds.I.Editor.get_points())
      coords[:] = 0.125 + coords/4.0
      self.seeds.update()

    # Compute Laguerre diagram
    self.Laguerre = scene_graph.create_object(OGF.MeshGrob,'Laguerre')
    self.psi = np.zeros(self.N,np.float64)
    self.compute_Laguerre_diagram(self.psi)

    # Variables for Newton iteration (it is better to allocate them one for all)
    self.b = np.empty(self.N, np.float64) # right-hand side
    self.p = np.empty(self.N, np.float64) # Newton step

    # Measure of whole domain, desired areas and minimum legal area (KMT #1)
    self.compute_Laguerre_cells_measures(self.b)   # b <- areas of Laguerre cells
    self.Omega_measure = np.sum(self.b)            # Measure of the whole domain
    self.nu_i = self.Omega_measure / self.N        # Desired area for each cell
    self.area_threshold = 0.5 * min(np.min(self.b), self.nu_i) # KMT criterion #1

    # Change graphic attributes of diagram
    self.Omega.visible=False
    self.show()

    # Make Graphite's logger less verbose (so that we can better see our logs)
    gom.set_environment_value('log:features_exclude','Validate;timings')

  def compute(self):
    """
    @brief Computes the optimal transport
    @details Calls one_iteration() until measure error of worst cell
      is smaller than 1%
    """
    starttime = datetime.datetime.now()
    threshold = self.nu_i * 0.01 # 1% of desired cell area
    while(self.one_iteration() > threshold):
      self.Laguerre.redraw()
    self.log(f'Total elapsed time for OT: {datetime.datetime.now()-starttime}')

  def one_iteration(self):
    """
    @brief One iteration of Newton-Kitagawa-Merigot-Thibert iteration
    @return the measure error of the worst cell (L_infty norm of gradient)
    """
    self.log('===== Newton step')

    H = self.compute_Hessian() # Hessian of Kantorovich dual (sparse matrix)

    # rhs (minus gradient of Kantorovich dual) = desired areas - actual areas
    self.compute_Laguerre_cells_measures(self.b)
    self.b[:] = self.nu_i - self.b

    g_norm = np.linalg.norm(self.b)  # norm of gradient at current step (KMT #2)
    H.solve_symmetric(self.b,self.p) # solve for p in Lp=b
    alpha = 1.0                      # Steplength
    self.psi += self.p            # Start with Newton step

    # Divide steplength by 2 until both KMT criteria are satisfied
    main.lock_updates() # hide graphic updates (try this: uncomment + other one )
    for k in range(10):
      self.log(f' Substep: k={k}')

      # rhs (- grad of Kantorovich dual) = actual measures - desired measures
      self.compute_Laguerre_diagram(self.psi)
      self.compute_Laguerre_cells_measures(self.b)
      smallest_area = np.min(self.b) # for KMT criterion 1
      self.b -= self.nu_i

      # Check KMT criteria #1 (cell area) and #2 (gradient norm)
      g_norm_k = np.linalg.norm(self.b)
      KMT_1 = (smallest_area > self.area_threshold)  # criterion 1: cell area
      KMT_2 = (g_norm_k <= (1.0-0.5*alpha) * g_norm) # criterion 2: gradient norm
      self.log(f' KMT #1 (area): {KMT_1} {smallest_area}>{self.area_threshold}')
      self.log(f' KMT #2 (grad): {KMT_2} {g_norm_k}<={(1.0-0.5*alpha)*g_norm}')
      if KMT_1 and KMT_2:
         break

      alpha = alpha / 2.0
      self.psi -= alpha * self.p
    main.unlock_updates() # show graphic updates (uncomment also this one)

    worst_area_error = np.linalg.norm(self.b, ord=np.inf) # L_infty norm of grad
    self.log(f'Worst cell area error = {100.0 * worst_area_error / self.nu_i}%')
    return worst_area_error

  def compute_Laguerre_diagram(self, weights: np.ndarray):
    """
    @brief Computes a Laguerre diagram from a weight vector
    @param[in] weights the weights vector
    """
    self.seeds.I.Transport.compute_Laguerre_diagram(
      self.Omega, weights, self.Laguerre, 'EULER_2D'
    )
    self.Laguerre.update()
    self.Laguerre.I.Surface.triangulate()
    self.Laguerre.I.Surface.merge_vertices(1e-10)

  def compute_Hessian(self):
    """
    @brief Computes the matrix of the system to be solved at each Newton step
    @details Uses the current Laguerre diagram (in self.Laguerre).
    @return the Hessian matrix of the Kantorovich dual
    """

    H = NL.create_matrix(self.N,self.N) # Creates a sparse matrix using OpenNL

    # vertex v's coords are XY[v][0], XY[v][1]
    XY = np.asarray(self.Laguerre.I.Editor.get_points())

    # Triangle t's vertices indices are T[t][0], T[t][1], T[t][2]
    T = np.asarray(self.Laguerre.I.Editor.get_triangles())

    Tadj = np.asarray( # Trgls adjacent to t: Tadj[t][0], Tadj[t][1], Tadj[t][2]
      self.Laguerre.I.Editor.get_triangle_adjacents()
    )[:,[1,2,0]] # <- permute columns to match conventions for triangulations:
    # different in geogram meshes because they also support n-sided polygons

    # trgl_seed[t]: index of the seed s such that t is in s's Laguerre cell
    trgl_seed = np.asarray(self.Laguerre.I.Editor.find_attribute('facets.chart'))

    # The coordinates of the seeds
    seeds_XY = np.asarray(self.seeds.I.Editor.find_attribute('vertices.point'))
    diag = np.zeros(self.N,np.float64)   # Diagonal, initialized to zero
    NO_INDEX = ctypes.c_uint32(-1).value # special value for Tadj: edge on border


    # Create a qidx (quad index) array that encodes for each triangle edge e:
    #   [ i, j, v1, v2 ] where:
    #     i: seed index
    #     j: adjacent seed index (Laguerre cell on the other side of e)
    #     v1, v2: vertices of the triangle edge e
    # We assemble them in the same array so that we can remove the entries
    # that we do not want (triangle edges on the border of Omega, and triangle
    # edges that stay in the same Laguerre cell).

    qidx = np.column_stack((trgl_seed, Tadj[:,0], T[:,1], T[:,2],
                            trgl_seed, Tadj[:,1], T[:,2], T[:,0],
                            trgl_seed, Tadj[:,2], T[:,0], T[:,1]))
    #                           |         |        |       |
    #    0: seed index (i) -----'         |        '---+---'
    #    1: adj trgl accross e (for now) -'            |
    #    2,3: triangle edge (dual to Voronoi) ---------'

    qidx = qidx.reshape(-1,4) # reshape from (nt,12) to ((3*nt),4)

    qidx = qidx[qidx[:,1] != NO_INDEX]  # remove edges on border
    qidx[:,1] = trgl_seed[qidx[:,1]]    # 1: adjacent seed index (j)
    qidx = qidx[qidx[:,0] != qidx[:,1]] # remove edges that stay in same cell

    I = qidx[:,0].copy()  # get arrays of i's, j's, v1's and v2's.
    J = qidx[:,1].copy()  # We need to copy I and J to have contiguous arrays
    V1 = qidx[:,2]        # (H.add_coefficients() requires that).
    V2 = qidx[:,3]

    # Now we can compute a vector of coefficient (note: V1,V2,I,J are vectors)
    coeff = -self.distance(XY,V1,V2) / (2.0 * self.distance(seeds_XY,I,J))

    # Accumumate coefficients into matrix
    np.add.at(diag,I,-coeff)             # diag = minus sum extra-diagonal coeffs
    H.add_coefficients(I,J,coeff)        # accumulate coeffs into matrix
    H.add_coefficients_to_diagonal(diag) # accumulate diag
    return H

  def compute_Laguerre_cells_measures(self, measures: np.ndarray):
    """
    @brief Computes the measures of the Laguerre cells
    @out measures: the vector of Laguerre cells measures
    @details Uses the current Laguerre diagram (in self.Laguerre)
    """
    # See comments about XY,T,trgl_seed,nt in compute_Hessian()
    XY = np.asarray(self.Laguerre.I.Editor.get_points())
    T = np.asarray(self.Laguerre.I.Editor.get_triangles())
    trgl_seed = np.asarray(self.Laguerre.I.Editor.find_attribute('facets.chart'))
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
    axis = v1.ndim if hasattr(v1,'ndim') else 0
    return np.linalg.norm(XY[v2]-XY[v1],axis=axis)

  def show(self):
    """
    @brief Prepares the RVD for display
    @details Unglues the charts so that we better see the Laguerre cells
    """
    self.Laguerre.I.Surface.unglue_charts()
    self.Laguerre.shader.painting='ATTRIBUTE'
    self.Laguerre.shader.attribute='facets.chart'
    self.Laguerre.shader.colormap = 'plasma;false;732;false;false;;'
    self.Laguerre.shader.mesh_style = 'false;0 0 0 1;1'
    self.Laguerre.shader.autorange()

  def unshow(self):
    """
    @brief To be called before each computation
    @details Re-glues the charts so that the Hessian can be computed
    """
    self.Laguerre.I.Surface.merge_vertices(1e-10)

  def log(self, *args):
    """
    @brief Displays a log message if verbose is set
    """
    if self.verbose:
      msg = str.join('',[str(arg) for arg in args])
      print(msg)


transport = Transport(N,True)
# transport.verbose = True # uncomment to display Newton convergence


# ------------------------------------------
# GUI
# ------------------------------------------

# We need these two global functions to plug the GUI

locked = False # avoid running multiple times if user presses button.

def compute():
  global locked
  if locked:
    return
  locked = True
  transport.unshow()
  transport.compute()
  transport.show()
  locked = False

def one_iteration():
  global locked
  if locked:
    return
  locked = True
  transport.unshow()
  transport.one_iteration()
  transport.show()
  locked = False

# The GUI is written in Lua, and communicates
# with Python through Graphite's interop layer.
# The function for the GUI is in a big string,
# sent to the Lua interpreter.
# Calling back the Python function 'compute' from LUA is
# done as follows:
#   gom.interpreter('Python').globals.compute()
# It is called through main.exec_command() that is queued outside
# GUI rendering loop so that it can display animated graphics

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
      main.exec_command('gom.interpreter("Python").globals.compute()')
   end
   if imgui.Button('One iteration',-1,70) then
      main.exec_command('gom.interpreter("Python").globals.one_iteration()')
   end
end

graphite_main_window.add_module(OT_dialog)

""",save_in_history=False,log=False)
