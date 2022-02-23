-- Lua (Keep this comment, this is an indication for editor's 'run' command)

-- Example for Graphite object
-- Creates a new dialog box

-- A table that gather all variables related
-- with the new dialog
my_dialog = {}

-- Dialog visibility flag
my_dialog.visible = true

-- The name of the dialog
my_dialog.name = 'My dialog'

-- (optional): Initial position of the dialog
my_dialog.x = 100
my_dialog.y = 400

-- This function draws the dialog window

function my_dialog.draw_window()
   imgui.Text('Hello, world')
end

-- Registers the new "module" to the main Graphite window.
-- Note: if you register a module several time with the same
-- name, the latest one overwrites the previous ones (thus it
-- is possible to 're-run' the program from the editor when
-- you modify it).
-- Try now to add a button in the draw() function.

graphite_main_window.add_module(my_dialog)


