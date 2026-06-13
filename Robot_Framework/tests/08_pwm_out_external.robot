*** Settings ***
Library    ../resources/testbench_serial.py    /dev/ttyUSB1    115200
Suite Setup    Open Testbench And Dut Connections    /dev/ttyUSB0
Suite Teardown    Close All Connections


*** Test Cases ***
PWM OUT External DUT Capture Test
    Clear DUT Log Buffer

    # First send different valid PWM values to force DUT to print new readings.
    ${dummy1}=    Send Shell Command    pwm_out freq 1 1000 55
    ${retval_dummy1}=    Send Shell Command    retval
    Response Should Contain    ${retval_dummy1}    0

    ${dummy2}=    Send Shell Command    pwm_out freq 2 1000 30
    ${retval_dummy2}=    Send Shell Command    retval
    Response Should Contain    ${retval_dummy2}    0

    Sleep    2s
    ${dummy_logs}=    Read DUT Logs    2

    Clear DUT Log Buffer

    # Now send the real expected PWM values.
    ${response1}=    Send Shell Command    pwm_out freq 1 1000 50
    ${retval1}=      Send Shell Command    retval
    Response Should Contain    ${retval1}    0

    ${response2}=    Send Shell Command    pwm_out freq 2 1000 25
    ${retval2}=      Send Shell Command    retval
    Response Should Contain    ${retval2}    0

    ${logs}=    Read DUT Logs    6
    DUT PWM Output Should Match    ${logs}
    