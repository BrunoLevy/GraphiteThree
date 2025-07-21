# To be run outside Graphite (after gompy is installed)

import gompy.types.OGF as OGF

S = OGF.MeshGrob()
S.I.Shapes.create_sphere()
R = S.I.Surface.remesh_smooth()
R.I.Mesh.display_statistics()
R.save('remesh.obj')
