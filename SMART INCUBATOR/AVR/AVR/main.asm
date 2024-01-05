.include "m328pdef.inc"
.include "delayMacro.inc"
.include "UART.inc"
.include "div_macro.inc"

.def temp = r20
.def humidity = r21
.def espinpt = r16
.def intrputState = r17
.def buzzer_state = r24
.def current_state = r27
.def prev_state = r28
.def led_state = r25
.def fan_state = r26
.equ LED = PB5 ; LED connected to Digital 13
.equ buzzer = PB4 ; buzzer connected to Digital 12
.equ fan = PB3 ; fan connected to Digital 11



.cseg
.org 0x0000
Serial_begin
    ;Setting Pins as output and input
    SBI DDRB,LED
    SBI DDRB,buzzer
    SBI DDRB,fan
    SBI DDRD,PD1    ;TX pin as output
	SBI DDRD,PD4

    CBI DDRD,PD0    ;RX pin as input

    ;Setting the initial state of the LED and buzzer
    CBI PORTB,LED
    CBI PORTB,buzzer
    CBI PORTB,fan
    LDI espinpt,0
    LDI buzzer_state, 0
	SBI PORTD, PD4

mainloop:
    ;Reads value from the sensor
    
    Serial_read espinpt
    cpi espinpt, 'A'
    breq Led2_on
    cpi espinpt, 'B'
    breq Led2_off
    cpi espinpt, 'C'
    breq fan2_on
    cpi espinpt, 'D'
    breq fan2_off
    CPI espinpt, 'H'
    BREQ fan2_on
    CPI espinpt, 'I'
    BREQ fan2_off
    CPI espinpt, 'J'
    BREQ Led2_on
    CPI espinpt, 'K'
    BREQ Led2_off
    RJMP skip

	Led2_off:
    CBI PORTB, LED     ; Turn off the LED
	LDI led_state,0
    JMP skip

    LED2_on:
    SBI PORTB, LED     ; Turn on the LED
	LDI led_state,1
	JMP skip

    fan2_off:
    CBI PORTB, fan     ; Turn off the fan
	LDI fan_state,0
    LDI current_state, 1
    ;Beep the buzzer 3 times when the fan turns off
    LDI buzzer_state, 0
    JMP BeepBuzzer

    fan2_on:
    SBI PORTB, fan     ; Turn on the fan
    ;Beep the buzzer 2 times when the fan turns on
    LDI buzzer_state, 1
    LDI current_state, 0
	LDI fan_state, 1
    JMP BeepBuzzer
    
		buzzer_two:
    LDI prev_state, 1
    LDI r31, 2
	L1:
		SBI PORTB, buzzer     ; Turn on the buzzer
		delay 500
		CBI PORTB, buzzer     ; Turn off the buzzer
		delay 500
		DEC r31
		BRGE L1
    JMP skip
    BeepBuzzer:
    CP current_state, prev_state
    BREQ skip
    CPI buzzer_state, 0
    BREQ buzzer_two
    CPI buzzer_state, 1
    BREQ buzzer_three
	JMP mainloop

	buzzer_three:
    LDI prev_state, 0
	LDI r31, 3
	L2:
		SBI PORTB, buzzer     ; Turn on the buzzer
		delay 500
		CBI PORTB, buzzer     ; Turn off the buzzer
		delay 500
		DEC r31
		BRGE L2

		skip:
   
    ;Send fan and LED status to the ESP32
    Serial_writeReg_ASCII led_state
    Serial_writeChar '-'     ;To Seprate States
    Serial_writeReg_ASCII fan_state
	Serial_writeNewLine
	delay 100

    rjmp mainloop



 

