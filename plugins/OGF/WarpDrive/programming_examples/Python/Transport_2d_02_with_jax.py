# Tutorial on Optimal Transport
# Semi-discrete Optimal Transport in 2D, everything in Python (except 2D
# Laguere diagram construction)
# Version that uses jax and scipy
# Runs approximately at the same speed as the numpy version (but having a jax
# version paves the way to a GPU version and to more complicated derivatives
# computations)

import jax
jax.config.update('jax_enable_x64', True)
#jax.config.update('jax_disable_jit', True)
#jax.config.update("jax_explain_cache_misses", True)

import math, datetime
import jax.numpy as jnp
from jax import jit
import numpy as np
import scipy

OGF=gom.meta_types.OGF # shortcut to OGF.MeshGrob for instance

class Transport:

  def __init__(
      self, N: int, Nsides: int, shrink_points: bool,
      use_direct_solver: bool = True, verbose: bool = False
  ):
    """
    @brief Transport constructor
    @param[in] N number of points
    @param[in] Nsides number of edges of polygonal domain
    @param[in] shrink_points if set, regroup points in a small zone
    @param[in] use_direct_solver: direct (if set) or iterative solver otherwise
    @param[in] verbose log Newton iterations if set
    """
    self.N = N
    self.direct = use_direct_solver
    self.verbose = verbose

    self.H_evals = 0 # tracing H and g evaluations
    self.g_evals = 0 #

    scene_graph.clear() # Delete all Graphite objects

    # Create domain Omega (a square)
    self.Omega = scene_graph.create_object(OGF.MeshGrob,'Omega')
    if Nsides == 4:
      self.Omega.I.Shapes.create_quad()
    else:
      self.Omega.I.Shapes.create_ngon(nb_edges=max(Nsides,3))
    self.Omega.I.Surface.triangulate()
    self.Omega.visible = False

    # Create points (random sampling of Omega)
    self.Omega.I.Points.sample_surface(nb_points=N,Lloyd_iter=0,Newton_iter=0)
    self.seeds = scene_graph.objects.points

    # To demonstrate more interesting transport, cluster points in a zone
    if shrink_points:
      coords = np.asarray(self.seeds.I.Editor.get_points())
      coords[:,0:2] = 0.125 + coords[:,0:2]/4.0
      self.seeds.update()

    # Compute Laguerre diagram
    self.Laguerre = scene_graph.create_object(OGF.MeshGrob,'Laguerre')
    self.psi = jnp.zeros(self.N,jnp.float64)
    self.compute_Laguerre_diagram(self.psi)

    # Measure of whole domain, desired areas and minimum legal area (KMT #1)
    areas = self.compute_Laguerre_cells_measures()
    self.Omega_measure = jnp.sum(areas)             # Measure of the whole domain
    self.nu_i = self.Omega_measure / self.N        # Desired area for each cell
    self.area_threshold = 0.5*min(jnp.min(areas),self.nu_i) # KMT criterion  #1

    # Change graphic attributes of diagram
    self.Omega.visible=False
    self.show()

    # The coordinates of the seeds
    self.seeds_XY = self.asjax(
      self.seeds.I.Editor.find_attribute('vertices.point')
    )

    # Parameters for linear solver
    self.regularization = 0.0
    if self.direct:               # if using direct solver, one needs regulariz.
      self.regularization = 1e-6  # because matrix is singular ([1,1...1] in ker)

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

    H_cache_size = Transport.compute_Hessian_I_J_VAL_extradiagonal._cache_size()
    g_cache_size = Transport.cells_areas._cache_size()
    self.g_evals = 0
    self.H_evals = 0


    print(f'H cache size: {H_cache_size}, g cache size: {g_cache_size}')

    while(self.one_iteration() > threshold):
      self.Laguerre.redraw()

    print(f'Total elapsed time for OT: {datetime.datetime.now()-starttime}')

    H_cache_size = Transport.compute_Hessian_I_J_VAL_extradiagonal._cache_size()\
                   - H_cache_size
    g_cache_size = Transport.cells_areas._cache_size() - g_cache_size

    print(f'H recompiles: {H_cache_size}/{self.H_evals}' +
          f',  g recompiles: {g_cache_size}/{self.g_evals}')


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

    g_norm = jnp.linalg.norm(b)      # norm of gradient at current step (KMT #2)
    p = self.solve_linear_system(H,b) # solve for p in H*p=b
    alpha = 1.0                       # Steplength
    self.psi += p                     # Start with Newton step

    # Divide steplength by 2 until both KMT criteria are satisfied
    main.lock_updates() # hide graphic updates (try this: uncomment + other one )
    for k in range(10):
      self.log(f' Substep: k={k}')

      # g (grad of Kantorovich dual) at substep = actual areas - desired areas
      nb_non_empty = self.compute_Laguerre_diagram(self.psi)
      if nb_non_empty < self.N:
        self.log('>*< has empty cells')
      else:
        g = self.compute_Laguerre_cells_measures()
        smallest_area = jnp.min(g) # for KMT criterion 1
        g -= self.nu_i

        # Check KMT criteria #1 (cell area) and #2 (gradient norm)
        g_norm_k = jnp.linalg.norm(g)
        KMT_1 = (smallest_area > self.area_threshold)  # criterion 1: cell area
        KMT_2 = (g_norm_k <= (1.0-0.5*alpha) * g_norm) # criterion 2: grad norm
        self.log(f' KMT #1 (area):{KMT_1} {smallest_area}>{self.area_threshold}')
        self.log(f' KMT #2 (grad):{KMT_2} {g_norm_k}<={(1.0-0.5*alpha)*g_norm}')
        if KMT_1 and KMT_2:
          break

      alpha = alpha / 2.0
      self.psi -= alpha * p
    main.unlock_updates() # show graphic updates (uncomment also this one)

    worst_area_error = jnp.linalg.norm(b, ord=jnp.inf) # grad L_infty norm
    self.log(f'Worst cell area error = {100.0 * worst_area_error / self.nu_i}%')
    return worst_area_error

  def solve_linear_system(self, H, b):
    """
    @brief Solves a linear system
    @details Works in direct or iterative mode, with scipy
    @param[in] H the matrix of the linear system
    @param[in] b the right hand side
    @return p such that H p = b
    """
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
    @return number of non-empty cells
    """
    self.seeds.I.Transport.compute_Laguerre_diagram(
      self.Omega, np.asarray(weights), self.Laguerre, 'EULER_2D'
    )
    self.Laguerre.update()
    self.Laguerre.I.Surface.triangulate()
    self.Laguerre.I.Surface.merge_vertices(1e-10)

    # vertex v's coords are XY[v][0], XY[v][1]
    self.XY = self.asjax(self.Laguerre.I.Editor.get_points())

    # Triangle t's vertices indices are T[t][0], T[t][1], T[t][2]
    self.T = self.asjax(self.Laguerre.I.Editor.get_triangles())

    # Triangle t bebongs to Laguerre cell of seed Tseed[t]
    self.Tseed = self.asjax(
      self.Laguerre.I.Editor.find_attribute('facets.chart')
    )

    # Number of non-empty cells
    return jnp.sum(jnp.zeros(self.N,jnp.int32).at[self.Tseed].set(1))

  def compute_Hessian(self):
    """
    @brief Computes the matrix of the system to be solved at each Newton step
    @details Uses the current Laguerre diagram (in self.Laguerre). Works in
     scipy and in OpenNL mode. In the (scipy,iterative) combination, the
     diagonal of the matrix is stored separately in a dynamically created
     'diag' field of the returned scipy sparse matrix.
    @return the Hessian matrix of the Kantorovich dual
    """

    Tadj = self.asjax( # Trgls adjacent to t:Tadj[t][0], Tadj[t][1], Tadj[t][2]
      self.Laguerre.I.Editor.get_triangle_adjacents()
    )[:,[1,2,0]] # <- permute columns to match std convention for triangulations:
    #   different in geogram meshes because they also support n-sided polygons

    I,J,VAL = Transport.compute_Hessian_I_J_VAL_extradiagonal(
      self.XY, self.T, Tadj, self.Tseed, self.seeds_XY
    )
    diag = jnp.zeros(self.N,jnp.float64).at[I].add(-VAL)
    if self.regularization != 0.0:
      diag += self.regularization * self.nu_i

    # Beware parenthesis--------------v (construct sparse matrix from I,J,VAL)
    H = scipy.sparse.csr_matrix( (VAL,(I,J)), shape=(self.N,self.N) )
    if self.direct: # if using direct solver, inject diag coeffs into mtx
      s = jnp.arange(self.N,dtype=jnp.int32)
      H += scipy.sparse.csr_matrix( (diag,(s,s)), shape=(self.N,self.N) )
    else:
      H.diag = diag # store diagonal separately if using iterative solver

    self.H_evals += 1
    return H

  @jit
  def compute_Hessian_I_J_VAL_extradiagonal(XY, T, Tadj, Tseed, seeds_XY):
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
    J  = jnp.where(J[:] != NO_INDEX, I[J], NO_INDEX) # lookup seed on other side
    I  = jnp.concatenate((I,I,I))
    V1 = jnp.concatenate((T[:,1], T[:,2], T[:,0]))
    V2 = jnp.concatenate((T[:,2], T[:,0], T[:,1]))

    # Now we can compute the vector of coefficient (note: V1,V2,I,J are vectors)
    VAL = -Transport.distance(XY,V1,V2) / (2.0*Transport.distance(seeds_XY,I,J))

    # mask values associated with
    #   - border triangle edges (j == NO_INDEX)
    #   - triangle edges inside Laguerre cell (i == j)
    #   - padding (i == NO_INDEX and j == NO_INDEX)
    VAL = jnp.where(
      jnp.logical_and( J[:] != NO_INDEX, I[:] != J[:]),
      VAL[:], 0.0
    )

    # scipy does not clamp invalid indices so we need to mask them
    # It will add 0 to H(0,0) since we masked VAL right before
    I = jnp.where( I[:] != NO_INDEX, I[:], 0)
    J = jnp.where( J[:] != NO_INDEX, J[:], 0)

    return I, J, VAL

  def compute_Laguerre_cells_measures(self):
    """
    @brief Computes the measures of the Laguerre cells
    @return the vector of Laguerre cells measures
    @details Uses the current Laguerre diagram (in self.Laguerre)
    """
    # See comments about XY,T,trgl_seed,nt in compute_Laguerre_diagram()
    areas = jnp.zeros(self.N, jnp.float64)
    self.g_evals += 1
    return Transport.cells_areas(areas, self.XY, self.T, self.Tseed)

  @jit
  def cells_areas(areas_in, XY, T, Tseed):
    NO_INDEX=-1
    V1 = T[:,0]
    V2 = T[:,1]
    V3 = T[:,2]
    U = XY[V2] - XY[V1]
    V = XY[V3] - XY[V1]
    Tareas = jnp.abs(0.5*(U[:,0]*V[:,1] - U[:,1]*V[:,0]))
    Tareas = jnp.where(T[:,0] != NO_INDEX, Tareas[:], 0.0) # Mask padding
    #return jnp.add.at(areas_in, Tseed, Tareas, inplace=False)
    return areas_in.at[Tseed].add(Tareas)

  def distance(XY, v1, v2):
    """
    @brief Computes the length of a mesh edge
    @param[in] XY the coordinates of the mesh vertices
    @param[in] v1 , v2 the mesh extremities indices.
    @details v1 and v2 can be also arrays (then returns the array of distances).
    """
    return jnp.linalg.norm(XY[v2]-XY[v1],axis=1)

  def asjax(self,o):
    """
    @brief Accesses a GOM Vector as a jax array
    @details Needed because GOM typing is not fully compliant with CPython.
       In addition pads data to reduce JAX recompiling.
    """
    tmp = np.asarray(o)
    dtype = tmp.dtype
    if dtype == np.uint32: # change dtype, OOB indexing does not work with uint !
        dtype = jnp.int32
    tmp = jnp.asarray(tmp,dtype=dtype) # converted to JAX array
    chunk_size = 128                   # padding (avoid recompiling too often)
    pad = chunk_size - (tmp.shape[0] % chunk_size)
    if tmp.ndim == 1:
      return jnp.pad(tmp,(0,pad),constant_values=-1)
    else:
      return jnp.pad(tmp,((0,pad),(0,0)),constant_values=-1)

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


transport = Transport(1000, 4, True)

# ******************************************************************************
# GUI
# ******************************************************************************

# We need a couple of global functions that can be called by the GUI

# Lock/unlock mechanism to avoid running multiple commands if user presses
# buttons frantically

locked = False

def lock():
  global locked
  result = not locked
  locked = True
  if not result:
    print('Could not lock (another command is running)')
  return result

def unlock():
  global locked
  locked = False

# *************************************************************************

def restart(N, Nsides, shrink_points, use_direct_solver, verbose):
  global transport
  if lock():
    transport = Transport(
      N, Nsides, shrink_points, use_direct_solver, verbose
    )
    unlock()

def compute():
  if lock():
    transport.unshow()
    transport.compute()
    transport.show()
    unlock()

def one_iteration():
  if lock():
    transport.unshow()
    transport.one_iteration()
    transport.show()
    unlock()

def clear_JAX_caches():
  if lock():
    jax.clear_caches()
    unlock()


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
OT_dialog.h = 300
OT_dialog.width = 400
OT_dialog.N = 1000
OT_dialog.Nsides = 4
OT_dialog.shrink = true
OT_dialog.direct = true
OT_dialog.verbose = false

function OT_dialog.draw_window()
   if imgui.Button('Compute transport',-1,50) then
      main.exec_command('gom.interpreter("Python").globals.compute()')
   end
   if imgui.Button('One iteration',-1,50) then
      main.exec_command('gom.interpreter("Python").globals.one_iteration()')
   end
   imgui.Separator()
   if imgui.Button('Restart',-1,50) then
      gom.interpreter("Python").globals.restart(
         OT_dialog.N, OT_dialog.Nsides, OT_dialog.shrink,
         OT_dialog.direct,
         OT_dialog.verbose
      )
   end
   _,OT_dialog.N = imgui.InputInt('N',OT_dialog.N)
   _,OT_dialog.Nsides = imgui.InputInt('sides',OT_dialog.Nsides)
   _,OT_dialog.shrink = imgui.Checkbox('shrink', OT_dialog.shrink)
   _,OT_dialog.direct = imgui.Checkbox('direct solver', OT_dialog.direct)
   _,OT_dialog.verbose = imgui.Checkbox('verbose', OT_dialog.verbose)
   imgui.Separator()
   if imgui.Button('Clear JAX caches',-1,50) then
      gom.interpreter("Python").globals.clear_JAX_caches()
   end
end

graphite_main_window.add_module(OT_dialog)

""",save_in_history=False,log=False)
