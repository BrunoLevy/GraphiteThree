#include <OGF/WarpDrive/IO/xyzw_serializer.h>
#include <geogram/basic/line_stream.h>

namespace OGF {
   
   bool XYZWSerializer::load(
      const std::string& filename, Mesh& M,
      const MeshIOFlags& ioflags 
   ) {

      LineInput in(filename);
      if(!in.OK()) {
	 Logger::err("I/O") << "Could not open " << filename << std::endl;
	 return false;
      }
      
      M.clear();
      Attribute<double> mass(M.vertices.attributes(),"mass");
      
      while(in.get_line()) {
	 in.get_fields();
	 double x = in.field_as_double(0);
	 double y = in.field_as_double(1);
	 double z = in.field_as_double(2);
	 double w = in.field_as_double(3);
	 index_t v = M.vertices.create_vertex(vec3(x,y,z).data());
	 mass[v] = w;
      }
      
      geo_argused(ioflags);      
      return true;
   }
   

   bool XYZWSerializer::save(
       const Mesh& M, const std::string& filename,
       const MeshIOFlags& ioflags
   ) {
      geo_argused(M);
      geo_argused(filename);
      geo_argused(ioflags);
      return false;
   }
   
}
