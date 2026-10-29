// robo4 — sketch de MONTAGEM
// Carregue ANTES de parafusar os braços (horns) dos servos.
// Ele põe os 8 servos em 90° e mantém ali. Com eles parados, encaixe:
//   - a coxa na vertical, apontando para o chão;
//   - a canela formando ângulo reto (90°) com a coxa.
// Depois carregue o firmware/robo4 e faça o ajuste fino com o comando "o".

#include <Servo.h>

const uint8_t PINOS[8] = {2, 3, 4, 5, 6, 7, 8, 9};
Servo s[8];

void setup() {
  Serial.begin(9600);
  for (uint8_t i = 0; i < 8; i++) {
    s[i].attach(PINOS[i]);
    s[i].write(90);
    delay(300);               // liga um por vez para não puxar pico da fonte
  }
  Serial.println(F("Todos os servos em 90 graus. Pode encaixar as pecas."));
}

void loop() {}
