-- Lua

-- Utility to generate (crude) user documentation
-- from the meta-information in the Graphite Object Model.

-- \brief Generates the documentation for a given menu
-- \param F a file
-- \param grob an object
-- \param node nil (used internally for recursion)
-- \param path nil (used internally for recursion)
-- \param level nil (used internally for recursion)

latest_section = ''
cur_section = ''

latest_subsection = 'Misc'
cur_subsection = 'Misc'

function gen_doc(F, grob, node, path, level)
   if path == nil then
       path = ''
   end
   if node == nil then
      node = scene_graph_gui.menu_map.get(grob)
   end
   if level == nil then
      level = 0
   end
   for i,v in pairs(node.by_index) do
      if v.meta_class ~= nil then -- v is a MetaMethod

         if cur_section ~= latest_section then
	   F:write('\n\n'..'# '..cur_section..'\n\n')
	   latest_section = cur_section
	 end

         if cur_subsection ~= latest_subsection then
	   F:write('\n\n'..'## '..cur_subsection..'\n\n')
	   latest_subsection = cur_subsection
	 end

         F:write('- '..'`'..path .. '/' .. v.name..'`')

	 if(v.has_custom_attribute('help')) then
	    F:write(' : '..v.custom_attribute_value('help')..'\n')
	 end
	 
	 if(v.nb_args() ~= 0) then
	    local has_advanced_args = false
	    for i=0,v.nb_args()-1 do
	       if v.ith_arg_has_custom_attribute(i,'advanced') and
	          v.ith_arg_custom_attribute_value(i,'advanced') == 'true' then
		  has_advanced_args = true
	       end
            end

	    F:write('\n')

            if has_advanced_args then
	       F:write('  | arg name | type | default value | description | advanced | \n')
	       F:write('  |----------|------|---------------|-------------|----------| \n')
	    else
	       F:write('  | arg name | type | default value | description | \n')
	       F:write('  |----------|------|---------------|-------------| \n')
	    end
	    
	    for i=0,v.nb_args()-1 do
	       local name = v.ith_arg_name(i)
	       local argtype = v.ith_arg_type(i).name
	       local default = 'N/A'
	       if(v.ith_arg_has_default_value(i)) then
		   default=v.ith_arg_default_value_as_string(i)
	       end
	       local description = 'N/A'
	       if(v.ith_arg_has_custom_attribute(i,'help')) then
	          description=v.ith_arg_custom_attribute_value(i,'help')
	       end
	       local advanced = ' ';
	       if v.ith_arg_has_custom_attribute(i,'advanced') and
	          v.ith_arg_custom_attribute_value(i,'advanced') == 'true' then
	          advanced = '*'
	       end
	       if(has_advanced_args) then
  	            F:write('  | '.. name .. ' | ' .. argtype .. ' | ' .. default .. ' | ' .. description .. ' | ' .. advanced .. ' |\n')
               else
  	            F:write('  | '.. name .. ' | ' .. argtype .. ' | ' .. default .. ' | ' .. description .. ' |\n')	       
	       end
	    end
	 end
	 F:write('\n\n')
      else -- v is a node of the MenuMap
         local subpath = v.name
         if(path ~= '') then subpath = path .. '/' .. v.name end
	 if(level == 0) then cur_section = v.name end
	 if(level == 1) then cur_subsection = v.name end
	 gen_doc(F, grob, v, subpath, level + 1)
	 if(level == 1) then cur_subsection = 'Misc' end
      end
   end
end

S = scene_graph.create_object({classname='OGF::MeshGrob',name='S'})
F = io.open('Mesh.md','w')
F:write('# Graphite Mesh Commands reference\n\n\n')
gen_doc(F,S)
io.close(F)

main.stop()
