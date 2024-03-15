--  GOM/ImGUI/LUA coupling, for use with Skin_imgui
----------------------------------------------------

-- \brief creates editors for object commands properties based on their types.
-- \details it is implemented as a table that maps typenames to
--   'editing functions'. 

autogui = {}

-- \brief The handlers indexed by type names
-- \details Each handler is a function with the following prototype:
--   handler(object, property_name, mtype, tooltip) where
--   - object is the object beeing edited
--   - property_name is a string with the name of the property beeing edited
--   - mtype is the GOM meta-type of the property. It is used by enums (that
--     need it to query the list of names)
--   - tooltip is an optional tooltip

autogui.handlers = {}

-- \brief Flags to be used for imgui.TextInput
-- \details When editing properties, need to validate with enter else
--   transient values will be sent (not good)

autogui.input_text_flags = 0


-- \brief Margin to avoid the "auto-growing" effect on some dialogs
autogui.margin = 5

--------------------------------------------------------------------------------

-- \brief Replaces the underscores in a string with spaces
-- \param[in] name the string
-- \return the string with the underscores replaced by spaces

function autogui.remove_underscores(name)
  return name:gsub('_',' ')
end

-- \brief Wrapper arround imgui.Text() that removes the underscores

function autogui.Text(name)
   imgui.Text(autogui.remove_underscores(name))
end

-- \brief Displays an optional tooltip attached to the latest widget
-- \param[in] tooltip a string with the message to be displayed in
--  the tooltip or nil

function autogui.tooltip(tooltip)
   if tooltip ~= nil and imgui.IsItemHovered() and
      gom.get_environment_value('gui:tooltips') == 'true' then
      imgui.SetTooltip(tooltip)
   end
end

-- \brief Finds the help associated with a GOM meta-information
-- \param[in] minfo the GOM meta-information
-- \return the associated help or nil if no help is available

function autogui.help(minfo)
   if minfo.has_custom_attribute('help') then
      return minfo.custom_attribute_value('help')
   end   
end

-- \brief Gets the default size of an icon.
-- \return the default size of an icon.

function autogui.icon_size()
   return 18.0 * main.scaling()
end

-- \brief Edits a property of an object using a combo-box
-- \param[in] object the object beeing edited
-- \param[in] property_name the name of the property beeing edited
-- \param[in] values the ';'-separated list of possible values
-- \param[in] tooltip an optional text to be displayed in a tooltip or nil

function autogui.combo_box(object, property_name, values, tooltip)
   autogui.Text(property_name)
   autogui.tooltip(tooltip)

   -- If values start with ';', remove the spurious ';'
   if string.starts_with(values,';') then
      values = values:sub(2)
   end

   local old_value = tostring(object[property_name])

   local found = false
   local first_value = nil
   
   for val in string.split(values,';') do
      if first_value == nil then
         first_value = val
      end
      if val == old_value then
         found = true
      end
   end

   imgui.PushItemWidth(-autogui.margin)
   local sel,new_value = imgui.Combo(
      '##properties##'..property_name,
      old_value,
      values
   )
   imgui.PopItemWidth()

   -- If initial value was not one of the
   -- combobox values, then update to first
   -- combobox value.
   
   if not sel and not found then
      sel,new_value = true,first_value
   end

   if sel then
        object[property_name] = new_value   
   end
end

-- \brief Edits a value using a combo-box
-- \param[in] value the old value
-- \param[in] values the ';'-separated list of possible values
-- \return sel,new_value where sel is set to true if the value changed

function autogui.combo_box_value(name,old_value,values)

   local found = false
   local first_value = nil
   
   for val in string.split(values,';') do
      if first_value == nil then
         first_value = val
      end
      if val == old_value then
         found = true
      end
   end

   local sel,new_value = imgui.Combo(
      name,
      old_value,
      values
   )

   -- If initial value was not one of the
   -- combobox values, then update to first
   -- combobox value.
   
   if not sel and not found then
      sel,new_value = true,first_value
   end

   return sel,new_value
end

-- \brief Edits a property of an object using a 'combo-box' where text
--  can be also freely modified
-- \param[in] object the object beeing edited
-- \param[in] property_name the name of the property beeing edited
-- \param[in] values the ';'-separated list of possible values
-- \param[in] tooltip an optional text to be displayed in a tooltip or nil

function autogui.editable_combo_box(object, property_name, values, tooltip)
   autogui.Text(property_name)
   autogui.tooltip(tooltip)

   imgui.PushItemWidth(-1-autogui.icon_size()-25)
   
   local sel,new_value = imgui.TextInput(
	'##properties##1_'..property_name,
	tostring(object[property_name]),
	autogui.input_text_flags	
   )

   if sel then
      object[property_name] = new_value   
   end

   imgui.PopItemWidth()
   imgui.SameLine()
   if imgui.Button( imgui.font_icon('caret-down').. '##properties##btn_'..property_name) then
      imgui.OpenPopup('##properties##popup_'..property_name)   
   end

   if(imgui.BeginPopup('##properties##popup_'..property_name)) then
     for val in string.split(values,';') do
        if imgui.Selectable(val) then
	   object[property_name] = val
	end
     end
     imgui.EndPopup()
   end	 
end


-- \brief Handler for enums
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

function autogui.enum(object, property_name, menum, tooltip)
   local values=''
   for i=0,menum.nb_values()-1 do
      if #values ~= 0 then
         values = values .. ';'
      end
      values = values .. menum.ith_name(i)
   end
   autogui.combo_box(object, property_name, values, tooltip)
end

-- \brief Handler for strings
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

function autogui.string(object, property_name, mtype, tooltip)
   autogui.Text(property_name)
   autogui.tooltip(tooltip)
   imgui.PushItemWidth(-autogui.margin)
   local sel,new_value = imgui.TextInput(
	'##properties##'..property_name,
	tostring(object[property_name]),
	autogui.input_text_flags	
   )
   imgui.PopItemWidth()
   if sel then
        object[property_name] = new_value   
   end
end

-- \brief Handler for integers
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

function autogui.integer(object, property_name, mtype, tooltip)
   autogui.Text(property_name)
   autogui.tooltip(tooltip)
   imgui.PushItemWidth(-autogui.margin)
   local sel,new_value = imgui.InputInt(
	'##properties##'..property_name,
	object[property_name]
   )
   imgui.PopItemWidth()
   if sel then
        object[property_name] = new_value   
   end
end

-- \brief Handler for unsigned integers
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

function autogui.unsigned_integer(object, property_name, mtype, tooltip)
   autogui.Text(property_name)
   autogui.tooltip(tooltip)
   imgui.PushItemWidth(-autogui.margin)
   local sel,new_value = imgui.InputInt(
	'##properties##'..property_name,
	object[property_name]
   )
   imgui.PopItemWidth()
   if new_value < 0 then
        new_value = 0
   end
   if sel then
        object[property_name] = new_value   
   end
end

autogui.handlers['std::string']  = autogui.string
autogui.handlers['int']          = autogui.integer
autogui.handlers['unsigned int'] = autogui.unsigned_integer
autogui.handlers['GEO::index_t'] = autogui.unsigned_integer

-- \brief Handler for booleans
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['bool'] = function(object, property_name, mtype, tooltip)
   local old_val = (tostring(object[property_name]) == 'true')
   local sel,new_value = imgui.Checkbox(
       autogui.remove_underscores(property_name)..'##properties', old_val
   )
   autogui.tooltip(tooltip)   
   if sel then
        object[property_name] = tostring(new_value)
   end
end

-- \brief Edits a color
-- \param[in] label the text label to be displayed
-- \param[in] oldval the previous value of the color
-- \return a boolean that indicates whether the color was changed
--  and the new value of the color

function autogui.color(label,oldval,with_alpha)
   local value=oldval
   local words={}
   for word in string.split(value,' ') do
      words[#words+1] = word
   end
   local r = tonumber(words[1]);
   local g = tonumber(words[2]);
   local b = tonumber(words[3]);
   local a = tonumber(words[4]);
   local sel
   if with_alpha then
      sel,r,g,b,a = imgui.ColorEdit4WithPalette(
         autogui.remove_underscores(label),r,g,b,a
      )
   else 
      sel,r,g,b = imgui.ColorEdit3WithPalette(
         autogui.remove_underscores(label),r,g,b
      )
   end
   return sel,tostring(r)..' '..tostring(g)..' '..tostring(b)..' '..tostring(a)
end

-- \brief Handler for Colors
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['GEO::Color'] = function(object, property_name, mtype, tooltip)
   autogui.Text(property_name)
   autogui.tooltip(tooltip)
   imgui.SameLine()
   imgui.PushItemWidth(-autogui.margin)
   local sel,new_value = autogui.color(
	'##properties##'..property_name,
	object[property_name],
	true
   )
   imgui.PopItemWidth()
   if sel then
        object[property_name] = new_value   
   end
end

autogui.handlers['OGF::Color'] = autogui.handlers['GEO::Color']

-- \brief Handler for Boolean+Color
-- \details Used for instance by OGF::SurfaceStyle. The value is a ';'-separated
--   string with the boolean and the color.
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

function autogui.bool_color(object, property_name, mtype, tooltip)
   local value=object[property_name]
   local words={}
   for word in string.split(value,';') do
      words[#words+1] = word
   end
   local visible=(words[1]=='true') 
   local color=words[2]
   autogui.Text(property_name)
   autogui.tooltip(tooltip)
   local s1,s2
   s1,visible = imgui.Checkbox('##visible##'..property_name,visible)
   imgui.SameLine()
   imgui.PushItemWidth(-autogui.margin)
   s2,color = autogui.color(
	'##color##'..property_name,
	color,
	true
   )
   imgui.PopItemWidth()      
   if s1 or s2 then
      value = tostring(visible) .. ';' .. color
      object[property_name] = value
   end
end

-- \brief Handler for Boolean+Color+integer
-- \details Used for instance by OGF::EdgeStyle and OGF::PointStyle.
--   The value is a ';'-separated string with the boolean,color and integer.
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

function autogui.bool_color_int(object,property_name, mtype, tooltip)
   local value=object[property_name]
   local words={}
   for word in string.split(value,';') do
      words[#words+1] = word
   end
   local visible=(words[1]=='true') 
   local color=words[2]
   local size=tonumber(words[3])
   autogui.Text(property_name)
   autogui.tooltip(tooltip)
   local s1,s2,s3
   s1,visible = imgui.Checkbox(
      '##visible##'..property_name,visible
   )
   imgui.SameLine()
   s2,color = autogui.color(
	'##color##'..property_name,
	color
   )
   imgui.SameLine()
   imgui.PushItemWidth(-autogui.margin)   
   s3,size = imgui.InputInt(
      '##size##'..property_name,
      size
   )
   imgui.PopItemWidth()
   if s1 or s2 or s3 then
      if size < 1 then
         size = 1
      end
      value = tostring(visible) .. ';' .. color .. ';' .. tostring(size)
      object[property_name] = value
   end
end

autogui.handlers['OGF::SurfaceStyle'] = autogui.bool_color
autogui.handlers['OGF::EdgeStyle'] = autogui.bool_color_int
autogui.handlers['OGF::PointStyle'] = autogui.bool_color_int

-- \brief All the colormaps
-- \details They correspond to XPM files located in lib/icons/colormaps
--   The boolean indicates whether the colormap is perceptually correct

autogui.colormaps = {
  {'french', false},
  {'black_white', false},
  {'viridis', true},
  {'rainbow', false},
  {'turbo', false},  
  {'cei_60757', false},
  {'inferno', true},
  {'magma', true},
  {'parula', true},
  {'plasma', true},
  {'iso1', false},
  {'iso2', false},
  {'random', false},
  {'blue_red', false},
  {'blue_white', false},
  {'black-blue-white', false},
  {'transparent', false},
  {'grayscale', true}
}

-- \brief Edits a colormap name
-- \param[in] label the text label to be displayed
-- \param[in] oldval the previous colormap
-- \return a boolean that indicates whether the colormap was changed
--  and the new colormap

function autogui.colormap(label,oldval)
   local sel=false
   local newval=oldval
   if imgui.ImageButton(
       label..'_choose_colormap',
       main.resolve_icon('colormaps/'..oldval),
       100.0*main.scaling(),
       10.0*main.scaling()
   ) then
       imgui.OpenPopup(label);   
   end
   if imgui.BeginPopup(label) then
      for i=1,#autogui.colormaps do
         if imgui.ImageButton(
	    label..'_'..autogui.colormaps[i][1],
	    main.resolve_icon('colormaps/'..autogui.colormaps[i][1]),
	    80.0*main.scaling(),
	    8.0*main.scaling()
	 ) then
	    newval=autogui.colormaps[i][1]
	    sel=true
	    imgui.CloseCurrentPopup()
	 end
	 if autogui.colormaps[i][2] then
	    autogui.tooltip('Perceptually correct')	 
  	    imgui.SameLine()
	    imgui.Text('*')
         end
      end
      imgui.EndPopup()
   end
   return sel,newval
end

-- \brief Handler for ColormapStyle
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::ColormapStyle'] = function(
   object,property_name,mtype,tooltip
)
   imgui.Separator()
   local value=object[property_name]
   local words={}
   for word in string.split(value,';') do
      words[#words+1] = word
   end
   local colormap=words[1]
   local smooth=(words[2]=='true')
   local cmrepeat=tonumber(words[3])
   local show=(words[4]=='true')
   local invert=(words[5]=='true')
   local sel1,sel2,sel3,sel4,sel5

   autogui.Text(property_name)
   autogui.tooltip(tooltip)

   sel1,colormap = autogui.colormap('##colormap##'..property_name,colormap)
   sel5,invert = imgui.Checkbox('##invert##'..property_name,invert)
   imgui.SameLine()
   if invert then
      imgui.Image(
          main.resolve_icon('flop',true),
          autogui.icon_size(),autogui.icon_size()
      )
   else
      imgui.Image(
          main.resolve_icon('flip',true),
          autogui.icon_size(),autogui.icon_size()
      )
   end
   imgui.SameLine()   
   sel2,smooth = imgui.Checkbox('##smooth##'..property_name,smooth)
   imgui.SameLine()
   imgui.Image(
      main.resolve_icon('smooth_colormap',true),
      autogui.icon_size(),autogui.icon_size()
   )
   imgui.PushItemWidth(-autogui.margin)   
   sel3,cmrepeat = imgui.InputInt('##repeat##'..property_name,cmrepeat)
   if cmrepeat<0 then
      cmrepeat=0
   end
   imgui.PopItemWidth()

   if sel1 or sel2 or sel3 or sel4 or sel5 then
       object[property_name] =
            colormap .. ';' ..
	    tostring(smooth)   .. ';' ..
	    tostring(cmrepeat) .. ';' ..
	    tostring(show)     .. ';' ..
	    tostring(invert) 
   end
   imgui.Separator()   
end

-- \brief Handler for OGF::GrobClassName
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::GrobClassName'] = function(
   object, property_name, mtype, tooltip
)
   local values = gom.get_environment_value('grob_types')
   autogui.combo_box(object, property_name, values, tooltip)
end

-- \brief Handler for OGF::FullScreenEffectName
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::FullScreenEffectName'] = function(
   object, property_name, mtype, tooltip
)
   local values = gom.get_environment_value('full_screen_effects')
   autogui.combo_box(object, property_name, values, tooltip)
end


-- \brief Handler for OGF::MeshGrobName
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::MeshGrobName'] = function(
   object, property_name, mtype, tooltip
)
   local values = gom.get_environment_value('OGF::MeshGrob_instances')
   autogui.combo_box(object, property_name, values, tooltip)
end

-- \brief Handler for OGF::NewMeshGrobName
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::NewMeshGrobName'] = function(
   object, property_name, mtype, tooltip
)
   local values = gom.get_environment_value('OGF::MeshGrob_instances')
   autogui.editable_combo_box(object, property_name, values, tooltip)
end


-- \brief Handler for OGF::LuaGrobName
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::LuaGrobName'] = function(
   object, property_name, mtype, tooltip
)
   local values = gom.get_environment_value('OGF::LuaGrob_instances')
   autogui.combo_box(object, property_name, values, tooltip)
end


-- \brief Handler for OGF::NewLuaGrobName
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::NewLuaGrobName'] = function(
   object, property_name, mtype, tooltip
)
   local values = gom.get_environment_value('OGF::LuaGrob_instances')
   autogui.editable_combo_box(object, property_name, values, tooltip)
end

-- \brief Handler for OGF::PluginName
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::DynamicModuleName'] = function(
   object, property_name, mtype, tooltip
)
   local values = gom.get_environment_value('modules')
   autogui.combo_box(object, property_name, values, tooltip)
end


-- \brief Handler for displaying a combobox
-- \details the list of values in the combobox is obtained by querying a
--   property of the object. The name of this property is given by the
--   'values' custom attribute of object.property_name. Works for both
--   properties and command arguments.
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers.combo_box = function(
   object,property_name,mtype,tooltip
)
   local values_custom_attribute

   if object.meta_class ~= nil then
      -- object is a GOM object (Shader).
      -- Retrieve 'values' custom attribute of the property.
      values_custom_attribute = object.meta_class.
            find_property(property_name).custom_attribute_value('values')
   else
      -- object is a command state dictionnary.
      -- Retrieve 'values' custom attribute of the arg in the meta-method.
      for i = 0,object.mmethod_.nb_args()-1 do
         if object.mmethod_.ith_arg_name(i) == property_name then
	     values_custom_attribute =
	         object.mmethod_.ith_arg_custom_attribute_value(i,'values')
	 end
      end
   end

   local values = values_custom_attribute

   -- if values start with a '$' sign, execute the content
   -- with object as the context to get the actual list of
   -- values (used for instance to generate list of attributes
   -- from "$grob.attributes")
   if values:sub(1,1) == '$' then
     values = values:sub(2,values:len())
     -- evaluate 'values' using object as context
     -- (Lua is so cool !)
     values = load(
        'return '..values,       -- code to be evaluated
	'gom_attribute: values', -- tag for error messages
        'bt',                    -- both binary and text
	object                   -- environment
     )()
   end
   
   autogui.combo_box(object, property_name, values, tooltip)
end

-- \brief Handler for editing an integer with a slider
-- \details the min and max are obtained by quering the 'min' and 'max'
--   custom attributes of the meta property
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers.slider_int = function(
   object,property_name,mtype,tooltip
)
   local min = 0
   local max = 100
   if object.meta_class ~= nil then
      local mprop = object.meta_class.find_member(property_name)
      min = tonumber(mprop.custom_attribute_value('min'))
      max = tonumber(mprop.custom_attribute_value('max'))
   end
   local sel,val
   sel,val = imgui.SliderInt(
       autogui.remove_underscores(property_name),
       object[property_name], min, max, '%d' 
   )
   if sel then
      object[property_name] = val
   end
   autogui.tooltip(tooltip)
end

-- true when the current dialog created by autogui is in a popup.
-- it is required to avoid creating FileDialogs from popups (which
-- closes the dialog unexpectingly). TODO: find a better fix.
autogui.in_popup = false

-- \brief Handler for file names
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \param[in] extensions semi-colon separated list of extensions or nil
--   (matches any file)
-- \param[in] create_file true if file can be created
-- \see autogui.handlers

function autogui.file_name(
   object, property_name, mtype, tooltip, extensions, create_file
)
   if extensions == nil then
      extensions = '*'
   end
   autogui.Text(property_name)
   autogui.tooltip(tooltip)
   imgui.PushItemWidth(-23-autogui.icon_size())
   local sel,new_value = imgui.TextInput(
	'##properties##'..property_name,
	tostring(object[property_name]),
	autogui.input_text_flags	
   )
   imgui.PopItemWidth()
   if sel then
        object[property_name] = new_value   
   end

   if not autogui.in_popup then
     imgui.SameLine()
     local flags;
     if create_file then
       flags = ImGuiExtFileDialogFlags_Save
     else
       flags = ImGuiExtFileDialogFlags_Load   
     end

     if imgui.Button(
       imgui.font_icon('folder-open').. '##properties##btn_'..property_name
     ) then
       imgui.OpenFileDialog(
           '##attr##file_dlg##'..property_name,
            extensions,
            '', -- default filename
 	    flags
         )
     end
   
     sel,new_value = imgui.FileDialog(
        '##attr##file_dlg##'..property_name,object[property_name]
     )
   end

   if sel then
        object[property_name] = new_value   
   end
end

-- \brief Handler for new file names
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \param[in] extensions semi-colon separated list of extensions
-- \see autogui.handlers

function autogui.new_file_name(
   object, property_name, mtype, tooltip, extensions
 )
   autogui.file_name(object, property_name, mtype, tooltip, extensions, true)
end

autogui.handlers['OGF::FileName']         = autogui.file_name
autogui.handlers['OGF::NewFileName']      = autogui.new_file_name

autogui.handlers['OGF::ImageFileName'] = function(
   object, property_name, mtype, tooltip
 )
   extensions = gom.get_environment_value('image_read_extensions')
   extensions = string.gsub(extensions,'*.','')
   autogui.file_name(object, property_name, mtype, tooltip, extensions)
end

autogui.handlers['OGF::NewImageFileName'] = function(
      object, property_name, mtype, tooltip
   )
--   extensions = gom.get_environment_value('image_write_extensions')
--   extensions = string.gsub(extensions,'*.','')   
   autogui.new_file_name(object, property_name, mtype, tooltip, extensions)
end

autogui.handlers['OGF::LuaFileName'] = function(
   object, property_name, mtype, tooltip
 )
   autogui.file_name(object, property_name, mtype, tooltip, 'lua')
end

autogui.handlers['OGF::NewLuaFileName'] = function(
      object, property_name, mtype, tooltip
   )
   autogui.new_file_name(object, property_name, mtype, tooltip, 'lua')
end

-- \brief User API function for editing object properties
-- \param[in] object the object
-- \param[in] mproperty the meta-property of the property beeing edited

function autogui.property(object, mproperty)
   local bkp = autogui.input_text_flags
   autogui.input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue

   local tooltip = autogui.help(mproperty)
   if mproperty.type().meta_class.name == 'OGF::MetaEnum' then
        autogui.enum(object, mproperty.name, mproperty.type(), tooltip)
   else
        local k = mproperty.type_name()
        local handler = autogui.handlers[k]
	if mproperty.has_custom_attribute('handler') then
	   local handler_name = mproperty.custom_attribute_value('handler')
	   handler = autogui.handlers[handler_name]
	end
        if handler == nil then
	    if string.ends_with(k, 'FileName') then
	       if string.starts_with(k, 'OGF::New') then
	          handler = autogui.new_file_name
	       else
	          handler = autogui.file_name
	       end
	    else 
               handler = autogui.string
            end	       
        end
        handler(object,mproperty.name,mproperty.type(),tooltip)
   end

   autogui.input_text_flags = bkp
end

-- \brief User API function for editing command arguments
-- \param[in] args a map with the arguments
-- \param[in] mslot the meta-slot that corresponds to the command
-- \param[in] i the index of the argument

function autogui.slot_arg(args,mslot,i)
   local type_name = mslot.ith_arg_type_name(i) 
   local mtype = gom.resolve_meta_type(type_name)
   if mtype.meta_class.name == 'OGF::MetaEnum' then
        autogui.enum(args, mslot.ith_arg_name(i), mtype, tooltip)
	return
   end
   local handler = autogui.handlers[type_name]
   if mslot.ith_arg_has_custom_attribute(i,'handler') then
      local handler_name = mslot.ith_arg_custom_attribute_value(i,'handler')
      handler = autogui.handlers[handler_name]
   end
   if handler == nil then
      handler = autogui.string
   end
   local tooltip
   if mslot.ith_arg_has_custom_attribute(i,'help') then
      tooltip = mslot.ith_arg_custom_attribute_value(i,'help')
   end
   handler(args,mslot.ith_arg_name(i),mtype,tooltip)
end

-- \brief Keeps the state (arg lists) of the invoked commands
-- \details Command state is indexed by
--  'instance_name.classname.method' strings.

autogui.command_state  = {}

-- \brief Default values of parameters indexed by type name

autogui.default_value = {}
autogui.default_value['bool']         = 'false'
autogui.default_value['float']        = '0.0'
autogui.default_value['double']       = '0.0'
autogui.default_value['unsigned int'] = '0'
autogui.default_value['int']          = '0'
autogui.default_value['OGF::index_t'] = '0'
autogui.default_value['GEO::index_t'] = '0'

-- \brief Tests whether a parameter needs quotes
-- \details This is used to make the history look better (would work with
--  quotes everywhere)

function autogui.ith_arg_needs_quotes(mmethod, i)
   return (autogui.default_value[mmethod.ith_arg_type_name(i)] == nil)
end

-- \brief Initializes a LUA table with the default values of the parameters
--  of a meta-method
-- \param[in] mmethod the meta-method
-- \return args a table indexed by argument names and initialized with the
--  default values.

function autogui.init_args(mmethod)
   local args = {}
   for i=0,mmethod.nb_args()-1 do
      local val
      if mmethod.ith_arg_has_default_value(i) then
         val = mmethod.ith_arg_default_value_as_string(i)
      else
         mtype_name = mmethod.ith_arg_type_name(i)
         val = autogui.default_value[mtype_name]
	 if val == nil then
	    mtype = gom.resolve_meta_type(mtype_name)
	    if mtype.meta_class.name == 'OGF::MetaEnum' then
	       val = mtype.ith_name(0)
	    else
	       val = ''
	    end   
	 end
      end
      args[mmethod.ith_arg_name(i)] = val
   end
   return args
end

-- \brief Converts a table with name-argument pairs into a string that can 
--  be sent to the LUA interpreter
-- \param[in] mmethod the meta-method
-- \param[in] args the table with the name-argument pairs
-- \return the table as a string

function autogui.args_to_string(mmethod,args)
   local result = '{'
   for i=0,mmethod.nb_args()-1 do
      local name=mmethod.ith_arg_name(i)
      local value=tostring(args[name])
      if #result > 1 then
         result = result .. ','
      end
      if autogui.ith_arg_needs_quotes(mmethod,i) then
         result = result .. name .. '=' .. '\'' .. value .. '\''
      else
         result = result .. name .. '=' .. value 
      end
   end
   if #result > 1 then
       result = result .. ','
   end
   result = result .. '_invoked_from_gui=true'
   result = result .. '}'
   return result
end


-- \brief Programmatically open a dialog for a command
-- \details object is just used as a key to keep track of the latest
--   opened command (it is not necessarily the target of the invokation)
-- \param[in] object the object
-- \param[in] mmethod the meta-method that corresponds to the command
-- \param[in] object_as_string a string with a way to retrieve the object,
--   that will be passed to the LUA interpreter
-- \param[in] args optional default values for the arguments

function autogui.open_command_dialog(object,mmethod,object_as_string,args)
  local k = object.name..':'..object.meta_class.name..':'..mmethod.name
  if autogui.command_state[k] == nil then
     autogui.command_state[k] = autogui.init_args(mmethod)
     autogui.command_state[k].object_name_ = object.name
     autogui.command_state[k].object_ = object
     autogui.command_state[k].object_as_string_ = object_as_string
     autogui.command_state[k].mmethod_ = mmethod
     autogui.command_state[k].target_ = object.name
     autogui.command_state[k].show_as_window_ = true
     autogui.command_state[k].width_  = 250*main.scaling()
     autogui.command_state[k].height_ = 250*main.scaling()
     autogui.command_state[k].x_ = 400
     autogui.command_state[k].y_ = 200
     if args ~= nil then
        for ak,av in pairs(args) do
	   autogui.command_state[k][ak] = av
	end
     end
  end
end

-- \brief Programmatically open a dialog for a command that applies to
--  the current object.
-- \param[in] cmdclass the classname of the command ('OGF::MeshGrobxxxCommands')
-- \param[in] cmdname the function name of the command
-- \param[in] args optional default values for the arguments

function autogui.open_command_dialog_for_current_object(cmdclass, cmdname, args)
   local o = scene_graph.current()
   local mmethod = gom.resolve_meta_type(cmdclass).find_method(cmdname)
   local o_as_string =
      'scene_graph.resolve(\'' ..
      o.name ..
      '\').query_interface(\'' ..
      cmdclass ..
      '\')'
   autogui.open_command_dialog(o,mmethod,o_as_string,args)
end

-- \brief Handles the dialog for an object command
-- \details object is just used as a key to keep track of the latest
--   opened command (it is not necessarily the target of the invokation)
-- \param[in] object the object
-- \param[in] mmethod the meta-method that corresponds to the command
-- \param[in] object_as_string a string with a way to retrieve the object,
--   that will be passed to the LUA interpreter

function autogui.command_dialog(object,mmethod,object_as_string)

  local k = object.name..':'..object.meta_class.name..':'..mmethod.name
  if autogui.command_state[k] == nil then
     autogui.command_state[k] = autogui.init_args(mmethod)
     autogui.command_state[k].object_name_ = object.name
     autogui.command_state[k].object_ = object
     autogui.command_state[k].object_as_string_ = object_as_string
     autogui.command_state[k].mmethod_ = mmethod
     autogui.command_state[k].target_ = object.name
     autogui.command_state[k].grob = object
  end

  if autogui.command_state[k].show_as_window_ then
     imgui.Text(
        'Target: ' ..
	autogui.remove_underscores(autogui.command_state[k].object_name_)
     )
  elseif imgui.MenuItem(
     imgui.font_icon('code-branch')..'  '..	 
     autogui.remove_underscores(mmethod.name)..'##command_apply'
  ) then
     autogui.command_state[k].show_as_window_ = true
  end

  autogui.tooltip(autogui.help(mmethod))
  imgui.Separator()

  local has_advanced_args = false
  
  for i=0,mmethod.nb_args()-1 do
     if mmethod.ith_arg_has_custom_attribute(i,'advanced') and
        mmethod.ith_arg_custom_attribute_value(i,'advanced') == 'true' then
	has_advanced_args = true
     end	 
  end

  for i=0,mmethod.nb_args()-1 do
     if (not has_advanced_args or 
         not mmethod.ith_arg_has_custom_attribute(i,'advanced') or
	 mmethod.ith_arg_custom_attribute_value(i,'advanced') == 'false'	
     ) then 
        autogui.slot_arg(autogui.command_state[k],mmethod,i)
     end	
  end

  if has_advanced_args then
     if imgui.TreeNode(
        'Advanced'..'##'..object_as_string..'.'..mmethod.name
     ) then
        imgui.TreePop()
	for i=0,mmethod.nb_args()-1 do	
	   if (mmethod.ith_arg_has_custom_attribute(i,'advanced') and
      	       mmethod.ith_arg_custom_attribute_value(i,'advanced') == 'true'	
	   ) then
	       autogui.slot_arg(autogui.command_state[k],mmethod,i)	
	   end
	 end  
     end
  end

  imgui.Separator()
  if imgui.Button(
     imgui.font_icon('cog')
  ) then
      local bkp1 = autogui.command_state[k].show_as_window_
      local bkp2 = autogui.command_state[k].object_name_
      autogui.command_state[k] = autogui.init_args(mmethod)

      autogui.command_state[k].show_as_window_ = bkp1
      autogui.command_state[k].object_name_ = bkp2      

      autogui.command_state[k].object_ = object
      autogui.command_state[k].object_as_string_ = object_as_string
      autogui.command_state[k].mmethod_ = mmethod
  end
  autogui.tooltip('reset factory settings')
  imgui.SameLine()

  local doit_recycle_button = false
  if not autogui.command_state[k].show_as_window_ then
     doit_recycle_button = imgui.Button(imgui.font_icon('sync-alt'))
     autogui.tooltip('apply command without closing menu')
     imgui.SameLine()
  end

  local doit_apply_button = imgui.Button(
     imgui.font_icon('check')..'##commands',-autogui.margin,0
  )
  autogui.tooltip('apply command')

  autogui.command_state[k].width_,
  autogui.command_state[k].height_ = imgui.GetWindowSize()

  --   Increase height a little bit so that resizing corner
  -- does not 'eat' too much and scrollbar does not appear. 
  autogui.command_state[k].height_ = autogui.command_state[k].height_ + 5

  autogui.command_state[k].x_,
  autogui.command_state[k].y_ = imgui.GetWindowPos()
  
  if doit_apply_button or doit_recycle_button then
      local args_string = autogui.args_to_string(
         mmethod,autogui.command_state[k]
      )
      -- If the 'apply do not close' button was pushed, or if the command
      -- is displayed in a separate window, we directly call the LUA function
      -- (this does not close the menu)
      if doit_recycle_button then
         main.exec_command_now(
   	    object_as_string..'.'..mmethod.name..'('..args_string..')', false
         )
      -- else the command is in the menu and the apply button was pushed,
      -- and we queue the LUA command, this allows the command to display
      -- progress bars
      else
         main.exec_command(
   	    object_as_string..'.'..mmethod.name..'('..args_string..')', false
         )
	 -- Close the menu (if we were in menu mode).
	 if not autogui.command_state[k].show_as_window_ then
  	    imgui.CloseCurrentPopup()
	 end   
      end	 
  end
end

-- Draws all the command dialogs that
-- have been switched to window mode.

function autogui.command_dialogs()

   for k,v in pairs(autogui.command_state) do

      -- Hide window / delete command state if target object was
      -- deleted in the meanwhile.

      if autogui.command_state[k].object_ ~= scene_graph 
         and not scene_graph.is_bound(autogui.command_state[k].object_name_)
      then
         autogui.command_state[k] = nil
      elseif autogui.command_state[k].show_as_window_ then
         local sel

	 imgui.SetNextWindowSize(
	    autogui.command_state[k].width_  +  4.0*main.scaling(),
	    autogui.command_state[k].height_ + 20.0*main.scaling(),
	    ImGuiCond_Appearing
	 )

	 imgui.SetNextWindowPos(
	    autogui.command_state[k].x_,
	    autogui.command_state[k].y_,
	    ImGuiCond_Appearing
	 )

         sel,autogui.command_state[k].show_as_window_ = imgui.Begin(
	     imgui.font_icon('code-branch')..'  '..
	     autogui.remove_underscores(
	        autogui.command_state[k].mmethod_.name
	     ) .. '##dialog##' .. k,
             autogui.command_state[k].show_as_window_
         )

         autogui.command_dialog(
	   autogui.command_state[k].object_,
	   autogui.command_state[k].mmethod_,
	   autogui.command_state[k].object_as_string_
	 )
         imgui.End()
      end
   end
end

-- \brief Handles the menu item for an object command
-- \details object is just used as a key to keep track of the latest
--   opened command (it is not necessarily the target of the invokation)
-- \param[in] object the object
-- \param[in] mmethod the meta-method that corresponds to the command
-- \param[in] object_as_string a string with a way to retrieve the object,
--   that will be passed to the LUA interpreter

function autogui.command_menu_item(object,mmethod,object_as_string)
   if mmethod.nb_args() == 0 then 
         -- Need to add '##' suffix with slot name with underscores
         -- else ImGui generates a shortcut I think (to be understood)
      if imgui.MenuItem(
          autogui.remove_underscores(mmethod.name)..'##'..
          mmethod.name
      ) then
         main.exec_command(
             object_as_string..'.'..mmethod.name..
             '({_invoked_from_gui=true})', false
          )
      end
      autogui.tooltip(autogui.help(mmethod))
   else
      if imgui.BeginMenu(
         autogui.remove_underscores(mmethod.name)
      ) then
          autogui.in_popup = true
          autogui.command_dialog(object, mmethod, object_as_string)
	  autogui.in_popup = false
          imgui.EndMenu()
      end
   end
end

-- \brief Tests whether an object member should be displayed in the GUI
-- \param[in] object the object
-- \param[in] mmeber the meta-member
-- \retval true if the member should be displayed
-- \retval false otherwise

function autogui.member_is_visible(object, mmember)
  -- Ignore members of the base classes
  if (
      mmember.container_meta_class().name == 'OGF::Object' or
      mmember.container_meta_class().name == 'OGF::Node'
  ) then
    return false
  end

  -- Ignore readonly properties
  if (
      mmember.meta_class.name == 'OGF::MetaProperty' and
      mmember.read_only()
  ) then
    return false
  end

  -- Take 'visible_if' custom attribute into account
  if object ~= nil then
     for i=0,mmember.nb_custom_attributes()-1 do
        -- Evaluate 'visible_if' custom attribute using object
	-- as context (Lua is so cool !!)
        if(
            mmember.ith_custom_attribute_name(i) == 'visible_if' and
            not load(
	       'return '..mmember.ith_custom_attribute_value(i),
	       'gom_attribute: visible_if', 'bt', object)()
        ) then
           return false
        end
        if(
            mmember.ith_custom_attribute_name(i) == 'visible' and
	    mmember.ith_custom_attribute_value(i) == 'false'
        ) then
           return false
        end
     end
  end
  
  return true
end

-- \brief The properties editor
-- \param[in,out] object the object to be edited

function autogui.properties_editor_properties(object)
   local mclass = object.meta_class
   for i=0,mclass.nb_properties()-1 do
      local mproperty = mclass.ith_property(i)
      -- If property is an Object and has the 'aggregate_properties'
      -- custom attribute, then edit its properties as well (recursive
      -- call).
      if mproperty.has_custom_attribute('aggregate_properties') and
         object[mproperty.name] ~= nil then
         autogui.properties_editor_properties(object[mproperty.name])
      elseif autogui.member_is_visible(object, mproperty) then 
         autogui.property(object,mproperty)
      end	 
   end
end

-- \brief Generates pushbuttons for slots with no
--  arguments.
-- \details Used for instance by properties 'autorange'

function autogui.properties_editor_functions(object)
   local mclass = object.meta_class
   for i=0,mclass.nb_slots()-1 do
      local mslot = mclass.ith_slot(i)
      if (
          autogui.member_is_visible(object, mslot) and
          mslot.nb_args() == 0 and
	  mslot.return_type_name() == 'void'
      ) then
         if imgui.Button(autogui.remove_underscores(mslot.name), -1, 0) then
            object[mslot.name]()
	 end
	 autogui.tooltip(autogui.help(mslot))
      end	 
   end
end


-- \brief Translates a shader user name to a shader type name
-- \param[in] grob_classname the classname of the Grob the shader
--  is attached to
-- \param[in] user_shader_name the user shader name
-- \return the classname of the shader

function shader_user_to_internal(grob_classname, user_shader_name)
   return 'OGF::'..user_shader_name..string.
      strip_prefix(grob_classname,'OGF::')..'Shader'
end

-- \brief Translates a shader classname to a user shader name
-- \param[in] grob_classname the classname of the Grob the shader
--  is attached to
-- \param[in] shader_classname the class name of the shader
-- \return the user name of the shader

function shader_internal_to_user(grob_classname, shader_classname)
   local result = string.strip_suffix(
      shader_classname,
      string.strip_prefix(grob_classname, 'OGF::')..'Shader'
   )
   result = string.strip_prefix(result,'OGF::')
   return result
end

-- \brief Handles the GUI for selecting a shader of an object
-- \param[in] grob the object

function shader_selector(grob)
    local xxx = {} -- we need a []-able object
    xxx.shader = shader_internal_to_user(
         grob.meta_class.name,
         grob.shader.meta_class.name
    )
    xxx.old_shader = xxx.shader
    autogui.combo_box(
      xxx,
      'shader',
      gom.get_environment_value(
           grob.meta_class.name .. "_shaders"
      ),
      tooltip
    )
    if xxx.shader ~= xxx.old_shader then
       local bkp = scene_graph.current_object
       scene_graph.current_object = grob.name
       scene_graph.scene_graph_shader_manager.shader(xxx.shader)
       scene_graph.current_object = bkp
    end
end

-- \brief Keeps the window state of the properties editors
-- \details Property editor state is indexed by 'instance_name.classname'
--  strings.

autogui.property_editor_state  = {}

-- \brief Handles the GUI for an object's property editor
-- \param[in] object the object to be edited. For Graphite objects,
--  pass the associated shader here.
-- \param[in] called_from_inspect true if called from autogui.inspect()
-- \param[in] no_windowify if set, do not show the button to windowify the
--  dialog.
-- \details This function is used to handle both menus and dialogs. It needs
--  to be called inside an opened menu or dialog. It memorizes the menu or
--  dialog state. In menu mode, it generates the button to switch to dialog
--  mode.

function autogui.properties_editor(object,called_from_inspect,no_windowify)
   local name = string.strip_prefix(object.meta_class.name,'OGF::')
   local is_shader = not called_from_inspect and
                     object.meta_class.is_a(
		        gom.resolve_meta_type('OGF::Shader')
		     )
   local k = object.string_id
   if is_shader then
      k = 'SHADER' .. k
      name = object.grob.name      
   end

   if autogui.property_editor_state[k] == nil then
      autogui.property_editor_state[k] = {}
      autogui.property_editor_state[k].object_ = object
      autogui.property_editor_state[k].name_ = name
      autogui.property_editor_state[k].show_as_window_ = called_from_inspect
      autogui.property_editor_state[k].object_id_ = object.string_id      
   end

   if called_from_inspect then
      autogui.property_editor_state[k].inspect_ = true
      autogui.property_editor_state[k].show_as_window_ = true            
      return
   end
   
   imgui.PushID(k)

   if not no_windowify and
      not autogui.property_editor_state[k].show_as_window_ then
      if imgui.MenuItem(
         imgui.font_icon('edit')..'  ' .. name .. ' properties'
      ) then
         autogui.property_editor_state[k].show_as_window_ = true      
      end
      imgui.Separator()
   end

   if is_shader then
       local grob = object.grob
       if gom.get_environment_value('gui:shaders_selector') == 'true' then
          shader_selector(grob)
       end	  
       object = grob.shader
       autogui.property_editor_state[k].object = object
       autogui.properties_editor_functions(object)
       imgui.Separator()		 
   end
   autogui.properties_editor_properties(object)
   if is_shader then
      imgui.Separator()
      if imgui.Button(
        imgui.font_icon('check')..      
        "  Apply to all".."##ops##"..name,-1,autogui.icon_size()+6
      ) then
         local current_bkp = scene_graph.current_object
         scene_graph.current_object = name
         scene_graph.scene_graph_shader_manager.apply_to_scene_graph()
         scene_graph.current_object = current_bkp
      end
      autogui.tooltip('Applies graphic attributes to all objects')      
   end   
   
   autogui.property_editor_state[k].width_,
   autogui.property_editor_state[k].height_ = imgui.GetWindowSize()

   autogui.property_editor_state[k].x_,
   autogui.property_editor_state[k].y_ = imgui.GetWindowPos()
   
   imgui.PopID(k)
end

-- \brief Draws all the windowed property editors.
-- \details This function needs to be called after the menu
--  handler, to draw all the property editors that were switched
--  to dialog mode by the user.

function autogui.property_editors()

   for k,v in pairs(autogui.property_editor_state) do

      -- Hide window / delete command state if target object was
      -- deleted meanwhile. Special version for shaders

      if not v.inspect_ and
         v.object_.meta_class.is_a(gom.resolve_meta_type('OGF::Shader')) and
         not scene_graph.is_bound(v.name_)
      then
         autogui.property_editor_state[k] = nil
         return
      end

      -- Hide window / delete command state if target object was
      -- deleted meanwhile. General version (but it does not work,
      -- to be understood). I need to debug garbage collection of
      -- LUA/GOM objects...

      if gom.resolve_object(
         autogui.property_editor_state[k].object_id_,true
      ) == nil then
         autogui.property_editor_state[k] = nil
	 return
      end

      if v.show_as_window_ then
         local sel

         if not v.inspect_ then
   	    imgui.SetNextWindowSize(
	        v.width_  + 4.0*main.scaling(),
	        v.height_ + 2.0*main.scaling(),
	        ImGuiCond_Appearing
	    )

	    imgui.SetNextWindowPos(
	        v.x_,
	        v.y_,
	        ImGuiCond_Appearing
	    )
	 end   

         sel,v.show_as_window_ = imgui.Begin(
	     imgui.font_icon('edit')..'  '..
	     v.name_ .. " properties",
             v.show_as_window_
         )
         autogui.properties_editor(
	   v.object_
	 )
         imgui.End()
      end
   end
end

-- Creates a graphical object inspector for
--   a given Graphite object.
-- Note: use with care, you are on your own
--  (if the object is destroyed in the meanwhile,
--   it crashes !)
function autogui.inspect(object)
   if object.meta_class == nil then
      print('Object '..tostring(object)..' is not a Graphite object')
   end
   autogui.properties_editor(object, true)
end
