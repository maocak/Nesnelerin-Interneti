#include <DHT.h> // Dht11 kütüphanesi eklenir.
#include <ESP8266WiFi.h> // ESP8266 kütüphanesi eklenir.
String apiKey = "U5UJJDKIJAY1JQIO";     // Thinspeak API numarası yazılır. 
const char* server = "184.106.153.149"; //Thinspeak IP adresi
const char *ssid =  "MAO";     // Kullanılan kablosuz ağ ismi
const char *pass =  "furkanirem777"; // Kablosuz ağ şifresi
#define DHTPIN D3  // DHT11 sensörünün bağlandığı dijital pin numarası 
DHT dht(DHTPIN, DHT11); //pin çıkışı ve türü tanımlanan DHT sensörü için nesne oluşturulur.
WiFiClient client; //Client.connect() öğesinde tanımlandığı şekilde belirtilen bir İnternet IP adresine ve bağlantı noktasına bağlanabilen bir istemci oluşturur.

const int topraknemsensor = A0; // Toprak nem sensörünün bağlandığı analog pin numarası
const int supompapin = D0; //Su motor pininin bağladığı dijital pin numarası
unsigned long zamanaralik = 10000; //Beklememiz gereken 1. zaman aralığı (10sn)
unsigned long oncekizaman = 0;
unsigned long zamanaralik1 = 1000; //Beklememiz gereken 2. zaman aralığı (1sn)
unsigned long oncekizaman1 = 0;
float topraknemoran; //topraknemoran değerinin tutulacağı değişken 
float sicaklik;  // sıcaklık değerinin tutulacağı değişken 
float nem;   //nem değerinin tutulacağı değişken

void setup()
{
Serial.begin(115200); //Seriport ekranı 115200 veri akış hızında başlatılır.
delay(10); //10msn beklenir.
pinMode(supompapin, OUTPUT); //Su pompası pini çıkış olarak atanır. 
digitalWrite(supompapin, LOW); // Su pompası motoru kapalı olarak başlar. 
dht.begin(); //DHT11 sensörü başlatılır.
Serial.println("Bağlanılıyor..."); //Ekrana yazdırılır. 
Serial.println(ssid); //Bağlanılan kablosuz ağın ismi seriport ekranına yazdırılır. 
WiFi.begin(ssid, pass); //WiFi kitaplığının ağ ayarlarını başlatır ve kablosuz ağa bağlantı sağlar.
while (WiFi.status() != WL_CONNECTED)
{
delay(500);
Serial.print(".");  // Kablosuz ağa bağlanana kadar seriport ekradan . yazdırılır. 
}
Serial.println("");
Serial.println("WiFi bağlanıldı."); //Bağlantı başarılı olursa yazar. 
}

void loop()
{
unsigned long simdikizaman = millis(); // Şimdiki zaman kaydedilir. 

nem = dht.readHumidity(); // DHT11 sensöründen nem değeri okunur. 
sicaklik = dht.readTemperature(); // DHT11 sensöründen sıcaklık değeri okunur. 

if (isnan(nem) || isnan(sicaklik)) //Eğer DHT11 sensöründen değerler okunamazsa...
{
Serial.println("DHT11 sensöründen okuma hatası!"); //Seri porta hata yazar. 
return;
}

topraknemoran = ( 100.00 - ( (analogRead(topraknemsensor) / 1023.00) * 100.00 ) ); //Okunan 0-1023 arası analog değer formül yardımıyla yüzdelik değere dönüştürülür.

if ((unsigned long)(simdikizaman - oncekizaman1) >= zamanaralik1) { //Eğer şimdiki zaman ile önceki zaman arasındaki fark 1sn'den büyükse...
Serial.print("Toprak nem oranı= "); //Ekrana toprak nem oranı yazdırılır ve...
Serial.print(topraknemoran);
Serial.println("%");
oncekizaman1 = millis(); //...önceki zaman değeri millis komutuna tekrar atanır. 
}

if (topraknemoran < 50) { //Eğer topraktaki nem oranı %50'in altında ise...
digitalWrite(supompapin, HIGH);  // Su pompasının motoru çalışır 
}
if (topraknemoran > 50 && topraknemoran < 55) { //Eğer topraktaki nem oranı %50 ile %55 arası ise motor su akıtmaya devam eder. 
digitalWrite(supompapin, HIGH);        //turn on motor pump
}
if (topraknemoran > 56) { //Eğer toprak nem oranı yüksekse(56'tan)...
digitalWrite(supompapin, LOW); // Su motoru durur. 
}

if ((unsigned long)(simdikizaman - oncekizaman) >= zamanaralik) { //Eğer şimdiki zaman ile önceki zaman arasındaki fark 10sn'den büyükse...
sendThingspeak();  //Okunan değerler thinspeak sunucusuna yollanır. 
oncekizaman = millis(); //önceki zaman değeri millis komutuna tekrar atanır.
client.stop(); //Sunucu ile bağlantı kesilir. Değerlerin okunması işlemi tekrar başlar. 
}
}

void sendThingspeak() { //Okunan değerlerin sunucuya yollanması işlemi bu fonksiyon yardımıyla yapılır. 
if (client.connect(server, 80)) //Belirtilen bir IP adresine ve bağlantı noktasına bağlanır.
{
String postStr = apiKey;  // postStr dizesine API anahtarı eklenir.
postStr += "&field1=";
postStr += String(topraknemoran); // Toprak nem sensöründen elde edilen oran eklenir. 
postStr += "&field2=";
postStr += String(sicaklik);   // Gönderilecek string'e sıcaklık değeri eklenir. 
postStr += "&field3=";
postStr += String(nem); // Gönderilecek string'e nem değeri eklenir. 
postStr += "\r\n\r\n";

client.print("POST /update HTTP/1.1\n"); //POST komutu yardımıyla her 10sn bir okunan değerler sunucuya yollanır. 
client.print("Host: api.thingspeak.com\n"); //Sunucu yolu belirtilir.
client.print("Connection: close\n"); //
client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n"); //Sunucuya bağlantı sağlayacak olan thingspeak API anahtarı kullanılır. 
client.print("Content-Type: application/x-www-form-urlencoded\n"); //Sunucuya yollanan içerik türü belirlenir.
client.print("Content-Length: "); 
client.print(postStr.length());  //string olarak yollanan karakterlerin uzunluğu sunucuya yollanır. 
client.print("\n\n"); //Yeni satır karakterine ulaşınca karakterlerin sunucuya yollanması tekrarlanır. 
client.print(postStr);   // Eklenen string değerler postStr ile sunucuya yollanır. 
Serial.print("Toprak nem oranı: "); //Seriport ekranına yazdırılır. 
Serial.print(topraknemoran); //Hesaplanan toprak nem oranı ekrana yazdırılır. 
Serial.print("%. Sıcaklık: "); //Seriport ekranına yazdırılır. 
Serial.print(sicaklik); //Hesaplanan sıcaklık değeri seriport ekrana yazdırılır. 
Serial.print(" C, Nem: "); //Seriport ekrana yazdırılır. 
Serial.print(nem); //Hesaplanan nem değeri seriport ekrana yazdırılır. 
Serial.println("%. Thingspeak sunucusuna veriler yollandı.");
}
}
