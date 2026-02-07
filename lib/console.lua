--  GUI for the console, for use with Skin_imgui
------------------------------------------------

console_gui = {}
console_gui.name = 'Console'
console_gui.visible = false
console_gui.x = main.margin()
console_gui.y = 2*main.height/3 + main.margin() - main.status_bar_height()
console_gui.w = main.width - 2*main.margin()
console_gui.h = main.height/3 - 3*main.margin()
console_gui.window_flags = ImGuiWindowFlags_NoScrollbar
console_gui.menubar = false
console_gui.icon = '@terminal'
console_gui.help = 'Console and command entry'
console_gui.resizable = true

function console_gui.show()
   console_gui.visible = true
end

function console_gui.draw_menu()
   if imgui.BeginMenu('File...') then
     if imgui.MenuItem('Load and run...') then
    	imgui.OpenFileDialog(
	    '##console##load_dlg',
	    'lua',
	    '', -- default filename
	    ImGuiExtFileDialogFlags_Load
	)
     end
     imgui.EndMenu()
  end
  if imgui.BeginMenu('History...') then
     if imgui.MenuItem('Save history...') then
    	imgui.OpenFileDialog(
	    '##console##save_dlg',
	    'lua',
	    'history.lua',
	    ImGuiExtFileDialogFlags_Save
	)
     end
     if imgui.MenuItem('Clear history') then
        gom.clear_history()
     end
     imgui.EndMenu()
  end
end

function console_gui.draw_window()
   graphite_gui.console_height = imgui.GetWindowHeight() + 5
   console_gui.visible = main.draw_console(console_gui.visible)
end

function console_gui.draw_extra()
   local sel
   local filename=''
   sel,filename = imgui.FileDialog('##console##load_dlg',filename)
   if sel then
      gom.execute_file(filename)
   end
   local sel,filename = imgui.FileDialog('##console##save_dlg',filename)
   if sel then
      print('save '..filename)
      gom.save_history(filename)
   end
end

graphite_main_window.add_module(console_gui)

gom.connect(main.error_occured, console_gui.show)
