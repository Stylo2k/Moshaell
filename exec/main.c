#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <sys/mman.h>
#include "exec.h"
#include <alloca.h>

int main(int argc, char** argv, char** envp) {
    setbuf(stdout, NULL);
    // argument 1 is the name of the elf file
    
    // Open the given binary, read the ELF headers and perform some sanity checks
    // on the file. If the file is not an ELF file, or if it is not a 32-bit
    // executable, or if it is not an executable at all, then exit with an error.
    // that the values in the ELF header are usable for your application (for example, you cannot accept a binary compiled for ARM).

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", argv[1]);
        exit(1);
    }

    // Read the ELF header, and check that it is a valid ELF file
    // 64-bit, little-endian, executable
    Elf64_Ehdr elf_header;
    fread(&elf_header, sizeof(elf_header), 1, file);

    if (elf_header.e_ident[EI_MAG0] != ELFMAG0 ||
        elf_header.e_ident[EI_MAG1] != ELFMAG1 ||
        elf_header.e_ident[EI_MAG2] != ELFMAG2 ||
        elf_header.e_ident[EI_MAG3] != ELFMAG3) {
        printf("Error: %s is not an ELF file\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_ident[EI_CLASS] != ELFCLASS64) {
        printf("Error: %s is not a 64-bit ELF file\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_ident[EI_DATA] != ELFDATA2LSB) {
        printf("Error: %s is not a little-endian ELF file\n", argv[1]);
        exit(1);
    }


    // make sure the elf file is compiled for x86-64
    if (elf_header.e_machine != EM_X86_64) {
        printf("Error: %s is not compiled for x86-64\n", argv[1]);
        exit(1);
    }

    // make sure the elf file is an executable
    if (elf_header.e_type != ET_EXEC && elf_header.e_type != ET_DYN) {
        printf("Error: %s is not an executable\n", argv[1]);
        exit(1);
    }    

    if (elf_header.e_version != EV_CURRENT) {
        printf("Error: %s is not a current version\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_phoff == 0) {
        printf("Error: %s has no program header\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_phentsize != sizeof(Elf64_Phdr)) {
        printf("Error: %s has an invalid program header entry size\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_phnum == 0) {
        printf("Error: %s has no program header entries\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_shoff == 0) {
        printf("Error: %s has no section header\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_shentsize != sizeof(Elf64_Shdr)) {
        printf("Error: %s has an invalid section header entry size\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_shnum == 0) {
        printf("Error: %s has no section header entries\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_shstrndx == 0) {
        printf("Error: %s has no section header string table\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_shstrndx >= elf_header.e_shnum) {
        printf("Error: %s has an invalid section header string table index\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_entry == 0) {
        printf("Error: %s has no entry point\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_phoff >= elf_header.e_shoff) {
        printf("Error: %s has an invalid program header offset\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_shoff >= elf_header.e_shoff + elf_header.e_shnum * elf_header.e_shentsize) {
        printf("Error: %s has an invalid section header offset\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_phoff >= elf_header.e_shoff + elf_header.e_shnum * elf_header.e_shentsize) {
        printf("Error: %s has an invalid program header offset\n", argv[1]);
        exit(1);
    }

    if (elf_header.e_phoff + elf_header.e_phnum * elf_header.e_phentsize >= elf_header.e_shoff) {
        printf("Error: %s has an invalid program header offset\n", argv[1]);
        exit(1);
    }


    // determine the total span of the LOAD segments then allocate the amount of memory needed using mmap

    // read the program header table into memory
    Elf64_Phdr* program_header_table = malloc(elf_header.e_phnum, elf_header.e_phentsize);
    fseek(file, elf_header.e_phoff, SEEK_SET);
    fread(program_header_table, elf_header.e_phentsize, elf_header.e_phnum, file);

    // find the total span of the LOAD segments
    uint64_t min_vaddr = 0;
    uint64_t max_vaddr = 0;

    for (int i = 0; i < elf_header.e_phnum; i++) {
        if (program_header_table[i].p_type == PT_LOAD) {
            min_vaddr = MIN(min_vaddr, program_header_table[i].p_vaddr);
            max_vaddr = MAX(max_vaddr, program_header_table[i].p_vaddr + program_header_table[i].p_memsz);
        }
    }
    min_vaddr = ROUNDDOWN(min_vaddr, 4096);
    max_vaddr = ROUNDUP(max_vaddr, 4096);

    // allocate the memory
    uint64_t size = max_vaddr - min_vaddr;
    // size *=2 ;
    void* memory = mmap(NULL, size, PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // read the LOAD segments into memory
    for (int i = 0; i < elf_header.e_phnum; i++) {
        if (program_header_table[i].p_type == PT_LOAD) {
            fseek(file, program_header_table[i].p_offset, SEEK_SET);
            // copy the segment into memory at the correct location
            fread(memory + program_header_table[i].p_vaddr, 1, program_header_table[i].p_filesz, file);
        }
    }

    // use mmprotect
    for (int i = 0; i < elf_header.e_phnum; i++) {
        if (program_header_table[i].p_type == PT_LOAD) {
            int prot = 0;
            if (program_header_table[i].p_flags & PF_R) {
                prot |= PROT_READ;
            }
            if (program_header_table[i].p_flags & PF_W) {
                prot |= PROT_WRITE;
            }
            if (program_header_table[i].p_flags & PF_X) {
                prot |= PROT_EXEC;
            }
            Elf64_Addr start_addr = ROUNDDOWN(program_header_table[i].p_vaddr, 4096);
            Elf64_Addr end_addr = ROUNDUP(program_header_table[i].p_vaddr + program_header_table[i].p_memsz, 4096);
            mprotect(memory + start_addr, end_addr - start_addr, prot);   
        }
    }

    // grow the stack using alloca
    uint64_t* stack = alloca(1024 * 1024); 
    uint64_t* stack_start = stack;

    // put argc on the stack
    *stack = argc - 1;
    stack++;

    // put argv on the stack
    for (int i = 1; i < argc; i++) {
        *stack = (uint64_t)argv[i];
        stack++;
    }

    // put a null pointer on the stack
    *stack = NULL;
    stack++;

    // put the envp
    while(*envp) {
        *stack = (uint64_t)*envp;
        stack++;
        envp++;
    }
    // at this point envp is pointing to a null pointer
    *stack = NULL;
    stack++;
    envp++;

    // now the envp is pointing to the auxv
    while(*envp) {
        *stack = (uint64_t)*envp;
        envp++;
        if (*stack == AT_PHDR) {
            stack++; 
            *stack = (uint64_t)(memory + elf_header.e_phoff);
        } else if (*stack == AT_PHENT) {
            stack++;
            *stack = (uint64_t)elf_header.e_phentsize;
        } else if (*stack == AT_PHNUM) {
            stack++;
            *stack = (uint64_t)elf_header.e_phnum;
        } else if (*stack == AT_ENTRY) {
            stack++;
            *stack = (uint64_t)(memory + elf_header.e_entry);
        } else {
            stack++;
            *stack = envp;
        }
        envp++;
        stack++;
    }
    // at this point envp is pointing to a null pointer
    *stack = NULL;

    // now the stack is ready to be passed to the entry point
    jump(elf_header.e_entry + memory, stack_start);

    // now that the program is loaded into memory, we can close the file
    fclose(file);
    return 0;
}