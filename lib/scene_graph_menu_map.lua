-- \brief MenuMap Handles object commands menus
-- \details With the 'menu' gom_attribute, the user
--   can redesign the menu hierarchy, thus generating
--   it needs a twopass algorithm. MenuMap stores the
--   menu hierarchy associated with a Grob class.

scene_graph_gui.menu_map = {}

-- \brief Creates a new node
-- \return an empty new node
function scene_graph_gui.menu_map.new_node()
   local result = {}
   result.by_name = {}
   result.by_index = {}
   return result
end

-- \brief Creates a menu item associated with a command
-- \param[in] node the node in which the item should be created
-- \param[in] mslot the MetaSlot associated with the command
function scene_graph_gui.menu_map.create_menu_item(node, mslot)
   local item_name = autogui.remove_underscores(mslot.name)
   node.by_name[item_name] = mslot
   table.insert(node.by_index, mslot)
end

-- \brief Finds or create a submenu in a node
-- \param[in] node the node
-- \param[in] submenu_name the name of the submenu
-- return the node associated with the submenu. It is
--  created if it did not exist already.
function scene_graph_gui.menu_map.find_or_create_submenu(node, submenu_name)
   local submenu = node.by_name[submenu_name]
   if submenu == nil then
      submenu = scene_graph_gui.menu_map.new_node()
      submenu.name = submenu_name
      node.by_name[submenu_name] = submenu
      table.insert(node.by_index,submenu)
   end
   return submenu
end

-- \brief Inserts a command in the menu hierarchy
-- \param[in] node the root node of the hierarchy
-- \param[in] menu_path the path in the menu, with
--  submenus separated by '/'.
function scene_graph_gui.menu_map.insert(node, menu_path, mslot)
   if menu_path == nil then
       scene_graph_gui.menu_map.create_menu_item(node,mslot)
   else
       local head,tail = scene_graph_gui.menu_map.split_menu_path(menu_path)
       local submenu = scene_graph_gui.menu_map.find_or_create_submenu(
           node, head
       )
       scene_graph_gui.menu_map.insert(submenu,tail,mslot)
   end
end

-- \brief Splits a menu path into its head component
--  and the rest of the path.
-- \return the first component of the path
--  and the rest or nil if the past had a single
--  component
function scene_graph_gui.menu_map.split_menu_path(menu_path)
   local pos = menu_path:find('/')
   if pos == nil then
      return menu_path,nil
   else
      return menu_path:sub(1,pos-1),menu_path:sub(pos+1)
   end
end

-- \brief Gets the MenuMap associated with a grob.
-- \return the MenuMap.
function scene_graph_gui.menu_map.get(grob)
   -- We could cache it if need be, but LUA is so fast
   -- that it is probably not worth it...
   local menus = scene_graph_gui.menu_map.new_node()
   local grob_class_name = grob.meta_class.name
   local k = grob_class_name..'_commands'
   local commands_str = gom.get_environment_value(k)
   for command_class_name in string.split(commands_str,';') do
      -- We skip OGF::SceneGraphSceneCommands because they
      -- are now implemented in the context menu of each object
      -- (rename, delete etc...)
      if command_class_name ~= 'OGF::SceneGraphSceneCommands' then
         local default_menu_name = command_class_name
	 local command_mclass = gom.resolve_meta_type(command_class_name)
	 -- Command may be associated with a base class, so we find
	 -- the name of this base class in the 'grob_class_name' attribute
	 -- of the Command and strip it to generate the menu name.
         default_menu_name = string.remove_prefix(
	    default_menu_name,
	    command_mclass.custom_attribute_value('grob_class_name')
	 )
         default_menu_name = string.remove_suffix(default_menu_name,'Commands')
         local mclass = gom.resolve_meta_type(command_class_name)
         for i=0,mclass.nb_slots()-1 do
            local mslot = mclass.ith_slot(i)
	    local menu_name = default_menu_name
	    if mslot.has_custom_attribute('menu') then
   	       local submenu_name = mslot.custom_attribute_value('menu')
	       -- Remove trailing '/' if present
	       if submenu_name:sub(-1) == '/' then
		   submenu_name = submenu_name:sub(1,-2)
	       end
               -- If submenu starts with '/' it is an absolute path
	       if submenu_name:sub(1,1) == '/' then
                   menu_name = submenu_name:sub(2)
                   -- Particular case: SceneGraph commands starting with '/',
                   -- to be rooted in the menu bar, are stored in the '/menubar'
                   -- menumap (and handled with
                   -- specific code in graphite_gui.draw_menu_bar())
                   if(grob.meta_class.name == 'OGF::SceneGraph') then
                      menu_name = 'menubar/'..menu_name
                   end
	       else
	          menu_name = menu_name .. '/' .. submenu_name
	       end
	    end
            if autogui.member_is_visible(nil, mslot) then
	       scene_graph_gui.menu_map.insert(menus, menu_name, mslot)
	    end
         end
      end
   end
   return menus
end

-- \brief Draws the command menus for a grob.
-- \param[in] grob the grob
-- \param[in] node the submenu to draw, or nil
--   if nil, the whole menu hierarchy is drawn.
function scene_graph_gui.menu_map.draw(grob, node)
   if node == nil then
     node = scene_graph_gui.menu_map.get(grob)
   end
   for i,v in pairs(node.by_index) do
      if v.meta_class ~= nil then
         local command_class = v.container_meta_class()
	 local command_class_name = command_class.name
         local request = grob.I[command_class.name][v.name]
         autogui.command_menu_item(request)
      else
         if v.name ~= 'menubar' then
            if imgui.BeginMenu(v.name) then
               scene_graph_gui.menu_map.draw(grob, v)
	       imgui.EndMenu()
	    end
         end
      end
   end
end
