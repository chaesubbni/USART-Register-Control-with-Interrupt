# USART-Register-Control-with-Interrupt

본 프로젝트는 외부 라이브러리에 전혀 의존하지 않고, ATmega328P MCU의 하드웨어 레지스터를 직접 제어하여 USART(비동기 시리얼) 통신을 밑바닥부터 구현한 프로젝트입니다. 컴퓨터로 문자 1개를 전송하면 MCU가 수신해 숫자를 1 증가시킨 후 다시 컴퓨터로 송신해 최종적으로 문자를 입력할때마다 증가된 값을 입력받는 구조입니다.
<img width="737" height="492" alt="image" src="https://github.com/user-attachments/assets/4e3d5c65-901c-4b91-a246-64170ba40df5" />

## ⚙️ Hardware Flow
통신이 이루어지는 과정은 크게 3가지 하드웨어 파트로 나뉩니다.
<img width="820" height="767" alt="image" src="https://github.com/user-attachments/assets/2d5c2dff-7f0e-40b0-b2c3-a0b375715251" />


1. **Clock Generator (클럭 생성부):** MCU의 메인 클럭(16MHz)을 쪼개어 목표 통신 속도(Baud Rate)에 맞는 정확한 타이밍을 생성합니다.
2. **Transmitter (송신부):** CPU가 `UDRn` 레지스터에 데이터를 쓰면, Shift Register로 넘어가 패리티 비트와 함께 1비트씩 외부 핀(`TxDn`)으로 순차 송신됩니다.
3. **Receiver (수신부):** 외부 핀(`RxDn`)으로 들어오는 신호를 샘플링해 노이즈를 걸러내고, Shift Register에서 8비트로 조립한 뒤 `UDRn` 수신 버퍼에 보관합니다.

## 🛠 Core Registers Analysis

통신을 제어하기 위해 반드시 이해해야 하는 5가지 핵심 레지스터입니다.

* **`UDRn` (I/O Data Register)**
  * 송신 버퍼(TXB)와 수신 버퍼(RXB)가 같은 주소를 공유합니다.
  * **주의:** 비트 연산(`|=`, `&=`)을 통한 Read-Modify-Write는 절대 금지됩니다. 수신 데이터 증발 및 원치 않는 송신을 막기 위해 반드시 **대입 연산자(`=`)**로 통째로 8비트를 읽고 써야 합니다.
* **`UCSRnA` (Status Register)**
  * 통신의 실시간 상태를 보여주는 계기판입니다.
  * **주요 플래그:** `RXCn`(수신 완료), `TXCn`(송신 완료), `UDREn`(송신 버퍼 비어있음)
  * 에러 감지: `FEn`(프레임 에러), `DORn`(오버런 에러), `UPEn`(패리티 에러)
* **`UCSRnB` (Control Switch)**
  * 송수신 기능 및 인터럽트 활성화 스위치입니다. (`RXENn`, `TXENn`, `RXCIEn` 등)
* **`UCSRnC` (Communication Format)**
  * 통신 규격을 정하는 계약서 역할을 합니다. (비동기 모드, 패리티 없음, 정지 비트 1개, 8비트 데이터 크기 등)
* **`UBRRnH / UBRRnL` (Baud Rate Register)**
  * 목표 Baud Rate(예: 9600bps)를 맞추기 위한 분주값을 설정합니다. 통신 오차율(±2% 이내 권장)을 고려하여 설정해야 합니다.

## 🚀 Implementation Steps

1. **속도 설정:** `UBRRn` 레지스터에 데이터시트를 참고하여 계산된 분주값 대입.
2. **규격 계약:** `UCSRnC`에서 데이터 크기, 패리티 유무, 정지 비트 설정.
3. **스위치 ON:** `UCSRnB`에서 송수신 활성화 및 필요시 인터럽트 활성화.
4. **데이터 흐름 제어:** 송신 시 `UDREn` 플래그 체크, 수신 시 `RXCn` 플래그 체크 후 `UDRn` 버퍼 접근.
