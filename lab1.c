// File:        	Lab1.c
// Developer:	M. Batchelder, S. Thornburg and CENG 448/548 Students
// Processor:	PIC32MX795F512L
// Board:       	MAX32
// Compiler:	XC32
// IDE:         	MPLAB-X
// Date:        	September 12, 2017
// Status:      	In development
// Description:
//          1. fixed2String function development
//          2. ADC conversion and conversion to volts to be printed on UART1


//-----------------------------------------------------------------------
// INCLUDES
//-----------------------------------------------------------------------
#define _SUPPRESS_PLIB_WARNING 1
#include <p32xxxx.h>	// replace with <xc.h> for CCI
// #include <stdint.h>	// uncomment for CCIt rig
#include <stdio.h>
#include <plib.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

//-----------------------------------------------------------------------
// PRAGMA
//-----------------------------------------------------------------------
// 80 MHz main clock and 40 MHz Peripheral clock
#pragma config POSCMOD=HS, FNOSC=PRIPLL
#pragma config FPLLIDIV=DIV_2, FPLLMUL=MUL_20, FPLLODIV=DIV_1
#pragma config FPBDIV=DIV_2, FWDTEN=OFF, CP=OFF, BWP=OFF

//-----------------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------------
#define U_TX 0x0400
#define U_RX 0x1000
#define U_ENABLE 0x8008 // enable, BREGH = 1, 1 stop, no parity
#define BRATE 86 	// 115,200 Baud Rate = FPB / 4(U1BRG + 1))  FPB = freq of peripheral clock
// U1BRG = ((FPB/Desired Baud Rate)/4) ? 1
// if BREGH = 0 then use 16 in place of 4 in above formulas

//-----------------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------------

//----- Initialize UART1
void initU1(void) {
    U1BRG = BRATE; // initialize the baud rate generator
    U1MODE = U_ENABLE;
    U1STA = U_TX | U_RX;
} // initU1

//----- Output character to UART1
void putU1(char c) {
    while (U1STAbits.UTXBF); // wait while TX buffer is full
    U1TXREG = c;
} // putU1

//----- Output zero terminated string to UART1
void putsU1(char *s) {
    while (*s) // loop until *s = \0, end of string
        putU1(*s++);
} // putsU1

//Compiler helper function that allows the use of printf
void _mon_putc(char c) {
    putU1(c);
} // mon_putc


// --------------------------------------------------------------------
// Convert signed fixed point value to a char string in  base specified
// --------------------------------------------------------------------
char * fixed2string(
        int val, 			// 32-bit signed value to be converted
        unsigned int noFracBits, 	// position of binary point in val
        unsigned int base, 		// base to convert into
        unsigned int noDigits, 	// number of digits in converted fraction
        char buf[], 		// pointer to buffer to hold string
        unsigned int bufSize) 	// size of buffer
{
    unsigned int neg, radixPt, whole, wholeBits, frac, fracDigit, startIndex, i, j;

    // Error checks
    if (noFracBits > 31) return ("ERROR noFracBits > 31");
    if (base < 2) return ("base too small");
    if (base > 16) return("base too large");

    // set radix point in middle of string
    radixPt = bufSize / 2;
    buf[radixPt] = '.';

    // Take absolute value
    if (val < 0) {
        neg = TRUE;
        val = -val;
    }

    else neg = FALSE;

    // ***** STUDENT CODE STARTS HERE *****


    // isolate the whole [part] and the fraction [part]
    whole = val;
    frac = val;
    whole = whole >> noFracBits;

    wholeBits = 0;

	for( j = whole;j!=0; j = j>>1) {
        wholeBits++;
    }
    j = (1 << noFracBits) - 1;
    frac = frac & j;

    printf("\n\r Val: %d \n\r", val);
    printf("\n\r Whole: %d \n\r", whole);
    printf("\n\r Frac: %d \n\r", frac);
    printf("\n\r FracBits: %d \n\r", noFracBits);
    */
    // convert the whole part by continued division
    // result is LS digit first so start at '.' in buf and go up in buffer

    //****REPLACE WHOLE WITH WHAT VAR BEN USES IN HIS CODE****
    //i is the size of the whole number portion -1 due to sign bit
    for ( i = (radixPt + 1) ; i && whole ; i++ )
    {
        //Get value to fill string
        buf[i] = "0123456789abcdef"[whole % base];
        printf("\n\r Buff whole: %c \n\r", buf[i]);
        printf("\n\r Whole: %c \n\r", whole);
        //Do intiger division to get next val to mod with
        whole = whole/base;
    }

    startIndex = i;
    printf("\n\rStartIndex: %d Radix: %d\n\r", startIndex, radixPt);

    // insert minus sign if negative
	if ( neg == TRUE )
    {
        buf[startIndex] = '-';
    }

    // convert the fraction part by continued multiplication
    // result is MS digit first so start at '.' In buf and go down in buffer
    //printf("\n\r Frac: %d \n\r", frac);
	for ( i = (radixPt - 1); i && frac ; i--)
    {
        //Dec value is whole number portion of mult result
        frac *= base;
        buf[i] = "0123456789abcdef"[(frac >> (noFracBits + 1))];
        printf("\n\r Buf: %c\n\r", buf[i]);
        //Do mult and shift value to get rid of int portion accounting for sign bit
        //Shift back
        frac &= ((1 << noFracBits ) - 1);
        printf("\n\r Frac: %d \n\r", (frac));
    }

    buf[i] = '\0'; // End of string marker

    return &buf[startIndex];
} // fixed2String


//-----------------------------------------------------------------------
// ADC FUNCTIONS
//-----------------------------------------------------------------------
#define POT     0      // 10k potentiometer on AN2 input
// I/O bits set as analog in by setting corresponding bits to 0
#define AINPUTS 0xfffe // Analog inputs for POT pin A0 (AN2)

// initialize the ADC for single conversion, select input pins

void initADC(int amask) {
    AD1PCFG = amask; // select analog input pins
    AD1CON1 = 0x00E0; // auto convert after end of sampling
    AD1CSSL = 0; // no scanning required
    AD1CON2 = 0; // use MUXA, AVss/AVdd used as Vref+/-
    AD1CON3 = 0x1F3F; // max sample time = 31Tad
    AD1CON1SET = 0x8000; // turn on the ADC
} //initADC

int readADC(int ch) {
    AD1CHSbits.CH0SA = ch; // select analog input channel
    AD1CON1bits.SAMP = 1; // start sampling
    while (!AD1CON1bits.DONE); // wait to complete conversion
    return ADC1BUF0; // read the conversion result
} // readADC


//-----------------------------------------------------------------------
// MAIN
//-----------------------------------------------------------------------
char stringBuffer[32];		// Debugger can watch global variables

main() {
    int i, a, b, c, d;
    float volts;
    //char stringBuffer[32];		// Moved to global so debugger can watch changes as step program

    // initializations
    initU1(); // Initialize UART1
    initADC(AINPUTS); // initialize the ADC

    // main loop
    while (1) {
        a = readADC(POT); // select the POT input and convert
        volts = (a * 3.3) / 1024.0;
        printf("\r\nvolts = %f \r\n", volts);

        a = a * 0x0D33; // 3.3 scaled 10, product scaled 20, (adc * 3.3 / 1024) * 1024
/*
        putsU1("\r\r\nVoltage by fixed point = ");
        putsU1(fixed2string(a, 20, 10, 3, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");
*/

        putsU1("\r\r\n Student case: ");
        putsU1(fixed2string(41, 2, 10, 4, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        putsU1("\r\r\n Test 1:  ");
        putsU1(fixed2string(0xffffffa0, 7, 10, 3, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        putsU1("\r\r\n Test 2:  ");
        putsU1(fixed2string(0x6a, 6, 10, 3, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        putsU1("\r\r\n Test 3:  ");
        putsU1(fixed2string(0x53, 5, 10, 3, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        putsU1("\r\r\nTest 4:  ");
        putsU1(fixed2string(0, 5, 10, 3, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        putsU1("\r\r\nTest 5:  ");
        putsU1(fixed2string(-1, 5, 10, 3, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        putsU1("\r\r\nTest 6:  ");
        putsU1(fixed2string(-1, 5, 17, 3, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        putsU1("\r\r\nTest 7:  ");
        putsU1(fixed2string(-1, 32, 10, 3, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");




        int alpha = (int) (1.667 * 64);
        int beta = (int) (-0.75 * 128);
        int gamma = (int) (2.6 * 32);
        int result;

        putsU1("\r\r\n");
        putsU1("alpha = ");
        putsU1(fixed2string(alpha, 6, 2, 16, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        putsU1("\r\r\n");
        putsU1("beta = ");
        putsU1(fixed2string(beta, 7, 2, 16, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        putsU1("\r\r\n");
        putsU1("gamma = ");
        putsU1(fixed2string(gamma, 5, 2, 16, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        result = alpha * beta; // Q13
        putsU1("\r\r\n");
        putsU1("alpha*beta= ");
        putsU1(fixed2string(result, 13, 2, 16, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        result = result >> 8; // Q5
        putsU1("alpha*beta s5 = ");
        putsU1(fixed2string(result, 5, 2, 16, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        result = result + gamma;
        putsU1("\r\r\n");
        putsU1("alpha*beta + gamma s5= ");
        putsU1(fixed2string(result * 10000, 5, 2, 116, stringBuffer, sizeof (stringBuffer)));
        putsU1("\r\r\n");

        return 0;

    } // main loop
} // main
