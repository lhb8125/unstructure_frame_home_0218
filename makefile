#CXX=mpicxx.mpich
#CXX=/home/export/online1/systest/swrh/lhb/software/OpenMPI/bin/mpicxx
CXX=/home/export/online1/amd_dev1/software/MPICH/bin/mpicxx

LINKFLAG=-fPIC

DEPFLAG=-MMD -MP

CXXFLAG=-O2

LIB= -lcgns -lhdf5 -lparmetis -lmetis -lm -lz -ldl

INCLUDE=-I/home/export/online1/amd_dev1/software/YAML/include \
	-I/home/export/online1/amd_dev1/software/CGNS/CGNS-3.4.0/src/SampleScripts/include \
    -I/home/export/online1/amd_dev1/software/MPICH/include \
    -I/home/export/online1/amd_dev1/software/ParMETIS/parmetis-4.0.3/x86Install_64_64/include \
	-I./src/ \
    -I./src/loadBalancer/ \
	-I./src/mesh/ \
	-I./src/region/ \
	-I./src/topology  

LIBDIR=-L/home/export/online1/amd_dev1/software/YAML/lib \
       -L/home/export/online1/amd_dev1/software/HDF5/hdf5-1.10.5/lib \
       -L/home/export/online1/amd_dev1/software/CGNS/CGNS-3.4.0/src/SampleScripts/lib \
       -L/home/export/online1/amd_dev1/software/ParMETIS/parmetis-4.0.3/x86Install_64_64/lib 

CXXOBJS=src/loadBalancer/loadBalancer.o test/test.o src/mesh/section.o \
		src/topology/topology.o src/mesh/mesh.o src/mesh/nodes.o \
		src/mesh/boundary.o

DEPS:=$(CXXOBJS:.o=.d)

EXE=test.out

${EXE}: ${CXXOBJS}
	${CXX} ${LINKFLAG} -o ${EXE} $^ ${INCLUDE} ${LIBDIR} ${LIB}

${CXXOBJS}:%.o:%.cpp
	${CXX} ${CXXFLAG} ${DEPFLAG} -c $< -o $@ ${INCLUDE}

clean:
	rm -f ${EXE} ${CXXOBJS}

-include $(DEPS)

#g++ -c loadBalancer/loadBalancer.cpp ${INCLUDE} -o loadBalancer/loadBalancer.o
#g++ -c test/test.cpp -I./ -I./loadBalancer/ ${INCLUDE} -o test.o
#g++ -o test.out test.o loadBalancer/loadBalancer.o ${INCLUDE} ${LIB} -lyaml-cpp
#./test.out
