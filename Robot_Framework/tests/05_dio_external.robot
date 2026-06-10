*** Settings ***
Library    ../resources/testbench_serial.py    /dev/ttyUSB0    115200
Suite Setup    Open Connection
Suite Teardown    Close Connection


*** Test Cases ***
Communication Ping Test
    ${response}=    Send Shell Command    tb_ping
    Response Should Contain    ${response}    PONG

DIO External DUT Mirror Test
    ${response}=    Send Shell Command    tb_dio_test
    DIO Test Response Should Pass    ${response}

DIO External Write High And Read Test
    ${write_response}=    Send Shell Command    tb_dio_write 1
    Response Should Contain    ${write_response}    DIO_WRITE OK VALUE 1
    ${read_response}=    Send Shell Command    tb_dio_read
    Response Should Contain    ${read_response}    DIO_READ VALUE 1

DIO External Write Low And Read Test
    ${write_response}=    Send Shell Command    tb_dio_write 0
    Response Should Contain    ${write_response}    DIO_WRITE OK VALUE 0
    ${read_response}=    Send Shell Command    tb_dio_read
    Response Should Contain    ${read_response}    DIO_READ VALUE 0
