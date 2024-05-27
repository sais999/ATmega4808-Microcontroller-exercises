#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define T1 1024
#define SW5_PIN 5  // Switch 5 pin on PORTF
#define SW6_PIN 6  // Switch 6 pin on PORTF
volatile int sw5;
volatile int sw6;
volatile int correct_code;
volatile int incorrect_try;
volatile int tim0;
volatile int terminate;
volatile int alarm;
int main(void){
	//interrupt
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	//PIN is output
	PORTD.DIR |= 0b00000001; //PIN0_bm right

	//TIMER
	/* Load the Compare or Capture register with the timeout value*/
	TCB0.CCMP |= T1;
	/* Enable Capture or Timeout interrupt */
	TCB0.INTCTRL |= TCB_CAPT_bm;
	TCB0.CTRLB |= TCB_CNTMODE_INT_gc;
	
	//initialize the ADC for Free-Running mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
	ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit //Enable Debug Mode
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Window Comparator Mode
	ADC0.WINLT |= 10; //Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE |= ADC_WINCM0_bm; //Interrupt when RESULT < WINL
	
	//prescaler=1024
	//prescaler=1024
	TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
	TCA0.SINGLE.PER = 254; //select the resolution
	TCA0.SINGLE.CMP0 = 127; //select the duty cycle
	//select Single_Slope_PWM
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
	//enable interrupt Overflow
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	//enable interrupt COMP0
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;




	terminate=0;
	sei();
	while (terminate==0){
		
		tim0=0;
		sw5=0;
		sw6=0;
		correct_code=0;
		incorrect_try=0;
		alarm=0;
		while(correct_code==0 && terminate==0){//anamoni gia isagogi sindiasmou
			;
		}
		correct_code=0;
		incorrect_try=0;
		terminate=0;
		sw5=0;
		sw6=0;
		//energopiisi timer
		TCB0.CTRLA |= TCB_CLKSEL_CLKDIV1_gc ;
		TCB0.CTRLA |= TCB_ENABLE_bm;
		while (tim0==0){//perimene timer
			;
		}
		tim0=0;
		//energopiisi sinagermou
		ADC0.CTRLA |= ADC_ENABLE_bm; //start conversion
		ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
		while(alarm==0){//perimene RESULT<WILNT
			;
		}
		//entopismos mikroteris timis apo to katofli
		alarm_on();
		//energopiisi timer
		TCB0.CTRLA |= TCB_CLKSEL_CLKDIV1_gc ;
		TCB0.CTRLA |= TCB_ENABLE_bm;
		//anamoni gia isagogi sindiasmou i 3 lathos prospathies i liksi timer
		while(correct_code==0 && terminate==0 && incorrect_try < 3 && tim0==0){
			;
		}
		if (incorrect_try >= 3 || tim0==1 ){//mpeni mono otan liksi timer i 3 lathos prospathies
			TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable
			while(correct_code==0 && terminate==0){//anamoni gia isagogi sindiasmou
				;
			}
		}
		correct_code=0;
		terminate=0;
		alarm_off();
		TCB0.CTRLA &= ~TCB_ENABLE_bm;
		// To stop the PWM signal
		TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
	}


}
ISR(PORTF_PORT_vect){
	if ((PORTF.INTFLAGS & (1 << SW5_PIN)) == (1 << SW5_PIN)){
		//clear the interrupt flag
		int z = PORTF.INTFLAGS;
		PORTF.INTFLAGS=z;
		if (sw5==0 && sw6==0){
			sw5=1;
			}else if (sw5==1 && sw6==1){
			sw5=2;
			}else{
			incorrect_try++;//lathos kodikos
			sw5=0;
			sw6=0;
		}
		
		
		}else if ((PORTF.INTFLAGS & (1 << SW6_PIN)) == (1 << SW6_PIN)){
		//clear the interrupt flag
		int z = PORTF.INTFLAGS;
		PORTF.INTFLAGS=z;
		if (sw5==1 && sw6==0){
			sw6=1;
			}else if (sw5==2 && sw6==1){
			sw6=2;
			correct_code=1;
			terminate=1;
			}else{
			incorrect_try++;//lathos kodikos
			sw5=0;
			sw6=0;
		}
		
	}
	
}
ISR(TCB0_INT_vect){
	TCB0.INTFLAGS |= TCB_CAPT_bm; /* Clear the interrupt flag */
	TCB0.CNT = 0b00000000;
	TCB0.CTRLA &= ~TCB_ENABLE_bm;
	tim0=1;
	
}
ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	alarm=1;
	ADC0.CTRLA &= ~ADC_ENABLE_bm; //stop conversion
}
//PWM falling edge
ISR(TCA0_OVF_vect){
	//clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	alarm_off();
}

//TIMER (PWM rising edge)
ISR(TCA0_CMP0_vect){
	//clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	alarm_on();
}


void  alarm_on(){
	//on
	PORTD.OUTCLR= 0b00000001; //PIN0_bm
}
void alarm_off(){
	//LED is off
	PORTD.OUT |= 0b00000001; //PIN0_bm
}

