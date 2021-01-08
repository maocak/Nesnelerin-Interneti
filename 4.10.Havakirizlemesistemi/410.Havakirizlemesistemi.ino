#include "MQ135.h"  //Gaz sensör kütüphanesini koda eklenir. 
#include <SoftwareSerial.h> // Arduino UNO üzerinden seri veri akışını sağlayacak olan kütüphane eklenir.
#define DEBUG true
SoftwareSerial esp8266(9,10); // // RX, TX bağlantıları pin10 ve pin9 dijital pinlere bağlanır.
const int sensorPin= A0; //Gaz sensör pini AO analog pin olarak atanır. 
int havakalitesi; //İlgili değişkenler tanımlanır. 
int kirmiziled=6;
int yesilled=7;
int sariled=13;

#include <LiquidCrystal.h> //LCD ekran kütüphanesi ekrana eklenir. 
LiquidCrystal lcd(12,11, 5, 4, 3, 2); //LCD pin uçları tanımlanır.

void setup() {
Serial.begin(9600); //Seri port ekranı çalışma hızı belirlenir.
esp8266.begin(9600); // ESP8266 modülünün veri akış hızı 9600 olarak belirlenmelidir. 
pinMode(sensorPin, INPUT); //Gaz sensörü pini çıkış olarak tanımlanır. 
pinMode(8, OUTPUT); //Buzzer pini giriş olaran tanımlanır. 
pinMode(kirmiziled, OUTPUT); //Kırmızı, yeşil ve sarı LEDlerin pini giriş olarak tanımlanır. 
pinMode(yesilled, OUTPUT);
pinMode(sariled, OUTPUT);

lcd.begin(16,2); //LCD ekran başlatılır. 
lcd.setCursor (0,0); //İlk satıra yazdırılır. 
lcd.print ("Hava kalitesi");
lcd.setCursor (0,1); //2.satıra yazdırılır. 
lcd.print ("Izleme sistemi");
delay(1000); //1 saniye beklenir. 

sendData("AT+RST\r\n",2000,DEBUG); // ESP8266 modüle reset atılır. 
sendData("AT+CWMODE=2\r\n",1000,DEBUG); // ESP8266 modülü acces point olarak ayarlanır. configure as access point
sendData("AT+CIFSR\r\n",1000,DEBUG); // Web sayfasına bağlanılacak olan IP adresi belirlenir. 
sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // ESP8266 modülü, birden çok bağlantı için yapılandırılır. 
sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // Sunucuya 80 nolu port kullanılarak bağlanır. 
lcd.clear(); //LCD ekran temizlenir. 
}
void loop() {

MQ135 gasSensor = MQ135(A0); //Sensörden değer okuması yapılır. 
float havakalitesi = gasSensor.getPPM(); //Okunan değer, havakalitesi isimli değişkene atanır. 
if(esp8266.available()) // ESP8266 modülünün mesaj yollayıp yollamadığı kontrol edilir.  
{
if(esp8266.find("+IPD,")) //ESP8266 modülü, yollanacak veri uzunluğuna ulaşılana kadar arabelleğe alınan verileri okur. 
{
delay(1000); //1 saniye beklenir. 
                 
int connectionId = esp8266.read()-48; //Read () işlevi ASCII ondalık değerini döndürdüğü ve 0 olan ilk ondalık sayı 48'de başladığı için 48 çıkarıyoruz. 
String webpage = "<h1>NIT Hava Kalitesi Izleme Sistemi</h1>"; //Sunucuya yollanacak veriler birbirine eklenerek yollanır. 
webpage+="<meta http-equiv='refresh' content='10; URL=""'>"; //Sunucu sayfasının her 10 saniyede yenilenmesini sağlayarak web sayfasında yeni veri gösterilir. 
webpage += "<p><h2>";   
webpage+= " Hava kalitesi: "; //Okunan hava kalitesi değeri sunucu web sayfasına yazdırılır. 
webpage+= havakalitesi;
webpage+=" PPM";
webpage += "<p>";
if (havakalitesi<=1000) //Eğer sensörden okunan değer, 1000'in altında ise...
{
webpage+= "Temiz hava"; //...web sayfasına yollanacak yazı "Temiz hava" olur.
}
else if(havakalitesi<=2000 && havakalitesi>=1000) //Eğer sensörden okunan değer, 1000 ile 2000 arasında ise...
{
webpage+= "Kirli hava"; //...web sayfasına "Kirli hava" yollanır.
}
else if (havakalitesi>=2000 ) //Eğer sensörden okunan değer, 2000'in üstünde ise...
{
webpage+= "Tehlike!"; //...web sayfasına "Tehlike!" yazısı yollanır.  
}
webpage += "</h2></p></body>"; 
String cipSend = "AT+CIPSEND="; //Sunucu web sayfasına yollanacak verilerin uzunluğu belirlenir. 
cipSend += connectionId;
cipSend += ",";
cipSend +=webpage.length();
cipSend +="\r\n";
     
sendData(cipSend,1000,DEBUG); //Fonksiyona yollanacak verilerin uzunluğu yollanır. 
sendData(webpage,1000,DEBUG); //Fonkisyona okunan veriler yollanır. 
     
String closeCommand = "AT+CIPCLOSE=";  //Bağlantı kapatılır. 
closeCommand+=connectionId; // Bağlantı kimliği eklenir.
closeCommand+="\r\n"; //Alt satıra inerek yeni satırdan başlar. 
     
sendData(closeCommand,3000,DEBUG); //Fonksiyona bağlantıyı kapatma At komutu yollanır. 
}
}
lcd.setCursor (0, 0);
lcd.print ("Ha.klt:");
lcd.print (havakalitesi);
lcd.print ("PPM ");
lcd.setCursor (0,1);
if (havakalitesi<1000) //Eğer okunan sensör değeri 1000'in altında ise...
{
lcd.print("Temiz hava"); //LCD ekrana, Temiz hava yazar. 
digitalWrite(8, LOW);  //Buzzer ses çıkarmaz. 
digitalWrite(yesilled, HIGH); //Yeşil LED yanık, diğer LEDler sönüktür. 
digitalWrite(kirmiziled, LOW);
digitalWrite(sariled, LOW);
}
else if( havakalitesi>=1000 && havakalitesi<=2000 ) //Eğer okunan sensör değeri 1000 ile 2000'in altında ise...
{
lcd.print("Kirli hava"); //LCD ekrana, Kirli hava yazar.
digitalWrite(8, HIGH );  //Buzzer öter.
digitalWrite(kirmiziled, LOW); //Sarı LED yanar, diğer LEDler sönüktür.
digitalWrite(yesilled, LOW);
digitalWrite(sariled, HIGH);
}
else if (havakalitesi>2000 ) //Eğer okunan sensör değeri 2000'in üstünde ise...
{
lcd.print("Tehlike!!!"); //LCD ekrana, Tehlike!!! yazar.
digitalWrite(8, HIGH);   //Buzzer öter.
digitalWrite(kirmiziled, HIGH); //Kırmızı LED yanar, diğer LEDler sönüktür.
digitalWrite(yesilled, LOW);
digitalWrite(sariled, LOW);
}
delay(1000);
}
//AT komutlarını çalıştıran sendData fonksiyonu
String sendData(String command, const int timeout, boolean debug) //ESP8266 modülünden veri alınır ve seriport ekrana yazdırılır. 
{
String response = ""; 
esp8266.print(command); //Okunan karakter ESP8266 modülüne yollanır.  
long int time = millis();
while( (time+timeout) > millis())
{
while(esp8266.available())
{
// ESP8266 modülünde olan veri seri port ekrana yollanır.  
char c = esp8266.read(); // Bir sonraki karakter okunur. 
response+=c;
}  
}
if(debug)
{
Serial.print(response); //AT komutlarından alınan yanıtlar seri porta aktarılır. 
}
return response; 
}
