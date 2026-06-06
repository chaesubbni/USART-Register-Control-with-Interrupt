#define F_CPU 16000000UL  // MCU 메인 클럭 16MHz 설정
#include <avr/io.h>
#include <avr/interrupt.h>

// 1씩 증가하는 값을 저장할 전역 변수 (인터럽트 내부에서 사용하므로 volatile 필수)
volatile uint16_t count = 0;

// 편리한 출력을 위해 숫자를 문자열로 깨서 송신하는 함수
void USART_transmit_number(uint16_t num) {
	char buffer[10];
	int i = 0;
	
	if (num == 0) {
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = '0';
		return;
	}
	
	while (num > 0) {
		buffer[i++] = (num % 10) + '0';
		num /= 10;
	}
	
	// 역순으로 한 글자씩 송신
	for (int j = i - 1; j >= 0; j--) {
		while (!(UCSR0A & (1 << UDRE0))); // UDRE0 플래그 체크
		UDR0 = buffer[j];                 // 대입 연산자로 송신 버퍼에 탑재
	}
}

// 문자열 송신 함수
void USART_transmit_string(const char* str) {
	while (*str) {
		while (!(UCSR0A & (1 << UDRE0))); // 대기실이 비었는지 계기판 확인
		UDR0 = *str++;                    // 데이터 버스를 통해 한 글자씩 탑재
	}
}

// USART 초기화 함수
void USART_init(void) {
	// 1단계: 속도 설정 (16MHz 클럭 기준 9600bps -> 분주값 103)
	UBRR0H = 0;
	UBRR0L = 103;

	// 2단계: 통신 규격 계약 (비동기 모드, 패리티 없음, 정지 비트 1개, 데이터 크기 8비트)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// 3단계: 송수신 마스터 스위치 및 수신 완료 인터럽트 스위치 켜기
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
}

int main(void) {
	USART_init();  // 통신 하드웨어 세팅
	sei();         // 글로벌 인터럽트 스위치 ON (두뇌 가동)

	USART_transmit_string("USART Interrupt Test Start!\r\n");
	USART_transmit_string("Press any key to increment the count.\r\n\r\n");

	while (1) {
		// 메인 루프는 완벽하게 비워둡니다.
		// 하드웨어가 스스로 감시하다가 일을 처리할 것입니다.
	}
}

// 4단계: 실시간 데이터 흐름 (수신 완료 인터럽트 서비스 루틴)
ISR(USART_RX_vect) {
	// 1. 보관실(UDR0)에서 데이터를 읽는 순간 RXC0 플래그(알림 전구)가 자동으로 꺼집니다.
	// 외부에서 보낸 쓰레기 값을 읽어 대기실을 비워줍니다.
	char dummy = UDR0;

	// 2. 내부 연산: 카운트 값 1 증가
	count++;

	// 3. 컴퓨터(시리얼 모니터)에 결과 발사
	USART_transmit_string("Input Detected! Current Count -> ");
	USART_transmit_number(count);
	USART_transmit_string("\r\n");
}