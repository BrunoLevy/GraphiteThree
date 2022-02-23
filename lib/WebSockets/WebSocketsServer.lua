-- Reading notes
--
-- Lua
-- https://developer.gnome.org/libsoup/2.60/
-- https://developer.gnome.org/libsoup/stable/libsoup-server-howto.html
-- https://developer.gnome.org/libsoup/stable/libsoup-2.4-WebSockets.html
-- https://github.com/pavouk/lgi/blob/master/docs/guide.md
-- https://gist.github.com/marmis85/2087e7d0c9cdd2b7512d
-- https://www.xul.fr/html5/websocket.php
--
-- Open in browser:
--   HTML:       http://127.0.0.1:7440
--   WebSockets: http://127.0.0.1:7440/unix
--      (use example in https://www.xul.fr/html5/websocket.php)

--==============================================================================
-- GraphiteServer handles both HTTP and WebSocket connections
--==============================================================================

GraphiteServer = {}
GraphiteServer.debug = true
GraphiteServer.connections = {} -- Keeps refs to active cnx, else they are GCed 
GraphiteServer.port = 7440

--==============================================================================
-- External packages (accessed through lgi: Lua wrapper for gnome/GObject libs)
--==============================================================================

-- ... Debian 10 'Buster' has lgi in 5.3. The LUA bundled with geogram does
-- not find it (so I added the path)
-- (Note: the lgi for LUA 5.2 also works with the LUA 5.3 bundled with geogram,
--  it is slightly modified to make it possible)
if lgi == nil then
  package.path=
        package.path..';/usr/share/lua/5.3/?.lua;/usr/share/lua/5.3/?/init.lua'
  package.cpath=
        package.cpath..';/usr/lib/x86_64-linux-gnu/lua/5.3/?.so'
  lgi = require('lgi')
end 

-- gobject = lgi.GObject (not needed directly)
-- gio     = lgi.Gio     (not needed directly)
soup    = lgi.Soup -- For HTTP and WebSocket protocols
glib    = lgi.GLib -- For main event loops management
require('json')    -- For marshalling / unmarshalling remote calls arguments

--==============================================================================
-- Implementation of the server
--==============================================================================

-- The callback for handling HTTP requests
--
function GraphiteServer.HTTP_callback(
   server, message, path, query, context, data
)
   if GraphiteServer.debug then
      print("===== HTTP REQUEST ====")
      print('method ='..tostring(message.method))
      print('path   ='..tostring(path))
      print('query  ='..tostring(query))
      print('context='..tostring(context))
      print('data   ='..tostring(data))
      print(' --Headers--')
      local iter = soup.MessageHeadersIter.init(message.request_headers)
      local a=1,b
      while a ~= nil do
         a,b = iter:next()
         print('   '..tostring(a)..'='..tostring(b))
      end
   end      
   if message.method == 'GET' then
      local buff = soup.Buffer.new(
         '<H1> Hello from Graphite HTTP server written in LUA !! </H1>'
      )
      message.response_body:append_buffer(buff)
   end
end

function GraphiteServer.HTTP_callback_wrapper(
   server, message, path, query, context, data
)
   local OK, message = pcall(
       GraphiteServer.HTTP_callback,
       server, message, path, query, context, data
   )
   if not OK then
      print('Error in HTTP_callback: '..message)
   end
end

-- The callback for WebSocket messages
--
function GraphiteServer.ws_message_callback(
   connection, type, message, user_data
)
   if GraphiteServer.debug then
     print('==== WEBSOCKET MESSAGE ====')
     print(connection)   
     print(type)
     print(message.data)
     print(user_data)
   end     

   local args = json.decode(message.data)

   if args.action == 'notify' then
      print('Client says:'..args.content)
   elseif args.action == 'execute' then
      main.exec_command(args.content)
      connection:send_text(json.encode({
         action="notify",
         content="Server executed Lua command:"..args.content
      }))
   end

end

function GraphiteServer.ws_message_callback_wrapper(
   connection, type, message, user_data
)
   local OK,message = pcall(
      GraphiteServer.ws_message_callback,
      connection, type, message, user_data
   )
   if not OK then
      print('Error in ws_message_callback: '..message)
   end
end

-- The callback for WebSocket connections
--
function GraphiteServer.ws_callback(
   server, connection, path, client, user_data
)

   if GraphiteServer.debug then
     print('==== WEBSOCKET CONNECTION ====')
     print(connection)
   end
   
   GraphiteServer.connections[connection] = true
   
   connection.on_message = GraphiteServer.ws_message_callback_wrapper
   
   connection.on_closed  = function(connection)
      if GraphiteServer.debug then   
         print('==== WEBSOCKET CONNECTION CLOSED ====')
         print(connection)
      end	  
      GraphiteServer.connections[connection] = nil
   end

   connection.on_error   = function(connection, error)
       print('WebSocketConnection error:')
       print(error.message)
   end

-- connection.on_closing not used
-- connection.on_pong    not used

   connection:send_text(json.encode({
      action="notify",
      content="Server ready."
   }))
   
end

function GraphiteServer.ws_callback_wrapper(
   server, connection, path, client, user_data
)
   local OK,message = pcall(
      GraphiteServer.ws_callback,
      server, connection, path, client, user_data
   )
   if not OK then
      print('Error in ws_callback: '..message)
   end
end

-- Does one round of event handling 
--   Note: Standard way of launching the main server loop 
--     loop = glib.MainLoop.new()
--     loop:run()
--   We do not do that like that because we need to interleave
--     Graphite's main loop and GLib's event loop
--
function GraphiteServer.handle_events()
   glib.MainContext.default():iteration(false)
   -- Re-trigger a graphic update
   scene_graph.update()
end

-- Installs the event handlers and starts the server
function GraphiteServer.start()
   GraphiteServer.server = soup.Server:new()

   GraphiteServer.server:listen_all(GraphiteServer.port,0)
   GraphiteServer.server:add_handler(
      nil, GraphiteServer.HTTP_callback_wrapper, nil, nil
   )
   GraphiteServer.server:add_websocket_handler(
      '/unix', nil, nil, GraphiteServer.ws_callback_wrapper, nil, nil
   )

   -- Display URIs on which the server is listening
   for _,uri in pairs(GraphiteServer.server:get_uris()) do
      print('Server listens: '..uri:to_string())
   end

   -- Connect server handler to graphic updates
   gom.connect(win.redraw_request, GraphiteServer.handle_events)
   -- Trigger first graphic update that will in-turn
   --  trigger event handling (that will re-trigger graphic
   --  update and so on and so forth...)
   scene_graph.update()
end


GraphiteServer.start()

