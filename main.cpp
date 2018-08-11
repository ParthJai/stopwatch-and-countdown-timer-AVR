/*
PORTA= LCD DATA LINES
PINB0,B2=RS AND E
COUNTDOWN=PINC0
STOPWATCH=PIND7
RESET=PIND3
STOPWATCH PAUSE=PIND2
CURSOR LEFT=PINC1
CURSOR RIGHT=PIND5
INCREMENT=PIND6
DECREMENT=PIND4
FOR COUNTDOWN TIMER ADJUST SECOND FIRST AND THEN MINUTES
RESET ANY TIME IF ANYTHING GOES WRONG OR TO STOP COUNTING IN BETWEEN
SOMETIMES COUNTDOWN TIMER COUNTS TOO FAST. PRESS RESET IF ONE FEELS THAT IT IS GOING FAST
SOMETIMES YOU GET RANDOM CHARACTERS AT LCD. THIS IS DUE TO NOISE AT POWER OR DATA PINS. PRESS RESET
*/
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
void send_a_command(char cmnd);
void send_a_character(char characeter);
void send_a_string(char *string);
volatile int min=0,sec=-1,x=0,i=0;
char str[3];
int main(void)
{
	DDRB|=1<<PINB2|1<<PINB0;
	DDRD&=~(1<<PIND7|1<<PIND6|1<<PIND5|1<<PIND4|1<<PIND3|1<<PIND2);
	DDRC&=~(1<<PINC0|1<<PINC1);
	PORTC|=1<<PINC0|1<<PINC1;
	PORTB&=~(1<<PINB2|1<<PINB0);
	PORTD|=1<<PIND7|1<<PIND6|1<<PIND5|1<<PIND4|1<<PIND3|1<<PIND2;
	DDRA=0xFF;
	PORTA=0;
	x:
	_delay_ms(50);
	send_a_command(0x01);//clear display
	send_a_command(0x38);//16*2 lcd
	send_a_command(0x0E);//screen and cursor on
	int loc=1;
	send_a_command(0x01);
    send_a_string("1>countdown");
	send_a_command(0xC0);//cursor on 2nd line
	send_a_string("2>stopwatch");
	sei();
    while (1) 
    {
		if (bit_is_clear(PINC,0))
		{					
		    i=1;
			break;
		}
		if (bit_is_clear(PIND,7))
		{
			i=2;
			send_a_command(0x01);
			send_a_string("min:sec");
			send_a_command(0xC0);
			send_a_string("0:00");
			send_a_command(0x10);
			break;
        }//select stopwatch or countdown
	}
	    switch (i)
	    {
		    case 1:
				send_a_command(0x01);
				send_a_string("min:sec");
				send_a_command(0xC0);
				send_a_string("0:00");				
				while(1)
				{
					if(bit_is_clear(PINC,0))//timer config
					{
						TCCR1B|=1<<CS10|1<<CS11;
						TCCR1B|=1<<WGM12;
						TIMSK|=1<<OCIE1A;
						TCNT1=0;
						OCR1A=15624;
					}
					if(bit_is_clear(PINC,1))//shift cursor left
					{
						send_a_command(0x10);loc--;//get position of cursor by loc
						_delay_ms(200);
					}
					if(bit_is_clear(PIND,5))//cursor right
					{
						send_a_command(0x14);
						_delay_ms(200);
					}
					if(bit_is_clear(PIND,6))//increment sec or min based on loc
					{
						if(loc==0)
						{
							 sec++;
							 if(sec>59)
							{
								sec=0;min++;
								send_a_command(0xC0);
								itoa(min,str,10);//convert int to string
								send_a_string(str);
								send_a_string(":00");
								send_a_command(0x10);
							}
							if(sec>=10)
							{
								send_a_command(0x10);
							}
							itoa(sec,str,10);
							send_a_string(str);
							send_a_command(0x10);
							_delay_ms(200);
						}	
					else
					{
						min++;
						send_a_command(0xC0);
						itoa(min,str,10);
						send_a_string(str);
						if(min>=10)
						{
							send_a_string(":");
							itoa(sec,str,10);
							send_a_string(str);
						}
						send_a_command(0x10);
						_delay_ms(200);
					}	
					}
					if(bit_is_clear(PIND,4))//decrement sec or min based on loc
					{
						if(loc==0)
						{
						sec--;
						if(sec<0)
						{
							sec=59;min--;
							send_a_command(0xC0);
							if(min<0)
							send_a_string("99:");
						}
						if(sec<10)
						{
							send_a_command(0x10);
							send_a_string("0");
						}
						else
						{
							send_a_command(0x10);
						}
						itoa(sec,str,10);
						send_a_string(str);
						send_a_command(0x10);
						_delay_ms(200);						
						}
					    else
					    {
						    min--;
						    itoa(min,str,10);
						    send_a_string(str);
							send_a_command(0x10);
						    _delay_ms(200);
					    }
					}
				if(bit_is_clear(PIND,3))//reset and stop timer
				{
					TCNT1=0;
					TCCR1B&=~(1<<CS10|1<<CS11);
					sec=0;min=0;
					goto x;
				}
				}
				break;
		    case 2:
			    while (1)
			   {
					if (bit_is_clear(PIND,7))//timer config
					{
						TCCR1B|=1<<CS10|1<<CS11;
						TCCR1B|=1<<WGM12;
						TIMSK|=1<<OCIE1A;
						OCR1A=15624;
					}
					if(bit_is_clear(PIND,3))//reset
					{
						TCNT1=0;
						TCCR1B&=~(1<<CS10|1<<CS11);
						TIMSK&=~1<<OCIE1A;
						sec=0;min=0;
						goto x;
					}
					if(bit_is_clear(PIND,2))//pause
					{
						TCCR1B&=~(1<<CS10|1<<CS11);
					}
			   }			
		}
}
void send_a_command(char cmnd)
{
	PORTB&=~1<<PINB0; //RS pin
	PORTB|=1<<PINB2; //enable pin
	PORTA=cmnd;
	_delay_ms(50);
	PORTB&=~1<<PINB2;
}
void send_a_character(char character)
{
	PORTB|=1<<PINB0;
	PORTB|=1<<PINB2;
	PORTA=character;
	_delay_ms(50);
	PORTB&=~1<<PINB2;
}
void send_a_string(char *string)
{
	while(*string>0)
	send_a_character(*string++);
}
ISR (TIMER1_COMPA_vect)
{
	if(i==2)//run it if stopwatch is selected
	{
		    sec++;
			if (sec>59)
			{
				send_a_command(0xC0);
				min++;
				itoa(min,str,10);
				send_a_string(str);
				send_a_string(":00");
				send_a_command(0x10);
				sec=0;
			}
			else
			{
				itoa(sec,str,10);
				if (sec>=10)
				send_a_command(0x10);
				send_a_string(str);
				send_a_command(0x10);
			}		
	}
	else//countdown block
	{
		    x++;//delay countdown at start
			if(x>2)
			{
			     sec--;
			     send_a_command(0x10);
			if(sec>=10)
			{
				send_a_command(0x10);
			}
			else
			{
				send_a_command(0x10);
				send_a_string("0");
			}
			if(sec<0)
			{
				send_a_command(0xC0);
				min--;
				if(min<0)
				{
					TIMSK&=~1<<TICIE1;
					asm volatile ("ret");//stop the countdown at 00:00 and exit ISR FORCEFULLY.
				}
				itoa(min,str,10);
				if(min<10)
				{
					send_a_string("0");
					send_a_string(str);
					send_a_string(":");
				}
				else//tweaking display when minutes are >=10
				{
				    itoa(min,str,10);
				    send_a_string(str);			
					send_a_string(":");		
				}
				sec=59;
			}
			itoa(sec,str,10);
			send_a_string(str);
			}
	}
}
