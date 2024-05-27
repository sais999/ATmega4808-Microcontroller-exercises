#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
int wait=0;
int x=1;
int button=0;
int main(void){
	//PIN is output
	PORTD.DIR |= 0b00000001; //PIN0_bm lepides
	//PIN is output
	PORTD.DIR |= 0b00000010; //PIN1_bm vasi
	//PIN is output
	PORTD.DIR |= 0b00000100; //PIN2_bm adc
	//LED is off
	PORTD.OUT |= 0b00000001; //PIN0_bm
	//LED is off
	PORTD.OUT |= 0b00000010; //PIN1_bm
	//LED is off
	PORTD.OUT |= 0b00000100; //PIN2_bm
	
	//pullup enable and Interrupt enabled with sense on both edges
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
	ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit //Enable Debug Mode
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Window Comparator Mode
	ADC0.WINLT |= 10; //Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE |= ADC_WINCM0_bm; //Interrupt when RESULT < WINLT
	sei();
	

	while(x==1){
		lepides_off();
		vasi_off();
		while(wait==0){
			;
		}
		if (button==1){//energopiite o anemistiras normal mode
			ADC0.CTRLA |= ADC_ENABLE_bm; //start adc
			ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
			//TCA SPLIT MODE
			//prescaler=1024
			TCA0.SPLIT.CTRLD = 1;
			TCA0.SPLIT.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
			TCA0.SPLIT.LPER = 19; //select the resolution/lepides
			TCA0.SPLIT.LCMP0 = 10; //select the duty cycle/lepides 50%
			TCA0.SPLIT.LCNT = 0; //Low Counter
			TCA0.SPLIT.HPER = 38; //select the resolution/vasi
			TCA0.SPLIT.HCMP0 = 15; //select the duty cycle/vasi 40%
			TCA0.SPLIT.HCNT = 10; //High Counter // 10 gia kathisterisi na min simvenoun interrupt mazi
			//select Single_Slope_PWM
			TCA0.SPLIT.CTRLB =TCA_SPLIT_HCMP0EN_bm | TCA_SPLIT_LCMP0EN_bm;
			//enable interrupt Overflow
			TCA0.SPLIT.INTCTRL = TCA_SPLIT_HUNF_bm | TCA_SPLIT_LUNF_bm;
			//enable interrupt CMP0
			TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable
			while (button==1){
				;
			}
			}else if (button==2){//energopiite o anemistiras x2 mode
			TCA0.SPLIT.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
			TCA0.SPLIT.CTRLD = 1;
			TCA0.SPLIT.LPER = 38; //select the resolution/lepides(2x)
			TCA0.SPLIT.LCMP0 = 20; //select the duty cycle/lepides 50%
			TCA0.SPLIT.LCNT = 0; //Low Counter
			TCA0.SPLIT.HPER = 38; //select the resolution/vasi
			TCA0.SPLIT.HCMP0 = 15; //select the duty cycle/vasi 40%
			TCA0.SPLIT.HCNT = 10; //High Counter 10 gia kathisterisi na min simvenoun interrupt mazi
			//select Single_Slope_PWM
			TCA0.SPLIT.CTRLB =TCA_SPLIT_HCMP0EN_bm | TCA_SPLIT_LCMP0EN_bm;
			//enable interrupt Overflow
			TCA0.SPLIT.INTCTRL = TCA_SPLIT_HUNF_bm | TCA_SPLIT_LUNF_bm;
			//enable interrupt CMP0
			TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable
			while(button==2){
				;
			}
			}else if (button==3){//apenergopiite o anemistiras
			wait=0;
			button=0;
			//apenergopiisi tca adc
			ADC0.CTRLA &= ~ADC_ENABLE_bm; //stop conversion
			TCA0.SPLIT.CTRLA &= ~TCA_SPLIT_ENABLE_bm;// disable tca
		}
	}
}
void  lepides_on(){
	//on
	PORTD.OUTCLR= 0b00000001; //PIN0_bm
}
void lepides_off(){
	//LED is off
	PORTD.OUT |= 0b00000001; //PIN0_bm
}

void vasi_on(){
	//on
	PORTD.OUTCLR= 0b00000010; //PIN1_bm
}
void vasi_off(){
	//LED is off
	PORTD.OUT |= 0b00000010; //PIN1_bm
}

void adc_on(){
	//on
	PORTD.OUTCLR= 0b00000100; //PIN2_bm
}
void adc_off(){
	//LED is off
	PORTD.OUT |= 0b00000100; //PIN2_bm
}

ISR(PORTF_PORT_vect){
	//clear the interrupt flag
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	adc_off();
	wait=1;
	button++;
}
//Aneveni pros psili stathmi
ISR(TCA0_LUNF_vect){
	//clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	//Kinisi Vasis On
	lepides_on();
	//Kinisi Lepidon On
	vasi_on();
}

//Proxora pros xamili stathmi
ISR(TCA0_HUNF_vect){
	//clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	//Kinisi Vasis Off
	lepides_off();
	//Kinisi Lepidon Off
	lepides_off();
}

ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	adc_on();
	wait=0;
	button=0;
	ADC0.CTRLA &= ~ADC_ENABLE_bm; //stop conversion
	//apenergopiisi adc kai tca
	ADC0.CTRLA &= ~ADC_ENABLE_bm; //stop conversion
	TCA0.SPLIT.CTRLA &= ~TCA_SPLIT_ENABLE_bm; // disable tca
}
