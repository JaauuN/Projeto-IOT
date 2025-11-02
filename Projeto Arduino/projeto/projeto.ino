#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "pitches.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
long  tempoInicioRodada;
const int botoes[] = {7, 6, 5};
const int leds[] = {4, 3, 2};
const int buzzer = 8;
const int numeroLeds = 3;
int ledAceso = -1; // Qual LED está aceso (-1 = nenhum)
int NOTE_SUSTAIN = 100;

long pontuacao = 0; // Contador de pontuação
long tempoLimite = 1300; // Tempo maximo para reagir
bool iniciarJogo = false; // Controle do estado do jogo
int faseAtual = 1; // Controle da fase atual do jogo
int mecanica = 1; // Sistema de mecanicas
// Controle do tempo minimo e maximo da seleção aleatorio dos leds
long delayMin = 1000;
long delayMax = 5000;

void escolherNovoLed() {
  int proximoLed;
  do {
    proximoLed = random(numeroLeds);
  } while (proximoLed == ledAceso);

  ledAceso = proximoLed;
  digitalWrite(leds[ledAceso], HIGH);
  mecanica = random(1,3); // Seleção aleatoria entre as 2 mecanicas
  // Toque do buzzer para diferenciar quando é clique duplo e quando é clique normal.
  if (mecanica == 2) {
    tone(buzzer, 1500, 100);
    delay(200);
    tone(buzzer, 1500, 100);
  } else {
    tone(buzzer, 1500, 100);
  }
  
  tempoInicioRodada = millis();
}

void Iniciar() {
  //Reset completo de todos os valores de controle de fase e tempo toda vez que o jogo é iniciado novamente.
  iniciarJogo = true;
  pontuacao = 0;
  faseAtual = 1;
  tempoLimite = 1300;
  delayMin = 1000;
  delayMax = 3000;
  ledAceso = -1; 

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando jogo...");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pontos: 0");
  
  // Este delay faz com que o jogador não consiga clicar antes do led acender.
  long delayparaClicar = random(1000, 3001); 
  delay(delayparaClicar);
  escolherNovoLed();
}

void gameOver() {
  tone(buzzer, 200, 500); 
  if (ledAceso != -1) { 
     digitalWrite(leds[ledAceso], LOW);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print("Pontos: ");
  lcd.print(pontuacao);
  delay(4000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aperte qualquer");
  lcd.setCursor(0, 1);
  lcd.print("Botao p/ resetar");

  iniciarJogo = false;
  ledAceso = -1;
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Aperte qualquer");
  lcd.setCursor(0, 1);
  lcd.print("Botao p/ iniciar");

  pinMode(buzzer, OUTPUT);

  for (int i = 0; i < numeroLeds; i++) {
    pinMode(leds[i], OUTPUT);
  }

  for (int i = 0; i < numeroLeds; i++) {
    pinMode(botoes[i], INPUT_PULLUP); 
  }

  randomSeed(analogRead(A0));
}

void loop() {
  if (!iniciarJogo) {
    // Estrutura de repetição para ler a quantidade de leds que tambem é a quantidade de botoes
    // E se qualquer um dos botoes for apertado, inicia o jogo
    for (int i = 0; i < numeroLeds; i++) {
      if (digitalRead(botoes[i]) == LOW) { // <<< MUDANÇA 2
        Iniciar();
        while(digitalRead(botoes[0]) == LOW || // <<< MUDANÇA 3
              digitalRead(botoes[1]) == LOW || 
              digitalRead(botoes[2]) == LOW) {
           delay(10);
        }
        break;
      }
    }
  } else {
    if (millis() - tempoInicioRodada > tempoLimite) {
      gameOver();
      return;
    }
    //O jogador apertou algum botão?
    for (int i = 0; i < numeroLeds; i++) {
      if (digitalRead(botoes[i]) == LOW) {
        if (ledAceso == -1) {
            gameOver();
            break;
        }

        // Se acertar o botao aceso
        if (i == ledAceso) {
          long tempoDeReacao = millis() - tempoInicioRodada;
          if (mecanica == 2) {
            while(digitalRead(botoes[i]) == LOW); 
            long segundoClick = millis();
            while(digitalRead(botoes[i]) == HIGH) {
              if (millis() - segundoClick > 400) {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Click duplo");
                lcd.setCursor(0, 1);
                lcd.print("Muito lento");
                delay(1500);
                gameOver();
                return;
              }
              if (millis() - tempoInicioRodada > tempoLimite) {
                gameOver();
                return;
              }
            }
          }
          // Som de moeda com o buzzer pego neste site: https://gist.github.com/bboyho/0fcd284f411b1cfc7274336fde6abb45
          tone(8,NOTE_B5,100);
          delay(100);
          tone(8,NOTE_E6,850);
          delay(300);
          noTone(8);

          int pontosGanhos = 0;
          String mensagem = "Lento demais...";

          // Esta parte é onde identifica o tempo de reação do jogador e atualiza a pontuação baseado no tempo de reação
          if (tempoDeReacao <= 400) {
            pontosGanhos = 300;
            mensagem = "Rapido! +300";
          } else if (tempoDeReacao <= 600) {
            pontosGanhos = 100;
            mensagem = "Bom! +100";
          } else if (tempoDeReacao <= 800) {
            pontosGanhos = 50;
            mensagem = "Lento. +50";
          }

          pontuacao += pontosGanhos;
          // Esta parte muda a fase do jogo para o total de pontos que você tiver ganho
          int faseAntiga = faseAtual;
          if (pontuacao >= 2800) { 
          faseAtual = 4;
        } else if (pontuacao >= 1700) {
          faseAtual = 3;
        } else if (pontuacao >= 800) {
          faseAtual = 2;
        }

        // Esta parte é a configuração de tempo quando troca de fase baseado nos pontos acima
        if (faseAtual > faseAntiga) {
          switch (faseAtual) {
            case 2:
              tempoLimite = 1100;
              delayMin = 900;
              delayMax = 2000;
              break;
            case 3:
              tempoLimite = 700;
              delayMin = 600;
              delayMax = 800;
              break;
            case 4:
              tempoLimite = 300;
              delayMin = 100;
              delayMax = 1000;
              break;
          }

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Fase ");
          lcd.print(faseAtual);
          lcd.setCursor(0, 1);
          lcd.print("Iniciando...");
          // Som do buzzer quuando passa de fase retirado deste site https://forum.arduino.cc/t/piezo-buzzer-win-and-fail-sound/133792
          tone(buzzer,NOTE_A5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_B5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_C5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_B5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_C5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_D5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_C5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_D5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_E5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_D5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_E5);
           delay(NOTE_SUSTAIN);
           tone(buzzer,NOTE_E5);
           delay(NOTE_SUSTAIN);
           noTone(8);
           delay(2000);
        }
          
    
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Pontos: ");
          lcd.print(pontuacao);
          lcd.setCursor(0, 1);
          lcd.print(mensagem);

          digitalWrite(leds[ledAceso], LOW);
          
          ledAceso = -1; 

          // Espera soltar o botão ANTES do delay
          while(digitalRead(botoes[i]) == LOW);
          long tempoDeEspera = random(delayMin, delayMax);
          delay(tempoDeEspera);
          
          if (iniciarJogo) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Pontos: ");
            lcd.print(pontuacao);
            lcd.setCursor(0, 1);
            lcd.print("Aperte!!");

            escolherNovoLed();
          }
          break;

        } else {
            while(digitalRead(botoes[i]) == LOW);
            gameOver();
            break;
        }
      }
    }
  }

}