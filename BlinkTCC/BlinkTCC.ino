#define BLYNK_TEMPLATE_ID "TMPL2KqGawTHM"
#define BLYNK_TEMPLATE_NAME "TCCarduinoCozinha"
#define BLYNK_AUTH_TOKEN "fj4Z9g4gXqzDlfePQKmvxEEhEoul5FZs"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

BlynkTimer timer;

// Pinos virtuais
int pinTemp = V1;         // Sensor de temperatura
int pinUmi = V0;          // Sensor de umidade
int pinFumaca = V3;       // Sensor de fumaça
int alertaFumaca = V4;    // Alerta de fumaça
int pinChamas = V2;       // Sensor de chamas
int alertaChamas = V6;    // Alerta de chamas
int pinGas = V7;          // Sensor de gás
int alertaGas = V8;       // Alerta de gás
int ledWhiteControl = V9; // Controle da luz branca na fita RGB
int ledRGBControl = V10;  // Controle das cores RGB na fita RGB

// Pinos físicos
int buzzerPin = 5;         // Pino do buzzer (GPIO 5)
int chamasSensor = 16;     // Pino do sensor de chamas (GPIO 16)
int dhtPin = 17;           // Pino do sensor DHT11 (GPIO 17)
int gasSensorPin = 36;     // Pino do sensor de gás (ADC1_CH0 no ESP32)
int fumacaSensorPin = 39;  // Pino do sensor de fumaça (ADC1_CH3 no ESP32)
int ledRedPin = 22;        // Pino para controle do LED vermelho da fita RGB
int ledGreenPin = 21;      // Pino para controle do LED verde da fita RGB
int ledBluePin = 19;       // Pino para controle do LED azul da fita RGB

// Canais PWM
int pwmChannelRed = 0;
int pwmChannelGreen = 1;
int pwmChannelBlue = 2;
int pwmFrequency = 5000;
int pwmResolution = 8;

// Variáveis para controle de notificação
bool fogoFlag = false;
bool fumacaFlag = false;
bool gasFlag = false;

// Inicialização do sensor DHT
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

char ssid[] = "YourSSID";
char pass[] = "YourPassword";

void setup() {
  // Configuração dos pinos como saída ou entrada
  pinMode(buzzerPin, OUTPUT);
  pinMode(chamasSensor, INPUT_PULLUP);
  pinMode(gasSensorPin, INPUT);
  pinMode(fumacaSensorPin, INPUT);

  // Configuração dos canais PWM
  ledcSetup(pwmChannelRed, pwmFrequency, pwmResolution);
  ledcSetup(pwmChannelGreen, pwmFrequency, pwmResolution);
  ledcSetup(pwmChannelBlue, pwmFrequency, pwmResolution);

  // Associa os canais PWM aos pinos
  ledcAttachPin(ledRedPin, pwmChannelRed);
  ledcAttachPin(ledGreenPin, pwmChannelGreen);
  ledcAttachPin(ledBluePin, pwmChannelBlue);

  // Inicia a comunicação serial
  Serial.begin(115200);

  // Inicia o sensor DHT
  dht.begin();

  // Configuração do Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer para leitura dos sensores
  timer.setInterval(1000L, leituraSensores);
}

void loop() {
  Blynk.run();
  timer.run(); // Executa as funções do timer
}

void leituraSensores() {
  // Leitura do sensor DHT (umidade e temperatura)
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  // Verifica se a leitura do sensor DHT falhou
  if (isnan(umidade) || isnan(temperatura)) {
    Blynk.virtualWrite(alertaChamas, "Falha ao ler do sensor DHT!");
    return;
  }

  // Envia as leituras para o Blynk
  Blynk.virtualWrite(pinTemp, temperatura);
  Blynk.virtualWrite(pinUmi, umidade);

  // Leitura do sensor de fumaça
  int fumaca = digitalRead(fumacaSensorPin);
  if (fumaca == HIGH && !fumacaFlag) {
    Blynk.notify("Alerta! Fumaça Detectada");
    Blynk.virtualWrite(alertaFumaca, "Alerta! Fumaça Detectada");
    fumacaFlag = true;
  } else if (fumaca == LOW) {
    fumacaFlag = false;
    Blynk.virtualWrite(alertaFumaca, "");
  }

  // Leitura do sensor de chamas
  int chamas = digitalRead(chamasSensor);
  if (chamas == HIGH && !fogoFlag) {
    Blynk.notify("Alerta! Chamas Detectadas");
    Blynk.virtualWrite(alertaChamas, "Alerta! Chamas Detectadas");
    fogoFlag = true;
    digitalWrite(buzzerPin, HIGH); // Liga o buzzer
  } else if (chamas == LOW) {
    fogoFlag = false;
    digitalWrite(buzzerPin, LOW); // Desliga o buzzer
    Blynk.virtualWrite(alertaChamas, "");
  }

  // Leitura do sensor de gás
  int gas = analogRead(gasSensorPin);
  if (gas > 400 && !gasFlag) { // Supondo que 400 seja o limite para detecção de gás
    Blynk.notify("Alerta! Gás Detectado");
    Blynk.virtualWrite(alertaGas, "Alerta! Gás Detectado");
    gasFlag = true;
  } else if (gas <= 400) {
    gasFlag = false;
    Blynk.virtualWrite(alertaGas, "");
  }
}

// Função Blynk para controlar a luz branca na fita RGB
BLYNK_WRITE(ledWhiteControl) {
  int ledState = param.asInt();
  if (ledState == 1) {
    ledcWrite(pwmChannelRed, 255);   // Máximo valor para vermelho
    ledcWrite(pwmChannelGreen, 255); // Máximo valor para verde
    ledcWrite(pwmChannelBlue, 255);  // Máximo valor para azul (branco)
  } else {
    ledcWrite(pwmChannelRed, 0);
    ledcWrite(pwmChannelGreen, 0);
    ledcWrite(pwmChannelBlue, 0);
  }
}

// Função Blynk para controlar as cores RGB na fita RGB
BLYNK_WRITE(ledRGBControl) {
  int redValue = param[0].asInt();   // Valor do canal vermelho
  int greenValue = param[1].asInt(); // Valor do canal verde
  int blueValue = param[2].asInt();  // Valor do canal azul

  ledcWrite(pwmChannelRed, redValue);
  ledcWrite(pwmChannelGreen, greenValue);
  ledcWrite(pwmChannelBlue, blueValue);
}
