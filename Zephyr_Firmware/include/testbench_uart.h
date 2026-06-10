#ifndef TEST_BENCH_UART_HEADER
#define TEST_BENCH_UART_HEADER

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
/* 
 * The Test Bench provides Uart Features features on 2 UART Channels -> UART2 PA2(Tx) PA3(Rx) and UART6 PA11(Tx) PA12(Rx)
 * 
 * 
 * This Driver provide the following APIs
 * uart read <channel>
 * Describtion 
 *  This Command Reads whatever the ECU has replied with since the last time you checked.
 * Example
 *  uart read 1 -> Returns: "ENGINE_RUNNING_OK"
 * 
 * uart send <channel> <message>  
 * Description
 *  This Command Transmits a string over the TX pin to the ECU.
 * Example
 *  uart send 1 "START_ENGINE"
 * 
 * uart clear <channel>
 * Description
 *  This Command Flushes the receive buffer to clear out any old/garbage data left in the buffer from previous tests. 
 * Example
 *  uart clear 1
 * 
 * uart set_baud <channel> <baudrate>
 * Description
 *  This Command Changes the speed of the UART connection dynamically. This allows your Test Bench to talk to a 9600 baud sensor
 *  and a 115200 baud ECU without needing to recompile the firmware!
 * Example
 *  uart set_baud 1 9600
 * 
*/


// argv[1] is uart channel , argv[2] is the string to be sent.
int cmd_uart_send(const struct shell *sh, size_t argc, char **argv);

// argv[1] is uart channel to read from.
int cmd_uart_read(const struct shell *sh, size_t argc, char **argv);

// argv[1] is uart channel to clear its buffer
int cmd_uart_clear(const struct shell *sh, size_t argc, char **argv);

// argv[1] is uart channel, argv[2] is BaudRate Number
int cmd_uart_set_baud(const struct shell *sh, size_t argc, char **argv);

int testBench_uart_Init();

#endif