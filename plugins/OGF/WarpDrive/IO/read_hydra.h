/*
 * C++ classes for loading Hydra binary files
 * Bruno Levy, March 2024
 */

#ifndef H_READ_HYDRA_H
#define H_READ_HYDRA_H

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>

#ifndef READ_HYDRA_LIB_ONLY
#define WITH_READ_HYDRA
#endif

#ifdef WITH_READ_HYDRA
#define geo_assert(x) assert(x)
#endif

namespace GEO {

    /******************************************************************/
    
    /**
     * \brief Loads data from Fortran files
     * \details Fortran files are organized into records. Each record
     *  corresponds to a WRITE statement in format. Each record starts
     *  and ends with a record marker, that contains the length of the
     *  record, in bytes, encoded as a 32 bits integer.
     */
    class FortranFile {
    public:

        /**
         * \brief FortranFile constructor
         * \param[in] filename the file to be read
         * \details Throws an exception if file cannot be opened
         */
        FortranFile(const std::string &filename) {
            f_ = fopen(filename.c_str(),"rb");
            if(f_ == nullptr) {
                throw(std::logic_error(
                          "FortranFile: could not open " + filename
                ));
            }
            record_size_ = 0;
        }

        /**
         * \brief FortranFile destructor
         */
        ~FortranFile() {
            if(f_ != nullptr) {
                fclose(f_);
                f_ = nullptr;
            }
        }

        /**
         * \brief Forbids copy
         */
        FortranFile(const FortranFile&) = delete;

        /**
         * \brief Forbids copy
         */
        FortranFile& operator=(const FortranFile&) = delete;

        /**
         * \brief Tests whether a record is open
         * \details A record is open if we are between a begin_record()
         *  and end_record() call
         * \retval true if a record is open
         * \retval false otherwise
         */
        bool record_is_open() const {
            return record_size_ != 0;
        }
        
        /**
         * \brief Opens a FORTRAN record, and reads record size.
         * \return the length of the record, in bytes
         * \pre !record_is_open()
         */
        uint32_t begin_record() {
            geo_assert(!record_is_open());
            if(fread(&record_size_, sizeof(uint32_t), 1, f_) != 1) {
                throw(std::logic_error(
                          "FortranFile::begin_record(): fread() error"
                ));
            }
            record_start_pos_ = size_t(ftell(f_));
            return record_size_;
        }

        /**
         * \brief Closes a FORTRAN record
         * \details Tests whether the record markers match. Throws an
         *  exception if it is not the case.
         * \return the length of the record, in bytes
         * \pre record_is_open()
         */
        uint32_t end_record() {
            geo_assert(record_is_open());
            uint32_t check;
            if(fread(&check, sizeof(uint32_t), 1, f_) != 1) {
                throw(std::logic_error(
                          "FortranFile::end_record(): fread() error"
                ));
            }
            if(check != record_size_) {
                throw(
                    std::logic_error(
#ifdef WITH_READ_HYDRA
                        "invalid FORTRAN record size"
#else                        
                        String::format(
                            "invalid FORTRAN record size:expected %d got %d",
                            record_size_, check
                        )
#endif                        
                    )
                );
            }
            record_size_= 0;
            return check;
        }

        /**
         * \brief Skips the contents of the currently open record and
         *  closes it
         * \return the length of the record in bytes
         * \pre record_is_open()
         */
        uint32_t skip_end_record() {
            geo_assert(record_is_open());
            if(
                fseek(
                    f_, (long int)(record_start_pos_+record_size_), SEEK_SET
                ) != 0
            ) {
                throw(std::logic_error(
                          "FortranFile::skip_end_record(): fseek() error"
                ));
            }
            return end_record();
        }

        /**
         * \brief Opens, skips and closes a record.
         * \pre !record_is_open()
         */
        uint32_t skip_record() {
            begin_record();
            return skip_end_record();
        }

        /**
         * \brief Reads data of arbitrary type
         */
        template <class T> void read(T& out) {
            if(fread(&out, sizeof(T), 1, f_) != 1) {
                throw(std::logic_error(
                          "FortranFile::read(): fread() error"
                ));
            }
        }
        
    private:
        FILE* f_;
        uint32_t record_size_;
        size_t record_start_pos_;
    };

    /************************************************************/

    /**
     * Loads binary files generated by the Hydra cosmological simulator
     */
    class HydraFile : public FortranFile {
    public:

        /**
         * \brief HydraFile constructor
         * \parmam[in] filename the name of the file to be loaded
         */
        HydraFile(const std::string& filename) : FortranFile(filename) {
            memset(ver,    0, sizeof(ver));
            memset(&ibuf,  0, sizeof(ibuf));
            memset(&ibuf1, 0, sizeof(ibuf1));
            memset(&ibuf2, 0, sizeof(ibuf2));
        }

        /**
         * \brief Forbids copy
         */
        HydraFile(const HydraFile& rhs) = delete;

        /**
         * \brief Forbids copy
         */
        HydraFile& operator=(const HydraFile& rhs) = delete;

        /**
         * \brief Gets the number of particles
         * \details load_header() needs to be called first
         * \return the total number of particles (DM, gas, ...)
         */
        uint32_t nb_particles() const {
            return uint32_t(ibuf2.d.nobj);
        }

        /**
         * \brief Gets the version of Hydra that generated this file
         * \details load_header() needs to be called first
         * \return the version as a floating-point number
         */
        float version() const {
            return float(ver[0]) + 0.1f * float(ver[1]) + 0.01f * float(ver[2]);
        }

        /**
         * \brief Loads the header of a Hydra file
         * \details This loads the version number, as well as
         *  ibuf, ibuf1 and ibuf2 that contain all the parameters of the
         *  simulation
         */
        void load_header() {
            begin_record();
            read(ver);
            end_record();
            
            begin_record();
            read(ibuf);
            read(ibuf1);
            read(ibuf2);
            end_record();
        }

        /**
         * \brief Skips particle types
         */
        void skip_itype() {
            if(skip_record()/sizeof(uint32_t) != nb_particles()) {
                throw(std::logic_error("Invalid itype size"));
            }
        }

        /**
         * \brief Skips particle masses
         */
        void skip_rm() {
            if(skip_record()/sizeof(float) != nb_particles()) {
                throw(std::logic_error("Invalid rm size"));
            }
        }

        /**
         * \brief Skips particle coordinates
         */
        void skip_r() {
            if(skip_record()/(3*sizeof(float)) != nb_particles()) {
                throw(std::logic_error("Invalid r size"));
            }
        }

        /**
         * \brief Skips particle velocities
         */
        void skip_v() {
            if(skip_record()/(3*sizeof(float)) != nb_particles()) {
                throw(std::logic_error("Invalid v size"));
            }
        }

        /**
         * \brief Saves the floating point array in the next record
         *  to a file
         * \param[in] filename the name of the file, if it ends with
         *  ".bin" then a binary file is saved, else an ascii file
         * \details Throws an exception if the file cannot be saved
         *  or if record size does not fit the legal size for an 
         *  array of floats
         */
        void save_vector_array_record(const std::string& filename) {
            std::cerr << "Saving current record to: " << filename << std::endl;
            uint32_t size = begin_record();
            if(size % 12 != 0) {
                throw(std::logic_error(
                          "record size not multiple of 12 (cannot be vec array)"
                ));
            }
            size /= 12;

            // If filename ends with ".bin" then output a binary file
            bool binary =
                (filename.length() >= 4 &&
                 filename.substr(filename.length()-4) == ".bin");

            bool xyz = 
                (filename.length() >= 4 &&
                 filename.substr(filename.length()-4) == ".xyz");
                
            FILE* out = fopen(filename.c_str(),binary ? "wb" : "w");
            
            if(out == nullptr) {
                throw(std::logic_error(
                          "could not create file " + filename
                ));
            }

            bool OK = true;

            // Write number of points if extension is ".xyz" file
            // (so that Graphite can load it faster)
            if(xyz) {
                OK = OK && (fprintf(out,"%d\n",size) != 0);
            }
            
            for(uint32_t i=0; i<size; ++i) {
                float xyz[3];
                read(xyz);
                if(binary) {
                    OK = OK && (fwrite(xyz, sizeof(xyz), 1, out) == 1);
                } else {
                    OK = OK && (
                        fprintf(
                            out,"%.8g %.8g %.8g\n", xyz[0], xyz[1], xyz[2]
                        ) != 0
                    );
                }

                if(!OK) {
                    throw(std::logic_error("Error while writing file"));
                }
            }
            
            fclose(out);
            
            end_record();
        }
        
    public:
        int32_t ver[3];
        struct {
            union {
                int32_t I[100];
                struct {
                    int32_t itime;  /**< timestep */
                    int32_t itstop; /**< timestep to stop at */
                    int32_t itdump; /**< dump every itdump timesteps */
                    int32_t itout;  /**< next timestep to output results at */
                    float time;     /**< simulation time */
                    float atime;    /**< expansion factor */
                    float htime;    /**< Hubble parameter */
                    float dtime;    /**< current timestep */
                    float Est;      /**< start energy */
                    float T;        /**< kinetic energy */
                    float Th;       /**< thermal energy */
                    float U;        /**< potential energy */
                    float Radiation;/**< radiated energy */
                    float Esum;     /**< energy integral */
                    float Rsum;     /**< total radiated energy */
                    float cputo;    /**< cpu time */
                    float tstop;    /**< time to stop at */
                    float tout;     /**< next time to output result at */
                    int32_t icdump; /**< index of list of req output t, tout1 */
                    int32_t pad1;   /**< padding */
                    float Tlost;    /**< kinetic energy of escaping particles */
                    float Qlost;    /**< thermal energy of escaping particles */
                    float Ulost;    /**< pot     energy of escaping particles */
                    int32_t pad2;   /**< padding */
                    float soft2;    /**< current Plummer softening squared */
                    float dta;      /**< minimum timestepfrom acceleration */
                    float dtcs;     /**< minimum timestep from sound speed */
                    float dtv;      /**< minimum timestep from velocity */
                } d;
            };
        } ibuf1;
        
        struct {
            union {
                int32_t I[100];
                struct {
                    int32_t irun;   /**< run number */
                    int32_t nobj;   /**< total number of particles */
                    int32_t ngas;   /**< nbr of gas prtcles (MUST BE FIRST) */
                    int32_t ndark;  /**< number of dark matter particles */
                    int32_t intl;   /**< must be 0 (1 for interlacing) */
                    int32_t nlmx;   /**< maximum number of refinement levels */
                    float perr;     /**< percent err for gravy, use 7.7 or 2.0 */
                    float dtnorm;   /**< mult for tstep, 
                                         use 1 unless particles go haywire */
                    float sft0;     /**< z=0 softening */
                    float sftmin;   /**< minimum softening */
                    float sftmax;   /**< maximum softening */
                    int32_t pad3;   /**< padding */
                    float h100;     /**< Hubble param in units of 100 km/s/Mpc */
                    float box100;   /**< z=0 boxsize in Mpc/h100 */
                    float zmet0;    /**< metal. of gas rel. to solar, if const */
                    float lcool;    /**< cooling switch (1=on) */
                    float rmbary;   /**< baryonic particle mass (grid units) */
                    float rmnorm0;  /**< intermediate in norm of gravity force */
                    int32_t pad4;   /**< padding */
                    int32_t pad5;   /**< padding */
                    float tstart;   /**< time of initial conditions */
                    float omega0;   /**< z=0 val of omega */
                    float xlambda0; /**< z=0 val of lambda */
                    float h0t0;     /**< z=0 val of Hubble param x age of Univ */
                    float rcen[3];  /**< middle of box in input co-ordinates */
                    float rmax2;    /**< sq of max dist from ctr in isol sims */
                } d;
            };
        } ibuf2;
        
        
        struct {
            union {
                int32_t I[200];
                struct {
                    float tout1[50]; /**< list of desired output times */
                } d;
            };
        } ibuf;
    };
    
    /************************************************************/
}

#ifdef WITH_READ_HYDRA

void usage() {
    std::cerr << "usage: read_hydra <-pos pos_file> <-velo velo_file> inputfile"
              << std::endl;
    exit(1);
}

int main(int argc, char** argv) {
    int i = 1;
    std::string input;
    std::string output_pos;
    std::string output_velo;
    
    while(i < argc) {
        std::string cur_arg = argv[i];
        ++i;
        if(cur_arg[0] == '-') {
            if(i >= argc) {
                std::cerr << "missing file name for " << cur_arg
                          << std::endl;
                usage();
            }
            if(cur_arg == "-pos") {
                output_pos = argv[i];
            } else if(cur_arg == "-velo") {
                output_velo = argv[i];
            } else {
                std::cerr << "invalid arg " << cur_arg
                          << std::endl;
                usage();
            }
            ++i;
        } else {
            if(input != "") {
                std::cerr << argv[0] << ": more than 1 input file specified"
                          << std::endl;
                usage();
            }
            input = cur_arg;
        }
    }

    if(input == "") {
        std::cerr << argv[0] << ": missing input file"
                  << std::endl;
        usage();
    }

    try {
        std::cerr << "Loading " << input << std::endl;
        std::cerr << "Loading header..." << std::endl;
        GEO::HydraFile in(input);
        in.load_header();
        fprintf(stderr,"Hydra version %f\n",in.version())      ;
        fprintf(stderr," |- irun     =%d\n",in.ibuf2.d.irun)     ;
        fprintf(stderr," |- nobj     =%d\n",in.ibuf2.d.nobj)     ;
        fprintf(stderr," |- ngas     =%d\n",in.ibuf2.d.ngas)     ;
        fprintf(stderr," |- ndark    =%d\n",in.ibuf2.d.ndark)    ;
        fprintf(stderr," |- h100     =%f\n",in.ibuf2.d.h100)     ;
        fprintf(stderr," |- box100   =%f\n",in.ibuf2.d.box100)   ;
        fprintf(stderr," |- tstart   =%f\n",in.ibuf2.d.tstart)   ;
        fprintf(stderr," |- omega0   =%f\n",in.ibuf2.d.omega0)   ;
        fprintf(stderr," |- xlambda0 =%f\n",in.ibuf2.d.xlambda0) ;
        fprintf(stderr," |- h0t0     =%f\n",in.ibuf2.d.h0t0)     ;
        fprintf(stderr," |- itime    =%d\n",in.ibuf1.d.itime)  ;
        fprintf(stderr," |- itstop   =%d\n",in.ibuf1.d.itstop) ;
        fprintf(stderr," |- itdump   =%d\n",in.ibuf1.d.itdump) ;
        fprintf(stderr," |- itout    =%d\n",in.ibuf1.d.itout)  ;
        fprintf(stderr," |-  time    =%f\n",in.ibuf1.d.time)   ;
        fprintf(stderr," |- atime    =%f\n",in.ibuf1.d.atime)  ;
        fprintf(stderr," |- htime    =%f\n",in.ibuf1.d.htime)  ;
        fprintf(stderr," |- dtime    =%f\n",in.ibuf1.d.dtime)  ;
        fprintf(stderr," |- tstop    =%f\n",in.ibuf1.d.tstop)  ;
        fprintf(stderr," |- tout     =%f\n",in.ibuf1.d.tout)   ;
        fprintf(stderr," |- icdump   =%d\n",in.ibuf1.d.icdump) ;

        uint32_t NPART = in.nb_particles();
        fprintf(stderr," `---> nb particles = %d\n",NPART) ;

        std::cerr << "Loading itype..." << std::endl;        
        in.skip_itype();
        
        std::cerr << "Loading rm..." << std::endl;                
        in.skip_rm();
        
        std::cerr << "Loading r..." << std::endl;                
        if(output_pos == "") {
            in.skip_r();
        } else {
            in.save_vector_array_record(output_pos);
        }
        
        std::cerr << "Loading v..." << std::endl;                
        if(output_velo == "") {
            in.skip_v();
        } else {
            in.save_vector_array_record(output_velo);
        }
        
    } catch(const std::logic_error& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
}

#endif
#endif
