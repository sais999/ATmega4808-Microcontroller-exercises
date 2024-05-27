#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define T1 30
#define T2 5
#define T3 10
int x=1;
int y=1;
int p=0;
int z=0;
int u=0;
int zero=0;
int main() {
	PORTD.DIR |= 0b0000001;
	PORTD.DIR |= 0b0000010;
	PORTD.DIR |= 0b0000100;
	PORTD.OUT |= 0b00000001; //LED is off
	PORTD.OUT |= 0b00000010; //LED is off
	PORTD.OUT |= 0b00000100; //LED is off
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	//TIMER TRENOU
	/* Load the Compare or Capture register with the timeout value*/
	TCB0.CCMP = T1;
	/* Enable Capture or Timeout interrupt */
	TCB0.INTCTRL = TCB_CAPT_bm;
	
	TCB0.CTRLB = TCB_CNTMODE_INT_gc;
	
	
	//TIMER FANARI PEZON
	/* Load the Compare or Capture register with the timeout value*/
	TCB1.CCMP = T2;
	
	/* Configure TCB in Periodic Timeout mode */
	TCB1.CTRLB = TCB_CNTMODE_INT_gc;
	
	/* Enable Capture or Timeout interrupt */
	TCB1.INTCTRL = TCB_CAPT_bm;



	//timer gia koumpi t=T3
	/* Load the Compare or Capture register with the timeout value*/
	TCB2.CCMP = T3;
	
	/* Configure TCB in Periodic Timeout mode */
	TCB2.CTRLB = TCB_CNTMODE_INT_gc;
	
	/* Enable Capture or Timeout interrupt */
	TCB2.INTCTRL = TCB_CAPT_bm;


	aftokinita();

	sei();
	
	
	while(x==1){
		sei();
		
		p=0;
		z=0;
		u=0;
		aftokinita();
		TCB0.CTRLA |= TCB_CLKSEL_CLKDIV1_gc ; //ksekina to treno
		TCB0.CTRLA |= TCB_ENABLE_bm;
		
		while(z==0){ //perimenoume to treno na ftasi
			;
		}
		// to treno eftase
		pezoi(); //pernoun oi pezoi
		TCB1.CTRLA |= TCB_CLKSEL_CLKDIV1_gc ;
		TCB1.CTRLA |= TCB_ENABLE_bm;
		while(p==0){ //perimenoume to roloi ton pezon na teliosi
			;
		}
		//to roloi ton pezon teliose
		aftokinita();
		
		
		
		
		
		
	}
	
	
}

void aftokinita(){//pernoun ta aftokinita
	PORTD.OUT |= 0b00000010; //pezoi kokkino
	PORTD.OUTCLR= 0b00000100; //aftokinita prasino
}

void pezoi(){//otan perna treno pernoun oi pezoi
	PORTD.OUTCLR= 0b00000010; //pezoi prasino
	PORTD.OUT |= 0b00000100; //aftokinita kokkino
}


ISR(TCB0_INT_vect){
	TCB0.INTFLAGS = TCB_CAPT_bm; /* Clear the interrupt flag */
	TCB0.CTRLA |= zero;
	z=1;
}
ISR(TCB1_INT_vect){
	TCB1.INTFLAGS = TCB_CAPT_bm; /* Clear the interrupt flag */
	TCB1.CTRLA |= zero;
	p=1;
}
ISR(TCB2_INT_vect){
	TCB2.INTFLAGS = TCB_CAPT_bm; /* Clear the interrupt flag */
	TCB2.CTRLA |= zero;
	u=1;
}
ISR(PORTF_PORT_vect){//koumpi pezon
	//clear the interrupt flag
	int z = PORTF.INTFLAGS;
	PORTF.INTFLAGS=z;
	TCB1.CTRLA |= TCB_CLKSEL_CLKDIV1_gc ;//energopiise timer gia fanari pezon
	TCB1.CTRLA |= TCB_ENABLE_bm;
	pezoi();
	while(p==0){//perimene to timer
		;
	}
	p=0;
	aftokinita();
	TCB2.CTRLA |= TCB_CLKSEL_CLKDIV1_gc ;//energopiise timer gia interrupts
	TCB2.CTRLA |= TCB_ENABLE_bm;
	cli();
	while(u==0){//perimene to timer
		;
	}
	u=0;
	sei();
	
}




