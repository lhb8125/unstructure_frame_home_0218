module utility
  implicit none
  
  include 'mpif.h'
  integer(kind = 4) COMM_SUM
  parameter(COMM_SUM = MPI_SUM)
  integer(kind = 4) COMM_MIN
  parameter(COMM_MIN = MPI_MIN)
  integer(kind = 4) COMM_MAX
  parameter(COMM_MAX = MPI_MAX)
  integer(kind = 4) COMM_PROD
  parameter(COMM_PROD = MPI_PROD)
  integer(kind = 4) COMM_LAND
  parameter(COMM_LAND = MPI_LAND)
  integer(kind = 4) COMM_LOR
  parameter(COMM_LOR = MPI_LOR)
  integer(kind = 4) COMM_BAND
  parameter(COMM_BAND = MPI_BAND)
  integer(kind = 4) COMM_BOR
  parameter(COMM_BOR = MPI_BOR)
  integer(kind = 4) COMM_MAXLOC
  parameter(COMM_MAXLOC = MPI_MAXLOC)
  integer(kind = 4) COMM_MINLOC
  parameter(COMM_MINLOC = MPI_MINLOC)
  integer(kind = 4) COMM_LXOR
  parameter(COMM_LXOR = MPI_LXOR)
  integer(kind = 4) COMM_BXOR
  parameter(COMM_BXOR = MPI_BXOR)
  integer(kind = 4) COMM_REPLACE 
  parameter(COMM_REPLACE = MPI_REPLACE)
  
  ! buffer to hold contents
  character(4096) :: iobuf = ' '

contains

  subroutine init_utility_fort()
   implicit none
   include 'mpif.h'
   integer :: ierr
   logical :: flag
   
   call MPI_INITIALIZED(flag, ierr)
  
   if( flag .eqv. .false. ) call MPI_INIT(ierr)
   
   call init_utility()
  
  end subroutine init_utility_fort

  subroutine flush2proc(pid)
    implicit none
    integer(kind = 4) :: pid

    call fort_sout(pid, trim(iobuf), len(trim(iobuf) )  )
    iobuf = ' '
  end subroutine flush2proc

  subroutine flush2master()
    implicit none
    
    !write(*,*) trim(iobuf)
    call fort_cout(trim(iobuf), len(trim(iobuf) )  )
    iobuf = ' '
  end subroutine flush2master

  subroutine flush2para()
    implicit none
   
    call fort_pout(trim(iobuf), len(trim(iobuf) ) )
    iobuf = ' '
  end subroutine flush2para


end module utility
