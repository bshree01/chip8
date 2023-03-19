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
                    switch (opcode & 0x00FF)    //Look at last two bytes
                    {
                        case 0x00E0:
                            //Clear screen
                            printf("case 00E0: clear the screen\n");
                            break;
                        case 0x00EE:
                            //Return from subroutine
                            printf("case 00EE: return from subroutine\n");
                            break;
                        default:
                            printf("*********0nnn opcodes not implemented************");

                    }
                    break;
                case 0x1:
                    //Jump to address NNN (0x1NNN)
                    printf("case 1NNN: jump to address NNN\n");
                    break;
                case 0x2:
                    //Call subroutine at address NNN (0x2NNN)
                    printf("case 2NNN: call subroutine at address NNN\n");
                    break;
                case 0x3:
                     //Skip next instruction if Vx = NN (0x3XNN)
                    printf("case 3XNN: skip next instruction if Vx = NN\n");
                    break;
                case 0x4:
                    //Skip next instruction if Vx = !NN (0x4XNN)
                    printf("case 4XNN: skip next instruction if Vx != NN\n");
                    break;
                case 0x5:
                    if ((opcode & 0x000F) == 0 )
                    {
                         //Skip next instruction if Vx = Vy (0x5XY0)
                        printf("case 5XY0: skip next instruction if Vx = Vy\n");
                    }
                    else
                    {
                        printf("*************0x500n is undefined opcode where n != 0***************");
                    }
                    break;
                case 0x6:
                    //Load value kk into register Vx (0x6XKK)
                    printf("case 6XKK: Load KK into register at Vx");
                    break;
                case 0x7:
                    //Adds value kk to value in register Vx (0x7XKK)
                    printf("case 7XKK: Vx = Vx + KK");
                    break;
                case 0x8:
                    switch (opcode & 0x000F)    //Look at last byte
                    {
                        case 0x0000:
                            //Set Vx = Vy
                            printf("case 8xy0: Set Vx = Vy\n");
                            break;
                        case 0x0001:
                            //Set Vx = Vx OR Vy
                            printf("case 8xy1: Set Vx = Vx OR Vy\n");
                            break;
                        case 0x0002:
                            //Set Vx = Vx AND Vy
                            printf("case 8xy2: Set Vx = Vx AND Vy\n");
                            break;
                        case 0x0003:
                            //Set Vx = Vx XOR Vy
                            printf("case 8xy3: Set Vx = Vx XOR Vy\n");
                            break;
                        case 0x0004:
                            //Set Vx = Vx + Vy; set VF = carry (if Vx + Vy is greater than 8 bits, aka 255 VF is set to 1; otherwise VF = 0; only lowest 8 bits are kept in Vx)
                            printf("case 8xy4: Set Vx = Vx + Vy; and VF = carry\n");
                            break;
                        case 0x0005:
                            //Set Vx = Vx - Vy; set VF = NOT borrow (if Vx > Vy, then VF = 1; otherwise VF = 0)
                            printf("case 8xy5: Set Vx = Vx - Vy; VF = NOT borrow\n");
                            break;
                        case 0x0006:
                            //Set Vx = Vx >> 1 (aka devided by 2); VF = LSB of Vx (the one that shifts away)
                            printf("case 8xy6: Set Vx = Vx >> 1; VF = LSB of Vx\n");
                            break;
                        case 0x0007:
                            //Set Vx = Vy - Vx; set VF = NOT borrow (if Vy > Vx, then VF = 1; otherwise VF = 0)
                            printf("case 8xy7: Set Vx = Vy - Vx; VF = NOT borrow\n");
                            break;
                        case 0x000E:
                            //Set Vx = Vx << 1 (aka multiplied by 2); VF = MSB of Vx (the one that shifts away)
                            printf("case 8xyE: Set Vx = Vx << 1; VF = MSB of Vx\n");
                            break;
                        default:
                            printf("*********8nn8, 8nn9, 8nnA, 8nnB, 8nnC, 8nnD, 8nnF opcodes not implemented************");
                    }
                    break;
                case 0x9:
                    if ((opcode & 0x000F) == 0 )
                    {
                         //Skip next instruction if Vx != Vy (0x9XY0)
                        printf("case 9XY0: skip next instruction if Vx != Vy\n");
                    }
                    else
                    {
                        printf("*************0x900n is undefined opcode where n != 0***************");
                    }
                    break;
                case 0xA:
                    //Set Register I = nnn (0xAnnn)
                    //ToDo: what is I?
                    printf("case Annn: Set Reg I = nnn\n");
                    break;
                case 0xB:
                    //Jump to location nnn + V0
                    //ToDo: what is V0?
                    printf("case Bnnn: Jump to location nnn + V0\n");
                    break;
                case 0xC:
                    //Set Vx = random byte ANK kk (0xCXKK) 
                    printf("case Cxkk: Vx = random byte AND kk\n");
                    break;
                case 0xD:
                    //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision (0xDXYn)
                    //Sprites are XORed onto the existing screen. If this causes any pixels to be erased, VF = 1, otherwise 0.
                    //Wrap-around
                    //ToDo: what is I? 
                    printf("case Dxyn: Display n-byte sprite starting at mem location I at (Vx, Vy); \n");
                    break;
                case 0xE:
                    switch (opcode & 0x00FF)    //Look at last two bytes
                    {
                        case 0x009E:
                            //Skip next instruction if the key with the value of Vx is pressed (0xEX9E)
                            printf("case EX9E: Skip next instruction if key with the value of Vx is pressed\n");
                            break;
                        case 0x00A1:
                            //Skip next instruction if the key with the value of Vx is NOT pressed (0xEXA1)
                            printf("case EXA1: Skip next instruction if key with the value of Vx is NOT pressed\n");
                            break;
                        default:
                            printf("*********0xEnnn opcodes not implemented besides 0xEX9E and 0xEXA1************");

                    }
                    break;
                case 0xF:
                    switch (opcode & 0x00FF)    //Look at last two bytes
                    {
                        case 0x0007:
                            //Set Vx = delay timer value (0xFx07)
                            printf("case FX07: Vx = delay timer value\n");
                            break;
                        case 0x000A:
                            //Wait for a key press, store the value of the key in Vx (0xFx0A)
                            printf("case FX0A: Wait for key press; Vx = value of the key\n");
                            break;
                        case 0x0015:
                            //Set delay timer = Vx (0xFx15)
                            printf("case FX15: Set delay timer to Vx\n");
                            break;
                        case 0x0018:
                            //Set sound timer = Vx (0xFx18)
                            printf("case FX18: Set sound timer = Vx\n");
                            break;
                        case 0x001E:
                            //Set I = I + Vx (0xFx1E)
                            printf("case FX1E: Set I = I + Vx\n");
                            break;
                        case 0x0029:
                            //Set I = location of sprite for digit Vx (0xFx29)
                            printf("case FX29: Set I = location of sprite for digit Vx\n");
                            break;
                        case 0x0033:
                            //Store Binary Coded Decimal (BCD) representation of Vx in memory locations I, I+1, and I+2 (0xFx33)
                            printf("case FX33: Store Binary Coded Decimal (BCD) representation of Vx in memory locations I, I+1, and I+2\n");
                            break;
                        case 0x0055:
                            //Store registers V0 through Vx in memory starting at location I (0xFx55)
                            printf("case FX55: Store registers V0 through Vx in memory starting at location I\n");
                            break;
                        case 0x0065:
                            //Read registers V0 through Vx from memory starting at location I (0xFx65)
                            printf("case FX65: Read registers V0 through Vx from memory starting at location I\n");
                            break;
                        default:
                            printf("*********0xF0nn opcodes not implemented besides 0xF007, 0xF00A, 0xF015, 0xF018, 0xF01E, 0xF029, 0xF033, 0xF055, 0xF065************");
                    }
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
