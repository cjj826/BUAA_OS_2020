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
int time_read() {
	int addr = 0x15000000;
	int offset = 0x0010;

	int time;
	//bcopy(&time, addr+0x0, 4);
	//bcopy(addr+offset, &time, 4);
	syscall_write_dev(&time, addr+0x0, 4);
	syscall_read_dev(&time, addr+offset, 4);
	return time;
}
void raid0_write(u_int secno, void *src, u_int nsecs) {
	u_int offset_begin = secno * 0x200;
    u_int offset_end = offset_begin + nsecs * 0x200;
    u_int offset = 0;
    //u_int ide_addr = 0x13000000;
    //u_char write_mode = 1;
    //u_char status = 0;
	u_int num = 0;

	while (offset_begin + offset < offset_end) {
		u_int now = secno + num;
		u_int k = now / 2;
		u_int tar = (now % 2) ? 2 : 1;
		ide_write(tar, k, src + offset, 1);
		num++;
		offset += 0x200;
	}

}

void raid0_read(u_int secno, void *dst, u_int nsecs) {
    u_int offset_begin = secno * 0x200;
    u_int offset_end = offset_begin + nsecs * 0x200;
    u_int offset = 0;
    //u_int ide_addr = 0x13000000;
    //u_char write_mode = 1;
    //u_char status = 0;
    u_int num = 0;

    while (offset_begin + offset < offset_end) {
        u_int now = secno + num;
        u_int k = now / 2;
        u_int tar = (now % 2) ? 2 : 1;
        ide_read(tar, k, dst + offset, 1);
        num++;
        offset += 0x200;
    }

}
int raid4_valid(u_int diskno) {
	u_int ide_addr = 0x13000000;
	if (syscall_write_dev((u_int)&diskno, ide_addr + 0x0010, 4) < 0) {
        user_panic("set diskno failed!");
    }
	u_int now_offset = 0;
        //set the offset
    if (syscall_write_dev((u_int)&now_offset, ide_addr + 0x0, 4) < 0) {
        user_panic("set offset failed!");
	}
	u_char read_mode = 0;
        //set the read_mode
    if (syscall_write_dev(&read_mode, ide_addr + 0x0020, 1) < 0) {//char lb
        user_panic("set read_mode failed!");
	}
        //if success?
    u_char status = 0;
    if (syscall_read_dev(&status, ide_addr + 0x0030, 1) < 0) {
       user_panic("get status failed!");
    }
    if (status == 0) {
		return 0;    
	}
	return 1;
}

void user_bxor(const void *src, void *dst) {
	int i;
	char * data = (char *) src;
	char * d = (char *) dst;
	for (i = 0; i < 512; i++) {
		d[i] = data[i] ^ data[i+512] ^ data[i + 512 * 2] ^ data[i + 512*3];
	}
}

int user_cmp(void * a, void * b) {
	int i;
	char *c = (char *) a;
	char *d = (char *) b;
	for (i = 0; i < 512; i++) {
		if (c[i] != d[i]) {
			break;
		}
	}
	if (i == 512) {
		return 1;
	}
	return 0;
}
	
int raid4_write(u_int blockno, void *src) {
	u_int i, j;
	u_int bs = blockno * 2;
	u_int base_offset;
	u_int useless = 0;
	for (i = 1; i <= 4; i++) {//
		if (raid4_valid(i) == 0) {
			useless++;
			continue;
		}
		base_offset = (i - 1) * 0x200;
		for (j = 0; j <= 1; j++) {
			ide_write(i, j + bs, src + base_offset + 512 * 4 * j, 1);//
		}
	}
	if (raid4_valid(5) == 0) {
		useless++;
	} else {
		char buf[512];
		for (i = 0; i <= 1; i++) {
			user_bzero(buf, 512);
			user_bxor(src + i * 512 * 4, buf);
			ide_write(5, bs + i, buf, 1);
		}
	}
	
	return useless;
}
int raid4_read(u_int blockno, void *dst) {
	u_int invalid = 0;
	u_int i;
	for (i = 1; i <= 5; i++) {
		if (raid4_valid(i) == 0) {
			invalid++;
		}
	}
	u_int j;
    u_int bs = blockno * 2;
    u_int base_offset;
    u_int useless = 0;
	if (invalid == 0) {
		for (i = 1; i <= 4; i++) {//
			base_offset = (i - 1) * 0x200;
			for (j = 0; j <= 1; j++) {
				ide_read(i, j + bs, dst + base_offset + 512 * 4 * j, 1);//
			}
		}
		char buf[512];
		char old[512];
        for (i = 0; i <= 1; i++) {
            user_bzero(buf, 512);
			user_bzero(old, 512);
            //ide_read(5, i + bs, buf, 1);
			user_bxor(dst + i * 512 * 4, buf);
			ide_read(5, i + bs, old, 1);
            if (user_cmp(buf, old)) {
				return 0;
			} else return -1;
        }
	} else if (invalid > 1) {
		return invalid;
	} else {
		//
		return 1;
	}

}


void
ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs)
{
	// 0x200: the size of a sector: 512 bytes.
	u_int offset_begin = secno * 0x200;
    u_int offset_end = offset_begin + nsecs * 0x200;
    u_int offset = 0;
	u_int ide_addr = 0x13000000;
    u_char read_mode = 0;
    u_char status = 0;
    
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
        if (syscall_write_dev(&read_mode, ide_addr + 0x0020, 1) < 0) {//char lb
            user_panic("set read_mode failed!");
        }
        //if success?
        status = 0;
        if (syscall_read_dev(&status, ide_addr + 0x0030, 1) < 0) {
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
// /*** exercise 5.2 ***/
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
    u_char status = 0;
    
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
        if (syscall_write_dev(&write_mode, ide_addr + 0x0020, 1) < 0) {//char lb
            user_panic("set read_mode failed!");
        }
        //if success?
        status = 0;
        if (syscall_read_dev(&status, ide_addr + 0x0030, 1) < 0) {
            user_panic("get status failed!");
        }
        if (status == 0) {
            user_panic("write status failed!");
        }
        offset += 0x200;
    }
}
/*
void
ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs)
{
	// 0x200: the size of a sector: 512 bytes.
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;
    u_int ide = 0x13000000;
    u_int control;
	while (offset_begin + offset < offset_end) {
		// Your code here
		// error occurred, then panic.
        if (syscall_write_dev(&diskno, ide + 0x10, 4) < 0) {
            user_panic("ide_read error 1");
        }
        control = offset_begin + offset;
        if (syscall_write_dev(&control, ide, 4) < 0) {
            user_panic("ide_read error 2");
        }
        control = 0;
        if (syscall_write_dev(&control, ide + 0x20, 1) < 0) {
            user_panic("ide_read error 3");
        }
        if (syscall_read_dev(&control, ide + 0x30, 4) < 0) {
            user_panic("ide_read error 4");
        }
        if (control == 0) {
            user_panic("ide_read failure");
        }
        if (syscall_read_dev(dst + offset, ide + 0x4000, 512) < 0) {
            user_panic("ide_read error 5");
        }
        offset += 0x200;
	}
}

*/
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
/*void
ide_write(u_int diskno, u_int secno, void *src, u_int nsecs)
{
	// Your code here
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;
    u_int ide = 0x13000000;
    u_int control;
	// DO NOT DELETE WRITEF !!!
	writef("diskno: %d\n", diskno);

	while (offset_begin + offset < offset_end) {
		// copy data from source array to disk buffer.
		// if error occur, then panic.
        if (syscall_write_dev(&diskno, ide + 0x10, 4) < 0) {
            user_panic("ide_write error 1");
        }
        control = offset_begin + offset;
        if (syscall_write_dev(&control, ide, 4) < 0) {
            user_panic("ide_write error 2");
        }
        if (syscall_write_dev(src + offset, ide + 0x4000, 512) < 0) {
            user_panic("ide_write error 3");
        }
        control = 1;
        if (syscall_write_dev(&control, ide + 0x20, 1) < 0) {
            user_panic("ide_write error 4");
        }
        if (syscall_read_dev(&control, ide + 0x30, 4) < 0) {
            user_panic("ide_write error 5");
        }

        if (control == 0) {
            user_panic("ide write failure");
        }
        offset += 0x200;
	}
}*/
