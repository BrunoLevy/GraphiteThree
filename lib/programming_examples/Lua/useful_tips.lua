-- Lua (Keep this comment, this is an indication for editor's 'run' command)
--
-- Examples for scripting standard actions in Graphite
-- Modify the source, use 'Run' to take changes into account.
-- Read the comments at the end of this file for more details.

-- Delete all objects
scene_graph.clear()
-- Create object
scene_graph.create_object('OGF::MeshGrob')
-- Rename current object
scene_graph.current().rename('MySurface')
-- Change current object
scene_graph.current_object = 'MySurface'
-- Call a command
scene_graph.current().I.Shapes.create_sphere()
-- Change graphics attributes
scene_graph.current().shader.mesh_style='true; 0 0 0 1; 2'

-- If 'C' already exists, returns it, else creates it
-- Useful for scripts that you wand to execute several times
C = scene_graph.find_or_create_object('OGF::MeshGrob', 'C')
C.clear()
C.I.Shapes.create_box()


------------------------------------------------------------------------------------

-- Try this: press <TAB> for autocompletion (works for variables that are
--    known, i.e. as soon as the program has been already executed).
-- Try this: hover the mouse on object names and function calls for tooltips
--    (same as before, works for variables that are known)
-- Try this: select a block of code and execute it using <F5> (if there is no
--    selection, <F5> runs the whole program).
--
-- Try this: in the console, press 'tab' for autocompletion
-- Try this: in the console, type: gom.inspect(scene_graph.current().shader)

-- More details:
-- * Everything that you can do with the GUI can be scripted in Lua
--   (proof: the GUI is entirely written in Lua. Q.E.D.)
--
-- * You can gom.inspect() all Graphite objects
--
-- * Global Graphite objects are:
--      main: the OGF::Application
--      scene_graph
--      scene_graph.scene_graph_shader_manager : the 'shader manager' 
--      main.render_area                       : the 'rendering window'
--
-- * mesh_style is 'on/off; r g b a; width'  (a is supposed to be 1)
--
-- * Commands are accessible through object.I.XXX
--   Try this: in the console, type 'scene_graph.current().I' then
--      'tab' for autocompletion
--   Autocompletion in terminal in a function call gives the parameters names 
--      and default values
-- 
-- * The scene_graph has a current object
--   scene_graph.current() returns the current object
--   scene_graph.current_object returns the name of the current object
--   scene_graph.current_object='titi' sets the current object (note that 'titi'
--     is the name of the object you want to set)
--
-- * Object with name 'XXX' can be directly obtained through scene_graph.objects.XXX


