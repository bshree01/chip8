#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <string.h>

    //Declare memory array; initialize all to zero 
    unsigned char memory[4096] = {0};

    unsigned short opcode; //2 bytes
    unsigned short counter;
    unsigned short const counter_start = 0x200;

int main() 
{
    //Location of example Game ROM
    char file_name[] = ".\\roms\\games\\Space Invaders [David Winter].ch8";

    //Open the ROM file ("rb" means read mode, binary)
    FILE* rom_file = fopen(file_name, "rb");

    //Test for valid file
    if (rom_file == NULL)
    {
        printf("Error! Could not open the file");
        exit(-1); 
    }

    //Seek to counter_start (where the ROM load needs to begin)
    fseek(rom_file, counter_start, SEEK_SET);

    //Read the file
    fread(memory + counter_start, 1, 4096 - counter_start, rom_file);

    //Close the ROM file
    fclose(rom_file);

    //Initialize counter to ROM starting location (0x200)
    counter = counter_start;
        
    //0x200-0xFFF - Program ROM and work RAM

    //Temporary: look at every opcode in order
    for(int x = 0; x < 4096; x += 2)
    {
        //opcode = two bytes at the program counter
        opcode = memory[counter] << 8 | memory[counter + 1];

        //Move program counter + 2
        counter += 2;

        //Print all non-zero opcodes
        if (opcode != 0)
        {
            printf("opcode%i: %x\n", 1+x/2, opcode);
            //Decode opcodes
            switch (opcode >> 12)    //Look at first byte only
            {
                case 0x0:
                    printf("case 0\n");
                    break;
                case 0x1:
                    printf("case 1\n");
                    break;
                case 0x2:
                    printf("case 2\n");
                    break;
                case 0x3:
                    printf("case 3\n");
                    break;
                case 0x4:
                    printf("case 4\n");
                    break;
                case 0x5:
                    printf("case 5\n");
                    break;
                case 0x6:
                    printf("case 6\n");
                    break;
                case 0x7:
                    printf("case 7\n");
                    break;
                case 0x8:
                    printf("case 8\n");
                    break;
                case 0x9:
                    printf("case 9\n");
                    break;
                case 0xA:
                    printf("case A\n");
                    break;
                case 0xB:
                    printf("case B\n");
                    break;
                case 0xC:
                    printf("case C\n");
                    break;
                case 0xD:
                    printf("case D\n");
                    break;
                case 0xE:
                    printf("case E\n");
                    break;
                case 0xF:
                    printf("case F\n");
                    break;
                default:
                    printf("Oops, unknown opcode\n");
                    break;
            }

        }
    }


    printf("Reached end of program");
    return 0;
}
