-- ---------------------------------------------------------------------------
-- \file autogui_handlers.lua
-- \brief Low-level implementation for autogui commands and properties.
--  Creates handlers, that is, graphic editors, for command arguments
--  and properties based on their types.
-- \details Each handler is a function with the following prototype:
--   handler(object, property_name, mtype, tooltip) where
--   - object is the object beeing edited
--   - property_name is a string with the name of the property beeing edited
--   - mtype is the GOM meta-type of the property. It is used by enums (that
--     need it to query the list of names)
--   - tooltip is an optional tooltip
-- ---------------------------------------------------------------------------

-- ---------------------------------------------------------------------------
-- User API (used by autogui_commands.lua and autogui_properties.lua)
-- ---------------------------------------------------------------------------

-- \brief Gets a handler by meta-type
-- \param[in] mtype the meta type, for instance OGF.MeshGrobName
-- \return the handler (see comment at beginning of file)

function autogui.handler_by_meta_type(mtype)
   if mtype.is_a(OGF.MetaEnum) then
      return autogui.enum
   end
   return autogui.handler_by_name(mtype.name)
end

-- \brief Gets a handler by name
-- \details Used for instance when handler is explicitly specified in
--   a C++ header through a gom attribute.
-- \param[in] name the name of the handler, can be either
--   - a function name, for instance 'combo_box'
--   - or a type name, for instance 'OGF::MeshGrobName'
-- \return the handler (see comment at beginning of file)

function autogui.handler_by_name(name)
   local handler = autogui.handlers[name]
   if handler == nil then
      -- special case for OGF::FileName and OGF::NewFileName
      if string.ends_with(name, 'FileName') then
	  if string.starts_with(name, 'OGF::New') then
	      handler = autogui.new_file_name
	  else
	      handler = autogui.file_name
	  end
      else
          handler = autogui.string -- Fallback: editor for strings
      end
   end
   return handler
end

-- -------------------------------------------------------------------

-- \brief The table of handlers indexed by type names
-- \details Used internally

autogui.handlers = {}

-- -------------------------------------------------------------------

-- \brief Flags to be used for imgui.TextInput
-- \details When editing properties, need to validate with enter else
--   transient values will be sent (not good)

autogui.input_text_flags = 0

-- \brief Margin to avoid the "auto-growing" effect on some dialogs

autogui.margin = 10

-- \brief true when the current dialog created by autogui is in a popup.
-- \details it is required to avoid creating FileDialogs from popups (which
-- closes the dialog unexpectingly). TODO: find a better fix.

autogui.in_popup = false

-- \brief true when the current dialog created by autogui is in a tree
-- \details used to toggle a more compact packing of GUI elements

autogui.in_tree = false

-- \brief default button size (for little square buttons for tools)

autogui.button_size = 25 * main.scaling()

-- ------------------------------------------------------------------------------
-- Utilities
-- ------------------------------------------------------------------------------

-- \brief Replaces the underscores in a string with spaces
-- \param[in] name the string
-- \return the string with the underscores replaced by spaces

function autogui.remove_underscores(name)
  return name:gsub('_',' ')
end


-- \brief Transorms a string into a string suitable to be displayed to user
-- \param[in] name the string
-- \return the string suitable to be displayed to user

function autogui.to_display_string(name)
   -- special case for mesh_style, surface_style etc... (remove '_style')
   if autogui.in_tree and name:ends_with('_style') then
      name = name:remove_suffix('_style')
   end
  if name:ends_with('_object') then
      name = name:remove_suffix('_object')
  end
  return autogui.remove_underscores(name)
end

-- \brief Wrapper arround imgui.Text() that removes the underscores

function autogui.Text(name)
   imgui.Text(autogui.to_display_string(name))
   if autogui.in_tree then
      imgui.SameLine()
   end
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

-- ------------------------------------------------------------------------------
-- Additional widgets
-- ------------------------------------------------------------------------------

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

   return sel
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

   imgui.PushItemWidth(-1-autogui.icon_size()-autogui.margin*4)

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
   if imgui.Button(
      imgui.font_icon('caret-down').. '##properties##btn_'..property_name
   ) then
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

-- ------------------------------------------------------------------------------
-- All the handlers
-- ------------------------------------------------------------------------------

-- \brief Handler for enums
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

function autogui.enum(object, property_name, menum, tooltip)
   if menum == nil then
       autogui.string(object, property_name, menum, tooltip)
       return
   end
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
-- \param[in] object, property_name, metype, tooltip handler parameters
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
  {'grayscale', true},
  {'misc', false},
  {'rainbow2', false},
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

-- \brief Handler for OGF::GrobName
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::GrobName'] = function(
   object, property_name, mtype, tooltip
)
   local values = gom.get_environment_value('grob_instances')
   autogui.combo_box(object, property_name, values, tooltip)
end

-- \brief Handler for OGF::NewGrobName
-- \param[in] object, property_name, menum, tooltip handler parameters
-- \see autogui.handlers

autogui.handlers['OGF::NewGrobName'] = function(
   object, property_name, mtype, tooltip
)
   local values = gom.get_environment_value('grob_instances')
   autogui.editable_combo_box(object, property_name, values, tooltip)
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
   local context = object -- the context for magic '$' evaluation

   if object.meta_class ~= nil then
      -- object is a GOM object (Shader).
      -- Retrieve 'values' custom attribute of the property.
      values_custom_attribute = object.meta_class.
            find_property(property_name).custom_attribute_value('values')
   else
      -- object is a command state dictionnary.
      -- Retrieve 'values' custom attribute of the arg in the meta-method.
      local mmethod = object.request_.method()
      for i = 0,mmethod.nb_args()-1 do
         if mmethod.ith_arg_name(i) == property_name then
	     values_custom_attribute =
	         mmethod.ith_arg_custom_attribute_value(i,'values')
	 end
      end
      context = object.request_.object() -- For '$' eval, OGF::Commands instance
   end

   local values = values_custom_attribute

   -- if values start with a '$' sign, execute the content
   -- with 'context' as the context to get the actual list of
   -- values (used for instance to generate list of attributes
   -- from "$grob.attributes"). In this case 'grob' is obtained
   -- through 'context.grob' (where 'context' is a OGF::Commands instance)
   if values:sub(1,1) == '$' then
     values = values:sub(2,values:len())
     -- evaluate 'values' using context (Lua is so cool !)
     values = load(
        'return '..values,       -- code to be evaluated
	'gom_attribute: values', -- tag for error messages
        'bt',                    -- both binary and text
	context                  -- environment
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
