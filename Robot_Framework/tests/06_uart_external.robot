*** Settings ***
Library    ../resources/testbench_serial.py    /dev/ttyUSB1    115200
Library    String
Suite Setup    Open Connection
Suite Teardown    Close Connection


*** Test Cases ***
Communication Ping Test
    ${response}=    Send Shell Command    tb_ping
    Response Should Contain    ${response}    PONG

UART External DUT Echo HELLO Test
    ${clear_response}=    Send Shell Command    uart clear 2
    Response Should Contain    ${clear_response}    UART2 RX Buffer Cleared

    ${send_response}=    Send Shell Command    uart send 2 HELLO
    Sleep    0.2s

    ${read_response}=    Send Shell Command    uart read 2
    ${received}=    Replace String    ${read_response}    uart read 2    ${EMPTY}
    ${received}=    Replace String    ${received}    blackpill:~$    ${EMPTY}
    ${received}=    Strip String    ${received}

    Log To Console    \nUART Echo Data:
    Log To Console    \nSent: HELLO
    Log To Console    \nReceived: ${received}

    Response Should Contain    ${received}    HELLO

UART External DUT Echo Name Test
    ${clear_response}=    Send Shell Command    uart clear 2
    Response Should Contain    ${clear_response}    UART2 RX Buffer Cleared

    ${send_response}=    Send Shell Command    uart send 2 ayman
    Sleep    0.2s

    ${read_response}=    Send Shell Command    uart read 2
    ${received}=    Replace String    ${read_response}    uart read 2    ${EMPTY}
    ${received}=    Replace String    ${received}    blackpill:~$    ${EMPTY}
    ${received}=    Strip String    ${received}

    Log To Console    \nUART Echo Data:
    Log To Console    \nSent: ayman
    Log To Console    \nReceived: ${received}

    Response Should Contain    ${received}    ayman
