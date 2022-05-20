/*
 * operations on IDE disk.
 */

#include "fs.h"
#include "lib.h"
#include <mmu.h>

// Overview:
// 	read data from IDE disk. First issue a read request through
// 	disk register and then copy data from disk buffer
// 	(512 bytes, a sector) to destination array.
//
// Parameters:
//	diskno: disk number.
// 	secno: start sector number.
// 	dst: destination for data read from IDE disk.
// 	nsecs: the number of sectors to read.
//
// Post-Condition:
// 	If error occurrs during the read of the IDE disk, panic.
//
// Hint: use syscalls to access device registers and buffers
/*** exercise 5.2 ***/
void
ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs)
{
	// 0x200: the size of a sector: 512 bytes.
	u_int offset_begin = secno * 0x200;
    u_int offset_end = offset_begin + nsecs * 0x200;
    u_int offset = 0;
	u_int ide_addr = 0x13000000;
    u_char read_mode = 0;
    u_int status = 0;
    
    while (offset_begin + offset < offset_end) {
        // Your code here
        // error occurred, then panic.
        //diskno, offset
        u_int now_offset = offset_begin + offset;
        //set the diskno
        if (syscall_write_dev((u_int)&diskno, ide_addr + 0x0010, 4) < 0) {
            user_panic("set diskno failed!");
        } 
        //set the offset
        if (syscall_write_dev((u_int)&now_offset, ide_addr + 0x0, 4) < 0) {
            user_panic("set offset failed!");
        }
        //set the read_mode
        if (syscall_write_dev((u_int)&read_mode, ide_addr + 0x0020, 1) < 0) {//char lb
            user_panic("set read_mode failed!");
        }
        //if success?
        status = 0;
        if (syscall_read_dev((u_int)&status, ide_addr + 0x0030, 4) < 0) {
            user_panic("get status failed!");
        }
        if (status == 0) {
            user_panic("read status failed!");
        }
        if (syscall_read_dev((u_int)(dst + offset), ide_addr + 0x4000, 0x200) < 0) {
            user_panic("read failed!");
        }
        offset += 0x200;
    }
}


// Overview:
// 	write data to IDE disk.
//
// Parameters:
//	diskno: disk number.
//	secno: start sector number.
// 	src: the source data to write into IDE disk.
//	nsecs: the number of sectors to write.
//
// Post-Condition:
//	If error occurrs during the read of the IDE disk, panic.
//
// Hint: use syscalls to access device registers and buffers
/*** exercise 5.2 ***/
void
ide_write(u_int diskno, u_int secno, void *src, u_int nsecs)
{
	// Your code here
	// int offset_begin = ;
	// int offset_end = ;
	// int offset = ;

	// DO NOT DELETE WRITEF !!!
	writef("diskno: %d\n", diskno);
	u_int offset_begin = secno * 0x200;
    u_int offset_end = offset_begin + nsecs * 0x200;
    u_int offset = 0;
	u_int ide_addr = 0x13000000;
    u_char write_mode = 1;
    u_int status = 0;
    
    // DO NOT DELETE WRITEF !!!
    writef("diskno: %d\n", diskno);
	
    while (offset_begin + offset < offset_end) {
        // Your code here
        // error occurred, then panic.
        //diskno, offset
        u_int now_offset = offset_begin + offset;
        //set the diskno
        if (syscall_write_dev((u_int)&diskno, ide_addr + 0x0010, 4) < 0) {
            user_panic("set diskno failed!");
        } 
        //set the offset
        if (syscall_write_dev((u_int)&now_offset, ide_addr + 0x0, 4) < 0) {
            user_panic("set offset failed!");
        }
        if (syscall_write_dev((u_int)(src + offset), ide_addr + 0x4000, 0x200) < 0) {
            user_panic("write failed!");
        }
        //set the write_mode
        if (syscall_write_dev((u_int)&write_mode, ide_addr + 0x0020, 1) < 0) {//char lb
            user_panic("set read_mode failed!");
        }
        //if success?
        status = 0;
        if (syscall_read_dev((u_int)&status, ide_addr + 0x0030, 4) < 0) {
            user_panic("get status failed!");
        }
        if (status == 0) {
            user_panic("write status failed!");
        }
        offset += 0x200;
    }
}
