/*
 * GccApplication13.c
 *
 * Created: 15/06/2016 15:19:57
 * Author : Ruan
 */ 

 #define F_CPU 14.7456E6

 #include <avr/io.h>
 #include <util/delay.h> 
 
 
 #define FOSC 16000000 // Clock Speed
 #define BAUD 9600
 #define MYUBRR FOSC/16/BAUD-1
 #include "stdio.h"

#define SCK PB1						//SCK (serial clock) pin number
#define CS PB0						//CS (chip select) pin number
#define SO PB3						//SO (serial out, also MISO) pin number

#define SCK_Port PORTB				//SCK port name   PB1
#define CS_Port PORTB				//CS port name   PB0
#define SO_Port PORTB				//so pin name   PB3

#define SO_Pin PINB					//SO pin name  PB2



#define SCK_DDR DDRB				//SCK DDR name
#define CS_DDR DDRB					//CS DDR name
#define SO_DDR DDRB					//SO DDR name

 volatile char output[128];
 volatile char ind = 0;



uint16_t gettemp(void){		
												//função para obter temperatura
	uint8_t bit = 0, bitnr = 12;										//variaveis
	uint8_t cont = 0;
	uint16_t data_temperatura = 0;
	

	/* como temos que fazer com que o cs mude de estado baixo pra alto 
	para funcionar, colocamos então o cs em nivel baixo, sinalizando que foi
	selecionado para leitura	*/
	CS_Port &= ~(1<<CS);											//selecionar porta chip select

	for(cont = 0 ; cont < 16 ; cont++){									//ler os 16 Bits 
		bit = 15 - cont;												//calculo do atual Bitnr
	
		/* variando o clock de alto para baixo, fazendo a leitura */
		SCK_Port |= (1 << SCK);											//SCK em nivel alto
	

		if((bit <= 14) && (bit >= 3)){									//apenas 12 bits dos 16 são importantes
			/* só os bits entrre 14 e 3 sao uteis para a leitura vide datasheet do max */
			if((SO_Pin & (1 << SO))){									// quando SO é 1 então
				bitnr--;												//decrementa o bitnr
				data_temperatura |= (1 << bitnr);								// poe rohdata para 1
			}else{														// quando SO não é 1 então
				bitnr--;												// decrementa o bitnr
				data_temperatura &= ~(1 << bitnr);								//  poe rohdata pra 0
			} 
			/* como são 12 bits de dados, na medida que for iniciada a leitura, o primeiro
			 bit não é importante para nós, assim como os bit abaixo de 3, assim vai para 12 a contagem
			 que é necessaria para nossa construção do sistema */
		}else{			
			//quando bit for maior que 14 e menor que 3, bitnr é 12
			bitnr = 12;
		}
		SCK_Port &= ~(1 << SCK);										//SCK LO	
	}

	CS_Port |= (1 << CS);												//cs em nivel alto para começar a funcionar		
	return data_temperatura;														
}

void USART_Init( unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}
void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}


static int putchar_buf(int c, FILE *__stream)
{
	output[ind++%128] = c;
	USART_Transmit(c);
	
	return c;
}

static FILE mystdout = FDEV_SETUP_STREAM(putchar_buf, NULL,_FDEV_SETUP_WRITE);



int main (){
	

	SO_DDR &= ~(1 << SO);						// define SO ddr(Data direction) como 0 ( input)
	CS_DDR |= (1 << CS);						// define o DDR do CS e SCK como 1 (output)
	SCK_DDR |= (1 << SCK);
	
	SO_Port |= (1<<SO);						   //pullups ligados			//Pullups an.

	CS_Port |= (1 << CS);				    	//CS HI	//tudo no default
	SCK_Port &= ~(1 << SCK);				    //SCK LO


	uint16_t data;
	uint16_t temperatura;


	USART_Init(MYUBRR);
	//leitura de dado puro do termopar
	while(1){
	
		data = gettemp();

		//dividido por 4 pois 12 bits são 4096 combinações possiveis e o termopar vai ate 1024 graus celsius
		temperatura = data*0.25;

		stdout = &mystdout;
		printf("temp é = %d\n",temperatura);
		_delay_ms(2000);
	}
	
}
