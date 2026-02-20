-- Docked command zone
--------------------------------------

command_gui = {}
command_gui.name = 'Command'
command_gui.icon = '@code-branch'
command_gui.help = 'If set, command dialogs are docked'
command_gui.x = 3*main.margin()+150*main.scaling()+5
command_gui.y = main.margin()
command_gui.w = 220*main.scaling()
command_gui.h = 300*main.scaling()
command_gui.not_in_tree = true

function command_gui.update_visibility()
   command_gui.inhibit = (command_gui.request == nil)
   toolbox_gui.inhibit = (command_gui.visible and command_gui.request ~= nil)
end

function command_gui.draw_window()
   if command_gui.request == nil then
      autogui.TextDisabled('<No active command>')
   else
      k = autogui.request_key(command_gui.request)
      cmd_state = autogui.command_state[k]
      if cmd_state ~= nil and not autogui.command_is_alive(cmd_state) then
         command_gui.request = nil
      else
         autogui.command_dialog(command_gui.request)
      end
   end
end

graphite_main_window.add_module(command_gui)
