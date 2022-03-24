/* This is a simplefied ELF reader.
 * You can contact me if you find any bugs.
 *
 * Luming Wang<wlm199558@126.com>
 */

#include "kerelf.h"
#include <stdio.h>
/* Overview:
 *   Check whether it is a ELF file.
 *
 * Pre-Condition:
 *   binary must longer than 4 byte.
 *
 * Post-Condition:
 *   Return 0 if `binary` isn't an elf. Otherwise
 * return 1.
 */
int is_elf_format(u_char *binary)
{
        Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;
        if (ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
                ehdr->e_ident[EI_MAG1] == ELFMAG1 &&
                ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
                ehdr->e_ident[EI_MAG3] == ELFMAG3) {
                return 1;
        }

        return 0;
}

/* Overview:
 *   read an elf format binary file. get ELF's information
 *
 * Pre-Condition:
 *   `binary` can't be NULL and `size` is the size of binary.
 *
 * Post-Condition:
 *   Return 0 if success. Otherwise return < 0.
 *   If success, output address of every section in ELF.
 */

/*
    Exercise 1.2. Please complete func "readelf". 
*/
int readelf(u_char *binary, int size)
{
        Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;

        int Nr;

        Elf32_Phdr *phdr = NULL;
		Elf32_Phdr *phdr1 = NULL;
		Elf32_Phdr *phdr2 = NULL;

        u_char *ptr_ph_table = NULL;
        Elf32_Half ph_entry_count;
        Elf32_Half ph_entry_size;


        // check whether `binary` is a ELF file.
        if (size < 4 || !is_elf_format(binary)) {
                printf("not a standard elf format\n");
                return 0;
        }
		Elf32_Addr p_vaddr_l1;
		Elf32_Addr p_vaddr_r1;
		Elf32_Addr p_vaddr_l2;
        Elf32_Addr p_vaddr_r2;
		Elf32_Addr page_addr1;
		Elf32_Addr page_addr2;

        // get section table addr, section header number and section header size.
		ptr_ph_table = (u_char *) (ehdr->e_phoff + binary);//offset+binary
		ph_entry_count = ehdr->e_phnum;
		ph_entry_size = ehdr->e_phentsize;
        // for each section header, output section number and section addr. 
        int order = 0;
		int i, j;
		int flag = 0;
		for (i = 0; i < ph_entry_count; i++) {
			for (j = 0; j < ph_entry_count; j++) {
				if (i == j) continue;
				phdr1 = (Elf32_Phdr *) (ptr_ph_table + i * ph_entry_size);
				phdr2 = (Elf32_Phdr *) (ptr_ph_table + j * ph_entry_size);
				p_vaddr_l1 = phdr1 -> p_vaddr;
				p_vaddr_r1 = phdr1 -> p_vaddr + phdr1 -> p_memsz;
				p_vaddr_l2 = phdr2 -> p_vaddr;
                p_vaddr_r2 = phdr2 -> p_vaddr + phdr2 -> p_memsz;
				page_addr1 = (p_vaddr_r1 >> 12) << 12;
				page_addr2 = (p_vaddr_l2 >> 12) << 12;
				if (page_addr1 == page_addr2) {
					flag = 1;
				}
				if (flag == 1) {
					if (p_vaddr_l2 < p_vaddr_r1) {
						flag = 2;
					}
				}
				
			}
		}
		if (flag) {
			if (flag == 1) printf("Overlay at page va : 0x%x\n", page_addr1);
			else if (flag == 2) printf("Conflict at page va : 0x%x\n", page_addr1);
		}
		else if (flag == 0) {
			for (order = 0; order < ph_entry_count; order++) {
            	phdr = (Elf32_Phdr *) (ptr_ph_table + order * ph_entry_size);
				printf("%d:0x%x,0x%x\n", order, phdr->p_filesz, phdr->p_memsz); 
        	}
		}
		
		// hint: section number starts at 0.


        return 0;
}
