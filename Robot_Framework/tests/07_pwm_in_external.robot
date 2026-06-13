*** Settings ***
Library    ../resources/testbench_serial.py    /dev/ttyUSB1    115200
Library    String
Suite Setup    Open Connection
Suite Teardown    Close Connection


*** Test Cases ***
Communication Ping Test
    ${response}=    Send Shell Command    tb_ping
    Response Should Contain    ${response}    PONG

PWM IN External DUT Channel 1 Test
    ${response}=    Send Shell Command    pwm_in read 1

    ${clean}=    Replace String    ${response}    pwm_in read 1    ${EMPTY}
    ${clean}=    Replace String    ${clean}    blackpill:~$    ${EMPTY}
    ${clean}=    Strip String    ${clean}

    Log To Console    \nPWM IN External DUT Result:
    Log To Console    Channel 1: ${clean}

    Response Should Contain    ${response}    Freq: 1000 Hz
    Response Should Contain    ${response}    Duty: 49.

PWM IN External DUT Channel 2 Test
    ${response}=    Send Shell Command    pwm_in read 2

    ${clean}=    Replace String    ${response}    pwm_in read 2    ${EMPTY}
    ${clean}=    Replace String    ${clean}    blackpill:~$    ${EMPTY}
    ${clean}=    Strip String    ${clean}

    Log To Console    \nPWM IN External DUT Result:
    Log To Console    Channel 2: ${clean}

    Response Should Contain    ${response}    Freq: 1000 Hz
    Response Should Contain    ${response}    Duty: 24.

PWM IN Frequency And Duty Separate Read Test
    ${freq1}=    Send Shell Command    pwm_in get_frequency 1
    ${duty1}=    Send Shell Command    pwm_in get_dutyCycle 1
    ${freq2}=    Send Shell Command    pwm_in get_frequency 2
    ${duty2}=    Send Shell Command    pwm_in get_dutyCycle 2

    Log To Console    \nPWM IN Separate Measurements:
    Log To Console    CH1 Frequency: ${freq1}
    Log To Console    CH1 Duty: ${duty1}
    Log To Console    CH2 Frequency: ${freq2}
    Log To Console    CH2 Duty: ${duty2}

    Response Should Contain    ${freq1}    Freq: 1000 Hz
    Response Should Contain    ${duty1}    Duty: 49.
    Response Should Contain    ${freq2}    Freq: 1000 Hz
    Response Should Contain    ${duty2}    Duty: 24.

    