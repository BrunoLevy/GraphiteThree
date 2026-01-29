--  A module that handles the gizmo
---------------------------------------------------------------

gizmo_gui = {}
gizmo_gui.visible = true
gizmo_gui.name = 'Gizmo'
gizmo_gui.icon = '@crosshairs'

-- a special function directly called in graphite_imgui.lua instead
-- of a standard module callback because it needs to be called outside
-- any window
function gizmo_gui.draw_gizmo()
    if not gizmo_gui.visible then
       return
    end
    x0 = graphite_gui.left_pane_width
    y0 = 20.0
    L =  150.0
    ImOGuizmo.SetRect(x0,y0,L)
    ImOGuizmo.BeginFrame(false)
    changed,new_rotation_matrix = ImOGuizmo.DrawGizmo(
       main.render_area.viewing_matrix,
       main.render_area.projection_matrix,
       1.0
    )
    x,y = imgui.GetMousePos()
    if x >= x0 and y >= y0 and x <= x0+L and y <= y0+L and
       imgui.IsMouseDoubleClicked(0) then
       camera_gui.home()
    elseif changed then
       xform.rotation_matrix = new_rotation_matrix
    end
end

graphite_main_window.add_module(gizmo_gui)
