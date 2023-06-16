-- Lua (keep this comment, it is an indication for editor's 'run' command)

-- displays an "animation" where each object of the scene graph is a frame

camera.draw_selected_only=true

for i = 0,scene_graph.nb_children-1,1 do
   scene_graph.current_object = scene_graph.ith_child(i).name
   main.draw()
end


