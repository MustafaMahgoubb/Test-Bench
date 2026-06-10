#include <zephyr/kernel.h>
#include "testbench_uart.h"

int main()
{
    // Initialize the UART interrupt drivers
    testBench_uart_Init();
    
    // The Shell Thread is running in the background
    return 0;
}