-- ------------------------------------------------------------------------------
-- \file autogui.lua
-- \brief automatic generation of the GUI (imgui) from the meta-information (gom)
-- \details has functions for
--   - editing object properties (autogui_properties.lua)
--   - creating dialogs for commands (autogui_commands.lua)
--   - both use some low-level functionalities (autogui_handlers.lua)
-- ------------------------------------------------------------------------------

autogui = {}
gom.execute_file("autogui_handlers.lua")
gom.execute_file("autogui_properties.lua")
gom.execute_file("autogui_commands.lua")
