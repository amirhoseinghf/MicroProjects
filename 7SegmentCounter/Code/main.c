#include <stm32f4xx.h>
#include <stdbool.h>

// Simple delay function for approximately half a second
void delayHalfSecond() {
    for (volatile int i = 0; i < 250000; i++);
}

int main() {
    signed int counter = 0;
    bool isOn = true;
    bool isIncrementing = false;
    bool isDecrementing = false;
    bool isWaiting = false;
    bool reset = false;

    // 7-segment display digit mappings
    const uint8_t digit_map[10] = {
        0xFC, // 0
        0x60, // 1
        0xDA, // 2
        0xF2, // 3
        0x66, // 4
        0xB6, // 5
        0xBE, // 6
        0xE0, // 7
        0xFE, // 8
        0xF6  // 9
    };

    // Enable GPIO ports A, B, and C
    RCC->AHB1ENR |= (1 << 0);
    RCC->AHB1ENR |= (1 << 1);
    RCC->AHB1ENR |= (1 << 2);

    // Set PA4-PA7 and PB0-PB7 as output for 7-segment display
    GPIOA->MODER |= 0x5555;
    GPIOB->MODER |= 0x5555;

    // Set PB3 and PB4 as general-purpose output
    GPIOB->MODER &= ~(0xF << 6);
    GPIOB->MODER |= (0x5 << 6);

    // Enable pull-up resistors for PB3 and PB4
    GPIOB->PUPDR |= (0x1 << 6);
    GPIOB->PUPDR |= (0x1 << 8);

    // Enable pull-up resistors for PC0 and PC1 (buttons)
    GPIOC->PUPDR |= (1 << 1);
    GPIOC->PUPDR |= (1 << 3);

    while (1) {
        // Handle waiting state when button is pressed
        if (isWaiting) {
            if ((GPIOC->IDR & (1 << 0))) {
                isWaiting = false;
                isIncrementing = true;
                isDecrementing = false;
                delayHalfSecond();
            } else if ((GPIOC->IDR & (1 << 1))) {
                isWaiting = false;
                isIncrementing = false;
                isDecrementing = true;
                delayHalfSecond();
            }
            continue;
        }

        // Handle reset state
        if (reset) {
            counter = 0;
            GPIOA->ODR = 0x8E;
            GPIOB->ODR = 0xFC;
            
            if ((GPIOC->IDR & (1 << 0))) {
                reset = false;
                isIncrementing = true;
                isDecrementing = false;
                delayHalfSecond();
            } else if ((GPIOC->IDR & (1 << 1))) {
                reset = false;
                isIncrementing = false;
                isDecrementing = true;
                delayHalfSecond();
            }
            continue;
        }

        // Handle increment button press
        if (GPIOC->IDR & (1 << 0)) {
            isOn = false;
            delayHalfSecond();
            int a = 0;
            while (GPIOC->IDR & (1 << 0)) {
                if (a++ > 1500000) {
                    isWaiting = true;
                    while (GPIOC->IDR & (1 << 0));
                    break;
                }
            }
            if (isWaiting) {
                continue;
            }
            isDecrementing = false;
            isIncrementing = true;
        } 
        // Handle decrement/reset button press
        else if (GPIOC->IDR & (1 << 1)) {
            isOn = false;
            int a = 0;
            while (GPIOC->IDR & (1 << 1)) {
                if (a++ > 1500000) {
                    reset = true;
                    while (GPIOC->IDR & (1 << 1));
                    break;
                }
            }
            if (reset) {
                continue;
            }
            while (GPIOC->IDR & (1 << 1));
            isIncrementing = false;
            isDecrementing = true;
        }

        // Display initial state if power is on
        if (isOn) {
            GPIOA->ODR = 0xEC;
            GPIOB->ODR = 0xFC;
        }
        
        // Increment counter and update display
        if (isIncrementing) {
            counter = (counter + 1) % 100;
            int ones = counter % 10;
            int tens = counter / 10;
            GPIOA->ODR = digit_map[ones];
            GPIOB->ODR = digit_map[tens];
            delayHalfSecond();
        }

        // Decrement counter and update display
        if (isDecrementing) {
            counter = (counter - 1) % 100;
            if (counter < 0) {
                counter = 0;
            }
            int ones = counter % 10;
            int tens = counter / 10;
            GPIOA->ODR = digit_map[ones];
            GPIOB->ODR = digit_map[tens];
            delayHalfSecond();
        }
    }
}