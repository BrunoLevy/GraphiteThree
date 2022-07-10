-- Lua

-- Utility to generate (crude) user documentation
-- from the meta-information in the Graphite Object Model.

-- \brief Generates the documentation for a given menu
-- \param F a file
-- \param grob an object
-- \param filter an optional menu name
-- \param node nil (used internally for recursion)
-- \param path nil (used internally for recursion)

function gen_doc(F, grob, filter, node, path)
   if path == nil then
      path = ''
   end
   if node == nil then
     node = scene_graph_gui.menu_map.get(grob)
   end
   for i,v in pairs(node.by_index) do
      if v.meta_class ~= nil then
         F:write('- '..'`'..path .. '/' .. v.name..'`')
	 if(v.meta_class.has_custom_attribute('help')) then
	    F:write(' : '..v.custom_attribute_value('help')..'\n')
	 end
	 if(v.nb_args() ~= 0) then
	    F:write('\n')
	    F:write('  | arg name | type | description |\n')
	    F:write('  |----------|------|-------------|\n')
	    for i=0,v.nb_args()-1 do
	       local name = v.ith_arg_name(i)
	       local type = v.ith_arg_type(i).name
	       local description = 'N/A'
	       if(v.ith_arg_has_custom_attribute(i,'help')) then
	          description=v.ith_arg_custom_attribute_value(i,'help')
	       end
	       F:write('  | '.. name .. ' | ' .. type .. ' | ' .. description .. ' |\n')
	    end
	 end
	 F:write('\n\n')
      else
         local subpath = v.name
         if(path ~= '') then subpath = path .. '/' .. v.name end
         if(filter == nil or subpath:starts_with(filter)) then
	       gen_doc(F,grob, filter, v, subpath)
         end
      end
   end
end


S = scene_graph.create_object({classname='OGF::MeshGrob',name='S'})
F = io.open('doc.md','w')
gen_doc(F,S,'Points')
io.close(F)
