*** Settings ***
Library    ../resources/testbench_serial.py    /dev/ttyUSB0    115200
Suite Setup    Open Connection
Suite Teardown    Close Connection


*** Test Cases ***
Communication Ping Test
    ${response}=    Send Shell Command    tb_ping
    Response Should Contain    ${response}    PONG

SPI External DUT Slave Ping Test
    ${response}=    Send Shell Command    tb_spi_slave_ping
    SPI Slave Ping Response Should Pass    ${response}

SPI External DUT Slave Ping Repeat Test
    ${response}=    Send Shell Command    tb_spi_slave_ping
    SPI Slave Ping Response Should Pass    ${response}
