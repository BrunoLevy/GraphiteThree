-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- commands.lua: How to create new Graphite commands in Lua
-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------

-- -----------------------------------------------------------------------
-- Example 1: commands associated with MeshGrob
-- -----------------------------------------------------------------------

-- Create a new enum type
menum = gom.meta_types.OGF.MetaEnum.create('Titi')
-- Declare enum values
menum.add_values({tutu=0,tata=1,toto=2})
-- Make new enum visible from GOM type system
gom.bind_meta_type(menum)

-- The functions that implement our commands
-- The first three ones just display their
-- arguments. The arguments can be accessed
-- using args.<argname>. There is also an
-- additional argument args.self, that
-- refers to our Commands class. it can be
-- used to retreive the MeshGrob the command
-- was invoked from, through args.self.grob

function trululu(args) 
    print('trululu args='..tostring(args))
    print('self='..tostring(args.self))
end

function foobar1(args)
    print('foobar1 args='..tostring(args))
    print('self='..tostring(args.self))
end

function foobar2(args)
    print('foobar2 args='..tostring(args))
    print('self='..tostring(args.self))
end

-- This function applies random perturbations to
-- the vertices of a mesh.

function randomize(args)

    main.save_state() -- needed by undo()/redo() 

   -- get our OGF::MeshGrobCustomCommands
   -- (it is args.self)
   local cmd = args.self
   -- now get the MeshGrob from the commands
   local S = cmd.grob   

   -- create a MeshGrobEditor to modify the mesh
   local E = S.I.Editor
   -- access to the array of vertices coordinates
   local point = E.find_attribute('vertices.point')

   -- Iterate on all the vertices
   for v=0,E.nb_vertices-1 do
      -- get coordinates of current vertex
      -- (point is a 'vector attribute', with 3
      --  elements per item).
      local x = point[3*v]
      local y = point[3*v+1]
      local z = point[3*v+2]
      x = x + args.howmuch * (math.random() - 0.5) 
      y = y + args.howmuch * (math.random() - 0.5) 
      z = z + args.howmuch * (math.random() - 0.5)  
      point[3*v]   = x
      point[3*v+1] = y
      point[3*v+2] = z
   end
end

-- We are going to create a subclass of OGF::MeshGrobCommands,
-- let us first get the metaclass associated with OGF::MeshGrobCommands
superclass = gom.meta_types.OGF.MeshGrobCommands 

-- Create our subclass, that we name OGF::MeshGrobCustomCommands
-- By default, our commands will land in a new menu 'Custom'
-- (name your class OGF::MeshGrobZorglubCommands if you want a 'Zorglub'
-- menu, or use custom attributes, see below).
mclass = superclass.create_subclass('OGF::MeshGrobCustomCommands')

-- Create a constructor for our new class.
-- For Commands classes, we just create the default constructor
-- (one can also create constructors with arguments, but we do not need that here)
mclass.add_constructor()

-- Create a new slot in our class. Its name is 'trululu' (will create a 'trululu'
-- menu item in the 'Custom' menu), and it will call the function trululu each
-- time the menu item is invoked. You are not obliged to use the same name for both
-- (but doing so makes sense).
mtrululu = mclass.add_slot('trululu',trululu) 
-- Now create the arguments using the add_arg() function, that takes
-- - the name of the argument
-- - the type of the argument
-- - an optional default value (encoded in a string)
-- argument type names are used by autogui to generate the right widgets in the
-- command dialog
mtrululu.add_arg('nb',   gom.meta_types.int,        42)
mtrululu.add_arg('name', gom.meta_types.std.string, 'coucou')
mtrululu.add_arg('titi', gom.meta_types.Titi,       'tata')
-- if you want you can specify an help bubble that will be displayed when the
-- title bar of the command dialog is hovered by the mouse cursor
mtrululu.create_custom_attribute('help','Applies Cray-Lorgan function to the data')

-- Let us create another command (associated with the same Commands class)
mfoobar1 = mclass.add_slot('foobar1', foobar1)
mfoobar1.add_arg('x',gom.meta_types.double,3.14)
mfoobar1.add_arg('y',gom.meta_types.double)
mfoobar1.add_arg('z',gom.meta_types.double)
-- You can optionally specify a submenu path for your command. Submenus
-- are separated by '/'. If your menupath starts with '/' then it is
-- direclty added to the menubar. This example simply creates a 'Foobars'
-- submenu in the 'Custom' menu
mfoobar1.create_custom_attribute('menu','Foobars')

-- And another command, also created in the 'Foobars' submenu
mfoobar2 = mclass.add_slot('foobar2', foobar2)
mfoobar2.add_arg('x',gom.meta_types.double,1)
mfoobar2.add_arg('y',gom.meta_types.double,2)
mfoobar2.add_arg('z',gom.meta_types.double,3)
mfoobar2.create_custom_attribute('menu','Foobars')

-- And finally our 'randomize' command
mrandomize = mclass.add_slot('randomize', randomize)
mrandomize.add_arg('howmuch',gom.meta_types.double,0.02)
-- Hep bubbles can also be associated with each command argument if you want
mrandomize.create_arg_custom_attribute('howmuch','help','amount of perturbation')
-- And this help bubble is associated with the command
mrandomize.create_custom_attribute('help','applies a random perturbation to the vertices of a mesh')
-- It is possible to create menu entries in any existing menu of Graphite, by starting
-- the menu path with '/'. 
mrandomize.create_custom_attribute('menu','/Mesh')

-- Make our new Commands visible from MeshGrob
scene_graph.register_grob_commands(gom.meta_types.OGF.MeshGrob,mclass)


-- -----------------------------------------------------------------------
-- Example 2: commands associated with the SceneGraph
-- -----------------------------------------------------------------------

-- It is sometimes inconvenient to have to create a MeshGrob before being
-- able to invoke any command associated to the MeshGrob. One possibility
-- is to associate commands with the SceneGraph. SceneGraph commands are always
-- displayed in Graphite, even when the SceneGraph is empty.
-- We are going to implement commands that create basic shapes in the
-- SceneGraph.

-- The function that implements our commands.
-- One can dispatch several method to the same Lua function,
-- the additional field args.method lets you determine the
-- method that was called.
-- (One can also use separate functions if preferred, as done
--  in the first part of this tutorial).

function create_shape(args) 
    print('create shape, args='..tostring(args))
    print('self='..tostring(args.self))

    main.save_state() -- needed by undo()/redo() 

    local o = scene_graph.create_object({
        classname='OGF::MeshGrob',name=args.name
    })

    if(args.method == 'square') then 
       x1 = 0
       y1 = 0
       x2 = args.size
       y2 = args.size
       if(args.center) then
          x1 =-x2/2
          x2 = x2/2
          y1 =-y2/2
          y2 = y2/2
       end
       o.I.Shapes.create_square(x1,y1,0,x2,y1,0,x2,y2,0,x1,y2,0)
    elseif(args.method == 'cube') then
       x1 = 0
       y1 = 0
       z1 = 0
       x2 = args.size
       y2 = args.size
       z2 = args.size
       if(args.center) then
          x1 =-x2/2
          x2 = x2/2
          y1 =-y2/2
          y2 = y2/2
          z1 =-z2/2
          z2 = z2/2
       end
       o.I.Shapes.create_cube(x1,y1,z1,x2,y2,z2)
    elseif(args.method == 'sphere') then 
       o.I.Shapes.create_sphere({radius=args.radius,precision=args.precision})
    end
end

-- Our new class is a subclass of OGF::SceneGraphCommands
baseclass = gom.meta_types.OGF.SceneGraphCommands
mclass    = baseclass.create_subclass('OGF::SceneGraphShapesCommands')

-- We need a constructor 
mclass.add_constructor()

-- The three commands and their arguments, with some examples of
-- custom attributes.
-- For creating a menu directly attached to the MenuBar,
-- note the absolute path '/Shapes' custom attribute
-- that starts with '/' 

msquare = mclass.add_slot('square', create_shape)
msquare.add_arg('name',gom.meta_types.OGF.NewMeshGrobName,'shape')
msquare.create_arg_custom_attribute('name','help','name of the object to create')
msquare.add_arg('size',gom.meta_types.double,1.0)
msquare.create_arg_custom_attribute('size','help','edge length of the square')
msquare.add_arg('center',gom.meta_types.bool,false)
msquare.create_arg_custom_attribute('center','help','if set, dimensions go from -size/2 to size/2 instead of [0,size]')
msquare.create_custom_attribute('menu','/Shapes')
msquare.create_custom_attribute('help','guess what ? it creates a square (what an informative help bubble !!)')

mcube = mclass.add_slot('cube', create_shape)
mcube.add_arg('name',gom.meta_types.OGF.NewMeshGrobName,'shape')
mcube.add_arg('size',gom.meta_types.double,1.0)
mcube.create_custom_attribute('menu','/Shapes')

msphere = mclass.add_slot('sphere',create_shape)
msphere.add_arg('name',gom.meta_types.OGF.NewMeshGrobName,'shape')
msphere.add_arg('radius',gom.meta_types.double,1.0)
msphere.add_arg('precision',gom.meta_types.int,4)
msphere.create_custom_attribute('menu','/Shapes')

-- Make our new class visible from SceneGraph

scene_graph.register_grob_commands(gom.meta_types.OGF.SceneGraph, mclass)

-- -----------------------------------------------------------------------
-- More information
-- -----------------------------------------------------------------------

-- Now the commands you have created behave exactly as commands written in
-- C++. For instance, one can invoke them from the scripting languages of
-- Graphite (Lua, Python). Try this:
--
-- - create a MeshGrob, call it S
-- - now, in Graphite's command line, type:
--      scene_graph.objects.S.I.Custom.randomize()
--   (press <tab> for automatic completion).
--
-- Your new command classes are a "citizen" of the Graphite Object Model,
-- with the same "rights" as the ones implemented in C++ (can be scripted
-- and invoked from the gui)

