// Defini��o de Rota � definida via terminal

#include <TinyGPS.h>

#define TIMEOUT 10000; // Valor para identificar Timeout por desconex�o

// inicializando vari�veis e objetos
TinyGPS gps1; // Objeto de classe de GPS
int levelSensorPin = A0; // Pino de sensor de n�vel de �gua
int flowSensorPin = 2; // Pino de sensor de fluxo de �gua
volatile long pulse;
unsigned long lastTime;
unsigned long lastFlowSignalTime; // Vari�vel para armazenar tempo de �ltimo sinal do sensor de fluxo
unsigned long lastLevelSignalTime; // Vari�vel para armazenar tempo de �ltimo sinal do sensor de n�vel
// Booleans para indicar desconex�o
bool isDisconnectedFlow;
bool isDisconnectedLevel;

void setup(){
	pinMode(flowSensorPin, INPUT); // definindo modo do sensor de fluxo
	// inicializando portas seriais
	Serial.begin(9600);
	Serial1.begin(9600);
	Serial2.begin(9600);
	attachInterrupt(digitalPinToInterrupt(flowSensorPin), increasePulseCounter, RISING); // configura��o de funcionamento e tratamento de dados do sensor de fluxo
	unsigned long lastFlowSignal = millis();
	unsigned long lastLevelSignal = millis();
}

void loop(){
	bool received = false; // booleano para informar estado de recebimento de dados do GPS
	while (Serial1.available()){
		char cIn = Serial1.read();
		received = gps1.encode(cIn); // check de recebimento de dados do GPS
	}
	if (received){
		// informa��es recebidas do GPS abaixo
		long latitude, longitude;
		unsigned long ageInfo; // Vari�vel para tempo que os dados foram recebidos
		gps1.get_position(&latitude, &longitude, &ageInfo); // Receber localiza��o em latitude e longitude
		
		// Dados referentes � data da informa��o abaixo
		int year;
		byte month, day, hour, minute, second, centisecond;
		gps1.crack_datertime(&year, &month, &day, &hour, &minute, &second, &centisecond); //  Receber Data e Hor�rio
	}
	int flowSensorState = digitalRead(flowSensorPin);
	
	if(flowSensorState == HIGH){ // Checagem de conex�o do sensor de fluxo
		lastFlowSignal = millis();
		isDisconnectedFlow = false;
	}

	if(millis() - lastFlowSignal > TIMEOUT && !isDisconnectedFlow){// Caso timeout tenha ocorrido
		isDisconnectedFlow = true;
		Serial2.println("Poss�vel Disconex�o! " + alerta(latitude, longitude, year, month, day, hour, minute, second, centisecond, ageInfo));
	}

	int levelSensorValue = analogRead(waterSensorPin); // Lendo valores do sensor de n�vel da �gua
	
	if(levelSensorValue >= 0 && levelSensorValue <= 1023){ // Checagem de conex�o do sensor de n�vel
		lastLevelSignal = millis();
		isDisconnectedLevel = false;
	}

	if(millis() - lastLevelSignal > TIMEOUT && !isDisconnectedLevel){// Caso timeout tenha ocorrido
		isDisconnectedLevel = true;
		Serial2.println("Poss�vel Disconex�o! " + alerta(latitude, longitude, year, month, day, hour, minute, second, centisecond, ageInfo));
	}

	volume = 2.663 * pulse; // Volume passando pelo sensor de fluxo (mL)
	if (millis() - lastTime > 1000){ //Condi��o feita para identificar mL/s
		pulse = 0;
		lastTime = millis();
		if(levelSensorValue > 500 && volume > 1000){ // Condi��o para detectar se os n�veis e fluxo da �gua est�o fortes no n�vel de alerta
			Serial2.println("Alerta! " + alerta(latitude, longitude, year, month, day, hour, minute, second, centisecond, ageInfo));
			// Acima, envio de informa��es a respeito do alerta, incluindo localiza��o e hor�rio
		}
	}
}

void increasePulseCounter() {
	pulse++; // Fun��o para incrementar contagem dos inputs do sensor de fluxo
}

// Fun��o para retornar informa��es sobre alerta
String alerta(long latitude, long longitude, int year, byte month, byte day, byte hour, byte minute, byte second, byte centisecond, unsigned long ageInfo) {
	return "Latitude: " + latitude + "; Longitude: " + longitude + "\n" + day + "/" + month + "/" +  year + " " + hour + ":" + minute + ":" + second + "\nIdade da informa��o: " + ageInfo + "(ms)";
}