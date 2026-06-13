*** Settings ***
Library    ../resources/testbench_serial.py    /dev/ttyUSB1    115200
Suite Setup    Open Connection
Suite Teardown    Close Connection


*** Test Cases ***
Communication Ping Test
    ${response}=    Send Shell Command    tb_ping
    Response Should Contain    ${response}    PONG

ADC Potentiometer Read Test
    ${response}=    Send Shell Command    tb_adc_read
    Response Should Contain    ${response}    ADC_READ RAW
    ADC Response Should Be Valid    ${response}