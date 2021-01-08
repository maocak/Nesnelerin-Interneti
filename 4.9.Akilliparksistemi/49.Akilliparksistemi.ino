#include <ESP8266WiFi.h>  // ESP8266 kütüphanesi eklenir.
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <FirebaseArduino.h> // Firebase kütüphanesi eklenir.

#define FIREBASE_HOST "parkuygulama.firebaseio.com"   // Firebase içinde yer alan proje ID ismi eklenir.
#define FIREBASE_AUTH "nwgpHOs8dEY7G4QnIrLPM9A4XUbZwWrMwUNBoPLC"     // Firebase'in oluşturduğu gizli anahtar girilir.
#define WIFI_SSID "MAO"    // Ev ya da işyerinde bulunan kablosuz ağ ismi(WiFi) girilir. 
#define WIFI_PASSWORD "furkanirem777"   //Wifi internet şifresi girilir.

String durum = "";    //durum adlı değişken string olarak atanır. 
String veritabanidurum = ""; //veritabanidurum adlı değişken string olarak atanır.

LiquidCrystal_I2C lcd(0x27, 16, 2);  //I2C adresi ve  16x2 LCD ekran tanımlanır. 
Servo servocikiskapi;    //Çıkış kapısında kullanılacak servo motor tanımlanır. 
Servo servogiriskapi;    //Giriş kapısında kullanılacak servo motor tanımlanır.
int bosyer;   //bosyer değişkeni integer olarak atanır. 
int toplamyersayi = 90; //toplamyersayi isimli değişkene 90 atanır. Bu toplam 90 adet parkta yer olduğunu bildirmektedir. 
int sayilan = 0; //sayilan isimli değişkene 0 atanır. 
int arabagiris = D0;   // arabagiris değişkenine D0 pin atanır.  
int arabacikis = D4;   //arabacikis değişkenine D4 pin atanır. 
int TRIG = D7;    //Mesafe sensörü trigger pin bağlantısı
int ECHO = D8;  // Mesafe sensörü echo pin bağlantısı
int led = D3;  // Park yeri LED'i(park yeri dolu ise yanar) 
int pos; //servo motorları kontrol etmek için kullanılacak değişkenler tanımlanır. 
int pos1;

long sure, uzaklik;  //Mesafe sensörü hesaplaması için kullanılacak değişkenler tanımlanır.    

void setup() {
Serial.begin (115200);  //seri veri iletişimi başlatılır.
delay(1000); // 1sn beklenir. 
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);   //Kablosuz internet ağına bağlantı sağlanır.
Serial.print("Bağlanıyor... ");
Serial.print(WIFI_SSID);  //Bağlanılan kablosuz ağ ismi seri port ekrana yazdırılır.
while (WiFi.status() != WL_CONNECTED) { 
Serial.print(".");   // Ağa bağlanmaya çalışırken ekranda . işareti yazar. 
delay(500); //Yarım saniye beklenir. 
}
Serial.println();
Serial.print("Bağlandı "); //Bağlantı sorunsuz şekilde sağlanırsa, bağlantı adı ve IP adresi seri port ekrana yazdırılır.
Serial.println(WIFI_SSID);
Serial.print("IP adresi : ");
Serial.println(WiFi.localIP());   //IP adresi seri port ekrana yazdırılır.
Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   // Firebase'e bağlantı sağlanır.

Wire.begin(D2, D1);  // I2C modülünün çalışması başlatılır. 
servocikiskapi.attach(D6); // Çıkış kapısında kullanılacak servomotor D6 pinine bağlanır. 
servogiriskapi.attach(D5);  // Giriş kapısında kullanılacak servomotor D5 pinine bağlanır.
pinMode(TRIG, OUTPUT); // Mesafe sensörün trig pini çıkış olarak atanır.   
pinMode(ECHO, INPUT);   // Mesafe sensörün echo pini giriş olarak atanır.
pinMode(led, OUTPUT);  // Park yeri LEDi çıkış olarak atanır. 
pinMode(arabacikis, INPUT);  // Çıkış kapısı IR sensör pini giriş olarak atanır. 
pinMode(arabagiris, INPUT);  // Giriş kapısı IR sensör pini giriş olarak atanır. 

lcd.begin();   //LCD ekran başlatılır. 
lcd.home();                            
lcd.setCursor(0, 0);   // LCD ekranın sol üst köşesinden itibaren...
lcd.print("Akilli Park");   //...Akıllı Park yazar. 
}

void loop() {

digitalWrite(TRIG, LOW);  // trigger çıkışı sinyal yollamadan önce 0 durumuna getirilir. 
delayMicroseconds(2); //2 milisaniye beklenir. 
digitalWrite(TRIG, HIGH);  //trigger çıkışından sinyal yollanır.
delayMicroseconds(10); //10 milisaniye beklenir.
digitalWrite(TRIG, LOW); // trigger çıkışı tekrar 0 lanarak sinyal kesilir. 
sure = pulseIn(ECHO, HIGH); // trigger çıkışı ile yollanan sinyal HIGH ile okunur ve sinyalin uzunluğu mikrosaniye cinsinden döndürülür.
uzaklik = (sure / 2) / 29.1;   //sinyalin gidip gelme süresinin yarısı alınır. Ve sonra, santimetre başına düşen mikro saniye süresine(29.1) bölünür.

Serial.print("Uzaklık: "); //Ölçülen uzaklık değeri seri port ekrana yazdırılır. 
Serial.print(uzaklik);Serial.println(" cm");

int giriskapisensor = digitalRead(arabagiris); // Giriş kapısındaki IR sensöründen okunan değer, giriskapisensor isimli değişkene atanır. 
if (giriskapisensor == 0) {   // Eğer okunan değer LOW(0) ise...
sayilan++;     //Sayma değeri 1 artırılır. 
Serial.print("Araba girişi = " ); Serial.println(sayilan ); //Giriş kapısından giren araba sayısı seriport ekrana yazdırılır. 
lcd.setCursor(0, 1);
lcd.print("Araba girisi"); //LCD ekrana "Araba girisi" yazdırılır. 
for (pos = 140; pos >= 45; pos -= 1) {  // Giriş kapısındaki servomotor 140 dereceden 45 dereceye dönerek kapı açılır. (1 derecelik adımlarla)
servogiriskapi.write(pos);                       
delay(5);
}
delay(3000); //Kapının açık kalma süresi 3 saniyedir. 

for (pos = 45; pos <= 140; pos += 1) {  // Kapı tekrar eski pozisyonuna 1 derecilik adımlarla geri gelir. 
servogiriskapi.write(pos);
delay(5);
}
bosyer = toplamyersayi - sayilan;  //90 olan toplam değerden, araba girişini sayan değer çıkarılır. 
veritabanidurum = String("Park yeri=") + String(bosyer) + String("/") + String(toplamyersayi);  //Hesaplanan değer, veritabanidurum isimli değişkene atanır.
Firebase.pushString("/Park durum/", veritabanidurum );   // Değer, firebase veri tabanına yollanır. 
lcd.clear(); //LCD ekran temizlenir. 
}

int cikiskapisensor = digitalRead(arabacikis);  // Çıkış kapısındaki IR sensöründen okunan değer, cikiskapisensor isimli değişkene atanır.
if (cikiskapisensor == 0) {    // Eğer okunan değer LOW(0) ise...
sayilan--;    //Sayma değeri 1 azaltılır.
Serial.print("Araba çıkışı = " ); Serial.println(sayilan); //Çıkış kapısından çıkan araba sayısı seriport ekrana yazdırılır.
lcd.setCursor(0, 1); 
lcd.print("Araba cikisi"); //LCD ekrana "Araba girisi" yazdırılır.
for (pos1 = 140; pos1 >= 45; pos1 -= 1) {  // Çıkış kapısındaki servomotor 140 dereceden 45 dereceye dönerek kapı açılır. (1 derecelik adımlarla)
servocikiskapi.write(pos1);
delay(5);
}
delay(3000); //Çıkış kapısı 3 saniye açık bekler. 

for (pos1 = 45; pos1 <= 140; pos1 += 1) {  // Kapı tekrar eski pozisyonuna 1 derecilik adımlarla geri gelir.
servocikiskapi.write(pos1);
delay(5);
}
bosyer = toplamyersayi - sayilan;  //90 olan toplam değerden, araba çıkışını sayan değer çıkarılır. 
veritabanidurum = String("Park yeri=") + String(bosyer) + String("/") + String(toplamyersayi); //Hesaplanan değer, veritabanidurum isimli değişkene atanır.
Firebase.pushString("/Park durum/", veritabanidurum );    // Değer, firebase veri tabanına yollanır.
lcd.clear(); //LCD ekran temizlenir. 
}

if (uzaklik < 6) {    //Park yerinde ölçülen uzaklık 6 cm'den az ise, LED yanar(park yerinde araba var) 
Serial.println("Park yeri dolu ");
digitalWrite(led, HIGH);
}

if (uzaklik > 6) {   //Park yerinde ölçülen uzaklık 6 cm'den fazla ise, LED yanar(park yerinde araba yok) 
Serial.println("Park yeri boş ");
digitalWrite(led, LOW);
}
 
durum = String("Park yeri= ") + String(bosyer) + String("/") + String(toplamyersayi);   // Boş olan yer, toplam yer sayısı string ile birleştirilerek durum isimli değişkene atanır.
 
lcd.setCursor(0, 0);
lcd.print(durum);   //Durum isimli değişken, LCD ekrana yazdırılır. 
 
}
