# To be run outside Graphite (after gompy is installed)

import gompy, gompy.types.OGF as OGF

try:
    gompy.interpreter().out("Hello, world !")
    S = OGF.MeshGrob()
    S.I.Shapes.create_sphere()
    R = S.I.Surface.remesh_smooth()
    R.I.Mesh.display_statistics()
    R.save('remesh.obj')
except Exception as e:
    gompy.interpreter().err(str(e))

gompy.interpreter().out("Goodbye, cruel world...")
