#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <string.h>

int main() 
{
    //Declare memory array; initialize all to zero 
    unsigned char memory[4096] = {0};
        
    //0x200-0xFFF - Program ROM and work RAM

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

    //    //Seek to the memory region where ROM will be loaded
    //    fseek(rom_file,0x200, SEEK_SET);

    //    //Read the file
    //    fread(memory + 0x200,1,4096-0x200,rom_file);


    //Read the file
    fread(memory,1,4096,rom_file);

    //Close the ROM file
    fclose(rom_file);

    for(int x = 0; x < 4096; x++)
    {
        printf("%u", memory[x]);
    }

    // They are all still 0, so memory isn't getting assigned the values of rom_file properly

    printf("Reached end of program");
    return 0;
}
