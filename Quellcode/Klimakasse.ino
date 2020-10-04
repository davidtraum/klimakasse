#include <SPI.h>
#include <MFRC522.h>

/**
Definiton der Pins für den Piezo-Buzzer und den NFC-Scanner
*/
#define BUZZER D2
#define SS_PIN D4
#define RST_PIN D3
MFRC522 mfrc522(SS_PIN, RST_PIN);


/**
Definition der CO2 Referenzwerte.
*/
#define MAX_CO2 15000.0 //Maximaler CO2 Wert der Skala (15t)
#define AVG_CO2 11100.0 //Durchschnittlicher deutscher CO2 Wert
#define SUS_CO2 4100.0 //Maximaler nachhaltiger CO2 Wert

bool mainpage = false; //Speichert ob sich das Programm im Scanmodus befindet.

class EString {
  public:
    EString(char data[]) {
      content = new char[sizeof(data)];
      length = sizeof(data);
      Serial.println("Creating E-String with length: ");
      Serial.println((int)length);
      Serial.println(data);
      for(char i = 0; i<length; i++) {
        content[i] = data[i];
      }
    }
    bool matches(const char data[]) {
      for(char i = 0; i<sizeof(data); i++){
        if(i>=length || content[i] != data[i]) {
          return false;
        }
      }
      return true;
    }
    void print() {
      Serial.print("Printing ");
      Serial.println((int)length);
      for(char i = 0; i<length; i++) {
        Serial.print(content[i]);
      }
    }
    char * content;
    char length;
};

/**
Diese Klasse umfasst die Informationen für ein Modell das gescannt werden kann.
Es wird ein Schlüssel, der anzuzeigende Text, die ID des NFC Chips und der CO2 Wert in Gramm pro Jahr übergeben.
*/
class ScanObject {
  public:
    ScanObject(String pName, String pDisplay, String pId, int pPrice):
      name{pName},display{pDisplay},id{pId},price{pPrice}
    {
    }
    
    bool matches(String str){
      return str==id;
    }
    
    String name;
    String display;
    String id;
    int price;
};

int total = 0;

/**
Definition der Modelle die gescannt werden.
Es wird jeweils ein Key, der angezeigte Text, die ID des NFC Chips und der "Preis" in Kilogramm CO2 gespeichert.
*/
ScanObject * objects[] = {
  new ScanObject("sockelbetrag", "Öffentlicher Sockelbetrag", "00 00 00 00 00 00 00", 730),
  new ScanObject("haus_klein", "Kleine Wohnfläche (< 100qm)", "04 7B 6B 92 D3 64 80", 170),
  new ScanObject("haus_mittel", "Mittlere Wohnfläche (< 200qm)", "04 75 97 92 D3 64 81", 930),
  new ScanObject("haus_gross", "Große Wohnfläche (> 200qm)", "04 61 70 92 D3 64 80", 2000),
  new ScanObject("holz", "Heizen mit Holz", "04 73 8D 02 2B 5E 80", 160),
  new ScanObject("erdgas", "Heizen mit Erdgas", "04 84 CA 02 2B 5E 80", 1300),
  new ScanObject("erdoel", "Heizen mit Erdöl", "04 8A 41 02 2B 5E 80", 1600),
  new ScanObject("milch", "Milchprodukte", "04 52 69 92 D3 64 81", 320),
  new ScanObject("fleisch", "Fleischprodukte", "04 6D B0 92 D3 64 81", 1000),
  new ScanObject("exotisch", "Exotische Lebensmittel", "04 28 B3 92 D3 64 80", 350),
  new ScanObject("abfall", "Lebensmittel wegwerfen", "04 81 60 92 D3 64 81", 500),
  new ScanObject("auto_klein", "Autofahren (< 10000km)", "04 2E 56 92 D3 64 80",1100),
  new ScanObject("auto_mittel", "Autofahren (< 30000km)", "04 7E 00 02 2B 5E 84",3300),
  new ScanObject("auto_gross", "Autofahren (> 30000km)", "04 89 50 02 2B 5E 80",8000),
  new ScanObject("opnv", "Öffentliche Verkehrsmittel", "04 02 C7 92 D3 64 81", 1100),
  new ScanObject("flugzeug", "Flugreise", "04 8E 54 92 D3 64 80", 3600),
  new ScanObject("kreuzfahrt", "Kreuzfahrt", "04 42 87 02 2B 5E 80", 800),
  new ScanObject("erneuerbar", "Erneuerbare Energie", "04 83 57 02 2B 5E 80", 250),
  new ScanObject("fossil", "Fossile Energie", "04 7A CB 02 2B 5E 80", 1900),
  new ScanObject("kleidung_wenig",  "Bekleidung (<200€)", "04 71 B8 02 2B 5E 80", 300),
  new ScanObject("kleidung_viel", "Bekleidung (>200€)", "04 9E C9 02 2B 5E 80", 1000),
  new ScanObject("elektro_wenig", "Unterhaltung (<200€)", "04 5E A2 02 2B 5E 80", 300),
  new ScanObject("elektro_viel", "Unterhaltung (>200€)", "04 6A 76 D2 2F 66 81", 1000),
  new ScanObject("strommix", "Strommix", "-", 1000),
};

ScanObject * scanned[16] = {nullptr};
 
//Die
void setup() 
{
  pinMode(BUZZER, OUTPUT);
  Serial.begin(115200);   // Serielle Verbindung für die Kommunikation mit dem Display wird initialisiert
  SPI.begin();      // Der SPI-Bus wird initialisiert
  mfrc522.PCD_Init();   // Die API für den NFC-Reader wird initialisiert
  mfrc522.PCD_SetAntennaGain(0x07<<4);
  nexSend();
  delay(1000);
  Serial.print("page page_start");
  nexSend();
  for(int i = 0; i<3; i++) {
  digitalWrite(BUZZER, HIGH);
  delay(50);
  digitalWrite(BUZZER, LOW);
  delay(200);
  }
}

/**
Markiert das Ende eines Befehls für das Nextion-Displays.
*/
void nexSend() {
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);
}

/**
Sendet einen Befehl an das Nextion-Display.
Dokumentation der Befehle
*/
void nexCommand(String cmd) {
  Serial.print(cmd);
  nexSend();
}

void copyString(char * orig, char * target) {
  target = new char[sizeof(orig)];
  for(char i = 0; i<sizeof(orig); i++){
    
  }
}

ScanObject * findObject(String pId) {
  for(ScanObject * obj : objects) {
    if(obj->matches(pId.c_str())){
      return obj;
    }
  }
  return nullptr;
}

ScanObject * findObjectName(String pName) {
  for(ScanObject * obj : objects) {
    if(obj->name == pName) {
      return obj;
    }
  }
  return nullptr;
}

bool isScanned(ScanObject * obj) {
  for(ScanObject * so : scanned) {
    if(so != nullptr && so == obj) {
      return true;
    }
  }
  return false;
}

bool isScanned(String pName) {
  return isScanned(findObjectName(pName));
}

void addToScanned(ScanObject * obj) {
  int index = 0;
  for(ScanObject * so : scanned) {
    if(so == nullptr) {
      scanned[index] = obj;
      return;
    }
    index++;
  }
}


char indexOfScannedName(String pName){
  char index = 0;
  for(ScanObject * so : scanned) {
    if(so == nullptr) {
      return -1;
    }else{
      if(so->name == pName) {
        return index;
      }
      index++;
    }
  }
  return -1;
}

/**
Ermittelt die Anzahl der gescannten Projekte.
*/
int scannedLength() {
  int count{0};
  for(ScanObject * so : scanned) {
    if(so == nullptr) {
      break;
    }else{
      count++;
    }
  }
  return count;
}

/**
Ein gescanntes Objekt wird aus der Liste entfernt.
*/
void removeFromScanned(ScanObject * obj) {
  int index = 0;
  for(ScanObject * so : scanned) {
    if(so == obj) {
      break;
    }else{
      index++;
    }
  }
  for(int i = index+1; i<scannedLength(); i++) {
    scanned[i-1] = scanned[i]; 
  }
}

/**
Sendet die aktuelle Liste der gescannten Objekte über die Serielle Schnittstelle
 an das Display
*/
void updateDisplay() {
  int index = 0;
  total = 0;
  for(ScanObject * obj : scanned){
    if(obj!=nullptr){
      Serial.print("slot_");
      Serial.print(index);
      Serial.print(".txt=\"");
      Serial.print(obj->display);
      Serial.print("\"");
      nexSend();

      Serial.print("price_");
      Serial.print(index);
      Serial.print(".txt=\"");
      Serial.print(obj->price);
      Serial.print(" kg");
      Serial.print("\"");
      nexSend();

      total += obj->price;
      
      index++;
    }else{
      break;
    }
  }
  Serial.print("t_sum.txt=\"");
  Serial.print((total/1000.0));
  Serial.print("t CO2\"");
  nexSend();

  Serial.print("v_max.val=");
  Serial.print(int((total/MAX_CO2)*100));
  nexSend();
}


/**
Diese Methode wird ausgeführt wenn ein Objekt gescannt und erkannt wurde.
Es wird der Spezialfall des Strommixes (Fossile & Erneuerbare Energie gemischt) behandelt und ein Piepton abgespielt.
*/
void scanObject(ScanObject * obj) {

  if(mainpage) {
      bool didScan = false;
      if(obj->name == "fossil" && isScanned("erneuerbar")) {
        scanned[indexOfScannedName("erneuerbar")] = findObjectName("strommix");
        didScan = true;
      }else if(obj->name == "erneuerbar" && isScanned("fossil")) {
        scanned[indexOfScannedName("fossil")] = findObjectName("strommix");
        didScan = true;
    }else{
      if(!isScanned(obj)){
        if((obj->name != "erneuerbar" && obj->name != "fossil") || !isScanned("strommix")){ 
          addToScanned(obj);
          didScan = true;
        }
      }
    }
    if(didScan) {
      updateDisplay();
      digitalWrite(BUZZER, HIGH);
      delay(100);
      digitalWrite(BUZZER, LOW);
    }
  }
}

/**
Errechnet das Fazit (z.B. wie viele Planeten man bräuchte wenn jeder diesen CO2  Ausstoß hätte) und sendet die Daten an das Display.
*/
void calculateFazit() {
  int earths = total/SUS_CO2;
  bool half = total-(earths*SUS_CO2) >= (SUS_CO2/2);
  for(int i = 0; i<earths; i++){
    Serial.print("earth_");
    Serial.print(i);
    Serial.print(".pic=9");
    nexSend();
  }
 if(half || earths==0) {
  Serial.print("earth_");
  Serial.print(earths);
  Serial.print(".pic=10");
  nexSend();
 }
 Serial.print("t_value.txt=\"");
 Serial.print((total/1000.0));
 Serial.print(" Tonnen CO2\"");
 nexSend();
 Serial.print("t_conslusion.txt=\"");
 if(total<SUS_CO2) {
  Serial.print("Dein Wert ist super!\r\nDu hast einen nachhaltigen CO2-Fußabdruck.");
 }else if(total<AVG_CO2) {
  Serial.print("Du bist besser als der deutsche Durchschnitt,\r\nsehr gut!\r\nDennoch ist noch etwas Luft nach oben.");
 }else{
  Serial.print("Dein Wert liegt leider über dem deutschen \r\nDurchschnitt. Hast du interesse \r\ndeinen Fußabdruck zu verbessern? \r\nSprich uns gerne an!");
 }

 Serial.print("\"");
 nexSend();
}

void resetData() {
  total = 0;
  for(int i = 0; i<16; i++) {
    scanned[i] = nullptr;
  }
}

/**
Aktiviert den Piezo-Pieper für das Kassengeräusch.
*/
void buzz() {
  digitalWrite(BUZZER, HIGH);
  delay(50);
  digitalWrite(BUZZER, LOW);
}

/**
Liest einen Befehl vom Display über die serielle Schnittstelle und interpretiert diesen.
*/
void parseCommand() {
  switch(Serial.read()) {
    case 'S':
      Serial.print("page page_main");
      nexSend();
      mainpage = true;
      scanObject(objects[0]);
      break;
    case 'F':
      mainpage = false;
      Serial.print("page page_fazit");
      nexSend();
      buzz();
      calculateFazit();
      break;
    case 'R':
      mainpage = false;
      resetData();
      Serial.print("page page_start");
      nexSend();
      buzz();
      break;
  }
}


/**
In der loop Routine wird durchgehend überprüft, ob sich ein NFC-Chip in der Reichweite des Scanners befindet.
*/
void loop() 
{
  //Wenn gerade die Hauptseite angezeigt wird, wird überprüft ob sich ein NFC-Chip in der Nähe befindet.
  if(mainpage && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
    String content= "";
    byte letter;
    //Die UID des Chips wird Byteweise gelesen
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
       content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
       content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    //Es wird überprüft, ob ein Objekt mit der ID des Chips gespeichert ist.
    ScanObject * obj = findObject(content.substring(1));
    if(obj != nullptr) {
      scanObject(obj);
    }else{
      
    }
  }

  //Wenn serielle Daten vorhanden sind wird ein Befehl vom Display gelesen.
  if(Serial.available() > 0) {
    parseCommand();
  }
}
