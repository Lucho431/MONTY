/* Private macro -------------------------------------------------------------*/
//#define TRIG_SR04 ( __HAL_TIM_SET_COUNTER(&htim5, 0xFFFFFFF5) )


/* Private define ------------------------------------------------------------*/
#define IN_BUMP_L  7 //
#define IN_BUMP_R  8
#define IN_IR_L  A0
#define IN_IR_R  A1
#define IN_IR_GIRO //??????
#define IN_ECO_ULTRASONIDO  3
#define IN_FOTOTR_L 12
#define IN_FOTOTR_R 13


#define OUT_PWM_L1  6
#define OUT_PWM_L2  5
#define OUT_PWM_R1 9
#define OUT_PWM_R2 10
#define OUT_TRIG_ULTRASONIDO 4


/* Private typedef -----------------------------------------------------------*/
typedef enum{
	QUIETO,
	AVANZANDO,
	RETROCEDIENDO,
	ROTANDO_IZQ,
	ROTANDO_DER,
	PIVOTE_IZQ_AVAN,
	PIVOTE_DER_AVAN,
	PIVOTE_IZQ_RETR,
	PIVOTE_DER_RETR,
}T_MOV;


/* Private variables ---------------------------------------------------------*/

//prueba//
uint8_t read_button = 1;
uint8_t last_button = 1;

//movimiento//
T_MOV status_movimiento = QUIETO;
T_MOV last_movimiento = QUIETO;

//sensores//
uint8_t SI, SF, SD;
uint8_t sensores_dist = 0;

//ticks//
uint8_t desbordeTIM7 = 0; //desborda cada 10 ms.

//SR-04//
uint32_t ic1 = 0;
uint32_t ic2 = 0;
uint8_t flancoEco = 0; //cuando cuenta 2, detectó ambos flancos. Valores mayores, son errores.
int32_t cuentaPulsos = 0;
uint16_t distanciaEco = 0; //distancia en cm.

//encoders//
int16_t encoderL;
int16_t encoderR;


/* Private function prototypes -----------------------------------------------*/
void SR_04 (void);
void movimiento (void);
void encoders (void);

void prueba01 (void);
void prueba02 (void);
void prueba03 (void);
void prueba04 (void);

void setup() {
  Serial.begin(9600);   //iniciar puerto serie
  
  //pinMode(sensorPin , INPUT_PULLUP);  
  pinMode(IN_BUMP_L , INPUT);
  pinMode(IN_BUMP_R , INPUT);
  pinMode(IN_IR_L , INPUT);
  pinMode(IN_IR_R , INPUT);
  pinMode(IN_ECO_ULTRASONIDO, INPUT);
  //pinMode(sensorPin , INPUT_PULLUP);
  
  pinMode(OUT_PWM_L1 , OUTPUT);
  pinMode(OUT_PWM_L2 , OUTPUT);
  pinMode(OUT_PWM_R1 , OUTPUT);
  pinMode(OUT_PWM_R2 , OUTPUT);
  pinMode(OUT_TRIG_ULTRASONIDO , OUTPUT);
  //pinMode(sensorPin , OUTPUT);
  
}
 
void loop(){

	//Serial.print ("hola");
	prueba04();

	  //movimiento();

	  if (desbordeTIM7 > 21){
		  //encoders();
		  desbordeTIM7 = 0;
	  }
  
  
  
}


/* Private function prototypes -----------------------------------------------*/
void SR_04 (void){

	switch (flancoEco){
		case 0:
		case 1:
			return;
		case 2:
			cuentaPulsos = (int32_t)(ic2 - ic1);
			if (cuentaPulsos < 23310){
				distanciaEco = cuentaPulsos * 34 / 2000;
			}else{
				distanciaEco = 400;
			}
			flancoEco = 0;
		break;
		default:
			flancoEco = 0;
		break;
	} //end switch flancoEco

} //end SR_04()

void movimiento (void){

	//sensores_dist = SI << 2 | SF << 1 | SD (logica negativa)
	SI = digitalRead(IN_IR_L) ;
	SD = digitalRead(IN_IR_R);
	if (distanciaEco < 25) SF = 0; else SF = 1;

	sensores_dist = SI << 2 | SF << 1 | SD;


	switch (status_movimiento) {
		case QUIETO:

			digitalWrite(OUT_PWM_L2, 0);
			digitalWrite(OUT_PWM_R2, 0);

			digitalWrite(OUT_PWM_L1, 0);
			digitalWrite(OUT_PWM_R1, 0);

			status_movimiento = AVANZANDO;
		break;
		case AVANZANDO:

			digitalWrite(OUT_PWM_L2, 1);
			digitalWrite(OUT_PWM_R2, 1);

			digitalWrite(OUT_PWM_L1, 0);
			digitalWrite(OUT_PWM_R1, 0);

			switch (sensores_dist) {
				case 0b110:
				case 0b101:
				case 0b100:
				case 0b000:
					status_movimiento = ROTANDO_IZQ;
					break;
				case 0b011:
				case 0b001:
					status_movimiento = ROTANDO_DER;
					break;
				default:
					break;
			} //end switch sensores_dist

		break;
		case ROTANDO_IZQ:

			digitalWrite(OUT_PWM_L2, 0);
			digitalWrite(OUT_PWM_R2, 1);

			digitalWrite(OUT_PWM_L1, 1);
			digitalWrite(OUT_PWM_R1, 0);

			switch (sensores_dist){
				case 0b111:
					status_movimiento = AVANZANDO;
					break;
				case 0b011:
					status_movimiento = ROTANDO_DER;
				default:
					break;
			} //end switch sensores_dist

		break;
		case ROTANDO_DER:
		
			digitalWrite(OUT_PWM_L2, 1);
			digitalWrite(OUT_PWM_R2, 0);

			digitalWrite(OUT_PWM_L1, 0);
			digitalWrite(OUT_PWM_R1, 1);

			switch (sensores_dist){
				case 0b111:
					status_movimiento = AVANZANDO;
					break;
				case 0b110:
					status_movimiento = ROTANDO_IZQ;
				default:
					break;
			} //end switch sensores_dist

		break;
		case RETROCEDIENDO:
		
			digitalWrite(OUT_PWM_L2, 0);
			digitalWrite(OUT_PWM_R2, 0);

			digitalWrite(OUT_PWM_L1, 1);
			digitalWrite(OUT_PWM_R1, 1);
			break;
		case PIVOTE_IZQ_AVAN:

		break;
		case PIVOTE_DER_AVAN:

		default:
		break;

	} //fin switch status_movimiento

} //fin movimiento()

void encoders (void){
/*
	encoderL = __HAL_TIM_GET_COUNTER(&htim3);
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	encoderR = __HAL_TIM_GET_COUNTER(&htim2);
	__HAL_TIM_SET_COUNTER(&htim2, 0);

	if (encoderL > 5){
		TIM4->CCR2--;
	}else if (encoderL < 5){
		TIM4->CCR2++;
	}

	if (encoderR > 5){
		TIM4->CCR1--;
	}else if (encoderR < 5){
		TIM4->CCR1++;
	}
*/ 
} //fin encoders()

void prueba01 (void){
		digitalWrite(OUT_PWM_L1, 1);
	digitalWrite(OUT_PWM_R1, 1);
	
	digitalWrite(OUT_PWM_L2, 0);
	digitalWrite(OUT_PWM_R2, 0);
	
	delay(1000);
	
	digitalWrite(OUT_PWM_L1, 1);
	digitalWrite(OUT_PWM_R1, 0);
	
	digitalWrite(OUT_PWM_L2, 0);
	digitalWrite(OUT_PWM_R2, 1);
	
	delay(1000);
	
	digitalWrite(OUT_PWM_L1, 0);
	digitalWrite(OUT_PWM_R1, 1);
	
	digitalWrite(OUT_PWM_L2, 1);
	digitalWrite(OUT_PWM_R2, 0);
	
	delay(1000);
	
	digitalWrite(OUT_PWM_L1, 0);
	digitalWrite(OUT_PWM_R1, 0);
	
	digitalWrite(OUT_PWM_L2, 1);
	digitalWrite(OUT_PWM_R2, 1);
	
	delay(1000);
	
	digitalWrite(OUT_PWM_L1, 0);
	digitalWrite(OUT_PWM_R1, 0);
	
	digitalWrite(OUT_PWM_L2, 0);
	digitalWrite(OUT_PWM_R2, 0);
	
	delay(5000);
} //fin prueba01 ()

void prueba02 (void){
	
		if(digitalRead(IN_BUMP_L) == 1){
		digitalWrite(OUT_PWM_L1, 0);
		digitalWrite(OUT_PWM_R1, 1);
	
		digitalWrite(OUT_PWM_L2, 0);
		digitalWrite(OUT_PWM_R2, 0);
	}else if (digitalRead(IN_BUMP_R) == 1){
		digitalWrite(OUT_PWM_L1, 0);
		digitalWrite(OUT_PWM_R1, 0);
	
		digitalWrite(OUT_PWM_L2, 0);
		digitalWrite(OUT_PWM_R2, 1);
	}else{
		digitalWrite(OUT_PWM_L1, 0);
		digitalWrite(OUT_PWM_R1, 0);
	
		digitalWrite(OUT_PWM_L2, 0);
		digitalWrite(OUT_PWM_R2, 0);
	}
} //fin prueba02 ()

void prueba03 (void){
	
	if(digitalRead(IN_FOTOTR_L) == 0){ //LOGICA NEGATIVA
		digitalWrite(OUT_PWM_L1, 1);
		digitalWrite(OUT_PWM_R1, 0);
	
		digitalWrite(OUT_PWM_L2, 0);
		digitalWrite(OUT_PWM_R2, 0);
	}else if (digitalRead(IN_FOTOTR_R) == 0){ //LOGICA NEGATIA
		digitalWrite(OUT_PWM_L1, 0);
		digitalWrite(OUT_PWM_R1, 0);
	
		digitalWrite(OUT_PWM_L2, 1);
		digitalWrite(OUT_PWM_R2, 0);
	}else{
		digitalWrite(OUT_PWM_L1, 0);
		digitalWrite(OUT_PWM_R1, 0);
	
		digitalWrite(OUT_PWM_L2, 0);
		digitalWrite(OUT_PWM_R2, 0);
	}
} //fin prueba03 ()

void prueba04 (void){
	
	uint32_t pulsos;
	
	digitalWrite(OUT_TRIG_ULTRASONIDO, LOW);  //para generar un pulso limpio ponemos a LOW 4us
	delayMicroseconds(4);
	digitalWrite(OUT_TRIG_ULTRASONIDO, HIGH);  //generamos Trigger (disparo) de 10us
	delayMicroseconds(10);
	digitalWrite(OUT_TRIG_ULTRASONIDO, LOW);
	
	//pulsos = pulseIn(IN_ECO_ULTRASONIDO, LOW, 14600);  //medimos el tiempo entre pulsos, en microsegundos. TIMEOUT : 14600 us == 250 cm   
	pulsos = pulseIn(IN_ECO_ULTRASONIDO, LOW);  //medimos el tiempo entre pulsos, SIN TIMEOUT
	distanciaEco = pulsos * 10 / 292/ 2;
   
	Serial.print("Distancia: ");
	Serial.println(distanciaEco);
	delay(500) ;
	
} //fin prueba04 ()
