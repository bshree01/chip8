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
        }
    }


    printf("Reached end of program");
    return 0;
}
