import os
import GLOVAR as gl

# initialization
gl._init()

software_home = '/home/export/online1/amd_dev1/software'
#software_home = '/home/export/online1/amd_dev1/liuhb/hsf/library'
hdf5_path = software_home + '/HDF5/hdf5-1.10.5/gcc_build'
cgns_path = software_home + '/CGNS/CGNS-3.4.0/src/SampleScripts'
#yaml_path = software_home + '/yaml-cpp/yaml-cpp-0.6.2'
yaml_path = software_home + '/yaml-cpp/yaml-cpp-yaml-cpp-0.5.3'

parmetis_path = software_home + '/ParMETIS/parmetis-4.0.3/gccInstall_64_64'


# Set our required environment
libraries 		= ['yaml-cpp', 'cgns', 'hdf5', 'parmetis', 'metis', 'stdc++', 'm', 'z', 'dl']
library_paths 	= [hdf5_path + '/lib',
				   cgns_path + '/lib',
				   parmetis_path + '/lib',
				   yaml_path + '/lib']
cppDefines 		= {}
#cppFlags 		= ['-fPIC', '-MMD', '-MP', '-DSCALAR_FLOAT64']
cppFlags 		= ['-fPIC',  '-DSCALAR_FLOAT64', '-Wno-write-strings']
debugFlags      = ['-g', '-O0', '-DDEBUG', '-Wall', '-Wextra', '-Werror', '-fno-elide-constructors']
#debugFlags      = ['-g']
optFlags		= ['-O2']
f90Flags		= ['-g', '-fcray-pointer']

# define the attributes of the build environment
env = Environment(ENV   = os.environ)
env.Append(LIBS 	    = libraries)
env.Append(LIBPATH 		= library_paths)
env.Append(CPPDEFINES 	= cppDefines)
env.Append(CPPFLAGS 	= cppFlags)
env.Append(F90FLAGS 	= f90Flags)
env.Append(CCCOMSTR     = "CC $SOURCES")
#env.Append(LINKCOMSTR   = "LINK $TARGET")

# get debug flag, the default is -O3
debug = ARGUMENTS.get('debug', '')
DBGFLAG = ''

if debug == 'true':
	env.Append(CPPFLAGS = debugFlags)
	DBGFLAG = 'Debug'
else:
	env.Append(CPPFLAGS = optFlags)
	DBGFLAG = 'Opt'

SWOPT = ''
INTFLAG = '-DLABEL_INT64'
INTSize = ''

if INTFLAG == '-DLABEL_INT32':
    INTSize = 'Int32'
elif INTFLAG == '-DLABEL_INT64':
    INTSize = 'Int64'
else:
    print('Error: label size is not defined!')

env.Append(CPPFLAGS = INTFLAG)


# get platform information, the default is x86
# if sw preferred, using "scons pla=sw"
gl.platform = ARGUMENTS.get('pla', 'x86')

if gl.platform == 'sw':
	env['CC']   = 'sw5cc'
	env['CXX']  = 'swg++453'
	env['F90']  = 'mpif90'
	env['AR']   = 'swar'
	env['LINK'] = 'swg++453'
	env.Append(CPPPATH = ['/usr/sw-mpp/mpi2/include'])
else:

	mpi_path = software_home + '/MPICH/gcc_build'
	env['CC']   = mpi_path + '/bin/mpicc'
	env['CXX']  = mpi_path + '/bin/mpicxx'
	env['F90']  = mpi_path + '/bin/mpifort'
	#env['AR']   = 'ar'
	#env['LINK'] = software_home + '/MPICH/bin/mpicc'
	#env['LINK'] = software_home + '/MPICH/bin/mpicxx'
	env.Append(CPPPATH = [cgns_path + '/include'])
	env.Append(CPPPATH = [mpi_path + '/include'])
	env.Append(CPPPATH = [parmetis_path + '/include'])
	env.Append(CPPPATH = [yaml_path + '/include'])
	env.Append(F90PATH = [mpi_path + '/include'])

	# for unap
	env.Append(CPPPATH = ['/home/export/online1/amd_dev1/guhf/unap/x86Install/include'])

# env['LINKFLAGS'] = '-lstdc++'

Export('env')

src_name = 'src'
build_name = 'build'
test_name = 'test'

gl.include_path = gl.root_path + '/' + build_name + '/Include'
print(gl.root_path)

# add include path to -I
env.Append(CPPPATH = gl.include_path)

CXXNAME = os.path.basename(env['CXX'])

gl.fullName = CXXNAME + DBGFLAG + INTSize + SWOPT
gl.build_path = gl.root_path + '/' + build_name + '/' + gl.fullName
gl.source_path = gl.root_path + '/' + src_name
build_src_path = gl.build_path + '/' + src_name
build_test_path = gl.build_path + '/' + test_name

# out of source compiling settings
env.VariantDir(build_src_path, gl.source_path, duplicate=1)
env.SConscript(build_src_path + '/SConscript', duplicate=0, exports = 'env' )

env.VariantDir(build_test_path, gl.root_path + '/' + test_name, duplicate=1)
env.SConscript(build_test_path + '/SConscript', duplicate=0, exports = 'env' )
