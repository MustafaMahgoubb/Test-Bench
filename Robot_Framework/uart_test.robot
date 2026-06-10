*** Settings ***
Documentation     Hardware-in-the-Loop Test Suite for UART Driver.
...               REQUIREMENT: You MUST connect a jumper wire between PA11 (TX) and PA12 (RX) for Channel 2 Loopback testing!
Library           ../Host/TestBenchLibrary.py

Suite Setup       Clear UART Buffers
Suite Teardown    Close Connection

*** Variables ***
${TEST_CHANNEL}    2
${BAUDRATE}        115200

*** Test Cases ***

Test UART Send And Read (Loopback)
    [Documentation]    Sends a payload and verifies the identical payload is received via hardware loopback.
    Uart Clear    ${TEST_CHANNEL}
    
    # 1. Send the data
    ${payload} =    Set Variable    Robot_Test_Message
    Uart Send    ${TEST_CHANNEL}    ${payload}
    
    # 2. Wait for hardware transmission
    Sleep    0.2s
    
    # 3. Read back and verify
    ${response} =    Uart Read    ${TEST_CHANNEL}
    Should Be Equal    ${response}    ${payload}    msg=Failed! Did you connect the loopback wire?


Test UART Buffer Clearing
    [Documentation]    Ensures the clear command successfully drops old unread data.
    Uart Send    ${TEST_CHANNEL}    GarbageData123
    Sleep    0.2s
    
    Uart Clear    ${TEST_CHANNEL}
    
    ${response} =    Uart Read    ${TEST_CHANNEL}
    Should Not Contain    ${response}    GarbageData123    msg=Clear command failed, old data was still in the buffer!


Test Dynamic Baudrate Switching
    [Documentation]    Tests changing the baudrate on the fly.
    # 1. Change to a slower baudrate
    Uart Set Baudrate    ${TEST_CHANNEL}    9600
    Sleep    0.2s
    
    Uart Clear    ${TEST_CHANNEL}
    Uart Send    ${TEST_CHANNEL}    SlowMessage
    Sleep    0.5s    # Slower baudrate needs more time to transmit
    
    ${response} =    Uart Read    ${TEST_CHANNEL}
    Should Contain    ${response}    SlowMessage
    
    # 2. Restore standard baudrate
    Uart Set Baudrate    ${TEST_CHANNEL}    ${BAUDRATE}

*** Keywords ***

Clear UART Buffers
    Uart Clear    ${TEST_CHANNEL}
