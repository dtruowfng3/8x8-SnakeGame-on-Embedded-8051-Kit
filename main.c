#include "matrix.h"
#include <INTRINS.h>
#include <REG51.h>

#define COMMONPORTS P0

#define GPIO_KEY P1

sbit PAUSE = P3^3;    // Button for PAUSE

unsigned char snake[64];
unsigned char food;
unsigned char direction = 3; // 0: Up, 1: Down, 2: Left, 3: Right
unsigned char snake_length = 3;
unsigned char display_buffer[8];

unsigned char KeyValue; 
bit is_paused = 0;
bit is_game_over = 0;

void KeyDown(void)
{
    char a = 0;
    GPIO_KEY = 0x0F;  
    
    if (GPIO_KEY != 0x0F) {  
        
        GPIO_KEY = 0x0F; 

        switch (GPIO_KEY) {
            case 0x07: KeyValue = 0; break;  
            case 0x0B: KeyValue = 1; break;  
            case 0x0D: KeyValue = 2; break;  
            case 0x0E: KeyValue = 3; break;  
            default: KeyValue = 10; break;   
        }

        GPIO_KEY = 0xF0;
        switch (GPIO_KEY) {
            case 0x70: KeyValue += 0; break;  
            case 0xB0: KeyValue += 4; break; 
            case 0xD0: KeyValue += 8; break; 
            case 0xE0: KeyValue += 12; break; 
            default: KeyValue = 10; break;    
        }
        
        while ((a < 50) && (GPIO_KEY != 0xF0)) {
            a++;
        }
    }
}

unsigned char random() {
    static unsigned char seed = 0; 
    seed = (seed * 1103515245 + 12345) & 0x3F;  
    return seed; 
}

unsigned char generate_random_food_position() {
    unsigned char random_position = random();  
    return random_position;
}


void init_game() {
    unsigned char i;
	
    for (i = 0; i < 64; i++) {
        snake[i] = 0;
    }
    snake[0] = 28;

    food = generate_random_food_position();
    
    while (snake[food] != 0) {
        food = generate_random_food_position();
    }

    snake_length = 2;
		is_game_over = 0;
}

void update_direction() {
    KeyDown(); 

    if (KeyValue == 0 && direction != 1) {  
        direction = 0; 
    } else if (KeyValue == 1 && direction != 0) {  
        direction = 1; 
    } else if (KeyValue == 2 && direction != 3) { 
        direction = 2; 
    } else if (KeyValue == 3 && direction != 2) { 
        direction = 3; 
    }
}


void update_snake() {
    unsigned char i;
    update_direction();
    for (i = snake_length; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    if (direction == 0) { // Up
        snake[0] = (snake[0] - 8 + 64) % 64;
    }
    else if (direction == 1) { // Down
        snake[0] = (snake[0] + 8) % 64;
    }
    else if (direction == 2) { // Left
        if (snake[0] % 8 == 0) {
            snake[0] += 7;
        } else {
            snake[0] -= 1;
        }
    }
    else if (direction == 3) { // Right
        if (snake[0] % 8 == 7) {
            snake[0] -= 7;
        } else {
            snake[0] += 1;
        }
    }
}

void display_game_over() {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        display_buffer[i] = 0x00; // Xóa buffer
    }
		
		display_buffer[2] = 0x22; 
		display_buffer[3] = 0x14;
		display_buffer[4] = 0x08;
		display_buffer[5] = 0x14;		
    display_buffer[6] = 0x22; 

    for (i = 0; i < 8; i++) {
        Hc595SendByte(0x00);
        COMMONPORTS = TAB[i];
        Hc595SendByte(display_buffer[i]);
        delay(1);
    }
}

void delay_2() {
    unsigned int i;
    for (i = 0; i < 100; i++) {
				display_game_over();
        delay(1);
    }
}

void check_collision() {
    unsigned char i;

    for (i = 1; i < snake_length; i++) {
        if (snake[0] == snake[i]) {
						is_game_over = 1; 
						delay_2();	
						init_game();  
            return; 
        }
    }

    if (snake[0] == food) {
        snake_length++;  
        food = generate_random_food_position();  

        while (snake[food] != 0) {
            food = generate_random_food_position();
        }
    }
}

void update_display_buffer() {
    unsigned char i, row, col;
    for (i = 0; i < 8; i++) {
        display_buffer[i] = 0x00;
    }
    for (i = 0; i < snake_length; i++) {
        row = snake[i] / 8;
        col = snake[i] % 8;
        display_buffer[row] |= (1 << col);
    }
    row = food / 8;
    col = food % 8;
    display_buffer[row] |= (1 << col);
}

void display_snake() {
    unsigned char tab;
    for(tab = 0; tab < 8; tab++) {
        Hc595SendByte(0x00);
        COMMONPORTS = TAB[tab];
        Hc595SendByte(display_buffer[tab]);
        delay(1);
    }
}

void display_pause_message() {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        display_buffer[i] = 0x00; // Xóa buffer
    }
		
    display_buffer[3] = 0x3E; 
    display_buffer[4] = 0x1C; 
    display_buffer[5] = 0x08;
			

    for (i = 0; i < 8; i++) {
        Hc595SendByte(0x00);
        COMMONPORTS = TAB[i];
        Hc595SendByte(display_buffer[i]);
        delay(1);
    }
}

void main() {
    unsigned int i;

    init_game();  

    while (1) {
        if (PAUSE == 0) {
            is_paused = !is_paused; 
            while (PAUSE == 0);
        }

        if (!is_paused) {
            if (!is_game_over) {
                update_snake();  
                check_collision();  
                update_display_buffer();  

                for (i = 0; i < 35; i++) {  
                    display_snake();
                }
            }  
        } else {
            display_pause_message();
        }
    }
}