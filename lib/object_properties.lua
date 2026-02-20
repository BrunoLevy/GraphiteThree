--  A module that handles the GUI for current object properties
---------------------------------------------------------------

object_properties_gui = {}
object_properties_gui.visible = false
object_properties_gui.name = 'Properties'
object_properties_gui.icon = '@edit'
object_properties_gui.help = 'Display and edit properties of current object'
object_properties_gui.x = main.width-3*main.margin()-150*main.scaling()
object_properties_gui.y = main.margin()
object_properties_gui.w = 165*main.scaling()
object_properties_gui.h = 180*main.scaling()
object_properties_gui.resizable = true
object_properties_gui.not_in_tree = true

function object_properties_gui.draw_window()
  if scene_graph.current() ~= nil then
     autogui.properties_editor(scene_graph.current().shader)
  end
end

graphite_main_window.add_module(object_properties_gui)

----------------------------------------------------------
