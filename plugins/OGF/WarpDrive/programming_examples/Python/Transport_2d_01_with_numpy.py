# Tutorial on Optimal Transport
# "by-hand" computation of Hessian and gradient (almost fully in Python)
# Version that uses numpy

import math, datetime
import numpy as np
import scipy

OGF=gom.meta_types.OGF # shortcut to OGF.MeshGrob for instance

N = 1000             # number of points, try 10000, 100000
verbose = False      # set to True for logging stats during iterative algo
shrink_points = True # regroup points in smaller zone to make it more difficult

class Transport:

  def __init__(self, N: int, shrink_points: bool, verbose: bool):
    """
    @brief Transport constructor
    @param[in] N number of points
    @param[in] shrink_points if set, regroup points in a small zone
    """
    self.verbose = verbose
    self.N = N

    scene_graph.clear() # Delete all Graphite objects

    # Create domain Omega (a square)
    self.Omega = scene_graph.create_object(OGF.MeshGrob,'Omega')
    self.Omega.I.Shapes.create_quad()
    # self.Omega.I.Shapes.create_ngon(nb_edges=300) # try this instead of square
    self.Omega.I.Surface.triangulate()
    self.Omega.visible = False

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

    # Measure of whole domain, desired areas and minimum legal area (KMT #1)
    areas = self.compute_Laguerre_cells_measures()
    self.Omega_measure = np.sum(areas)             # Measure of the whole domain
    self.nu_i = self.Omega_measure / self.N        # Desired area for each cell
    self.area_threshold = 0.5*min(np.min(areas),self.nu_i) # KMT criterion  #1

    # Change graphic attributes of diagram
    self.Omega.visible=False
    self.show()

    # The coordinates of the seeds
    self.seeds_XY = self.asnumpy(
      self.seeds.I.Editor.find_attribute('vertices.point')
    )

    # Parameters for linear solver
    self.regularization = 0.0
    self.direct = False           # one can use a direct or an iterative solver
    if self.direct:               # if using direct solver, one needs regulariz.
      self.regularization = 1e-6  # because matrix is singular ([1,1...1] in ker)
    self.use_scipy = True         # one can use scipy or OpenNL for the solver

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
    print(f'Total elapsed time for OT: {datetime.datetime.now()-starttime}')

  def one_iteration(self):
    """
    @brief One iteration of Newton-Kitagawa-Merigot-Thibert iteration
    @return the measure error of the worst cell (L_infty norm of gradient)
    """
    self.log('===== Newton step')

    H = self.compute_Hessian() # Hessian of Kantorovich dual (sparse matrix)

    # rhs (minus gradient of Kantorovich dual) = desired areas - actual areas
    b = self.nu_i - self.compute_Laguerre_cells_measures()
    if self.regularization != 0.0:
      b -= self.regularization * self.nu_i * self.psi

    g_norm = np.linalg.norm(b)        # norm of gradient at current step (KMT #2)
    p = self.solve_linear_system(H,b) # solve for p in H*p=b
    alpha = 1.0                       # Steplength
    self.psi += p                     # Start with Newton step

    # Divide steplength by 2 until both KMT criteria are satisfied
    main.lock_updates() # hide graphic updates (try this: uncomment + other one )
    for k in range(10):
      self.log(f' Substep: k={k}')

      # g (grad of Kantorovich dual) at substep = actual areas - desired areas
      self.compute_Laguerre_diagram(self.psi)
      g = self.compute_Laguerre_cells_measures()
      smallest_area = np.min(g) # for KMT criterion 1
      g -= self.nu_i

      # Check KMT criteria #1 (cell area) and #2 (gradient norm)
      g_norm_k = np.linalg.norm(g)
      KMT_1 = (smallest_area > self.area_threshold)  # criterion 1: cell area
      KMT_2 = (g_norm_k <= (1.0-0.5*alpha) * g_norm) # criterion 2: gradient norm
      self.log(f' KMT #1 (area): {KMT_1} {smallest_area}>{self.area_threshold}')
      self.log(f' KMT #2 (grad): {KMT_2} {g_norm_k}<={(1.0-0.5*alpha)*g_norm}')
      if KMT_1 and KMT_2:
         break

      alpha = alpha / 2.0
      self.psi -= alpha * p
    main.unlock_updates() # show graphic updates (uncomment also this one)

    worst_area_error = np.linalg.norm(b, ord=np.inf) # grad L_infty norm
    self.log(f'Worst cell area error = {100.0 * worst_area_error / self.nu_i}%')
    return worst_area_error

  def solve_linear_system(self, H, b):
    """
    @brief Solves a linear system
    @details Works in direct or iterative mode, with scipy and with OpenNL
    @param[in] H the matrix of the linear system
    @param[in] b the right hand side
    @return p such that H p = b
    """
    if self.use_scipy:
      if self.direct:
        p = scipy.sparse.linalg.spsolve(H, b)
      else:
        linalg = scipy.sparse.linalg
        NxN = (self.N, self.N)
        # A: operator:       y <- (H + diag)*x
        # M: preconditioner: y <- diag@{-1}*x
        self.iter = 0
        p,info = linalg.cg(
          A=linalg.LinearOperator(NxN, matvec = lambda x: H@x + H.diag*x),
          b=b,
          M=linalg.LinearOperator(NxN, matvec = lambda x: x / H.diag),
          callback = self.log_iter if self.verbose else None,
          atol = 0.0, # normally the default, but larger on older scipy ver.
          tol  = 1e-3 # or rtol=1e-3 instead of tol, depends on scipy ver.
        )
        if info != 0:
          print(f'CG did not converge, info={info}',info)
    else:
      p = np.empty(self.N, np.float64)
      H.solve_symmetric(b, p, self.direct)
    return p

  def log_iter(self, x):
    """
    @brief Logs conjugate gradient iterations
    @details Used as a callback for scipy.sparse.linalg.cg()
    """
    self.iter = self.iter + 1
    if self.iter % 100 == 0:
      print(f'CG iter: {self.iter}')

  def compute_Laguerre_diagram(self, weights):
    """
    @brief Computes a Laguerre diagram from a weight vector
    @param[in] weights the weights vector
    """
    self.seeds.I.Transport.compute_Laguerre_diagram(
      self.Omega, np.asarray(weights), self.Laguerre, 'EULER_2D'
    )
    self.Laguerre.update()
    self.Laguerre.I.Surface.triangulate()
    self.Laguerre.I.Surface.merge_vertices(1e-10)

    # vertex v's coords are XY[v][0], XY[v][1]
    self.XY = self.asnumpy(self.Laguerre.I.Editor.get_points())

    # Triangle t's vertices indices are T[t][0], T[t][1], T[t][2]
    self.T = self.asnumpy(self.Laguerre.I.Editor.get_triangles())

    # Triangle t bebongs to Laguerre cell of seed Tseed[t]
    self.Tseed = self.asnumpy(
      self.Laguerre.I.Editor.find_attribute('facets.chart')
    )

  def compute_Hessian(self):
    """
    @brief Computes the matrix of the system to be solved at each Newton step
    @details Uses the current Laguerre diagram (in self.Laguerre). Works in
     scipy and in OpenNL mode. In the (scipy,iterative) combination, the
     diagonal of the matrix is stored separately in a dynamically created
     'diag' field of the returned scipy sparse matrix.
    @return the Hessian matrix of the Kantorovich dual
    """

    Tadj = self.asnumpy( # Trgls adjacent to t:Tadj[t][0], Tadj[t][1], Tadj[t][2]
      self.Laguerre.I.Editor.get_triangle_adjacents()
    )[:,[1,2,0]] # <- permute columns to match std convention for triangulations:
    #   different in geogram meshes because they also support n-sided polygons

    I,J,coeff = self.assemble_Hessian(
      self.XY, self.T, Tadj, self.Tseed, self.seeds_XY
    )
    diag = np.zeros(self.N,np.float64) # Diagonal (initialized to zero)
    np.add.at(diag,I,-coeff)           # =minus sum extra-diagonal coefficients
    if self.regularization != 0.0:
      diag = diag + self.regularization * self.nu_i

    if self.use_scipy: # Using scipy sparse matrices
      H = scipy.sparse.csr_matrix( (coeff,(I,J)), shape=(self.N,self.N) )
      if self.direct:
        s = np.arange(self.N,dtype=np.int32)
        H += scipy.sparse.csr_matrix( (diag,(s,s)), shape=(self.N,self.N) )
      else:
        H.diag = diag # store diagonal separately if using iterative solver
    else: # Using OpenNL sparse matrices
      H = NL.create_matrix(self.N,self.N) # Creates a sparse matrix using OpenNL
      H.add_coefficients(I,J,coeff,True)  # accumulate coeffs into H, ignore OOBs
      H.add_coefficients_to_diagonal(diag) # accumulate diagonal into H

    return H

  def assemble_Hessian(self, XY, T, Tadj, Tseed, seeds_XY):
    """
    @brief Assembles the Hessian of the Kantorovich dual
    @param[in] XY (nv,3) array with the vertices of the triangles
    @param[in] T (nt,3) array of triangle vertices indices
    @param[in] Tadj (nt,3) array of adjacent triangles
    @param[in] Tseed indices (nt) for each triangle the associated seed index
    @param[in] seeds_XY (N,3) coordinates of the seeds
    @return I,J,VAL row,column,value arrays, with the extra-diagonal coeffs
    @details One needs to compute the diagonal (= -sum of extra-diagonal coeffs)
    """
    NO_INDEX = -1 # Special value for invalid indices (edge on border)

    # Compute one entry per triangle half-edge (3*nt entries) with:
    # I is the seed associated with the triangle
    # J is the seed on the other side of the triangle's edge (NO_INDEX on border)
    # V1 and V2 are the two vertices of the triangle
    I  = Tseed
    J  = Tadj.transpose().flatten()
    J  = np.where(J[:] != NO_INDEX, I[J], NO_INDEX) # lookup seed on other side
    I  = np.concatenate((I,I,I))
    V1 = np.concatenate((T[:,1], T[:,2], T[:,0]))
    V2 = np.concatenate((T[:,2], T[:,0], T[:,1]))

    # Remove (i,j,v1,v2) index quadruplets that correspond to
    #   - border triangle edges (j == NO_INDEX)
    #   - triangle edges inside Laguerre cell (i == j)
    qidx = np.column_stack((I,J,V1,V2))
    qidx = qidx[np.logical_and(I != J, J != NO_INDEX)]

    I  = qidx[:,0] # re-extract I,J,V1,V2
    J  = qidx[:,1]
    V1 = qidx[:,2]
    V2 = qidx[:,3]

    # Now we can compute the vector of coefficient (note: V1,V2,I,J are vectors)
    coeff = -self.distance(XY,V1,V2) / (2.0 * self.distance(seeds_XY,I,J))

    # Need to copy I,J (NL::Vector does not support non-contiguous arrays)
    return I.copy(), J.copy(), coeff

  def compute_Laguerre_cells_measures(self):
    """
    @brief Computes the measures of the Laguerre cells
    @return the vector of Laguerre cells measures
    @details Uses the current Laguerre diagram (in self.Laguerre)
    """
    # See comments about XY,T,trgl_seed,nt in compute_Laguerre_diagram()
    measures = np.zeros(N)
    np.add.at(measures, self.Tseed, self.triangle_area(self.XY,self.T))
    return measures

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

  def asnumpy(self,o):
    """
    @brief Accesses a GOM Vector as a numpy array
    @details Adapts dtype, so that NO_INDEX entries are set to -1
     instead of ctype.uint.value(-1)
    """
    dtype = np.float64
    if o.element_meta_type == OGF.index_t:
       dtype = np.int32
    return np.asarray(o,dtype=dtype)

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


transport = Transport(N, shrink_points, verbose)

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
#  with jax.log_compiles():
#     transport.one_iteration()
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
