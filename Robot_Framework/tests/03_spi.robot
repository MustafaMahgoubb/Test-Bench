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
    SPI Test Response Should Be Valid    ${response}

SPI Transfer 0x55 Test
    ${response}=    Send Shell Command    tb_spi_transfer 0x55
    SPI Transfer Response Should Match    ${response}    0x55

SPI Transfer 0xA5 Test
    ${response}=    Send Shell Command    tb_spi_transfer 0xA5
    SPI Transfer Response Should Match    ${response}    0xA5

SPI Transfer 0xFF Test
    ${response}=    Send Shell Command    tb_spi_transfer 0xFF
    SPI Transfer Response Should Match    ${response}    0xFF