#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cpu.h"
#include "terminal.h"

void sleep_ms(long int msec){
	struct timespec sleep_time;

	sleep_time.tv_sec = msec/1000;
	sleep_time.tv_nsec = (msec%1000)*1000000;
	nanosleep(&sleep_time, NULL);
}

char* platform_file_path(char *name);

FILE* rom_file_open(char *name) {
    char *path = platform_file_path(name);
    
    if (path == NULL) {
        return NULL;
    }
    
    return fopen(path, "rb");
}

uint8_t memory[0x10000];

char str[1024];

void print_state(CPU_6502 cpu) {
    sprintf(str, "A: %02x X:%02x Y:%02x SP:%02x P:%02x PC:%02x\n", (int) cpu.A_reg, (int) cpu.X_reg, (int) cpu.Y_reg, (int) cpu.SP_reg, (int) cpu.P_reg, (int) cpu.PC_reg);
    terminal_print_state(str);

    sprintf(str, "\nNext: %02x %02x %02x\n", (int) memory[cpu.PC_reg], (int) memory[cpu.PC_reg + 1], (int) memory[cpu.PC_reg + 2]);
    terminal_print_state(str);
}

// Special read memory routine for memory-mapped I/O
uint8_t read_mem(uint16_t index){
	uint8_t output = ' ';
	
	if (index >= 0xC081 && index <= 0xC0FF) {
        output = memory[index&0xFFFE];
	}
    else {
		output = memory[index];
	}

	if (index == 0xD010) {
		memory[0xD011] &= 0x7F;
	}
     else if ((index&0xFF0F) == 0xD002) {
		output = 0;
	}
    
	return output;
}

int refresh_display = 0;

// Special write memory routine for memory mapped I/O
void write_mem(uint16_t index, uint8_t value){
	if ((index&0xFF0F) == 0xD002) {
		if ((value&0x7F) == '\n' || (value&0x7F) == '\r') {     // Print \n instead of \r
            terminal_printchar('\n');
		}
        else if((value&0x7F) == 0x5F) {                         // Make the 0x5F character map to ASCII backspace
            terminal_print("\b \b");
		}
        else if((value&0x7F) >= 0x20 && (value&0x7F) != 127) {  // Output a character
            terminal_printchar(value&0x7F);
		}
        
        refresh_display = 1;
	}

    memory[index] = value;
}

CPU_6502 cpu;

unsigned long long int last_time;
unsigned long long int last_cycles;
unsigned char last_cycle_diff;

int emulator_init(void) {
    FILE *fp;
    struct timespec current_time;
    
    cpu.A_reg = 0;
    cpu.X_reg = 0;
    cpu.Y_reg = 0;
    cpu.SP_reg = 0;
    cpu.P_reg = 0;
    cpu.PC_reg = 0xE000;
    
    terminal_init();
    
    // Load Integer Basic
    fp = rom_file_open("BASIC");
    if (fp && fread(memory + 0xE000, 1, 0x1000, fp) == 0x1000){
        fclose(fp);
    }
    else {
        if (fp) {
            fclose(fp);
        }
        
        terminal_error("Warning: could not load file named \"BASIC\".\nStarting without Apple 1 BASIC loaded\n", -1);
    }
    
    // Load Woz's ACI
    fp = rom_file_open("WOZACI");
    if (fp && fread(memory + 0xC000, 1, 0x100, fp) == 0x100){
        fclose(fp);
    }
    else {
        if (fp) {
            fclose(fp);
        }
        
        terminal_error("Warning: Could not load WOZACI due to file error", -1);
    }
    
    memcpy(memory + 0xC100, memory + 0xC000, 0x100);
    
    // Load Woz's monitor
    fp = rom_file_open("WOZMON");
    if (fp && fread(memory + 0xFF00, 1, 0x100, fp) == 0x100){
        fclose(fp);
    }
    else {
        if (fp){
            fclose(fp);
        }
        
        terminal_error("Warning: Could not load WOZMON due to file error. Stopped", -1);
        return 1;
    }
    
    reset_6502(&cpu, read_mem);
    cpu.cycles = 0;
    last_cycles = 0;
    
    // Initialize the timing
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    last_time = 0;
    last_cycle_diff = 0;

    return 0;
}

unsigned int count = 0;

int emulator_frame(void) {
    int key_hit;

    /* Limit the speed of the processor
     * based on how many cycles the last
     * instruction took. The 6502 on the
     * Apple 1 was clocked at 1 MHz.
     */
    //last_time += last_cycle_diff;
    //if (last_time > 1000){
        //sleep_ms(last_time/1000);
        //last_time -= 1000;
    //}

    // Execute the instruction
    for (int i = 0; i < 10000; i++) {
        execute_6502(&cpu, read_mem, write_mem);
        
        if (refresh_display == 1) {
            terminal_refresh();
            refresh_display = 0;
        }
    }
    
    count = count + 1;

    // Handle keyboard I/O
    if ((key_hit = terminal_getch()) != TERMINAL_ERR) {
        printf("Test key\n");
        
        if (key_hit == 0x08 || key_hit == 0x7F) {           // Emulate the backspace character
            memory[0xD010] = 0xDF;
            memory[0xD011] |= 0x80;
        }
        else if (key_hit == '~') {                          // Emulate the control character
            key_hit = terminal_getch();
            
            if (key_hit == 'd' || key_hit == 'D') {         // Ctrl-D
                memory[0xD010] = 0x84;
                memory[0xD011] |= 0x80;
            }
            else if (key_hit == 'g' || key_hit == 'G') {    // Ctrl-G (bell character)
                memory[0xD010] = 0x87;
                memory[0xD011] |= 0x80;
            }
            else if(key_hit == '`') {                       // Escape
                memory[0xD010] = 0x9B;
                memory[0xD011] |= 0x80;
            }
        }
        else if(key_hit == '|') {
            terminal_printchar('\n');
        }
        else {
            // Convert lower case characters to upper case
            if (key_hit >= 'a' && key_hit <= 'z') {
                key_hit += 'A' - 'a';
            }

            // Replace \n with \r
            if (key_hit == '\n') {
                key_hit = '\r';
            }

            memory[0xD010] = key_hit|0x80;
            memory[0xD011] |= 0x80;
        }
    }

    last_cycle_diff = cpu.cycles - last_cycles;
    
    if (refresh_display == 1) {
        terminal_refresh();
        refresh_display = 0;
    }
    
    return 0;
}
