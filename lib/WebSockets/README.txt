Experimental system to remote-control Graphite using WebSockets
Note: works under Linux, not tested under Windows.
Required external dependencies: libsoup and lgi:
  - WebSocket protocol is implemented by libsoup
  - It is interfaced with Lua using lgi

Instructions:

1- From the File menu of the console, Load and Run WebSocketsServer.lua
2- Open WebSocketsClient.html in a browser

Notes:

Still experimental. Even if you do nothing, crashes after a while
(after a couple of hours). I think that my event handler is recursive
or something, and causes a stack overflow, to be investigated...
