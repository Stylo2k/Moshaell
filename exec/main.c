#include <stdio.h>
#include <stdlib.h>
#include <elf.h>


int main(int argc, char** argv) {
    // argument 1 is the name of the elf file
    
    // Open the given binary, read the ELF headers and perform some sanity checks
    // on the file. If the file is not an ELF file, or if it is not a 32-bit
    // executable, or if it is not an executable at all, then exit with an error.
    // that the values in the ELF header are usable for your application (for example, you cannot accept a binary compiled for ARM).

    File* file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", argv[1]);
        exit(1);
    }

    // Read the ELF header
    Elf32_Ehdr header;
    fread(&header, sizeof(Elf32_Ehdr), 1, file);

    // Check if the file is an ELF file
    if (header.e_ident[EI_MAG0] != ELFMAG0 ||
        header.e_ident[EI_MAG1] != ELFMAG1 ||
        header.e_ident[EI_MAG2] != ELFMAG2 ||
        header.e_ident[EI_MAG3] != ELFMAG3) {
        printf("Error: File %s is not an ELF file\n", argv[1]);
        exit(1);
    }

    // Check if the file is a 32-bit executable
    if (header.e_ident[EI_CLASS] != ELFCLASS32) {
        printf("Error: File %s is not a 32-bit executable\n", argv[1]);
        exit(1);
    }

    // Check if the file is an executable
    if (header.e_type != ET_EXEC) {
        printf("Error: File %s is not an executable\n", argv[1]);
        exit(1);
    }

    // Check if the file is compiled for x86
    if (header.e_machine != EM_386) {
        printf("Error: File %s is not compiled for x86\n", argv[1]);
        exit(1);
    }

    // Read the program header table
    Elf32_Phdr* programHeaderTable = malloc(header.e_phentsize * header.e_phnum);

    // Read the section header table
    Elf32_Shdr* sectionHeaderTable = malloc(header.e_shentsize * header.e_shnum);

    // Find the section header table string table
    Elf32_Shdr* sectionHeaderTableStringTable = &sectionHeaderTable[header.e_shstrndx];

    // Load all ‘segments’ of type LOAD into memory. This memory needs to be contiguous, in order to keep the relative pointers valid
    // For this, first determine the total span of all loadable segments (i.e. the start address of the first loadable segment until the end address of the last one) and allocate this amount of memory. Use mmap() for this.
    // Fill the allocated memory with the binary contents properly, taking into account all offsets so that the binary ‘structure’ is preserved.
    // Finally, protect the memory regions with the correct flags (as obtained from the program header) to enable execute- or write-access to the relevant regions.

    // Find the start address of the first loadable segment
    Elf32_Addr firstLoadableSegmentStartAddress = 0;
    for (int i = 0; i < header.e_phnum; i++) {
        if (programHeaderTable[i].p_type == PT_LOAD) {
            firstLoadableSegmentStartAddress = programHeaderTable[i].p_vaddr;
            break;
        }
    }

    // Find the end address of the last loadable segment
    Elf32_Addr lastLoadableSegmentEndAddress = 0;
    for (int i = 0; i < header.e_phnum; i++) {
        if (programHeaderTable[i].p_type == PT_LOAD) {
            if (programHeaderTable[i].p_vaddr + programHeaderTable[i].p_memsz > lastLoadableSegmentEndAddress) {
                lastLoadableSegmentEndAddress = programHeaderTable[i].p_vaddr + programHeaderTable[i].p_memsz;
            }
        }
    }

    // Allocate the memory
    void* memory = mmap(firstLoadableSegmentStartAddress, lastLoadableSegmentEndAddress - firstLoadableSegmentStartAddress, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    //after allocating fill the allocated memory with the binary contents properly
    for (int i = 0; i < header.e_phnum; i++) {
        if (programHeaderTable[i].p_type == PT_LOAD) {
            // Read the contents of the segment from the file
            void* segmentContents = malloc(programHeaderTable[i].p_filesz);
            fseek(file, programHeaderTable[i].p_offset, SEEK_SET);
            fread(segmentContents, programHeaderTable[i].p_filesz, 1, file);

            // Copy the contents to the allocated memory
            memcpy(memory + programHeaderTable[i].p_vaddr, segmentContents, programHeaderTable[i].p_filesz);

            // Free the segment contents
            free(segmentContents);
        }
    }

    // Finally, protect the memory regions with the correct flags (as obtained from the program header) to enable execute- or write-access to the relevant regions.
    for (int i = 0; i < header.e_phnum; i++) {
        if (programHeaderTable[i].p_type == PT_LOAD) {
            // Determine the protection flags
            int protectionFlags = 0;
            if (programHeaderTable[i].p_flags & PF_R) {
                protectionFlags |= PROT_READ;
            }
            if (programHeaderTable[i].p_flags & PF_W) {
                protectionFlags |= PROT_WRITE;
            }
            if (programHeaderTable[i].p_flags & PF_X) {
                protectionFlags |= PROT_EXEC;
            }

            // Protect the memory region
            mprotect(memory + programHeaderTable[i].p_vaddr, programHeaderTable[i].p_memsz, protectionFlags);
        }
    }


    // Find the .text section
    Elf32_Shdr* textSection = NULL;
    for (int i = 0; i < header.e_shnum; i++) {
        if (strcmp(sectionHeaderTableStringTable->sh_offset + sectionHeaderTable[i].sh_name, ".text") == 0) {
            textSection = &sectionHeaderTable[i];
            break;
        }
    }

    // Find the .data section
    Elf32_Shdr* dataSection = NULL;
    for (int i = 0; i < header.e_shnum; i++) {
        if (strcmp(sectionHeaderTableStringTable->sh_offset + sectionHeaderTable[i].sh_name, ".data") == 0) {
            dataSection = &sectionHeaderTable[i];
            break;
        }
    }


    // Find the .bss section
    Elf32_Shdr* bssSection = NULL;
    for (int i = 0; i < header.e_shnum; i++) {
        if (strcmp(sectionHeaderTableStringTable->sh_offset + sectionHeaderTable[i].sh_name, ".bss") == 0) {
            bssSection = &sectionHeaderTable[i];
            break;
        }
    }

    // Find the .rodata section
    Elf32_Shdr* rodataSection = NULL;
    for (int i = 0; i < header.e_shnum; i++) {
        if (strcmp(sectionHeaderTableStringTable->sh_offset + sectionHeaderTable[i].sh_name, ".rodata") == 0) {
            rodataSection = &sectionHeaderTable[i];
            break;
        }
    }

    // Find the .symtab section
    Elf32_Shdr* symtabSection = NULL;
    for (int i = 0; i < header.e_shnum; i++) {
        if (strcmp(sectionHeaderTableStringTable->sh_offset + sectionHeaderTable[i].sh_name, ".symtab") == 0) {
            symtabSection = &sectionHeaderTable[i];
            break;
        }
    }

    // Construct a new, fresh stack for the new program with the exact structure as discussed in appendix C. An easy trick for saving some errorprone work is also discussed there.

    // Allocate the stack
    void* stack = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Fill the stack with 0
    memset(stack, 0, 0x1000);

    // Set the stack pointer to the top of the stack
    void* stackPointer = stack + 0x1000;

    // Push the arguments to the stack
    for (int i = 0; i < argc; i++) {
        // Push the string to the stack
        stackPointer -= strlen(argv[i]) + 1;
        memcpy(stackPointer, argv[i], strlen(argv[i]) + 1);

        // Push the pointer to the stack
        stackPointer -= sizeof(void*);
        memcpy(stackPointer, &stackPointer, sizeof(void*));
    }

    // Push the environment variables to the stack
    for (int i = 0; i < envc; i++) {
        // Push the string to the stack
        stackPointer -= strlen(envp[i]) + 1;
        memcpy(stackPointer, envp[i], strlen(envp[i]) + 1);

        // Push the pointer to the stack
        stackPointer -= sizeof(void*);
        memcpy(stackPointer, &stackPointer, sizeof(void*));
    }

    // Push the auxiliary vector to the stack
    for (int i = 0; i < auxc; i++) {
        // Push the value to the stack
        stackPointer -= sizeof(auxv[i].a_un.a_val);
        memcpy(stackPointer, &auxv[i].a_un.a_val, sizeof(auxv[i].a_un.a_val));

        // Push the type to the stack
        stackPointer -= sizeof(auxv[i].a_type);
        memcpy(stackPointer, &auxv[i].a_type, sizeof(auxv[i].a_type));
    }

    // Set the exit point (register %rdx), load the stack pointer, and jump to the entry point of the new binary (or the interpreter). Since this needs to be done in assembly, the implementation for this is already provided on Brightspace, but you will still need to call this code yourself.






}