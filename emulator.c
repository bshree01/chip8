#include <SDL2/SDL.h> // Using for keyboard inputs and display output
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// ToDo: get rid of this
bool debug_flag = false; //Make true to print opcodes and skip display

// Processor variables
unsigned char memory[4096] = {0}; //Declare memory array; initialize all to zero 
unsigned short opcode; // 2 byte opcodes
unsigned short counter; // Program counter (pointing to current opcode memory location)
unsigned short const counter_start = 0x200; // Opcodes start at 0x200 in memory
unsigned short const max_game_size = (0x1000 - 0x200); // Max game file is 0x1000; Game data starts at 0x200
unsigned short stack[16] = {0}; // Subroutine stack
unsigned short stack_point = 0; // STack pointer for subroutine stack
unsigned short V[16] = {0}; // Declare registers
unsigned short I = 0; // Index register
unsigned short pc = 0; // Program counter
unsigned char gfx[32][64] = {0}; // Graphics pixels
unsigned char delay_timer = 0; // Delay timer defines in Chip-8 documentation
unsigned char sound_timer = 0; // Sount timer; timer goes off when it reaches 0
unsigned char key[16] = {0}; //Current state of the keypad
bool escape = false; // Flag for ESC key pressed to escape operational loop
bool draw_flag = false; // Flag for updating the display
bool skip_counter = false; // Flag for skipping the counter+=2 (for when you do Jump commands)

// Keyboard variables
char key_chars[16] = {'1', '2', '3', '4', 'q', 'w', 'e', 'r', 'a', 's', 'd', 'f', 'z', 'x', 'c', 'v'}; // Input keys [16]
unsigned char chip8_fontset[80] =           // Sprites to represent 0 through F
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F 
};

// Display variables
unsigned short const pixel_size = 20;
unsigned short const screen_height = 32 * pixel_size;
unsigned short const screen_width = 64 * pixel_size;
SDL_Renderer *renderer;

// Define functions
int sdl_monitor_keystrokes(void *data);
void draw_sprite(unsigned char, unsigned char, unsigned char);
void draw();
void initialize_emulator();
int emulate(); 

// Point of entry; named WinMain to appease SDL2
int WinMain(int argc, char* argv[])
{
    //Start thread to update keys
    SDL_Thread *thread = SDL_CreateThread(sdl_monitor_keystrokes, "UpdateKeyThread", NULL);
    if (thread == NULL) {
        printf("Failed to create thread: %s\n", SDL_GetError());
        exit(1);
    }

    initialize_emulator();
    int result = emulate();
    
    //Wait for key thread to exit
    SDL_WaitThread(thread, NULL);
    return result;
}

// Use SDL2 to upday key[] when keyboard inputs are pressed; This function is called in a separate thread
int sdl_monitor_keystrokes(void *data)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) 
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    //Create a window and a renderer
    SDL_Window *window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //ToDo: Do I need to reimplement this after optimizing?
    // // Initialize timer variables
    // int start_time_key_event = SDL_GetTicks();//ToDo var type
    // int frame_time_key_event = 0;
    // int wait_time_key_event = 0;
    // int total_time_key_event = 0;

    // Monitor for key events until ESC is pressed
    while (!escape)
    {
        // Handle events
        SDL_Event event;

        // For each event, see if they are the event we care about
        while (SDL_PollEvent(&event))
        {
            //ToDo: Do I need to reimplement this after optimizing?            
            // // Calculate frame time
            // frame_time_key_event = SDL_GetTicks() - start_time_key_event;

            // // Calculate wait time to achieve 2kHz frequency
            // wait_time_key_event = 1 - frame_time_key_event; // 500 microseconds = 2kHz

            // // Wait for the remaining time
            // if (wait_time_key_event > 0) {
            //     SDL_Delay(wait_time_key_event);
            //     total_time_key_event += frame_time_key_event + wait_time_key_event;
            // } else {
            //     total_time_key_event += frame_time_key_event;
            // }
            // // Update start time for the next iteration
            // start_time_key_event = SDL_GetTicks();

            // KeyDown or KeyUp event?
            switch (event.type)
            {
                case SDL_KEYDOWN:
                    // If escape is the key pressed, escape = true (and the program terminates)
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        escape = true;
                    }
                    
                    // Keydown event
                    for (int char_index = 0; char_index < 16; char_index++)
                    {
                        // If they key pressed is from key_chars, then update the corresponding key[char_index]
                        if (event.key.keysym.sym == key_chars[char_index])
                        {
                            key[char_index] = 1;
                            break;
                        }
                    }

                    break;
                case SDL_KEYUP:
                    // Keyup event
                    for (int char_index = 0; char_index < 16; char_index++)
                    {
                        // If they key released is from key_chars, then update the corresponding key[char_index]
                        if (event.key.keysym.sym == key_chars[char_index])
                        {
                            key[char_index] = 0;
                            break;
                        }
                    }
                    break;
                default:
                    // Other event
                    break;
            }   
        }
        //ToDo: Do I need to reimplement this after optimizing? 
        // SDL_Delay(1);   //Wait 1 ms (1kHz)
    }
   
    //Quit SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

// "Draw the sprite" by updating bits in gfx
void draw_sprite(unsigned char x_start, unsigned char y_start, unsigned char byte_count)
{
    unsigned char row = y_start;
    unsigned char col = x_start;
    unsigned char bit_index;

    V[0xF] = 0; //Default collision flag to 0

    //Draw sprite, checking each pixel (bit) for collision
    for (unsigned char byte_index = 0; byte_index < byte_count; byte_index++)
    {
        unsigned char byte = memory[I + byte_index];
        for (unsigned char bit_index = 0; bit_index < 8; bit_index++)
        {
            unsigned char bit = (byte >> bit_index) & 0x1; //Current bit in the sprite
            
            //Pointer to gfx pixel (representing the pixel on screen)
            unsigned char *pixel_pointer = &gfx[(row + byte_index) % 32][(col + (7 - bit_index)) % 64];

            //Look for collisions
            if (bit == 1 && *pixel_pointer == 1)
            {
                V[0xF] = 1;
            }

            //Draw by XOR the desired with current bits
            *pixel_pointer = *pixel_pointer ^ bit;
        }
    }

}

// Draw the entire screen based on the contents of gfx
void draw()
{
    // ToDo: Optimize this function. This is where the CPU usage is going crazy
    if (!debug_flag)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // set the background color
        SDL_RenderClear(renderer); // clear the renderer
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // set the pixel color

        // loop over all pixels and draw them
        for (int y = 0; y < screen_height; y++) {
            for (int x = 0; x < screen_width; x++) {
                if (gfx[y][x]) {
                    // Each pixel of gfx becomes a rectangle (with our fine screen resolution 64x32 pixels would be tiny)
                    SDL_Rect rect = { x * pixel_size, y * pixel_size, pixel_size, pixel_size };
                    SDL_RenderFillRect(renderer, &rect); // draw the pixel
                }
            }
        }

       SDL_RenderPresent(renderer); // update the screen
    }
}

void initialize_emulator()
{
//Set font characters to 0x00 through 0x80 in memory
    for (int font_index = 0; font_index < 80; font_index++)
    {
        memory[font_index] = chip8_fontset[font_index];
    }
    
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

    //Read the file
    fread(&memory[counter_start], 1, max_game_size, rom_file);

    //Close the ROM file
    fclose(rom_file);

    //Initialize counter to ROM starting location (0x200)
    counter = counter_start;
}

// Emulation loop
int emulate() 
{
    //ToDo: Do I need to reimplement this after optimizing?
    // // Initialize timer variables
    // int start_time_processor = SDL_GetTicks();//ToDo var type
    // int frame_time_processor = 0;
    // int wait_time_processor = 0;
    // int total_time_processor = 0;

    //Define local opcode variables
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
        //ToDo: Do I need to reimplement this after optimizing?
        // // Calculate frame time
        // frame_time_processor = SDL_GetTicks() - start_time_processor;

        // // Calculate wait time to achieve 2kHz frequency
        // wait_time_processor = 500 - frame_time_processor; // 500 microseconds = 2kHz

        // // Wait for the remaining time
        // if (wait_time_processor > 0) {
        //     SDL_Delay(wait_time_processor);
        //     total_time_processor += frame_time_processor + wait_time_processor;
        // } else {
        //     total_time_processor += frame_time_processor;
        // }

        // // Update start time for the next iteration
        // start_time_processor = SDL_GetTicks();

        SDL_Delay(1);   //Wait 1 ms (1kHz)

        //opcode = two bytes at the program counter
        opcode = memory[counter] << 8 | memory[counter + 1];

        if (debug_flag)
        {
            printf("opcode %04X: %04X\n", counter, opcode); //ToDo: This is temp. Remove later
        }

        //Set variables (they won't all be used every time, but this prevents duplicate code in each opcode)
        x = (opcode & 0x0F00) >> 8;
        y = (opcode & 0x00F0) >> 4;
        KK = opcode & 0x00FF;
        N = opcode & 0x000F;
        NN = opcode & 0x00FF;
        NNN = opcode & 0x0FFF;

        //Decode opcodes
        switch (opcode >> 12)    //Look at first byte only
        {
            case 0x0:
                switch (opcode & 0x00FF)    //Look at last two bytes
                {
                    case 0x00E0:
                        //Clear screen
                        if (debug_flag) printf("case 00E0: clear the screen\n");
                        memset(gfx,0, sizeof(unsigned char) * 32 * 64);
                        draw_flag = true;
                        break;
                    case 0x00EE:
                        //Return from subroutine
                        if (debug_flag) printf("case 00EE: return from subroutine\n");
                        stack_point--;
                        counter = stack[stack_point];
                        break;
                    default:
                        if (debug_flag) printf("*********0nnn opcodes not implemented************\n");
                }
                break;
            case 0x1:
                //Jump to address NNN (0x1NNN)
                if (debug_flag) printf("case 1NNN: jump to address NNN\n");
                counter = opcode & 0x0FFF;
                skip_counter = true; //Don't execute counter += 2
                break;
            case 0x2:
                //Call subroutine at address NNN (0x2NNN)
                if (debug_flag) printf("case 2NNN: call subroutine at address NNN\n");
                stack[stack_point] = counter;
                stack_point++;
                counter = opcode & 0x0FFF;
                skip_counter = true; //Don't execute counter += 2
                break;
            case 0x3:
                //Skip next instruction if Vx = NN (0x3XNN)
                if (debug_flag) printf("case 3XNN: skip next instruction if Vx = NN\n");
                
                if (V[x] == NN)
                {
                    counter += 2;
                }
                break;
            case 0x4:
                //Skip next instruction if Vx = !NN (0x4XNN)
                if (debug_flag) printf("case 4XNN: skip next instruction if Vx != NN\n");

                if (V[x] != NN)
                {
                    counter += 2;
                }
                break;
            case 0x5:
                if ((opcode & 0x000F) == 0 )
                {
                    //Skip next instruction if Vx = Vy (0x5XY0)
                    if (debug_flag) printf("case 5XY0: skip next instruction if Vx = Vy\n");
                    
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
                if (debug_flag) printf("case 6XKK: Load KK into register at Vx\n");
                
                V[x] = KK;
                break;
            case 0x7:
                //Adds value kk to value in register Vx (0x7XKK)
                if (debug_flag) printf("case 7XKK: Vx = Vx + KK\n");

                V[x] = V[x] + KK;
                break;
            case 0x8:
                switch (opcode & 0x000F)    //Look at last byte
                {
                    case 0x0000:
                        //Set Vx = Vy
                        if (debug_flag) printf("case 8xy0: Set Vx = Vy\n");

                        V[x] = V[y];
                        break;
                    case 0x0001:
                        //Set Vx = Vx OR Vy
                        if (debug_flag) printf("case 8xy1: Set Vx = Vx OR Vy\n");

                        V[x] = V[x] | V[y];
                        break;
                    case 0x0002:
                        //Set Vx = Vx AND Vy
                        if (debug_flag) printf("case 8xy2: Set Vx = Vx AND Vy\n");

                        V[x] = V[x] & V[y];
                        break;
                    case 0x0003:
                        //Set Vx = Vx XOR Vy
                        if (debug_flag) printf("case 8xy3: Set Vx = Vx XOR Vy\n");

                        V[x] = V[x] ^ V[y];
                        break;
                    case 0x0004:
                        //Set Vx = Vx + Vy; set VF = carry (if Vx + Vy is greater than 8 bits, aka 255 VF is set to 1; otherwise VF = 0; only lowest 8 bits are kept in Vx)
                        if (debug_flag) printf("case 8xy4: Set Vx = Vx + Vy; and VF = carry\n");

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
                        if (debug_flag) printf("case 8xy5: Set Vx = Vx - Vy; VF = NOT borrow\n");

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
                        if (debug_flag) printf("case 8xy6: Set Vx = Vx >> 1; VF = LSB of Vx\n");

                        V[0xF] = V[x] & 0x1; //VF = LSB
                        V[x] = V[x] >> 1;
                        break;
                    case 0x0007:
                        //Set Vx = Vy - Vx; set VF = NOT borrow (if Vy > Vx, then VF = 1; otherwise VF = 0)
                        if (debug_flag) printf("case 8xy7: Set Vx = Vy - Vx; VF = NOT borrow\n");

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
                        if (debug_flag) printf("case 8xyE: Set Vx = Vx << 1; VF = MSB of Vx\n");

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
                    if (debug_flag) printf("case 9XY0: skip next instruction if Vx != Vy\n");

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
                if (debug_flag) printf("case Annn: Set Reg I = nnn\n");

                I = NNN;
                break;
            case 0xB:
                //Jump to location nnn + V0
                if (debug_flag) printf("case Bnnn: Jump to location nnn + V0\n");

                counter = NNN + V[0];
                skip_counter = true; //Don't execute counter += 2
                break;
            case 0xC:
                //Set Vx = random byte AND kk (0xCXKK) 
                if (debug_flag) printf("case Cxkk: Vx = random byte AND kk\n");
                unsigned char random_byte;
                srand(time(NULL)); //Seed the random generator with the current time
                random_byte = rand() % 256; // Randome number between 0 and 255

                V[x] = random_byte & KK;
                break;
            case 0xD:
                //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision (0xDXYn)
                //Sprites are XORed onto the existing screen. If this causes any pixels to be erased, VF = 1, otherwise 0.
                //Wrap-around
                if (debug_flag) printf("case Dxyn: Display n-byte sprite starting at mem location I at (Vx, Vy); \n");

                //Draw sprite
                draw_sprite(V[x], V[y], N);
                draw_flag = true;
                break;
            case 0xE:
                switch (opcode & 0x00FF)    //Look at last two bytes
                {
                    case 0x009E:
                        //Skip next instruction if the key with the value of Vx is pressed (0xEX9E)
                        if (debug_flag) printf("case EX9E: Skip next instruction if key with the value of Vx is pressed\n");

                        if (key[V[x]] != 0)
                        {
                            counter += 2;
                        }
                        break;
                    case 0x00A1:
                        //Skip next instruction if the key with the value of Vx is NOT pressed (0xEXA1)
                        if (debug_flag) printf("case EXA1: Skip next instruction if key with the value of Vx is NOT pressed\n");

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
                        if (debug_flag) printf("case FX07: Vx = delay timer value\n");

                        V[x] = delay_timer;
                        break;
                    case 0x000A:
                        //Wait for a key press, store the value of the key in Vx (0xFx0A)
                        if (debug_flag) printf("case FX0A: Wait for key press; Vx = value of the key\n");

                        unsigned short key_pressed = 0xFF; //initialize to unused value

                        //Loop until a key is pressed
                        while(key_pressed == 0xFF)
                        {
                            SDL_Delay(1);   //Wait 1 ms (1kHz)

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
                        if (debug_flag) printf("case FX15: Set delay timer to Vx\n");

                        delay_timer = V[x];
                        break;
                    case 0x0018:
                        //Set sound timer = Vx (0xFx18)
                        if (debug_flag) printf("case FX18: Set sound timer = Vx\n");

                        sound_timer = V[x];
                        break;
                    case 0x001E:
                        //Set I = I + Vx (0xFx1E)
                        if (debug_flag) printf("case FX1E: Set I = I + Vx\n");

                        I = I + V[x];
                        break;
                    case 0x0029:
                        //Set I = location of sprite for digit Vx (0xFx29)
                        if (debug_flag) printf("case FX29: Set I = location of sprite for digit Vx\n");
                
                        I = V[x] * 5; //8x5 sprites. 0 starts at 0, 1 starts at 5, etc
                        break;
                    case 0x0033:
                        //Store Binary Coded Decimal (BCD) representation of Vx in memory locations I, I+1, and I+2 (0xFx33)
                        if (debug_flag) printf("case FX33: Store Binary Coded Decimal (BCD) representation of Vx in memory locations I, I+1, and I+2\n");

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
                        if (debug_flag) printf("case FX55: Store registers V0 through Vx in memory starting at location I\n");

                        for (int v_index = 0; v_index <= x; v_index++)
                        {
                            memory[I + v_index] = V[v_index];
                        }
                        I = I + (x + 1);
                        break;
                    case 0x0065:
                        //Read registers V0 through Vx from memory starting at location I (0xFx65)
                        if (debug_flag) printf("case FX65: Read registers V0 through Vx from memory starting at location I\n");

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

        if (!skip_counter)
        {
            //Move program counter + 2
            counter += 2;
        }
        skip_counter = false;

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
            draw();
            draw_flag = false;
        }
    }

    printf("Reached end of program");
    return 0;
}
