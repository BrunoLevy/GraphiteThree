-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------
-- commands.lua: How to create new Graphite commands in Lua
-- -------------------------------------------------------------------------
-- -------------------------------------------------------------------------

-- -----------------------------------------------------------------------
-- Example 1: commands associated with MeshGrob
-- -----------------------------------------------------------------------


-- The three functions that implement our three commands

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

-- We are going to create a subclass of OGF::MeshGrobCommands,
-- let us first get the metaclass associated with OGF::MeshGrobCommands
superclass = gom.resolve_meta_type('OGF::MeshGrobCommands')

-- Create our subclass, that we name OGF::MeshGrobCustomCommands
-- By default, our commands will land in a new menu 'Custom'
-- (name your class OGF::MeshGrobZorglubCommands if you want a 'Zorglub'
-- menu, or use custom attributes, see below).
mclass = superclass.create_subclass('OGF::MeshGrobCustomCommands')

-- Create a constructor for our new class (here it is the default constructor)
mclass.add_constructor()

-- Create a new slot in our class. Its name is 'trululu' (will create a 'trululu'
-- menu item in the 'Custom' menu), and it will call the function trululu each
-- time the menu item is invoked.
mtrululu = mclass.add_slot('trululu',trululu) 
-- Now create the arguments using add_arg(), that takes
-- - the name of the argument
-- - a string with the type of the argument
-- - an optional default value
-- argument type names are used by autogui to generate the right widgets in the
-- command dialog
mtrululu.add_arg('nb','int',42)
mtrululu.add_arg('name','std::string','coucou')
-- if you want you can specify an help bubble that will be displayed when the
-- title bar of the command dialog is hovered by the mouse cursor
mtrululu.create_custom_attribute('help','Applies Cray-Lorgan function to the data')

-- Let us create another command (associated with the same Commands class)
mfoobar1 = mclass.add_slot('foobar1', foobar1)
mfoobar1.add_arg('x','double',3.14)
mfoobar1.add_arg('y','double')
mfoobar1.add_arg('z','double')
-- You can optionally specify a submenu path for your command. Submenus
-- are separated by '/'. If your menupath starts with '/' then it is
-- direclty added to the menubar. This example simply creates a 'Foobars'
-- submenu in the 'Custom' menu
mfoobar1.create_custom_attribute('menu','Foobars')

-- And another command, also created in the 'Foobars' submenu
mfoobar2 = mclass.add_slot('foobar2', foobar2)
mfoobar2.add_arg('x','double',1)
mfoobar2.add_arg('y','double',2)
mfoobar2.add_arg('z','double',3)
mfoobar2.create_custom_attribute('menu','Foobars')

-- Make our new class visitible from GOM...
gom.bind_meta_type(mycommandclass)

-- ... and visible from MeshGrob
scene_graph.register_grob_commands('OGF::MeshGrob', mycommandclass.name)

-- -----------------------------------------------------------------------
-- Example 2: commands associated with the SceneGraph
-- -----------------------------------------------------------------------

-- It is sometimes inconvenient to have to create a MeshGrob before being
-- able to invoke any command associated to the MeshGrob. One possibility
-- is to add commands associated with the SceneGraph, that are always
-- displayed in Graphite, even when the SceneGraph is empty.
-- We are going to implement commands that create basic shapes in the
-- SceneGraph.

-- The function that implements our commands
-- One can dispatch several method to the same Lua function,
-- the additional field args.method lets you determine the
-- method that was calls.
-- (One can also use separate functions if preferred, as done
--  in the first part of this tutorial).

function create_shape(args) 
    print('create shape, args='..tostring(args))
    print('self='..tostring(args.self))

    o = scene_graph.create_object({
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
baseclass = gom.resolve_meta_type('OGF::SceneGraphCommands')
mclass    = baseclass.create_subclass('OGF::SceneGraphShapesCommands')

-- We need a constructor 
mclass.add_constructor()

-- The three commands and their arguments, with some examples of
-- custom attributes.
-- For creating a menu directly attached to the MenuBar with
-- SceneGraph commands, note that the '/menubar/Shapes' custom
-- attribute that starts with '/menubar' (unlike commands attached
-- to objects for which a single starting '/' suffices to make them
-- attached to the MenuBar).

msquare = mclass.add_slot('square', create_shape)
msquare.add_arg('name','OGF::NewMeshGrobName','shape')
msquare.create_arg_custom_attribute('name','help','name of the object to create')
msquare.add_arg('size','double',1.0)
msquare.create_arg_custom_attribute('size','help','edge length of the square')
msquare.add_arg('center','bool',false)
msquare.create_arg_custom_attribute('center','help','if set, dimensions go from -size/2 to size/2 instead of [0,size]')
msquare.create_custom_attribute('menu','/menubar/Shapes')
msquare.create_custom_attribute('help','guess what ? it creates a square (what an informative help bubble !!)')

mcube = mclass.add_slot('cube', create_shape)
mcube.add_arg('name','OGF::NewMeshGrobName','shape')
mcube.add_arg('size','double',1.0)
mcube.create_custom_attribute('menu','/menubar/Shapes')

msphere = mclass.add_slot('sphere',create_shape)
msphere.add_arg('name','OGF::NewMeshGrobName','shape')
msphere.add_arg('radius','double',1.0)
msphere.add_arg('precision','int',4)
msphere.create_custom_attribute('menu','/menubar/Shapes')

-- Make our new class visible from the Graphite Object Model
-- and from SceneGraph

gom.bind_meta_type(mclass)
scene_graph.register_grob_commands('OGF::SceneGraph', mclass.name)

-- -----------------------------------------------------------------------
-- More information
-- -----------------------------------------------------------------------

-- Now the commands you have created behave exactly as commands written in
-- C++. For instance, one can invoke them from the scripting languages of
-- Graphite (Lua, Python). Try this:
--
-- create a MeshGrob, call it S
-- in Graphite's command line, type
-- scene_graph.objects.S.I.Custom.trululu()
-- (press <tab> for automatic completion)
