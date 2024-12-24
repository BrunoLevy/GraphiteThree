
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

import math,numpy as np
OGF = gom.meta_types.OGF # shortcut to Graphite types

class Euler:

    def __init__(self,nb):
        self.stopped = False

        # Number of points
        self.nb = nb

        # Timestep
        self.tau = 0.001

        # Stiffness of the 'spring' pressure force that pulls the
        # points towards the centroids
        self.epsilon = 0.004
        self.inveps2 = 1.0/(self.epsilon*self.epsilon)

        # Gravity on earth in m/s^2
        self.G = np.asarray([0,-9.81])

        scene_graph.clear()

        # Create domain
        self.Omega = scene_graph.create_object(OGF.MeshGrob,'Omega')
        self.Omega.I.Shapes.create_quad()
        self.Omega.I.Surface.triangulate()

        # Create initial pointset
        self.Omega.I.Points.sample_surface(nb_points=nb)
        self.points_obj = scene_graph.objects.points
        E = self.points_obj.I.Editor

        # Attributes attached to each vertex:
        # point, mass, speed vector
        # take only x,y ------------------------v
        self.point = np.asarray(E.get_points())[:,0:2]
        self.mass = np.asarray(E.find_or_create_attribute('vertices.mass'))
        self.V = np.asarray(
            E.find_or_create_attribute(
                attribute_name='vertices.speed',dimension=2
            )
        )

        # Centroid of Laguerre cell, as Graphite vec and numpy array
        self.centroids_vec = gom.create(
            classname='OGF::NL::Vector',size=E.nb_vertices,dimension=2
        )
        self.centroid = np.asarray(self.centroids_vec)

        # Initialize masses with nice sine wave,
        # and heavy fluid on top.
        for v in range(E.nb_vertices):
            x = self.point[v,0]
            y = self.point[v,1]
            f =0.1*math.sin(x*10)
            if (y-0.5) > f:
                self.mass[v] = 3
            else:
                self.mass[v] = 1

        # Display mass attribute
        self.points_obj.shader.painting  = 'ATTRIBUTE'
        self.points_obj.shader.attribute = 'vertices.mass'
        self.points_obj.shader.colormap  = 'blue_red;true;0;false;false;;'
        self.points_obj.shader.autorange()

        # Initialize Euler
        self.points_obj.I.Transport.compute_optimal_Laguerre_cells_centroids(
            Omega=self.Omega,centroids=self.centroids_vec,mode='EULER_2D'
        )
        np.copyto(self.point,self.centroid) # point <- centroid
        self.V[:,:] = 0                     # V <- 0
        self.points_obj.update()

    def step(self):
        # Compute the centroids of the unique Laguerre diagram defined
        # from the points that has constant areas.
        self.points_obj.I.Transport.compute_optimal_Laguerre_cells_centroids(
            Omega=self. Omega,centroids=self.centroids_vec, mode='EULER_2D'
        )

        # Compute forces: F = spring_force(point, centroid) - m G Z
        F = self.inveps2 * (self.centroid - self.point) + \
            self.mass[:,np.newaxis] * self.G
        # V += tau * a ; F = ma ==> V += tau * F / m
        self.V += self.tau * F / self.mass[:,np.newaxis]
        # Update positions
        self.point += self.tau * self.V

        self.points_obj.redraw()

    def steps(self,n):
        self.stopped = False
        for i in range(n):
            if self.stopped:
                break
            self.step()

    def stop(self):
        self.stopped = True

euler = Euler(1000)

def euler_steps(n):
    euler.steps(n)

def euler_stop():
    euler.stop()

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
       main.exec_command(
          'gom.interpreter("Python").globals.euler_steps(Euler_dialog.nb_steps)'
       )
   end
   if imgui.Button('Stop',-1,0) then
      gom.interpreter('Python').globals.euler_stop()
   end
   imgui.PopItemWidth()
end

graphite_main_window.add_module(Euler_dialog)

""",save_in_history=False,log=False)
