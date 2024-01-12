-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- -------------------------------------------------------------------------
-- commands_example.lua How to create new commands in Lua
-- -------------------------------------------------------------------------

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

mycommandclass = gom.create({
    classname='OGF::DynamicMetaClass',
       class_name='OGF::MeshGrobMyCommands',
       super_class_name='OGF::MeshGrobCommands'
});

mconstructor = gom.create({classname='OGF::MetaConstructor', mclass=mycommandclass});
mtrululu     = gom.create({classname='OGF::DynamicMetaSlot', name='trululu', container=mycommandclass, action=trululu}) 

mtrululu.add_arg('nb','int',42)
mtrululu.add_arg('name','std::string','coucou')
mtrululu.create_custom_attribute('help','Applies Cray-Lorgan function to the data')

mfoobar1 = gom.create({classname='OGF::DynamicMetaSlot', name='foobar1', container=mycommandclass, action=foobar1})
mfoobar1.add_arg('x','double',3.14)
mfoobar1.add_arg('y','double')
mfoobar1.add_arg('z','double')
mfoobar1.create_custom_attribute('menu','Foobars')

mfoobar2 = gom.create({classname='OGF::DynamicMetaSlot', name='foobar2', container=mycommandclass, action=foobar2})
mfoobar2.add_arg('x','double',1)
mfoobar2.add_arg('y','double',2)
mfoobar2.add_arg('z','double',3)
mfoobar2.create_custom_attribute('menu','Foobars')

gom.bind_meta_type(mycommandclass)
scene_graph.register_grob_commands('OGF::MeshGrob', mycommandclass.name)

-- -----------------------------------------------------------------------
-- Example 2: commands associated with the SceneGraph
-- -----------------------------------------------------------------------

function create_shape(args) 
    print('create shape, args='..tostring(args))
    print('self='..tostring(args.self))

    o = scene_graph.create_object({
        classname='OGF::MeshGrob',name=args.name
    })

    -- one can dispatch several method to the same Lua function,
    -- the additional field args.method lets you determine the
    -- method that was calls.
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

baseclass = gom.resolve_meta_type('OGF::SceneGraphCommands')
mclass    = baseclass.create_subclass('OGF::SceneGraphShapesCommands')

mclass.add_constructor()

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

gom.bind_meta_type(mclass)
scene_graph.register_grob_commands('OGF::SceneGraph', mclass.name)

