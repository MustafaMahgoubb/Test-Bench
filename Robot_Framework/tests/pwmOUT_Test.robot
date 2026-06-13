*** Settings ***
Documentation    Testing the PWM Output capabilities of the HIL Bench.
Library          ../../Host/TestBenchLibrary.py    /dev/ttyUSB1    115200

Suite Teardown   Close Connection

*** Test Cases ***
Verify LED Recieves 50 PWM DutyCycle
    [Documentation]    Simulate a 50% PWM Signal and Check if the the External Hardwar Reacts

    # 1. Ask the Test Bench to generate the signal
    Set Pwm Duty Cycle    1    50

Give Wrong Dutycycle
    [Documentation]    Simulate a 200% DutyCycle and check if it throws an error.
    
    # The syntax is:
    # Run Keyword And Expect Error    <Expected Error Message>    <Your Keyword>    <Args>
    
    Run Keyword And Expect Error    *Invalid Duty Cycle*    Set Pwm Duty Cycle    1    200
    

Give Wrong Channel
    [Documentation]    give invalid pwm channel

    Run Keyword And Expect Error    *Invalid PWM Channel*    Set Pwm Duty Cycle    5    50
Stop PWM Signal Safely
    [Documentation]    Ensure the PWM can be turned off.
    
    Set Pwm Duty Cycle    1    0