#include <QTRSensors.h>
#include <TimerOne.h>
QTRSensors qtr;
const uint8_t SensorCount = 4;
uint16_t sensorValues[SensorCount];
unsigned int T;
unsigned int E = 0;

int motorEsquerda = 8;
int motorDireita = 13;
//
int F = 0;
int Contagem = 0;


int me = 11;
int md = 12;
unsigned char PwmD = 9;
unsigned char PwmE = 10;
//
int valorPID;
int erro;
int setpoint = 1500;

int Velocidade = 0;

float KP, KD, KI;
int velocidadeMotor1;
int velocidadeMotor2;

float I;
float P;
float D;

int erroAnterior = 0;
int novaVelocidadeMotor2;
int novaVelocidadeMotor1;
int DirecaoD = 1;
int DirecaoE = 1;

void Variaveis (void);
void Program (void);

void ConfigTimer(void) {
  Timer1.initialize(10000);
  Timer1.attachInterrupt(Tempo);
}
void Tempo(void) {
  noInterrupts();
  T++;
  E++;
  interrupts();
}
void setup()
{

  ConfigTimer();

  Serial.begin(9600);
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]) {
    A1, A2, A3, A4
  }, SensorCount);
  qtr.setEmitterPin(2);

  pinMode(motorEsquerda, OUTPUT);
  pinMode(motorDireita, OUTPUT);
  pinMode(me, OUTPUT);
  pinMode(md, OUTPUT);
  pinMode(PwmD, OUTPUT);
  pinMode(PwmE, OUTPUT);

  pinMode(A5, INPUT);
  pinMode(A0, INPUT);

  for (uint16_t i = 0; i < 400; i++)
  {
    qtr.calibrate();
    Serial.println("teste");
  }
}
///////////////// FIM DO SETUP //////////////////////
void loop() {
  if (Velocidade == 0) {
    Variaveis();
  }
  if (Velocidade != 0) {
    Program();
  }
}

void Variaveis (void) {
  if (T <= 200) {
    if (Contagem == 2 && digitalRead(6) == HIGH ) {
      Contagem = 3;
      T = 0;
      Serial.print(Contagem);
      delay(500);
    }
    else if (Contagem == 1 && digitalRead(6) == HIGH) {
      Contagem = 2;
      T = 0;
      Serial.print(Contagem);
      delay(500);
    }
    else if (Contagem == 0 && digitalRead(6) == HIGH) {
      Contagem = 1;
      T = 0;
      Serial.print(Contagem);
      delay(500);
    }
  }
  else if (Contagem != 0) {
    Velocidade = Contagem;
  }
  else {
    Contagem = 0;
    T = 0;
  }

  if (Velocidade == 1) {
    KP = 0.226825; KD = 0.36; KI=0;
    velocidadeMotor1 = 75;
    velocidadeMotor2 = 55;
    delay(3000);
    E=0;
  }
  if (Velocidade == 2) {
    KP = 0.222825; KD = 0.44; KI=0;
    velocidadeMotor1 = 100;
    velocidadeMotor2 = 85;
    delay(3000);
    E=0;
  }
  if (Velocidade == 3) {
    KP = 0.234; KD = 0.44; KI = 0.0097;
    velocidadeMotor1 = 140;
    velocidadeMotor2 = 110;
    delay(3000);
    E=0;
  }
}

void Program (void) {

  uint16_t position = qtr.readLineBlack(sensorValues);
  int leituraSensores = position;

  erro = leituraSensores - setpoint;
  P = erro;
  I = ( erro + erroAnterior);
  D = (erro - erroAnterior);
  valorPID = (P * KP + I * KI + D * KD);
  erroAnterior = erro;

  novaVelocidadeMotor2 = velocidadeMotor1 + valorPID;
  novaVelocidadeMotor1 = velocidadeMotor2 - valorPID;

  if (novaVelocidadeMotor1 <= 0) {
    novaVelocidadeMotor1 = -novaVelocidadeMotor1;
    DirecaoD = 0;
    if (novaVelocidadeMotor1 >= 100) {
      novaVelocidadeMotor1 = 100;
    }

  } else {
    DirecaoD = 1;
  }
  if (novaVelocidadeMotor2 <= 0) {
    novaVelocidadeMotor2 = -novaVelocidadeMotor2;
    DirecaoE = 0;
    if (novaVelocidadeMotor2 >= 100) {
      novaVelocidadeMotor2 = 100;
    }
  } else {
    DirecaoE = 1;
  }

  if (novaVelocidadeMotor1 >= 255) {
    novaVelocidadeMotor1 = 255;
  }
  if (novaVelocidadeMotor2 >= 255) {
    novaVelocidadeMotor2 = 255;
  }
  novaVelocidadeMotor1 = novaVelocidadeMotor1 * 4;
  novaVelocidadeMotor2 = novaVelocidadeMotor2 * 4;

  Timer1.pwm(PwmE, novaVelocidadeMotor1);
  Timer1.pwm(PwmD, novaVelocidadeMotor2);
  digitalWrite(me, DirecaoE);
  digitalWrite(motorEsquerda, digitalRead(me) ^ 1);
  digitalWrite(md, DirecaoD);
  digitalWrite(motorDireita, digitalRead(md) ^ 1);

      //Definição de tempo de parada.
  if ( E>=5600|| digitalRead(6) == HIGH) {
    digitalWrite(me, 1);
    digitalWrite(motorEsquerda, 1);
    digitalWrite(md, 1);
    digitalWrite(motorDireita, 1);
    Velocidade = 0;
    Contagem = 0;
    F = 0;
    erroAnterior = 0;
    delay(500);
  }
        //Codigo implementado para leitura e parada conforme sinais na pista,
        //Porem, por ma qualidade de pista e sensores, acabou nao sendo viavel
  //  if (analogRead(A5) >= 850 && analogRead(A0) <= 850) {
  //    if (T >= 3 && F == 1 && E>=2000) {
  //      digitalWrite(me, 1);
  //      digitalWrite(motorEsquerda, 1);
  //      digitalWrite(md, 1);
  //      digitalWrite(motorDireita, 1);
  //
  //      Velocidade = 0;
  //      Contagem = 0;
  //      F = 0;
  //      erroAnterior = 0;
  //
  //    }
  //    if (T >= 3 && F == 0) {
  //      F = 1;
  //      E = 0;
  //    }
  //
  //  } else {
  //    T = 0;
  //  }
}
