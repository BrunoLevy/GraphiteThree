# Doxygen documentation

All classes, methods and functions of Graphite and Geogram are documented
with Doxygen.

[Geogram documentation](http://alice.loria.fr/software/geogram/doc/html/index.html)

[Graphite documentation](http://alice.loria.fr/software/graphite/doc/html/index.html)

# Programmer's quick links

### Quick links to main Geogram functions and classes

- Mesh
  - GEO::Mesh 
  - GEO::mesh_load()
  - GEO::mesh_save()
- Geometry processing
  - GEO::compute_normals()
  - GEO::simple_Laplacian_smooth()
  - GEO::get_bbox()
  - GEO::mesh_repair()
  - GEO::remove_degree3_vertices()
  - GEO::remove_small_facets()
  - GEO::remove_small_connected_components()
  - GEO::orient_normals()
  - GEO::expand_border()
  - GEO::get_connected_components()
- Computational geometry
  - GEO::Delaunay, GEO::Delaunay3d
  - GEO::CentroidalVoronoiTesselation
  - GEO::RestrictedVoronoiDiagram
  - GEO::NearestNeighborSearch, GEO::KdTree
  - GEO::MeshFacetsAABB, GEO::MeshCellsAABB

### Quick links to main geogram internal classes

- Numerics
  - Numerical analysis: GEO::Optimizer
  - Computer arithmetics: GEO::expansion, GEO::expansion_nt
- System
  - Processes: GEO::Process, GEO::parallel_for()
  - Command line and logging: GEO::Logger, GEO::CmdLine, GEO::ProgressTask
  - Types: GEO::Memory
  - Files: GEO::FileSystem, GEO::LineInput  
  - Time: GEO::Stopwatch
- Generic internal implementation of geometric algorithms 
  - Implementation of Restricted Voronoi Diagram: GEOGen::RestrictedVoronoiDiagram
  - Implementation of volumetric convex clipping: GEOGen::ConvexCell  
  - Implementation of surfacic convex clipping: GEOGen::Polygon
