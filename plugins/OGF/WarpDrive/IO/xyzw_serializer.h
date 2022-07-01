#ifndef H_OGF_WARPDRIVE_IO_XYZW_SERIALIZER_H
#define H_OGF_WARPDRIVE_IO_XYZW_SERIALIZER_H

#include <OGF/WarpDrive/common/common.h>
#include <geogram/mesh/mesh_io.h>

namespace OGF {
   
   class WarpDrive_API XYZWSerializer : public MeshIOHandler {
    public:
        virtual bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags = MeshIOFlags()
        );

        virtual bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags = MeshIOFlags()
        );
   };
   
}


#endif
