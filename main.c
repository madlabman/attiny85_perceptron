/*
 * File:   main.c
 * Author: naito
 *
 * Created on April 1, 2020, 4:42 PM
 */

#define F_CPU 8000000UL // 8 MHz internal

//#include <xc.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>

#define  setbit(var, bit)    ((var) |= (1 << (bit)))
#define  clrbit(var, bit)    ((var) &= ~(1 << (bit)))

#define LED         4

#define PIN_SER     PB0     // Serial Input
#define PIN_RCLK    PB1     // Register Clock / Latch
#define PIN_SRCLK   PB2     // Shift Register Clock

unsigned char number = 0;           // Generated number to show
unsigned char wrong_number_mask;    // Mask of segments with mistake

// Segments
unsigned char const segments[10] PROGMEM = {
    0b01111110,
    0b00011000,
    0b10110110,
    0b10111100,
    0b11011000,
    0b11101100,
    0b11101110,
    0b00111000,
    0b11111110,
    0b11111100
};
// Hidden layer 15 neurons with 7 inputs
float const hidden_layer[105] PROGMEM = {
    0.16341859,
    0.5233046,
    0.47488767,
    0.6155165,
    0.25351578,
    0.48711824,
    0.97987974,
    0.626667,
    0.43113178,
    0.8728607,
    0.52054495,
    0.5449289,
    0.49932337,
    0.59050745,
    0.1638807,
    0.6694049,
    0.8521577,
    0.3107742,
    0.67062527,
    0.74838394,
    0.4389661,
    0.4763521,
    0.8249926,
    0.08502656,
    0.8713649,
    0.18841368,
    0.45856822,
    0.9797345,
    0.92911536,
    0.13239092,
    0.7831139,
    0.030230582,
    0.2581221,
    0.8633682,
    0.0012845993,
    0.21054268,
    0.7501363,
    0.4712528,
    0.13910532,
    0.17934132,
    0.26610994,
    0.12320602,
    0.55890965,
    0.8709482,
    0.57598996,
    0.80687517,
    0.66053367,
    0.81106544,
    0.7132157,
    0.84442675,
    0.7364616,
    0.19936663,
    0.17723691,
    0.43082,
    0.27899277,
    0.27576452,
    0.22149217,
    0.5737602,
    0.4120279,
    0.46194357,
    0.09794861,
    0.81253606,
    0.49194527,
    0.05985266,
    0.25376815,
    0.96616554,
    0.7972745,
    0.7780998,
    0.29032558,
    0.032227576,
    0.7534688,
    0.987557,
    0.501383,
    0.81433403,
    0.4664886,
    0.2532966,
    0.8310595,
    0.87923825,
    0.8963969,
    0.9990371,
    0.24076527,
    0.51936483,
    0.18027991,
    0.96634984,
    0.9333161,
    0.83871055,
    0.45238495,
    0.94107676,
    0.21204579,
    0.6608586,
    0.58787197,
    0.9202483,
    0.24066085,
    0.67513543,
    0.37659132,
    0.47848785,
    0.15530485,
    0.08194286,
    0.821376,
    0.331577,
    0.79074216,
    0.6692132,
    0.58459085,
    0.31307197,
    0.79375005
};
// Output layer 4 neurons with 15 inputs
float const output_layer[60] PROGMEM = {
    8.098582,
    8.723989,
    43.61337,
    32.542564,
    14.514653,
    -1.0342326,
    -20.847263,
    -16.544214,
    -41.47695,
    -7.365404,
    47.408962,
    -47.257042,
    -61.201145,
    28.724209,
    13.2434,
    -12.450257,
    23.628258,
    -3.9352498,
    -51.597954,
    -12.226936,
    -24.610945,
    13.796694,
    -65.521965,
    44.244183,
    -31.855234,
    -7.931867,
    63.10617,
    32.607647,
    11.257879,
    12.409513,
    -57.295475,
    15.352164,
    16.212158,
    -75.635284,
    -38.214024,
    30.044065,
    45.401295,
    -35.86614,
    2.5702903,
    -47.329697,
    27.093422,
    80.61391,
    30.219122,
    -16.313398,
    11.109878,
    66.575775,
    -17.516003,
    -7.2000203,
    87.199104,
    18.054693,
    16.730629,
    -76.54067,
    44.61119,
    -14.112023,
    0.8222234,
    -30.780258,
    -45.36784,
    -52.30533,
    43.68796,
    -17.567297
};

float sigmoid_function(float arg) {
    return 1 / (1 + pow(M_E, -arg));
}

char next_rand() {
    ADCSRA |= (1 << ADSC); 
    srand(ADCH);
    return rand();
}

void setup(void) {
    DDRB   = 0xFF;  // B port as output
    PORTB  = 0x00;  // RB0-6 to LOW
    // https://www.hackster.io/Ned_zib/getting-started-with-adc-and-serial-attiny85-0db9f6
    clrbit(DDRB, PB3);  // PB3 as input
    ADMUX  =
           (1 << ADLAR) |     // left shift result
           (0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
           (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
           (0 << MUX3)  |     // use ADC3 for input (PB3), MUX bit 3
           (0 << MUX2)  |     // use ADC3 for input (PB3), MUX bit 2
           (1 << MUX1)  |     // use ADC3 for input (PB3), MUX bit 1
           (1 << MUX0);       // use ADC3 for input (PB3), MUX bit 0
    ADCSRA = 
           (1 << ADEN)  |     // Enable ADC 
           (1 << ADPS2) |     // set prescaler to 64, bit 2 
           (1 << ADPS1) |     // set prescaler to 64, bit 1 
           (0 << ADPS0);      // set prescaler to 64, bit 0 
}

void send_byte_to_register(unsigned char byte) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        if ((byte >> (8 - (i + 1))) & 1) {  // MSB order
            setbit(PORTB, PIN_SER);  
        } else {
            clrbit(PORTB, PIN_SER);
        }
        clrbit(PORTB, PIN_SRCLK);
        setbit(PORTB, PIN_SRCLK);
    }
    clrbit(PORTB, PIN_RCLK);
    setbit(PORTB, PIN_RCLK);
}

void generate_number(void) {
    unsigned char buf = 0;
    do {
        buf = next_rand() % 10;
    } while (number == buf);
    number = buf;    
    send_byte_to_register(pgm_read_byte_near(segments + number));
}

void simulate_mistake(void) {
    wrong_number_mask = pgm_read_byte_near(segments + number);
    do {
        setbit(wrong_number_mask, next_rand() % 7 + 1);
        clrbit(wrong_number_mask, next_rand() % 7 + 1);
    } while (wrong_number_mask == pgm_read_byte_near(segments + number));
    send_byte_to_register(wrong_number_mask);
}

void predict_number(void) {
    float buf;                  // Buffer for sum
    unsigned char result = 0;   // Store 4-bit representation of predicted number
    unsigned char i, k;         // For loop
    float out_sum[4] = { 0 };   // Output of MLP buffer
    // Populate hidden layer
    for (i = 0; i < 15; i++) {
        buf = 0;
        for (k = 0; k < 7; k++) {
            if (wrong_number_mask >> (8 - (k + 1)) & 1) {
                buf += pgm_read_float_near(hidden_layer + i * 7 + k);
            }
        }
        buf = sigmoid_function(buf);
        for (k = 0; k < 4; k++) {
            out_sum[k] += pgm_read_float_near(output_layer + k * 15 + i) * buf;
        }
    }
    // Populate output layer
    for (i = 0; i < 4; i++) {
        if (sigmoid_function(out_sum[i]) > 0.5) {
            setbit(result, i);
        }
    }
    // Display result
    if (result > 9) {
        send_byte_to_register(0b11100111);  // E symbol with dot
    } else {
        send_byte_to_register(pgm_read_byte_near(segments + result) + 1);  // Place dot
    }
}

void logo() {
    unsigned char n = 1;
    while (n < 7) {
        send_byte_to_register(1 << n);
        n++;
        _delay_ms(100);
    }
}

int main(void) {
    setup();
    
    for (;;) {        
        //send_byte_to_register(0b01000110);  // L symbol
        logo();
        send_byte_to_register(0x00);
        _delay_ms(1000);

        generate_number();
        _delay_ms(1000);

        setbit(PORTB, LED);
        simulate_mistake();
        _delay_ms(1000);
        clrbit(PORTB, LED);

        predict_number();
        _delay_ms(1000);
    }
    
    return 0;
}