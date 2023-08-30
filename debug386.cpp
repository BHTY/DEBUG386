#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <Windows.h>

#define AWAITING_CONSTANT 0
#define STRING_CONSTANT 1
#define DONE 2

/*
To-do List
- File I/O
- Assembler
- Disassembler
- Debugging support
    - Breakpoints (exception handler for int3 & page faults)
    - Single-stepping
    - Register dumps
    - Stack trace

Pie in the sky
- PE loader (EXEs & DLLs)
- MS-DOS version
*/


char string[256];

void* allocate_executable_memory(size_t size) {
    DWORD oldProtection;
    void* buffer = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(buffer, 0, size);

    ((uint8_t*)(buffer))[0] = 0xb8;
    ((uint8_t*)(buffer))[1] = 0xef;
    ((uint8_t*)(buffer))[2] = 0xbe;
    ((uint8_t*)(buffer))[3] = 0xad;
    ((uint8_t*)(buffer))[4] = 0xde;
    ((uint8_t*)(buffer))[5] = 0xc3;

    return buffer;
}

char* os_ver(char* str) {
    char version_number[50];
    uint32_t ver = GetVersion();
    sprintf(version_number, "Version %d.%02d", LOBYTE(ver), HIBYTE(ver));

    if (ver > 0xFFFF) {
        sprintf(string, "Windows NT %s", version_number);
    }
    else {
        sprintf(string, "Windows %s", version_number);
    }

    return str;
}

int main(int argc, char** argv)
{
    int (*fun_ptr)(void);
    char cmd;
    int c;
    size_t sz;
    int i, p, n;
    int str_index = 0;

    int enter_mode = AWAITING_CONSTANT;

    uint8_t* start;

    while (1) {
        printf("- ");
        cmd = getchar();
        
        switch (cmd) {
            case 'A':
            case 'a':
                puts("Assembler not yet implemented!");
                break;

            case 'B':
            case 'b':
                puts("Breakpoint setting not yet implemented!");
                break;

            case 'D':
            case 'd':
                scanf("%x %x", &start, &sz);

                for (p = 0; p < sz; p++) {
                    printf("%p: ", start);

                    for (i = 0; i < 16; i++) {
                        printf("%02x ", start[i]);
                    }

                    printf(" | ");

                    for (i = 0; i < 16; i++) {
                        printf("%c", start[i]);
                    }

                    p += 16;
                    start += 16;
                    printf("\n");
                }

                break;

            case 'E':
            case 'e':
                scanf("%x", &start);
                enter_mode = AWAITING_CONSTANT;

                while ((c = getchar()) != '\n' && c != EOF) {
                    switch (c) {
                        case '"':
                            if (enter_mode == AWAITING_CONSTANT) {
                                enter_mode = STRING_CONSTANT;
                            }
                            else {
                                enter_mode = DONE;
                            }

                            break;                            
                        default:
                            if (enter_mode == STRING_CONSTANT) {
                                *start = c;
                                start++;
                            }
                            else if (enter_mode == AWAITING_CONSTANT) { //push character into temp buffer
                                if (c == ',') { //existing byte to push into memory
                                    sscanf(string, "%x", &n);
                                    *start = n;
                                    memset(string, 0, 256);
                                    start++;
                                    str_index = 0;
                                    enter_mode = AWAITING_CONSTANT;
                                }
                                else if (c == '\'') { //single quote (character literal)
                                    *start = getchar();
                                    getchar();
                                    start++;
                                    enter_mode = DONE;
                                }
                                else {
                                    string[str_index] = c;
                                    str_index++;
                                }
                            }
                            else {
                                if (c == ',') {
                                    enter_mode = AWAITING_CONSTANT;
                                }
                            }
                            break;
                    }
                }

                if (enter_mode == AWAITING_CONSTANT) {//existing byte to push into memory
                    sscanf(string, "%x", &n);
                    *start = n;
                    memset(string, 0, 256);
                    start++;
                    str_index = 0;
                }

                cmd = '\n';

                break;

            case 'F':
            case 'f':
                puts("Loading from file not yet implemented!");
                break;

            case 'G':
            case 'g':
                scanf("%x", &fun_ptr);
                printf("Subroutine returned %x\n", fun_ptr());
                break;

            case 'K':
            case 'k':
                puts("Stack trace not yet implemented!");
                break;

            case 'M':
            case 'm':
                puts("Removing breakpoints not yet implemented!");
                break;

            case 'Q':
            case 'q':
                exit(0);
                break;

            case 'R':
            case 'r':
                puts("Register dumping not yet implemented!");
                break;

            case 'S':
            case 's':
                puts("Saving to file not yet implemented!");
                break;

            case 'T':
            case 't':
                puts("Single stepping not yet implemented!");
                break;

            case 'U':
            case 'u':
                puts("Disassembling not yet implemented!");
                break;
            
            case 'V':
            case 'v':
                scanf("%x", &sz);
                printf("Allocated %d byte executable memory block at %p\n", sz, allocate_executable_memory(sz));
                break;

            case 'W':
            case 'w':
                puts("Register writing not yet implemented!");
                break;

            case '?':
                printf("DEBUG386 Version 0.1 by Will Klees running on %s\n", os_ver(string));
                puts("Command List");
                puts("A [addr]: Assembles code beginning from addr");
                puts("B [addr]: Places a breakpoint at addr");
                puts("D [addr] [bytes]: Displays the specified number of bytes beginning from addr");
                puts("E [addr] [byte1, byte2, ...]: Writes bytes into memory starting from addr");
                puts("F [filename] [addr] [bytes]: Loads the specified number of bytes from the file into memory beginning at addr");
                puts("G [addr]: Begins executing code from addr");
                puts("K: Stack frame trace");
                puts("M [addr]: Removes the breakpoint at addr");
                puts("Q: Quit debugger");
                puts("R: Dump registers");
                puts("S [filename] [addr] [bytes]: Dumps the specified number of bytes from memory beginning at addr to the file");
                puts("T: Single step execution");
                puts("U [addr] [bytes]: Disassembles the specified number of bytes starting from addr");
                puts("V [bytes]: Allocates a bytes-long region of executable memory");
                puts("W [reg] [value]: Write new value to register");
                break;

            default:
                printf("For a list of debugging commands, type ? at the prompt.\n");
                break;
        }

        if (cmd != '\n') {
            while ((c = getchar()) != '\n' && c != EOF) {}
        }
    }
}