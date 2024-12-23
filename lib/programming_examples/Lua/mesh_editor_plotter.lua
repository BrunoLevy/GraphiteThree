-- Lua (Keep this comment, this is an indicator for editor's 'run' command)

-- An example that shows how to create a mesh
-- programatically by plotting mathematical functions.

-- \brief Maps a grid to the graph of a sine product
-- \param[in] u , v grid coordinates, in [0,1]
-- \return the x , y , z mapping of the grid
function sineprod(u,v)
   local x = u
   local y = v
   local z = 0.2*math.sin(u*10)*math.sin(v*10)
   return x,y,z
end

-- \brief Maps a grid to a sphere
-- \param[in] u , v grid coordinates, in [0,1]
-- \return the x , y , z mapping of the grid
function sphere(u,v)
   local theta = u*2.0*math.pi
   local phi = (v-0.5)*math.pi
   local x = math.cos(theta) * math.cos(phi)
   local y = math.sin(theta) * math.cos(phi)
   local z = math.sin(phi)
   return x,y,z
end

-- \brief Creates a mesh from a mapping function
-- \param[in,out] S a MeshGrob
-- \param[in] func the grid mapping function, e.g.
--   one of sineprod, sphere
-- \param[in] NU , NV grid sizes
function plot_func(S, func, NU, NV)
-- uncomment to display the vertices
-- S.shader.vertices_style='true; 0 1 0 1; 1'
  S.shader.mesh_style='true; 0 0 0 1; 1'
  local E = S.I.Editor
  E.clear()
  for V = 0,NV-1 do
    for U = 0,NU-1 do
      local u = U/(NU-1)
      local v = V/(NV-1)
      -- mapping the (u,v) point to (x,y,z) coordinates
      -- using the function passed as parameter: Lua is
      -- really cool ! (note also the multiple return
      -- values, see sphere() and sineprod()
      local x,y,z = func(u,v)
      E.create_vertex({x,y,z})
    end
 end
 for V = 0,NV-2 do
    for U = 0,NU-2 do
      local i00 = V*NU+U
      local i10 = (V+1)*NU+U
      local i01 = V*NU+U+1
      local i11 = (V+1)*NU+U+1
      E.create_quad(i00, i10, i11, i01)
-- Try this: uncomment (funny, but much slower !)
--    S.redraw()
    end
  end
  E.connect_facets()
end

-- Creates the surface if it does not already exist,
-- so that one can modify and run the program multiple
-- times.
S = scene_graph.find_or_create_object(OGF.MeshGrob,'Surface')

plot_func(S, sphere, 20, 20)
--Try this: comment the prev. one and try this one.
--plot_func(S, sineprod, 100, 100)

--Try this: implement your own grid mapping functions
--(torus, Klein bottle, ...)
