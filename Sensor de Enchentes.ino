// Definição de Rota é definida via terminal

#include <TinyGPS.h>

#define TIMEOUT 10000; // Valor para identificar Timeout por desconexão

// inicializando variáveis e objetos
TinyGPS gps1; // Objeto de classe de GPS
int levelSensorPin = A0; // Pino de sensor de nível de água
int flowSensorPin = 2; // Pino de sensor de fluxo de água
volatile long pulse;
unsigned long lastTime;
unsigned long lastFlowSignalTime; // Variável para armazenar tempo de último sinal do sensor de fluxo
unsigned long lastLevelSignalTime; // Variável para armazenar tempo de último sinal do sensor de nível
// Booleans para indicar desconexão
bool isDisconnectedFlow;
bool isDisconnectedLevel;

void setup(){
	pinMode(flowSensorPin, INPUT); // definindo modo do sensor de fluxo
	// inicializando portas seriais
	Serial.begin(9600);
	Serial1.begin(9600);
	Serial2.begin(9600);
	attachInterrupt(digitalPinToInterrupt(flowSensorPin), increasePulseCounter, RISING); // configuração de funcionamento e tratamento de dados do sensor de fluxo
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
		// informações recebidas do GPS abaixo
		long latitude, longitude;
		unsigned long ageInfo; // Variável para tempo que os dados foram recebidos
		gps1.get_position(&latitude, &longitude, &ageInfo); // Receber localização em latitude e longitude
		
		// Dados referentes à data da informação abaixo
		int year;
		byte month, day, hour, minute, second, centisecond;
		gps1.crack_datertime(&year, &month, &day, &hour, &minute, &second, &centisecond); //  Receber Data e Horário
	}
	int flowSensorState = digitalRead(flowSensorPin);
	
	if(flowSensorState == HIGH){ // Checagem de conexão do sensor de fluxo
		lastFlowSignal = millis();
		isDisconnectedFlow = false;
	}

	if(millis() - lastFlowSignal > TIMEOUT && !isDisconnectedFlow){// Caso timeout tenha ocorrido
		isDisconnectedFlow = true;
		Serial2.println("Possível Disconexão! " + alerta(latitude, longitude, year, month, day, hour, minute, second, centisecond, ageInfo));
	}

	int levelSensorValue = analogRead(waterSensorPin); // Lendo valores do sensor de nível da água
	
	if(levelSensorValue >= 0 && levelSensorValue <= 1023){ // Checagem de conexão do sensor de nível
		lastLevelSignal = millis();
		isDisconnectedLevel = false;
	}

	if(millis() - lastLevelSignal > TIMEOUT && !isDisconnectedLevel){// Caso timeout tenha ocorrido
		isDisconnectedLevel = true;
		Serial2.println("Possível Disconexão! " + alerta(latitude, longitude, year, month, day, hour, minute, second, centisecond, ageInfo));
	}

	volume = 2.663 * pulse; // Volume passando pelo sensor de fluxo (mL)
	if (millis() - lastTime > 1000){ //Condição feita para identificar mL/s
		pulse = 0;
		lastTime = millis();
		if(levelSensorValue > 500 && volume > 1000){ // Condição para detectar se os níveis e fluxo da água estão fortes no nível de alerta
			Serial2.println("Alerta! " + alerta(latitude, longitude, year, month, day, hour, minute, second, centisecond, ageInfo));
			// Acima, envio de informações a respeito do alerta, incluindo localização e horário
		}
	}
}

void increasePulseCounter() {
	pulse++; // Função para incrementar contagem dos inputs do sensor de fluxo
}

// Função para retornar informações sobre alerta
String alerta(long latitude, long longitude, int year, byte month, byte day, byte hour, byte minute, byte second, byte centisecond, unsigned long ageInfo) {
	return "Latitude: " + latitude + "; Longitude: " + longitude + "\n" + day + "/" + month + "/" +  year + " " + hour + ":" + minute + ":" + second + "\nIdade da informação: " + ageInfo + "(ms)";
}