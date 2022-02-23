# Python example: compute manifold harmonics and display them using
# matplotlib.
# Note: with anaconda under Windows 10, sometimes numpy and matplotlib
# come with broken installations. To fix them, run 'Anaconda Prompt'
# in administrator mode (right click on entry in start menu, then in
# 'more...' submenu), then:
#   pip uninstall numpy
#   pip install numpy
#   pip uninstall matplotlib
#   pip install matplotlib==3.0.3


import sys,math,numpy,os.path

if not 'gom' in globals():
   import gompy
   scene_graph = gom.create(classname='OGF::SceneGraph',interpreter=gom)
else:
    # pyplot accesses sys.argv[0] that is not initialized by Graphite
    sys.argv =  ['graphite']

scene_graph.clear()    
    
file = gom.get_environment_value('PROJECT_ROOT')+'/lib/data/violin.obj'   
   
if os.path.isfile(file):
   scene_graph.load_object(file)
else:
   scene_graph.create_object('Mesh','S').I.Shapes.create_ngon(nb_edges=8)

S = scene_graph.current()
S.I.Surface.remesh_smooth(nb_points=500)
R = scene_graph.resolve('remesh')
R.I.Spectral.compute_manifold_harmonics(nb_eigens=100)

# ====================================================================

XYZ = numpy.asarray(R.I.Editor.find_attribute('vertices.point'))
VV  = numpy.asarray(R.I.Editor.find_attribute('vertices.eigen'))
T   = numpy.asarray(R.I.Editor.get_triangles())

X = XYZ[:,0]
Y = XYZ[:,1]

import matplotlib.pyplot as plt
import matplotlib.tri as mtri
import matplotlib.cm as cm

triang = mtri.Triangulation(X,Y,T)

def plot_eigen(n):
   plt.gca().set_aspect('equal')
   plt.tricontourf(triang, VV[:,n])
   plt.triplot(triang,'k-')
   plt.title('eigen' + str(n))

   
def highres_contours_plot_eigen(n):   
   plt.gca().set_aspect('equal')
#  plt.triplot(triang, lw=0.5, color='white')
   z = VV[:,n]
   refiner = mtri.UniformTriRefiner(triang)
   tri_refi, z_test_refi = refiner.refine_field(z, subdiv=3)
   zmin = numpy.amin(z)
   zmax = numpy.amax(z)
   levels = numpy.arange(zmin, zmax, 0.03*(zmax-zmin))
   cmap = cm.get_cmap(name='terrain', lut=None)
   plt.tricontourf(tri_refi, z_test_refi, levels=levels, cmap=cmap)
   plt.tricontour(tri_refi, z_test_refi, levels=levels,
               colors=['0.25', '0.5', '0.5', '0.5', '0.5'],
               linewidths=[1.0, 0.5, 0.5, 0.5, 0.5])
   plt.title('eigen' + str(n))

plt.figure()
for n in range(1,5):
   plt.subplot(220+n)
   highres_contours_plot_eigen(n*10)
plt.show()






