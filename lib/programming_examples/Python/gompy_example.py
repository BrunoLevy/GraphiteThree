# To be run outside Graphite (after gompy is installed)

# Try invalidate cache for MacOS
import importlib
importlib.invalidate_caches()

import gompy
OGF = gom.meta_types.OGF

S = OGF.MeshGrob()
S.I.Shapes.create_sphere()
R = S.I.Surface.remesh_smooth()
R.I.Mesh.display_statistics()
R.save('remesh.obj')
