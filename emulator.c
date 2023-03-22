#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>

#define KEY_ESC 0x1B

    //Declare memory array; initialize all to zero 
    unsigned char memory[4096] = {0};

    unsigned short opcode; //2 bytes
    unsigned short counter;
    unsigned short const counter_start = 0x200;
    
    unsigned short stack[16] = {0};
    unsigned short stack_point = 0;
    
    
    unsigned short V[16] = {0}; //Declare registers
    unsigned short I = 0; //Index register
    unsigned short pc = 0; //Program counter

    unsigned char gfx[64 * 32]; //Graphics pixels //ToDo: initial value?
    unsigned char delay_timer = 0;
    unsigned char sound_timer = 0;
    unsigned char key[16] = {0}; //Current state of the keypad

    char key_chars[16] = {'1', '2', '3', '4', 'Q', 'W', 'E', 'R', 'A', 'S', 'D', 'F', 'Z', 'X', 'C', 'V'};
    bool escape = false;

DWORD WINAPI update_key(LPVOID lpParam)
{
    while (!escape)
    {
        for (int char_index = 0; char_index < 16; char_index++)
        {
            if (GetAsyncKeyState(key_chars[char_index]) & 0x8000)
            {
                key[char_index] = 1;
            }
            else
            {
                key[char_index] = 0;
            }
        }

        //Check for escape key pressed
        if (GetAsyncKeyState(KEY_ESC))
        {
            escape = true;
        }
    }
    return 0;
}


int main() 
{
    //Location of example Game ROM
    char file_name[] = ".\\roms\\games\\Space Invaders [David Winter].ch8"; //ToDo: Add user input to select other games

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

    //Flag for updating the display
    bool draw_flag = false;
        
    //Start thread to update keys
    DWORD dwThreadId;
    HANDLE key_thread = CreateThread(NULL, 0, update_key, NULL, 0, &dwThreadId);
    if (key_thread == NULL)
    {
        printf("Failed to create thread\n");
        return 1;
    }

    //Define local variables
    unsigned short x;
    unsigned short y;
    unsigned short NN;
    unsigned short KK;
    unsigned short NNN;
    unsigned short not_borrow;
    unsigned short N;

    //Loop through until exit criteria breaks out of cycle
    while(!escape)
    {
        //opcode = two bytes at the program counter
        opcode = memory[counter] << 8 | memory[counter + 1];

        printf("opcode: %x\n", opcode); //ToDo: This is temp. Remove later
        //Decode opcodes
        switch (opcode >> 12)    //Look at first byte only
        {
            case 0x0:
                switch (opcode & 0x00FF)    //Look at last two bytes
                {
                    case 0x00E0:
                        //Clear screen
                        printf("case 00E0: clear the screen\n");
                        //ToDo: set gfx to clear state

                        draw_flag = true;
                        break;
                    case 0x00EE:
                        //Return from subroutine
                        printf("case 00EE: return from subroutine\n");
                        stack_point--;
                        counter = stack[stack_point];
                        break;
                    default:
                        printf("*********0nnn opcodes not implemented************\n");

                }
                break;
            case 0x1:
                //Jump to address NNN (0x1NNN)
                printf("case 1NNN: jump to address NNN\n");
                counter = opcode & 0x0FFF;
                break;
            case 0x2:
                //Call subroutine at address NNN (0x2NNN)
                printf("case 2NNN: call subroutine at address NNN\n");
                stack[stack_point] = counter;
                stack_point++;
                counter = opcode & 0x0FFF;
                break;
            case 0x3:
                //Skip next instruction if Vx = NN (0x3XNN)
                printf("case 3XNN: skip next instruction if Vx = NN\n");
                x = opcode & 0x0F00 >> 8;
                NN = opcode & 0x00FF;
                if (V[x] == NN)
                {
                    counter += 2;
                }
                break;
            case 0x4:
                //Skip next instruction if Vx = !NN (0x4XNN)
                printf("case 4XNN: skip next instruction if Vx != NN\n");
                x = opcode & 0x0F00 >> 8;
                NN = opcode & 0x00FF;
                if (V[x] != NN)
                {
                    counter += 2;
                }
                break;
            case 0x5:
                if ((opcode & 0x000F) == 0 )
                {
                    //Skip next instruction if Vx = Vy (0x5XY0)
                    printf("case 5XY0: skip next instruction if Vx = Vy\n");
                    x = opcode & 0x0F00 >> 8;
                    y = opcode & 0x00F0 >> 4;
                    if (V[x] == V[y])
                    {
                        counter += 2;
                    }
                }
                else
                {
                    printf("*************0x500n is undefined opcode where n != 0***************\n");
                }
                break;
            case 0x6:
                //Load value kk into register Vx (0x6XKK)
                printf("case 6XKK: Load KK into register at Vx\n");
                x = opcode & 0x0F00 >> 8;
                KK = opcode & 0x00FF;
                V[x] = KK;
                break;
            case 0x7:
                //Adds value kk to value in register Vx (0x7XKK)
                printf("case 7XKK: Vx = Vx + KK\n");
                x = opcode & 0x0F00 >> 8;
                KK = opcode & 0x00FF;
                V[x] = V[x] + KK;
                break;
            case 0x8:
                switch (opcode & 0x000F)    //Look at last byte
                {
                    case 0x0000:
                        //Set Vx = Vy
                        printf("case 8xy0: Set Vx = Vy\n");
                        x = opcode & 0x0F00 >> 8;
                        y = opcode & 0x00F0 >> 4;
                        V[x] = V[y];
                        break;
                    case 0x0001:
                        //Set Vx = Vx OR Vy
                        printf("case 8xy1: Set Vx = Vx OR Vy\n");
                        x = opcode & 0x0F00 >> 8;
                        y = opcode & 0x00F0 >> 4;
                        V[x] = V[x] | V[y];
                        break;
                    case 0x0002:
                        //Set Vx = Vx AND Vy
                        printf("case 8xy2: Set Vx = Vx AND Vy\n");
                        x = opcode & 0x0F00 >> 8;
                        y = opcode & 0x00F0 >> 4;
                        V[x] = V[x] & V[y];
                        break;
                    case 0x0003:
                        //Set Vx = Vx XOR Vy
                        printf("case 8xy3: Set Vx = Vx XOR Vy\n");
                        x = opcode & 0x0F00 >> 8;
                        y = opcode & 0x00F0 >> 4;
                        V[x] = V[x] ^ V[y];
                        break;
                    case 0x0004:
                        //Set Vx = Vx + Vy; set VF = carry (if Vx + Vy is greater than 8 bits, aka 255 VF is set to 1; otherwise VF = 0; only lowest 8 bits are kept in Vx)
                        printf("case 8xy4: Set Vx = Vx + Vy; and VF = carry\n");
                        x = opcode & 0x0F00 >> 8;
                        y = opcode & 0x00F0 >> 4;
                        if (V[y] > (0xFF - V[x])) //If Vy > 255 - Vx, then you'll have to carry
                        {
                            V[0xF] = 1; //Carry
                        } 
                        else
                        {
                            V[0xF] = 0; //No carry
                        }
                        V[x] = V[x] + V[y];
                        break;
                    case 0x0005:
                        //Set Vx = Vx - Vy; set VF = NOT borrow (if Vx > Vy, then VF = 1; otherwise VF = 0)
                        printf("case 8xy5: Set Vx = Vx - Vy; VF = NOT borrow\n");
                        x = opcode & 0x0F00 >> 8;
                        y = opcode & 0x00F0 >> 4;
                        not_borrow = 0;
                        if (V[x] > V[y]) //If Vx > Vy, then you won't get negative and therefore won't have to borrow; then VF = NOT borrow
                        {
                            not_borrow = 1;
                        } 
                        V[0xF] = not_borrow;
                        V[x] = V[x] - V[y];
                        break;
                    case 0x0006:
                        //Set Vx = Vx >> 1 (aka divided by 2); VF = LSB of Vx (the one that shifts away)
                        printf("case 8xy6: Set Vx = Vx >> 1; VF = LSB of Vx\n");
                        x = opcode & 0x0F00 >> 8;
                        y = opcode & 0x00F0 >> 4;
                        V[0xF] = V[x] & 0x1; //VF = LSB
                        V[x] = V[x] >> 1;
                        break;
                    case 0x0007:
                        //Set Vx = Vy - Vx; set VF = NOT borrow (if Vy > Vx, then VF = 1; otherwise VF = 0)
                        printf("case 8xy7: Set Vx = Vy - Vx; VF = NOT borrow\n");
                        x = opcode & 0x0F00 >> 8;
                        y = opcode & 0x00F0 >> 4;
                        not_borrow = 0;
                        if (V[y] > V[x]) //If Vy > Vx, then you won't get negative and therefore won't have to borrow; then VF = NOT borrow
                        {
                            not_borrow = 1;
                        } 
                        V[0xF] = not_borrow;
                        V[x] = V[y] - V[x];
                        break;
                    case 0x000E:
                        //Set Vx = Vx << 1 (aka multiplied by 2); VF = MSB of Vx (the one that shifts away)
                        printf("case 8xyE: Set Vx = Vx << 1; VF = MSB of Vx\n");
                        x = opcode & 0x0F00 >> 8;
                        y = opcode & 0x00F0 >> 4;
                        V[0xF] = V[x] & 0x8; //VF = MSB
                        V[x] = V[x] << 1;
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
                    x = opcode & 0x0F00 >> 8;
                    y = opcode & 0x00F0 >> 4;
                    if (V[x] != V[y])
                    {
                        counter += 2;
                    }
                }
                else
                {
                    printf("*************0x900n is undefined opcode where n != 0***************\n");
                }
                break;
            case 0xA:
                //Set Register I = nnn (0xAnnn)
                printf("case Annn: Set Reg I = nnn\n");
                NNN = opcode & 0x0FFF;
                I = NNN;
                break;
            case 0xB:
                //Jump to location nnn + V0
                printf("case Bnnn: Jump to location nnn + V0\n");
                NNN = opcode & 0x0FFF;
                counter = NNN + V[0];
                break;
            case 0xC:
                //Set Vx = random byte AND kk (0xCXKK) 
                printf("case Cxkk: Vx = random byte AND kk\n");
                unsigned char random_byte;
                srand(time(NULL)); //Seed the random generator with the current time
                random_byte = rand() % 256; // Randome number between 0 and 255
                x = opcode & 0x0F00 >> 8;
                KK = opcode & 0x00FF;
                V[x] = random_byte & KK;
                break;
            case 0xD:
                //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision (0xDXYn)
                //Sprites are XORed onto the existing screen. If this causes any pixels to be erased, VF = 1, otherwise 0.
                //Wrap-around
                printf("case Dxyn: Display n-byte sprite starting at mem location I at (Vx, Vy); \n");

                x = opcode & 0x0F00 >> 8;
                y = opcode & 0x00F0 >> 4;
                N = opcode & 0x000F;

                //ToDo: See display instructions above

                break;
            case 0xE:
                switch (opcode & 0x00FF)    //Look at last two bytes
                {
                    case 0x009E:
                        //Skip next instruction if the key with the value of Vx is pressed (0xEX9E)
                        printf("case EX9E: Skip next instruction if key with the value of Vx is pressed\n");
                        x = opcode & 0x0F00 >> 8;
                        if (key[V[x]] != 0)
                        {
                            counter += 2;
                        }
                        break;
                    case 0x00A1:
                        //Skip next instruction if the key with the value of Vx is NOT pressed (0xEXA1)
                        printf("case EXA1: Skip next instruction if key with the value of Vx is NOT pressed\n");
                        x = opcode & 0x0F00 >> 8;
                        if (key[V[x]] == 0)
                        {
                            counter += 2;
                        }
                        break;
                    default:
                        printf("*********0xEnnn opcodes not implemented besides 0xEX9E and 0xEXA1************\n");

                }
                break;
            case 0xF:
                switch (opcode & 0x00FF)    //Look at last two bytes
                {
                    case 0x0007:
                        //Set Vx = delay timer value (0xFx07)
                        printf("case FX07: Vx = delay timer value\n");
                        x = opcode & 0x0F00 >> 8;
                        V[x] = delay_timer;
                        break;
                    case 0x000A:
                        //Wait for a key press, store the value of the key in Vx (0xFx0A)
                        printf("case FX0A: Wait for key press; Vx = value of the key\n");
                        x = opcode & 0x0F00 >> 8;
                        unsigned short key_pressed = 0xFF; //initialize to unused value

                        //Loop until a key is pressed
                        while(key_pressed == 0xFF)
                        {
                            //Figure out which key was pressed
                            for (int key_index = 0; key_index < 16; key_index++)
                            {
                                key_pressed = key_index;
                                break;
                            }
                        }
                        V[x] = key_pressed;
                        break;
                    case 0x0015:
                        //Set delay timer = Vx (0xFx15)
                        printf("case FX15: Set delay timer to Vx\n");
                        x = opcode & 0x0F00 >> 8;
                        delay_timer = V[x];
                        break;
                    case 0x0018:
                        //Set sound timer = Vx (0xFx18)
                        printf("case FX18: Set sound timer = Vx\n");
                        x = opcode & 0x0F00 >> 8;
                        sound_timer = V[x];
                        break;
                    case 0x001E:
                        //Set I = I + Vx (0xFx1E)
                        printf("case FX1E: Set I = I + Vx\n");
                        x = opcode & 0x0F00 >> 8;
                        I = I + V[x];
                        break;
                    case 0x0029:
                        //Set I = location of sprite for digit Vx (0xFx29)
                        printf("case FX29: Set I = location of sprite for digit Vx\n");
                        x = opcode & 0x0F00 >> 8;
                        //To Do: set sprite location
                        I = V[x] * 5; //8x5 sprites. 0 starts at 0, 1 starts at 5, etc
                        break;
                    case 0x0033:
                        //Store Binary Coded Decimal (BCD) representation of Vx in memory locations I, I+1, and I+2 (0xFx33)
                        printf("case FX33: Store Binary Coded Decimal (BCD) representation of Vx in memory locations I, I+1, and I+2\n");
                        x = opcode & 0x0F00 >> 8;
                        unsigned short value = V[x];
                        unsigned short hundreds = value / 100;
                        unsigned short tens = (value % 100) / 10;
                        unsigned short ones = value % 10;

                        memory[I] = hundreds;
                        memory[I+1] = tens;
                        memory[I+2] = ones;

                        // Move address register
                        I += 3;

                        break;
                    case 0x0055:
                        //Store registers V0 through Vx in memory starting at location I (0xFx55)
                        printf("case FX55: Store registers V0 through Vx in memory starting at location I\n");
                        x = opcode & 0x0F00 >> 8;
                        for (int v_index = 0; v_index <= x; v_index++)
                        {
                            memory[I + v_index] = V[v_index];
                        }
                        I = I + (x + 1);
                        break;
                    case 0x0065:
                        //Read registers V0 through Vx from memory starting at location I (0xFx65)
                        printf("case FX65: Read registers V0 through Vx from memory starting at location I\n");
                        x = opcode & 0x0F00 >> 8;
                        for (int v_index = 0; v_index <= x; v_index++)
                        {
                            V[v_index] = memory[I + v_index];
                        }
                        I = I + (x + 1);
                        break;
                    default:
                        printf("*********0xF0nn opcodes not implemented besides 0xF007, 0xF00A, 0xF015, 0xF018, 0xF01E, 0xF029, 0xF033, 0xF055, 0xF065************\n");
                }
                break;
            default:
                printf("Oops, unknown opcode\n");
                break;
        }

        //Move program counter + 2
        counter += 2;

        //Update timers
        if (delay_timer > 0)
        {
            delay_timer--;
        }

        if(sound_timer > 0)
        {
            if(sound_timer == 0)
            {
                //ToDo: Implement Beep
            }
            sound_timer--;
        }

        //If the draw flag is set, update the screen
        if (draw_flag)
        {
            //ToDo: update the screen

            draw_flag = false;
        }

        //Wait (to emulate 60 Hz)
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 1000000000 / 60;
        nanosleep(&ts, NULL);

    }





    //Wait for key thread to exit
    WaitForSingleObject(key_thread, INFINITE);

    //Clean up thread
    CloseHandle(key_thread);

    printf("Reached end of program");
    return 0;
}
