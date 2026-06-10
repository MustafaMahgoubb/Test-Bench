*** Settings ***
Library    ../resources/testbench_serial.py    /dev/ttyUSB0    115200
Suite Setup    Open Connection
Suite Teardown    Close Connection


*** Test Cases ***
Communication Ping Test
    ${response}=    Send Shell Command    tb_ping
    Response Should Contain    ${response}    PONG

SPI Loopback Test
    ${response}=    Send Shell Command    tb_spi_test
    Response Should Contain    ${response}    SPI_TEST PASS TX 0xA5 RX 0xA5

SPI Transfer 0x55 Test
    ${response}=    Send Shell Command    tb_spi_transfer 0x55
    Response Should Contain    ${response}    SPI_TRANSFER TX 0x55 RX 0x55

SPI Transfer 0xA5 Test
    ${response}=    Send Shell Command    tb_spi_transfer 0xA5
    Response Should Contain    ${response}    SPI_TRANSFER TX 0xA5 RX 0xA5

SPI Transfer 0xFF Test
    ${response}=    Send Shell Command    tb_spi_transfer 0xFF
    Response Should Contain    ${response}    SPI_TRANSFER TX 0xFF RX 0xFF