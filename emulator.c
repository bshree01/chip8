#include <stdio.h>

int main() 
{
   unsigned char memory[4096] = {0};
   
   //assign memory to be all zeros to see if rom is actually getting loaded

   //Location of Game ROM
   char file_name[] = "\\roms\\games\\Space Invaders [David Winter].ch8";
   
   //Open the ROM file ("rb" means read mode, binary)
   FILE* rom_file = fopen(file_name, "rb");
   
   //Read the file
   fread(memory,1,4096,rom_file);

   for(int x = 0; x < 4096; x++)
   {
      printf("%u", memory[0]);
   }
   
   // They are all still 0, so memory isn't getting assigned the values of rom_file properly

   printf("Reached end of program");
   return 0;
}
