-- ==============================================================================
--                           Main GEL script for Graphite ///
--
--  This script is used in batch mode (graphite batch=true on the command line).
--  It just creates a Graphite SceneGraph.
--
--  Objects can be created with my_object = scene_graph.create_object("class name")
--    Example: my_object = scene_graph.create_object("class name")
--  Objects can be renamed
--    Example: my_object.rename("my_new_name")
--  Command classes can be created with my_object.query_interface("commands class name")
--    Example: my_object.query_interface("OGF::MeshGrobShapesCommands").create_cube()
--    There is also a short hand: my_object.query_interface("Shapes").create_cube()
--    (it tries to prepend "OGF::MeshGrob" and to append "Commands" to the argument)
--
-- ==============================================================================

gom.execute_file("graphite_common.lua")
scene_graph = OGF.SceneGraph.create(gom)
o = scene_graph.objects