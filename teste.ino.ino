#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h> 
#include <Stepper.h>
#include "pitches.h"
#include <EEPROM.h> //biblioteca para salvar dados na placa
#include <LiquidCrystal.h> //biblioteca para utilizar o display

#define SS_PIN 10
#define RST_PIN 9

// Leds indicadores acesso liberado ou negado
#define azul 5
#define verde 6
#define vermelho 7

//pino do sensor infravermelho
#define sensor 0

// Definicoes pino modulo RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); 

int estado_botao = 0;
const int botao = 2;
int pos;
int novoCartao = 0;
String cartoesCadastrados = "";
bool lerBotao = true;
//variavel do sensor
int obstaculo = 0;

Servo s;
char st[20];

MFRC522::MIFARE_Key key;

//define os pines que são utilizados com o display
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);

void setup() 
{
  pinMode(azul, OUTPUT);
  pinMode(verde, OUTPUT);
  pinMode(vermelho, OUTPUT);
  pinMode(botao, INPUT);
  //pinMode(botao_cancela, INPUT);
  ///controla contraste do lcd
  pinMode(8,OUTPUT);
  //pino do sensor de entrada
  pinMode(sensor, INPUT);
  analogWrite(8,90);  
  
  //digitalWrite(led_liberado, HIGH);
  //s.attach(3);
  // Inicia a serial
  Serial.begin(9600);
  // Inicia  SPI bus
  SPI.begin();
  // Inicia MFRC522
  mfrc522.PCD_Init(); 

  //define o numero de colunas e linhas do display
  lcd.begin(16,2);
  
  //acende Led Azul
  digitalWrite(azul, HIGH);
  //move motor para posição inicial
  //s.write(0);
  //delay(6000);
  //s.detach();
  //carregarCartoesCadastrados();
  mensagemInicial();  
}

void carregarCartoesCadastrados(){
  lcd.clear();
  lcd.setCursor(0,0);  
  lcd.print(" CARREGANDO...");
  Serial.println("Carregando cartoes cadastrados...");

for(int i = 0; i< 1024; i++){
  int aux = EEPROM.read(i);
  delay(2000);
  if (aux != 255){
    String cartao = "";
    char auxChar;
    int espacoBranco = 0;
    
    do{
      auxChar = char(EEPROM.read(i));
      cartao = cartao + auxChar;
      
      //Colocar " " (espaço em branco) a cada 2 letras
      espacoBranco ++;
      if (espacoBranco >= 2){
        cartao = cartao + " ";
        espacoBranco = 0;
      }

      
      i++;
    }while(auxChar != ';');
    i--;
    
    Serial.print("Cartao: ");
    Serial.println(cartao);
    
    cartoesCadastrados = cartoesCadastrados + cartao;
    
    Serial.print("Lista de Cartoes: ");
    Serial.println(cartoesCadastrados);

    
  }
 }
 delay(2000); 
}

void mensagemInicial(){
  digitalWrite(azul, HIGH);
  digitalWrite(verde, LOW);
  digitalWrite(vermelho, LOW);
  //limpa display e exibe mensagem
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("APROXIME A TAG");
  lcd.setCursor(0,1);
  lcd.print("  DO LEITOR...");
  Serial.println("Aproxime o cartão do leitor");
}

void loop() 
{
  estado_botao = digitalRead(botao);
  if (estado_botao == 1){
    novoCartao = 1;
    Serial.println("Aproxime o novo cartao");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MODO DE GRAVACAO");
    lcd.setCursor(0,1);
    lcd.print("AGUARDANDO A TAG...");
  }
  
  if(novoCartao > 0){
    novoCartao ++;  
    modoGravacao(); 

    if(novoCartao > 14){
      novoCartao = 0;
      digitalWrite(vermelho, HIGH);
      digitalWrite(azul, LOW);
      tone(4, NOTE_G3, 200);
      delay(200);
      tone(4, NOTE_G3, 200);
      delay(200);
      noTone(4);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("TEMPO ESGOTADO!");
      delay(2000);
      mensagemInicial();
    }
  }else{

      // Look for new cards
      if ( ! mfrc522.PICC_IsNewCardPresent()) 
      {
        return;
      }
      // Select one of the cards
      if ( ! mfrc522.PICC_ReadCardSerial()) 
      {
        return;
      }
    
      //Mostra UID na serial
      Serial.print("UID da tag :");
      String conteudo= "";
      byte letra;
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
         Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
         Serial.print(mfrc522.uid.uidByte[i], HEX);
         conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
         conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      Serial.println();
      Serial.print("Mensagem : ");
      conteudo.toUpperCase();
    
      if (cartaoExiste(conteudo.substring(1))){
        Serial.println("Acesso liberado");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ACESSO LIBERADO!");
        digitalWrite(azul, LOW);
        digitalWrite(verde, HIGH);
        tone(4, NOTE_G3, 200);
        delay(200);
        noTone(4);
        abrirPorta();
        delay(2000);
        mensagemInicial();
      }else{
        Serial.println("Cartao nao encontrado!");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(" ACESSO NEGADO!");
        digitalWrite(azul, LOW);
        digitalWrite(vermelho, HIGH);
        tone(4, NOTE_G3, 200);
        delay(400);
        tone(4, NOTE_G3, 200);
        delay(200);
        noTone(4);
        delay(2000);
        mensagemInicial();
      }
  }
}

void modoGravacao(){
      digitalWrite(azul, LOW);
      delay(1000);
      digitalWrite(azul, HIGH);
      
      // Aguarda a aproximacao do cartao
      if ( ! mfrc522.PICC_IsNewCardPresent()) 
      {
        return;
      }
      // Seleciona um dos cartoes
      if ( ! mfrc522.PICC_ReadCardSerial()) 
      {
        return;
      }
      // Mostra UID na serial
      Serial.print("UID da tag :");
      String conteudo= "";
      byte letra;
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
    
      String novoCartao = conteudo.substring(1);
      novoCartao.toUpperCase();
      novoCartao.trim();
      novoCartao = novoCartao + " ;";
    
      if(cartoesCadastrados.indexOf(novoCartao) < 0){
        cadastrarNovoCartao(novoCartao);
      }else{
        removerCartao(novoCartao);
      }
    
      delay(2000);
      mensagemInicial();
    
}

void cadastrarNovoCartao(String cartao){

  Serial.println("Adicionando cartao: ");
  Serial.println(cartao);
  lcd.clear();
  lcd.print("  LENDO CARTAO");
  delay(1000);

  cartoesCadastrados = cartoesCadastrados + cartao;
  
  //remove os espacos em branco do cartao e adiciona o ';'
  cartao.replace(" ","");

  //Adicionar o novo cartão na memória
  for(int i = 0; i< 1024; i++){
    
    int aux = EEPROM.read(i);
    
    if (aux == 255){
      for(int j = 0; j< cartao.length(); j++){
        EEPROM.write(i, byte(cartao.charAt(j)));
        i++;
      }
      break;
    }
  } 
  novoCartao = 0;
  digitalWrite(azul, LOW);
  digitalWrite(verde, HIGH);
  tone(4, NOTE_C4, 200);
  lcd.clear();
  lcd.print("CARTAO CADASTRADO");
  delay(2000);
  mensagemInicial();
}

void removerCartao(String cartao){
  Serial.println("Removendo cartao: ");
  Serial.println(cartao);  
  lcd.clear();
  lcd.print("REMOVENDO CARTAO...");
  
  cartoesCadastrados.replace(cartao, "");
  Serial.println("Cartoes atuais: ");
  Serial.println(cartoesCadastrados);
  
  
 //Remove o cartão na memória 
 String auxCartoesCadastrados = cartoesCadastrados;
 auxCartoesCadastrados.replace(" ","");
  for(int i = 0; i< 1024; i++){
    
   if(i < auxCartoesCadastrados.length()){
     EEPROM.write(i, byte(auxCartoesCadastrados.charAt(i)));
   }else{
     EEPROM.write(i, 255);
   }   
  } 
  novoCartao = 0;
  digitalWrite(azul, LOW);
  digitalWrite(verde, HIGH);
  tone(4, NOTE_C4, 200);
  lcd.clear();
  lcd.print("CARTAO REMOVIDO!");
  delay(2000);
  mensagemInicial();
}



void piscaLed(int valor){
  while (valor == 0){
    digitalWrite(azul, LOW);
    digitalWrite(azul, HIGH);
  }
  digitalWrite(azul, HIGH);
}

bool cartaoExiste(String cartao){
  if (cartoesCadastrados.indexOf(cartao) >= 0){
    return true;
    delay(500);
  }
  return false;
}

void abrirPorta(){  
 int j = 0;
 //abre porta
 s.attach(3);
 for (int i = 200; i <= 400; i++){
    s.write(i);
    Serial.println(i);
    alertaMovimento();
    if (i == 200){
      //escreve somente no início do movimento
      lcd.clear();      
      lcd.print(" ABRINDO PORTA!");      
    }
 }
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("    ABERTA");      
 
 s.detach();

 delay(3000);
 //fecha porta
 s.attach(3);
 lcd.clear();
 lcd.print("FECHANDO PORTA!");
 for (int i = 0; i > -170; i--){
    if (digitalRead(sensor) == 1){ // sem movimento
      s.write(i);
      Serial.println(i);
      alertaMovimento();
    }else{
      lcd.clear();
      while (digitalRead(sensor) == 0){                
        lcd.setCursor(0,0);
        lcd.print("MOVIMENTO PARADO");
        s.detach();
        Serial.println("Mantem porta fechada"); 
      }
      lcd.clear();
      lcd.print("FECHANDO PORTA!");
      s.attach(3);
    }    
 }
 s.detach();
}

void alertaMovimento(){
  tone(4, NOTE_G2, 10);
  delay(50);
}

