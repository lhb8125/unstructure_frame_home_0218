!-------------------------------------------------------------------------------
!   module defines precisions of logical, integer and real.
!-------------------------------------------------------------------------------
module var_kind_def
    implicit none

    integer(kind=4),parameter:: dpL =  kind(.true.)
    integer(kind=4),parameter:: dpI =  8
    integer(kind=4),parameter:: dpR =  8
end module var_kind_def

module var_global
    use var_kind_def
    implicit none
    integer, parameter:: strlen = 100
    integer, parameter:: dim = 3
    integer, parameter:: max_n_node = 50
    integer(dpI):: err_mem = 0
    integer:: my_id

    ! abstract interface
    !     function func()
    !         integer :: func
    !     end function func
    ! end interface

    ! contains
    ! function test_f()
    !     implicit none
    !     integer:: test_f
    
    !     write(*,*), "hello world!"
    !     test_f = 1
    ! end function test_f
end module var_global

module var_c_string
    use var_kind_def
    use iso_c_binding
    implicit none
    ! integer, parameter:: str_num = 10
    character(len=30):: str_arr(1)
    POINTER(str_ptr, str_arr)
end module

program main
    use var_kind_def
    use var_global
    use utility
    use var_c_string
    implicit none
    character(len=strlen):: config_file = './config.yaml'
    integer(dpI):: iele, iface, inode
    integer(dpI):: n_ele, n_face_i, n_face_b, n_node
    integer(dpI),allocatable:: e2n(:),e2n_pos(:),ele_type(:)
    integer(dpI),allocatable:: e2f(:),e2f_pos(:)
    integer(dpI),allocatable:: if2n(:),if2n_pos(:)
    integer(dpI),allocatable:: if2e(:),if2e_pos(:)
    real(dpR),allocatable:: b(:),x(:)
    real(dpR), allocatable:: vol(:), area(:), coord(:)
    real(dpR),allocatable:: pid(:)
    character(len=20):: field_name, field_type

    real(dpR) :: vol_new(1)
    POINTER(vol_new2, vol_new)
    integer(dpI):: ndim_new, n_ele_new, n_dim

    integer:: nPara, write_interval, str_len
    character(20):: mesh_file, result_file
    real:: delta_t

    ! procedure(func),pointer:: f_ptr => null()

    ! 初始化
    call init_utility_fort()
    call init(trim(config_file))

    ! 获取控制参数
    nPara = 5

    ! call get_string_para(mesh_file, str_len, nPara, &
    !     & "domain1"//C_NULL_CHAR, &
    !     & "region"//C_NULL_CHAR, &
    !     & "0"//C_NULL_CHAR, &
    !     & "path"//C_NULL_CHAR, &
    !     & "1"//C_NULL_CHAR)
    ! ! mesh_file = str_arr(5)
    ! ! call par_std_out("mesh file: %s \n", mesh_file)
    ! write(iobuf,*),"mesh file: ", mesh_file(1:str_len)
    ! call flush2master()
    ! nPara = 3
    ! call get_label_para(write_interval, nPara, &
    !     & "domain1"//C_NULL_CHAR, "solve"//C_NULL_CHAR, "writeInterval"//C_NULL_CHAR)
    ! ! call master_std_out("write internal: %d \n", write_interval)
    ! write(iobuf,*),"write_interval: ", write_interval
    ! call flush2master()
    ! call get_scalar_para(delta_t, nPara, "domain1"//C_NULL_CHAR, "solve"//C_NULL_CHAR, "deltaT"//C_NULL_CHAR)
    ! write(*,*),"delta_t: ", delta_t


    ! ! 获取基本单元数目
    call get_elements_num(n_ele)
    call get_inner_faces_num(n_face_i)
    call get_bnd_faces_num(n_face_b)
    call get_nodes_num(n_node)
    write(*,*), "elements num: ", n_ele
    write(*,*), "internal faces num: ", n_face_i
    write(*,*), "boundary faces num: ", n_face_b
    write(*,*), "nodes num: ", n_node

    ! 获取进程号
    call get_pid(my_id)
    allocate(pid(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, pid'
    do iele=1,n_ele
        pid(iele) = my_id
    end do
    ! 注册进程号场
    n_dim = 1
    call add_scalar_field("cell"//C_NULL_CHAR, "pid"//C_NULL_CHAR, pid, n_dim, n_ele)

    ! 获取单元与格点拓扑关系
    allocate(e2n_pos(n_ele+1), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, e2n_pos'
    call get_ele_2_node_pos(e2n_pos)
    allocate(e2n(e2n_pos(n_ele+1)), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, e2n'
    call get_ele_2_node(e2n)
    ! do iele=1,n_ele
    !     if(my_id .eq. 1) write(*,"(9I5)"),e2n_pos(iele), e2n(e2n_pos(iele)+0), e2n(e2n_pos(iele)+1), e2n(e2n_pos(iele)+2), e2n(e2n_pos(iele)+3), e2n(e2n_pos(iele)+4), e2n(e2n_pos(iele)+5), e2n(e2n_pos(iele)+6), e2n(e2n_pos(iele)+7)
    ! end do

    ! 获取网格单元与网格面拓扑关系
    allocate(e2f_pos(n_ele+1), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, e2f_pos'
    call get_ele_2_face_pos(e2f_pos)
    allocate(e2f(e2f_pos(n_ele+1)), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, e2f'
    call get_ele_2_face(e2f)
    ! do iele=1,n_ele
        ! write(*,"(9I5)"), e2f_pos(iele+1)-e2f_pos(iele),e2n(e2n_pos(iele)+0), e2n(e2n_pos(iele)+1), e2n(e2n_pos(iele)+2), e2n(e2n_pos(iele)+3), e2n(e2n_pos(iele)+4), e2n(e2n_pos(iele)+5), e2n(e2n_pos(iele)+6), e2n(e2n_pos(iele)+7)
    ! end do

    ! 获取内部网格面与格点拓扑关系
    allocate(if2n_pos(n_face_i+1), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, if2n_pos'
    call get_inn_face_2_node_pos(if2n_pos)
    allocate(if2n(if2n_pos(n_face_i+1)), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, if2n'
    call get_inn_face_2_node(if2n)

    ! 获取内部网格面与网格单元拓扑关系
    allocate(if2e_pos(n_face_i+1), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, if2n_pos'
    call get_inn_face_2_ele_pos(if2e_pos)
    allocate(if2e(if2e_pos(n_face_i+1)), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, if2n'
    call get_inn_face_2_ele(if2e)

    ! 获取格点坐标
    allocate(coord(n_node*dim), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, coord'
    call get_coords(coord)
    ! do inode=1,n_node
    !     if(my_id .eq. 1) write(*,*),coord((inode-1)*dim+1), coord((inode-1)*dim+2), coord((inode-1)*dim+3)
    ! end do

    ! 获取网格单元类型
    allocate(ele_type(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, ele_type'
    call get_ele_type(ele_type)

    ! 计算网格单元体积
    allocate(vol(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, vol'
    call calc_eles_vol(n_ele, e2n, e2n_pos, coord, ele_type, vol)

    ! 计算网格面面积
    allocate(area(n_face_i), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, area_b'
    call calc_faces_area(n_face_i, if2n, if2n_pos, coord, area)

    ! 注册vol场
    n_dim = 1
    call add_scalar_field("cell"//C_NULL_CHAR, "VOL"//C_NULL_CHAR, vol, &
        & n_dim, n_ele)
    call get_scalar_field("cell"//C_NULL_CHAR, "VOL"//C_NULL_CHAR, vol_new2, &
        & ndim_new, n_ele_new)

    ! b=A*x
    allocate(b(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, b'
    allocate(x(n_ele), stat=err_mem)
    if(err_mem .ne. 0) stop 'Error, fails to allocate memory, x'
    do iele=1,n_ele
        x(iele) = 1.0d0
        b(iele) = 0.0d0
    end do
    call calc_spmv(n_face_i, if2e, area, x, b)
    ! 注册b场
    n_dim = 1
    call add_scalar_field("cell"//C_NULL_CHAR, "b"//C_NULL_CHAR, b, n_dim, n_ele)


    ! 输出网格到CGNS文件中
    call write_mesh()
    ! 输出体积场到CGNS文件中
    call write_scalar_field("VOL"//C_NULL_CHAR, "cell"//C_NULL_CHAR)
    ! 输出b场到CGNS文件中
    call write_scalar_field("b"//C_NULL_CHAR, "cell"//C_NULL_CHAR)
    ! 输出进程号场到CGNS文件中
    call write_scalar_field("pid"//C_NULL_CHAR, "cell"//C_NULL_CHAR)

    call clear()
    ! f_ptr => test_f
    ! ndim = f_ptr()

end program main

subroutine calc_eles_vol(n_ele, e2n, e2n_pos, coord, ele_type, vol)
    use var_kind_def
    use var_global
    implicit none
    integer(dpI), intent(IN) :: n_ele
    integer(dpI), intent(IN) :: e2n(*), e2n_pos(*), ele_type(*)
    real(dpR), intent(IN) :: coord(*)
    real(dpR), intent(INOUT) :: vol(*)
    real(dpR), allocatable:: nodes_coord(:)
    integer :: iele,ie2n,inode,n_node,idim,idx

    allocate(nodes_coord(max_n_node*dim), stat=err_mem)
    do iele=1,n_ele
        n_node = e2n_pos(iele+1)-e2n_pos(iele)
        if(n_node .gt. max_n_node) stop 'Error, the count of nodes in one element exceed the maximum value'
        idx = 1
    	do ie2n=e2n_pos(iele), e2n_pos(iele+1)-1
            inode = e2n(ie2n)
            do idim=1,dim
                nodes_coord((idx-1)*dim+idim) = coord((inode-1)*dim+idim)
            end do
            idx = idx+1
        end do
        if(ele_type(iele) .eq. 17 .or. ele_type(iele) .eq. 19) then
            call calc_HEXA_vol(nodes_coord, vol(iele))
            ! write(*,*), iele, vol(iele)
    	else if(ele_type(iele) .eq. 10) then
            call calc_TETRA_vol(nodes_coord, vol(iele))
        else if(ele_type(iele) .eq. 12) then
            call calc_TETRA_vol(nodes_coord, vol(iele))
        else
            write(*,*), "the element type is not supported"
        end if
    end do
    if(allocated(nodes_coord)) deallocate(nodes_coord)
    
end subroutine calc_eles_vol

subroutine calc_faces_area(n_face, f2n, f2n_pos, coord, area)
    use var_kind_def
    use var_global
    implicit none
    integer(dpI), intent(IN):: n_face
    integer(dpI), intent(IN):: f2n(*), f2n_pos(*)
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(INOUT):: area(*)
    real(dpR), allocatable:: nodes_coord(:)
    integer:: iface, if2n, inode, n_node, idim, idx

    allocate(nodes_coord(max_n_node*dim), stat=err_mem)
    do iface=1,n_face
        n_node = f2n_pos(iface+1)-f2n_pos(iface)
        if(n_node .gt. max_n_node) stop 'Error, the count of nodes in one element exceed the maximum value'
        idx = 1
        do if2n=f2n_pos(iface), f2n_pos(iface+1)-1
            inode = f2n(if2n)
            do idim=1,dim
                nodes_coord((idx-1)*dim+idim) = coord((inode-1)*dim+idim)
            end do
            idx = idx+1
        end do
        if(n_node .eq. 4 .or. n_node .eq. 9) then
            call calc_QUAD_area(nodes_coord, area(iface))
            ! write(*,*), iface, area(iface)
        else if(n_node .eq. 3) then
            call calc_TRI_area(nodes_coord, area(iface))
        end if
    end do
    if(allocated(nodes_coord)) deallocate(nodes_coord)
    
end subroutine calc_faces_area

subroutine calc_HEXA_vol(coord, vol)
    use var_kind_def
    use var_global
    implicit none
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(OUT):: vol
    real(dpR):: v1(dim), v2(dim), v3(dim)

    ! v1 = coord(2)-coord(1)
    v1(1) = coord(1*dim+1)-coord(0*dim+1)
    v1(2) = coord(1*dim+2)-coord(0*dim+2)
    v1(3) = coord(1*dim+3)-coord(0*dim+3)

    ! v2 = coord(4)-coord(1)
    v2(1) = coord(3*dim+1)-coord(0*dim+1)
    v2(2) = coord(3*dim+2)-coord(0*dim+2)
    v2(3) = coord(3*dim+3)-coord(0*dim+3)

    ! v3 = coord(5)-coord(1)
    v3(1) = coord(4*dim+1)-coord(0*dim+1)
    v3(2) = coord(4*dim+2)-coord(0*dim+2)
    v3(3) = coord(4*dim+3)-coord(0*dim+3)

    ! vol = (v1xv2)*v3
    vol =     v3(1)*(v1(2)*v2(3)-v2(2)*v1(3))
    vol = vol+v3(2)*(v1(3)*v2(1)-v1(1)*v2(3))
    vol = vol+v3(3)*(v1(1)*v2(2)-v2(1)*v1(2))

    ! write(*,*), vol
    
end subroutine calc_HEXA_vol

subroutine calc_TETRA_vol(coord, vol)
    use var_kind_def
    use var_global
    implicit none
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(OUT):: vol
    real(dpR):: v1(dim), v2(dim), v3(dim)

    ! v1 = coord(2)-coord(1)
    v1(1) = coord(1*dim+1)-coord(0*dim+1)
    v1(2) = coord(1*dim+2)-coord(0*dim+2)
    v1(3) = coord(1*dim+3)-coord(0*dim+3)

    ! v2 = coord(3)-coord(1)
    v2(1) = coord(2*dim+1)-coord(0*dim+1)
    v2(2) = coord(2*dim+2)-coord(0*dim+2)
    v2(3) = coord(2*dim+3)-coord(0*dim+3)

    ! v3 = coord(4)-coord(1)
    v3(1) = coord(3*dim+1)-coord(0*dim+1)
    v3(2) = coord(3*dim+2)-coord(0*dim+2)
    v3(3) = coord(3*dim+3)-coord(0*dim+3)

    ! vol = (v1xv2)*v3
    vol =     v3(1)*(v1(2)*v2(3)-v2(2)*v1(3))
    vol = vol+v3(2)*(v1(3)*v2(1)-v1(1)*v2(3))
    vol = vol+v3(3)*(v1(1)*v2(2)-v2(1)*v1(2))
    vol = vol/6
    
end subroutine calc_TETRA_vol

subroutine calc_QUAD_area(coord, area)
    use var_kind_def
    use var_global
    implicit none
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(OUT):: area
    real(dpR):: v1(dim), v2(dim), v3(dim)

    ! v1 = coord(2)-coord(1)
    v1(1) = coord(1*dim+1)-coord(0*dim+1)
    v1(2) = coord(1*dim+2)-coord(0*dim+2)
    v1(3) = coord(1*dim+3)-coord(0*dim+3)

    ! v2 = coord(4)-coord(1)
    v2(1) = coord(3*dim+1)-coord(0*dim+1)
    v2(2) = coord(3*dim+2)-coord(0*dim+2)
    v2(3) = coord(3*dim+3)-coord(0*dim+3)

    ! v3 = v1xv2
    v3(1) = v1(2)*v2(3)-v2(2)*v1(3)
    v3(2) = v1(3)*v2(1)-v1(1)*v2(3)
    v3(3) = v1(1)*v2(2)-v2(1)*v1(2)

    area = sqrt(v3(1)*v3(1)+v3(2)*v3(2)+v3(3)*v3(3))

    ! write(*,*), area

end subroutine calc_QUAD_area

subroutine calc_TRI_area(coord, area)
    use var_kind_def
    use var_global
    implicit none
    real(dpR), intent(IN):: coord(*)
    real(dpR), intent(OUT):: area
    real(dpR):: v1(dim), v2(dim), v3(dim)

    ! v1 = coord(2)-coord(1)
    v1(1) = coord(1*dim+1)-coord(0*dim+1)
    v1(2) = coord(1*dim+2)-coord(0*dim+2)
    v1(3) = coord(1*dim+3)-coord(0*dim+3)

    ! v2 = coord(4)-coord(1)
    v2(1) = coord(3*dim+1)-coord(0*dim+1)
    v2(2) = coord(3*dim+2)-coord(0*dim+2)
    v2(3) = coord(3*dim+3)-coord(0*dim+3)

    ! v3 = v1xv2
    v3(1) = v1(2)*v2(3)-v2(2)*v1(3)
    v3(2) = v1(3)*v2(1)-v1(1)*v2(3)
    v3(3) = v1(1)*v2(2)-v2(1)*v1(2)

    area = sqrt(v3(1)*v3(1)+v3(2)*v3(2)+v3(3)*v3(3))/2

end subroutine calc_TRI_area

subroutine calc_spmv(n_face, f2c, A, x, b)
    use var_kind_def
    use var_global
    implicit none
    integer(dpI), intent(IN):: f2c(*)
    real(dpR), intent(IN):: A(*), x(*)
    real(dpR), intent(INOUT):: b(*)
    integer(dpI), intent(IN):: n_face
    integer:: iface, row, col

    do iface=1,n_face
        row    = f2c((iface-1)*2+1)
        col    = f2c((iface-1)*2+2)
        b(row) = b(row)+A(iface)*x(col)
        b(col) = b(col)+A(iface)*x(row)
    end do
    
end subroutine calc_spmv

