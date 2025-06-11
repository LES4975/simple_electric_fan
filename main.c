#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BUTTON_DDR		DDRD
#define BUTTON_PORT		PORTD
#define BUTTON_PIN		PIND
#define	BUTTON_OFF		1 // 선풍기 OFF
#define BUTTON_WEAK		2 // 약풍(50%)
#define BUTTON_STRONG	0 // 강풍(100%)

#define LED_DDR			DDRA
#define LED_PORT		PORTA

#define PAN_DDR			DDRB

typedef struct{
	volatile uint8_t *ddr;
	volatile uint8_t *pin;
	uint8_t btnPin;
}BUTTON;

enum {STOP_PAN, WEAK_WIND, STRONG_WIND}; //{0, 1, 2}

uint8_t FND_Number[] =
{0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x27, 0x7F, 0x67};
volatile uint8_t curState = STOP_PAN;

void buttonInit(BUTTON *button, volatile uint8_t *ddr, volatile uint8_t *pin, uint8_t pinNum);
void panInit();
void spinningPan(volatile uint8_t play);
void fndDisplay(volatile uint8_t play);

int main(void)
{
	BUTTON btn0, btn1, btn2;
	
	buttonInit(&btn0, &BUTTON_DDR, &BUTTON_PIN, BUTTON_OFF);
	buttonInit(&btn1, &BUTTON_DDR, &BUTTON_PIN, BUTTON_WEAK);
	buttonInit(&btn2, &BUTTON_DDR, &BUTTON_PIN, BUTTON_STRONG);
	
	DDRA = 0xff; //FND를 출력 모드로 설정
	PORTA = FND_Number[curState];
	panInit();
	
	// 외부 인터럽트 설정 (INT1, INT2, INT3 사용)
	EIMSK |= (1 << INT1) | (1 << INT2) | (1 << INT0); // INT1, INT2, INT3 활성화
	EICRA |= (1 << ISC11) | (1 << ISC21) | (1 << ISC01); // INT1, INT2, INT3을 하강엣지(1,0)에서 트리거
	EIFR |= (1 << INTF1) | (1 << INTF2) | (1 << INTF0); // 인터럽트 플래그 초기화

	
	sei(); // 전역 인터럽트 활성화

	while (1)
	{
		spinningPan(curState);
		fndDisplay(curState);
		
	}

	return 0;
}

void buttonInit(BUTTON *button, volatile uint8_t *ddr, volatile uint8_t *pin, uint8_t pinNum)
{
	button->ddr = ddr;
	button->pin = pin;
	button->btnPin = pinNum;
	*button->ddr &= ~(1<<button->btnPin); //버튼 핀을 입력 상태로
}

void panInit()
{
	PAN_DDR |= (1 << DDRB4); // OC0 핀을 출력으로 설정
	TCCR0 |= (1 << WGM00) | (1 << COM01) | (1 << WGM01) | (1 << CS01) | (1 << CS00); //Fast PWM 모드
}

void spinningPan(volatile uint8_t play)
{
	switch (play)
	{
		case STOP_PAN:
		OCR0 = 0;
		break;
		
		case WEAK_WIND:
		OCR0 = 127;
		break;
		
		case STRONG_WIND:
		OCR0 = 254;
		break;
	}
}

void fndDisplay(volatile uint8_t play)
{
	LED_PORT = FND_Number[play];
}

// 외부 인터럽트 ISR (PD1 ~ PD3 버튼 제어)
ISR(INT1_vect) { curState = STOP_PAN; }
ISR(INT2_vect) { curState = WEAK_WIND; }
ISR(INT0_vect) { curState = STRONG_WIND; }