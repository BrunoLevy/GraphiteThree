-- ==============================================================================
--              Main GEL script for Graphite /// in batch mode
--
--  This script is used in batch mode (graphite batch=true on the command line).
--  It just creates a Graphite SceneGraph.
--
--  Objects can be created with obj = scene_graph.create_object("class name")
--    Example: my_object = scene_graph.create_object("OGF::MeshGrob")
--  Objects can be renamed
--    Example: my_object.rename("my_new_name")
--  Command classes can be created with my_object.I.<command class name>
--    Example: my_object.I.Shapes.create_cube()
--
-- ==============================================================================

gom.execute_file("graphite_common.lua")
scene_graph = OGF.SceneGraph.create(gom)
o = scene_graph.objects