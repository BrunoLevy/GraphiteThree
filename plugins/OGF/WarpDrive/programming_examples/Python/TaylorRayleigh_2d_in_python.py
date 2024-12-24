
# This program implements a simple incompressible fluid simulator in Lua,
# it is there for educational purposes. More efficient implementations of the
# fluid simulator are available in the Transport->Compute->Eulerxx commands
# (the latter is used in TaylorRayleigh_2d.lua, TaylorRayleigh_3d.lua etc...)
#
# Note: with anaconda under Windows 10, sometimes numpy and matplotlib
# come with broken installations. To fix them, run 'Anaconda Prompt'
# in administrator mode (right click on entry in start menu, then in
# 'more...' submenu), then:
#   pip uninstall numpy
#   pip install numpy
#   pip uninstall matplotlib
#   pip install matplotlib==3.0.3

# TODO: more vector operations instead of by-component.
#       understand how to modify vertices coordinates in-place through slicing

import math,numpy
OGF = gom.meta_types.OGF # shortcut to Graphite types

Euler_stopped = False

N=1000 # Number of points.

def Euler_stop():
    global Euler_stopped
    Euler_stopped = True

def Euler_step():
   OT = points.I.Transport

   # Timestep
   tau = 0.001

   # Stiffness of the 'spring' pressure force that pulls the
   # points towards the centroids
   epsilon = 0.004

   # Gravity on earth in m/s^2
   G = numpy.asarray([0,-9.81])

   inveps2 = 1.0/(epsilon*epsilon)

   # Compute the centroids of the unique Laguerre diagram defined
   # from the points that has constant areas.
   OT.compute_optimal_Laguerre_cells_centroids(
       Omega=Omega,centroids=Acentroid,mode='EULER_2D'
   )

   # Update forces, speeds and positions (Explicit Euler scheme, super simple !)
   for v in range(E.nb_vertices):
      # Compute forces: F = spring_force(point, centroid) - m G Z
      Fx = inveps2 * (centroid[v,0] - point[v,0])
      Fy = inveps2 * (centroid[v,1] - point[v,1]) + mass[v] * G[1]
      # V += tau * a ; F = ma ==> V += tau * F / m
      V[v,0] = V[v,0] + tau * Fx / mass[v]
      V[v,1] = V[v,1] + tau * Fy / mass[v]
      # position += tau * V
      point[v,0] = point[v,0] + tau*V[v,0]
      point[v,1] = point[v,1] + tau*V[v,1]
   points.redraw()

def Euler_steps(n):
   global Euler_stopped
   Euler_stopped = False
   # todo: figure out why integers became floats through inter-language interop.
   for i in range(n):
      if Euler_stopped:
          break
      Euler_step()

# #####################
# Initialization
# #####################

scene_graph.clear()
Omega = scene_graph.create_object(classname='Mesh',name='Omega')
Omega.I.Shapes.create_quad()
Omega.I.Surface.triangulate()
Omega.I.Points.sample_surface(nb_points=N)
scene_graph.current_object = 'points'
points = scene_graph.objects.points
E = points.I.Editor

## Low level access to point coordinates
point = numpy.asarray(E.get_points())

## Attributes attached to each vertex:
## mass, speed vector and centroid of Laguerre cell
mass      = numpy.asarray(E.find_or_create_attribute('vertices.mass'))
V         = numpy.asarray(
    E.find_or_create_attribute(attribute_name='vertices.speed',dimension=2)
)
Acentroid = gom.create(
    classname='OGF::NL::Vector',size=E.nb_vertices,dimension=2
)
centroid  = numpy.asarray(Acentroid)

## Initialize masses with nice sine wave,
## and heavy fluid on top.
for v in range(E.nb_vertices):
   x = point[v,0]
   y = point[v,1]
   f =0.1*math.sin(x*10)
   if (y-0.5) > f:
      mass[v] = 3
   else:
      mass[v] = 1

# Display mass attribute.
points.shader.painting  = 'ATTRIBUTE'
points.shader.attribute = 'vertices.mass'
points.shader.colormap  = 'blue_red;true;0;false;false;;'
points.shader.autorange()

# Initialize Euler simulation.
# Start with points at centroids, and initial speeds at zero.
def Euler_init():
   OT = points.I.Transport
   OT.compute_optimal_Laguerre_cells_centroids(
       Omega=Omega,centroids=Acentroid,mode='EULER_2D'
   )
   for v in range(E.nb_vertices):
      point[v,0] = centroid[v,0]
      point[v,1] = centroid[v,1]
      V[v,0] = 0.0
      V[v,1] = 0.0
   points.update()

Euler_init()

# #####################
# GUI
# #####################

# The GUI is written in Lua, and communicates
# with Python through Graphite's interop layer.
# The function for the GUI is in a big string,
# sent to the Lua interpreter.

gom.interpreter("Lua").execute(command="""

require('math')

Euler_dialog = {}
Euler_dialog.visible = true
Euler_dialog.name = 'Euler'
Euler_dialog.x = 100
Euler_dialog.y = 400
Euler_dialog.w = 150
Euler_dialog.h = 250
Euler_dialog.width = 400
Euler_dialog.nb_steps = 1000
Euler_dialog.icon = '@flask'

function Euler_dialog.draw_window()
   imgui.PushItemWidth(-1)
   imgui.Text('nb timesteps')
    _,Euler_dialog.nb_steps =
       imgui.InputInt('##nb_steps',Euler_dialog.nb_steps)
   imgui.Separator()
   if imgui.Button('run Euler',-1,0) then
       -- we need to 'exec_command' rather than directly calling
       -- the function, this is to ensure that graphic updates will
       -- be possible during the simulation loop.
       main.exec_command('gom.interpreter("Python").globals.Euler_steps(Euler_dialog.nb_steps)')
   end
   if imgui.Button('Stop',-1,0) then
      gom.interpreter('Python').globals.Euler_stop()
   end
   imgui.PopItemWidth()
end

graphite_main_window.add_module(Euler_dialog)

""",save_in_history=False,log=False)
