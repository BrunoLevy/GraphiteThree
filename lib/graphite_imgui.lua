-- =============================================================================
--                Main Lua script for Graphite with ImGUI ///
-- =============================================================================

main = gom.create({classname='OGF::Application',interpreter=gom})

scene_graph = gom.create({classname='OGF::SceneGraph',interpreter=gom})
scene_graph.render_area = main.render_area
scene_graph.application = main
gom.connect(scene_graph.value_changed, main.render_area.update)
scene_graph.scene_graph_shader_manager =
    gom.create('OGF::SceneGraphShaderManager')
gom.connect(
    main.render_area.redraw_request,
    scene_graph.scene_graph_shader_manager.draw
)
gom.connect(
   scene_graph.values_changed,
   scene_graph.scene_graph_shader_manager.update_focus
) 
gom.connect(main.render_area.dropped_file, scene_graph.load_object)

--------------------------------------------------------------------
gom.execute_file("graphite_common.lua")
gom.execute_file("autogui.lua")
--------------------------------------------------------------------

gom.execute_file("preferences.lua")
gom.execute_file("main_window.lua")
gom.execute_file("scene_graph.lua")
gom.execute_file("camera.lua")
gom.execute_file("object_properties.lua")
gom.execute_file("toolbox.lua")
gom.execute_file("text_editor.lua")
gom.execute_file("console.lua")
gom.execute_file("histogram.lua")

preferences_window.bind_function_keys()

--------------------------------------------------------------------

graphite_gui = {}

function graphite_gui.presentation_mode()
   return gom.get_environment_value('gui:presentation_mode') == 'true'
end

function graphite_gui.draw_status_bar()
    if main.status_bar_is_active() then
        imgui.SetNextWindowPos(
   	   main.margin(),
	   main.height - main.margin() - main.status_bar_height(),
	   ImGuiCond_Always
        )
        imgui.SetNextWindowSize(
           main.width - 2*main.margin(),
	   main.status_bar_height(),
	   ImGuiCond_Always
        )
        main.draw_status_bar()
    end
end

function graphite_gui.draw_menu_bar()
   if imgui.BeginMainMenuBar() then
      imgui.PushID('##MainMenu')
      scene_graph_gui.file_menu()
      imgui.PopID()
      if imgui.BeginMenu('Scene##MainMenu') then
         scene_graph_gui.scene_graph_menu(false)
	 imgui.EndMenu()
      end
      if scene_graph.current() ~= nil then
          scene_graph_gui.grob_ops(
	     scene_graph.current(), true
	  )
      end
      imgui.EndMainMenuBar()
   end
end

-- =============================================================================

-- \brief Frame counter, to launch garbage collector every NNN frames
graphite_gui.frame = 0

-- \brief The function that handles the GUI
function graphite_gui.draw()
    if not graphite_gui.presentation_mode() then
       graphite_gui.draw_menu_bar()
       main.draw_dock_space()
    end
    graphite_main_window.draw()
    autogui.command_dialogs()

    graphite_gui.draw_status_bar()
    autogui.property_editors()
    preferences_window.draw()

    graphite_gui.frame = graphite_gui.frame+1
    if graphite_gui.frame % 50 == 0 then
         collectgarbage()
    end
end    

gom.connect(main.redraw_request, graphite_gui.draw)

--==============================================================================

function graphite_gui.escape()
   local gp_visible = false
   if GraphitePoint ~= nil then
      gp_visible = GraphitePoint.visible
   end      
   main.full_screen = false 
   preferences_window.load_preferences('user');
   if gp_visible then
      GraphitePoint.visible = true
   end
end

gom.connect(main.render_area.key_down, graphite_gui.escape)
   .if_arg({name='value', value='escape'})

gom.connect(main.render_area.key_down, main.set_full_screen_mode)
   .if_arg({name='value', value='F12'})   
   
--==============================================================================

gom.connect(main.started, post_init) 

--==============================================================================

main.declare_preference_variable(
   'gui:presentation_mode','false','Presentation mode'
)

main.declare_preference_variable(
   'gui:shaders_selector','true','Draw shaders selector'
)

--==============================================================================

-- Note: main.start() is directly invoked by Graphite's main.cpp
