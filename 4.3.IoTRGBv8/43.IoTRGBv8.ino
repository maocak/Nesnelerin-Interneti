#define BLYNK_PRINT Serial

#define kirmizi 5 //RGB LED için kırmızı, yeşil ve mavi LEDler için 5,6 ve 9 nolu PWM pinler atanır. 
#define yesil 6
#define mavi 9

#include <ESP8266_Lib.h> //kütüphaneler koda eklenir. 
#include <BlynkSimpleShieldEsp8266.h>

// Blynk Uygulamasında Auth Token alınmalıdır. 
// Proje Ayarlarına gidin (somun simgesi).
char auth[] = "dOQCOJZH7ZKOEUpVKALlRO3ifFss807J";

// Kablosuz ağ adı ve şifresi yazılmalıdır. 
char ssid[] = "*****";
char pass[] = "*****";

// Arduino UNO üzerinden seri veri akışını sağlayacak olan kütüphane eklenir. 
#include <SoftwareSerial.h>
SoftwareSerial EspSerial(2, 3); // RX, TX bağlantıları pin3 ve pin2 dijital pinlere bağlanır. 

// ESP8266 modülünün veri akış hızı 9600 olarak belirlenmelidir. 
#define ESP8266_BAUD 9600

ESP8266 wifi(&EspSerial); // ESP8266 kablosuz ağ nesnesi oluşturulur. 

// Blynk app uygulamasındaki kaydırıcı araç(slider Widget)tan gelen değer Virtual Pin 1'e her  yazıldığında bu fonksiyon  çağrılır. 
BLYNK_WRITE(V1)
{
int pindeger = param.asInt(); // pin V1'den gelen değer pindeger isimli değişkene atanır. 
Serial.print("V1 kaydırıcı değeri: ");
Serial.println(pindeger);
analogWrite(kirmizi, pindeger);
}
// Blynk app uygulamasındaki kaydırıcı araç(slider Widget)tan gelen değer Virtual Pin 2'e her  yazıldığında bu fonksiyon  çağrılır. 
BLYNK_WRITE(V2)
{
int pindeger1 = param.asInt(); // pin V2'den gelen değer pindeger isimli değişkene atanır.
Serial.print("V2 kaydırıcı değeri: ");
Serial.println(pindeger1);
analogWrite(yesil, pindeger1);
}
// Blynk app uygulamasındaki kaydırıcı araç(slider Widget)tan gelen değer Virtual Pin 3'e her  yazıldığında bu fonksiyon  çağrılır.
BLYNK_WRITE(V3)
{
int pindeger2 = param.asInt(); // // pin V3'den gelen değer pindeger isimli değişkene atanır.
Serial.print("V3 kaydırıcı değeri: ");
Serial.println(pindeger2);
analogWrite(mavi, pindeger2);
}

void setup()
{
pinMode(kirmizi, OUTPUT);  // kirmizi LED pini çıkış olarak tanımlanır.
pinMode(yesil, OUTPUT);  // Yeşil LED pini çıkış olarak tanımlanır. 
pinMode(mavi, OUTPUT);  // mavi LED pini çıkış olarak tanımlanır. 
  
// Seri veri iletişimi 9600 baud rate de başlatılır. 
Serial.begin(9600);

// ESP8266 modülünün baud rate oranı 9600 de başlatılır. 
EspSerial.begin(ESP8266_BAUD);
delay(10);

// Blynk uygulaması başlatılır. Burda Blynk sunucu IP numarası ve port 8080'de belirtilmiştir. 
Blynk.begin(auth, wifi, ssid, pass, "139.59.206.133", 8080);

}

void loop()
{
Blynk.run(); //Blynk programı çalıştırılır. 
}
