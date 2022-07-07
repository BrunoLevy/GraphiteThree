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
	 if(in.nb_fields() == 1) {
	     continue;
	 }
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
       geo_argused(ioflags);

       std::cerr << "============== SAVE" << std::endl;

       
       Attribute<double> mass;
       mass.bind_if_is_defined(M.vertices.attributes(),"mass");
       if(!mass.is_bound()) {
	   Logger::err("XYZWSerializer") << "Missing mass vertex attribute"
					 << std::endl;

	   return false;
       }

       FILE* f = fopen(filename.c_str(),"w");
       if(!f) {
	   Logger::err("XYZWSerializer") << filename
					 << ": could not create"
					 << std::endl;
	   return false;
       }
       fprintf(f,"%d\n",int(M.vertices.nb()));
       for(index_t v: M.vertices) {
	   vec3 p(M.vertices.point_ptr(v));
	   fprintf(
	       f,"%.17g %.17g %.17g %.17g\n",
	       p.x, p.y, p.z, mass[v]
	   );
       }
       fclose(f);
       
       return true;
   }
   
}
