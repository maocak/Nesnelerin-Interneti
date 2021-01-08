#include <SoftwareSerial.h>         
#define DEBUG true
SoftwareSerial esp8266(6,7);   

//trigger pin, echo pin ve kırmızı ve yeşil LED pinleri tanımlanır. 
#define trigPin 13
#define echoPin 12
#define kirmiziLED 11
#define yesilLED 10

void setup() {
Serial.begin (9600);//seri veri iletişimi başlatılır. 
esp8266.begin(9600); 

pinMode(trigPin, OUTPUT); //trigger pini çıkış olarak tanımlanır.(Trigger pinden çıkan sinyal echo pinine girer.)
pinMode(echoPin, INPUT);  //echo pini giriş olarak tanımlanır. 
pinMode(yesilLED, OUTPUT); //yeşil ve kırmızı LED ler çıkış olarak tanımlanır. 
pinMode(kirmiziLED, OUTPUT);

sendData("AT+RST\r\n",2000,DEBUG);            // ESP8266 modüle reset atılır.
sendData("AT+CWMODE=2\r\n",1000,DEBUG);       // ESP8266 modülü acces point olarak ayarlanır.
sendData("AT+CIFSR\r\n",1000,DEBUG);          // Web sayfasına bağlanılacak olan IP adresi belirlenir.
sendData("AT+CIPMUX=1\r\n",1000,DEBUG);       // ESP8266 modülü, birden çok bağlantı için yapılandırılır.
sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // Sunucuya 80 nolu port kullanılarak bağlanır.
}

void loop() { 
long sure, uzaklik; //süre ve uzaklık isimli 2 tane değişken tanımlanır. 
digitalWrite(trigPin, LOW);  // trigger çıkışı sinyal yollamadan önce 0 durumuna getirilir. 
delayMicroseconds(2); // 2ms beklenir. 
digitalWrite(trigPin, HIGH); //trigger çıkışından sinyal yollanır. 
delayMicroseconds(10); // 10 sn yollanır. Bunun nedeni yollanan sinyallerin karışmaması için
digitalWrite(trigPin, LOW); //trigger çıkışı tekrar 0 lanarak sinyal kesilir. 
sure = pulseIn(echoPin, HIGH); //trigger çıkışı ile yollanan sinyal HIGH ile okunur ve sinyalin uzunluğu mikrosaniye cinsinden döndürülür
uzaklik = (sure/2) / 29.1; //sinyalin gidip gelme süresinin yarısı alınır. Ve sonra, santimetre başına düşen mikro saniye süresine(29.1) bölünür. 
if (uzaklik < 4) {  // Uzaklık 4cm altına düşürse kırmızı LED yanar, yeşil LED söner. 
digitalWrite(kirmiziLED,HIGH); // 
digitalWrite(yesilLED,LOW);
}
else {
digitalWrite(kirmiziLED,LOW); //Uzaklık 4cm'den büyük 200 cm den küçükse kırmızı LED söner, yeşil LED yanar. 
digitalWrite(yesilLED,HIGH);
}
if (uzaklik >= 200 || uzaklik <= 0){ //Uzaklık 200cm in üstünden ise "Ölçülen uzaklık sınırların dışında" diye uyarı verir. 
Serial.println("Ölçülen uzaklık sınırların dışında");
}
else {
Serial.print(uzaklik);
Serial.println(" cm");
}
delay(500); 

if(esp8266.available())   // ESP8266 modülünün mesaj yollayıp yollamadığı kontrol edilir. 
{    
if(esp8266.find("+IPD,")) //ESP8266 modülü, yollanacak veri uzunluğuna ulaşılana kadar arabelleğe alınan verileri okur.
{
delay(1000); //1 saniye beklenir.
int connectionId = esp8266.read()-48; //read() işlevi ASCII ondalık değerini döndürdüğü ve 0 olan ilk ondalık sayı 48'de başladığı için 48 çıkarıyoruz.
                                            
String webpage = "<h1>NIT- COP KUTUSU IZLEME SISTEMI</h1>"; //Sunucuya yollanacak veriler birbirine eklenerek yollanır.
webpage+="<meta http-equiv='refresh' content='10; URL=""'>"; //Sunucu sayfasının her 10 saniyede yenilenmesini sağlayarak web sayfasında yeni veri gösterilir.
webpage += "<p><h2>";   
if (uzaklik<4) //Eğer sensörden okunan değer, 4 cm'in altında ise...
{
webpage+= "Cop kutusu: DOLU"; //Web sayfasına Çöp kutusu dolu yazısı yollanır. 
}
else   //Eğer sensörden okunan değer, 4 cm'in altında ise...
{
webpage+= "Cop kutusu: BOS"; //Web sayfasına Çöp kutusu boş yazısı yollanır.
}
webpage += "</h2></p></body>";  
String cipSend = "AT+CIPSEND="; //Sunucu web sayfasına yollanacak verilerin uzunluğu belirlenir.
cipSend += connectionId;
cipSend += ",";
cipSend +=webpage.length();
cipSend +="\r\n";

sendData(cipSend,1000,DEBUG); //sendData fonksiyonuna yollanacak verilerin uzunluğu yollanır.
sendData(webpage,1000,DEBUG);  //Fonksiyona okunan veriler yollanır.  
String closeCommand = "AT+CIPCLOSE=";  //Bağlantı kapatılır.
closeCommand+=connectionId; // Bağlantı kimliği eklenir.
closeCommand+="\r\n"; //Alt satıra inerek yeni satırdan başlar.
sendData(closeCommand,3000,DEBUG); //Fonksiyona bağlantıyı kapatma At komutu yollanır.
}
}
}
//AT komutlarını çalıştıran sendData fonksiyonu
String sendData(String command, const int timeout, boolean debug)//ESP8266 modülünden veri alınır ve seriport ekrana yazdırılır.
{
String response = "";   
esp8266.print(command); //Okunan karakter ESP8266 modülüne yollanır.
long int time = millis();
while( (time+timeout) > millis())
{
while(esp8266.available())
{
char c = esp8266.read(); // ESP8266 modülünde olan veri seri port ekrana yollanır.
response+=c; // Bir sonraki karakter okunur.
}  
}
if(debug)
{
Serial.print(response); //AT komutlarından alınan yanıtlar seri porta aktarılır.
}
return response;
}
