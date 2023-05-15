#include <ESP8266WiFi.h> //Wi-fi bağlantısı için kullanılan kütüphane koda eklenir. 
#include "DHT.h" //DHT11 nem ve sıcaklık sensörü kütüphanesi eklenir. 
#include <SFE_BMP180.h> //Basınç sensörü (BMP180) kütüphanesi eklenir. 
#include <Wire.h> //Standart olarak vardır. 
 
#define yukseklik 875.0 //Ankara,TR deniz seviyesinden yüksekliği
//Bulunulan yerin deniz seviyesinden yüksekliği manuel olarak girilmelidir. 
#define DHTPIN D3     // DHT11 sensörünün bağlandığı pin tanımlanır. 
#define DHTTYPE DHT11   // DHT 11 sensör türü olarak seçilir. 
 
SFE_BMP180 basinc; //basinc altında bir BM180 sensör nesnesi oluşturulur. 
DHT dht(DHTPIN, DHTTYPE); //dht isminde bir nem ve sıcaklık sensörü nesnesi oluşturulur. 
const char* ssid     = "***"; //Wi-fi kullanıcı ismi yazılır.
const char* password = "***"; //Wi-fi kullanıcı şifresi yazılır.
 
char status;
double T,P,basinc0; //Mutlak basınç ölçümü P değerinde saklanır. 
//BM180 aynı zamanda sıcaklık da ölçtüğü için ölçümde kullanılacak sıcaklık değeri T değerinde saklanır. 
//Göreceli basınc değeri için basinc0 değişkeni kullanılır. 
WiFiServer server(80); //Sunucuya bağlanmak için bir sunucu nesnesi oluşturulur. 

void setup() {
Serial.begin(9600); //Seri port ekranı çalışma hızı atanır. 
delay(100);
dht.begin(); //DHT11 sensörü başlatılır. 
Serial.print("Bağlanıyor... ");
Serial.println(ssid); //Bağlantı için kullanılacak wifi kullanıcı ismi ekrana yazdırılarak...  
WiFi.begin(ssid, password); //...wi-fi bağlantısı başlatılır.
 
while (WiFi.status() != WL_CONNECTED) { //Yarım saniye aralıklarla bağlantı kontrol edilir. Bu arada ekrana . yazdırılır. 
delay(500);
Serial.print(".");
}
 
Serial.println(""); //Eğer bağlantı başarılı olursa ekrana...
Serial.println("Kablosuz ağ(Wi-Fi)bağlandı."); //...yazdırılır. 
server.begin(); //Sunucunun çalışması başları ve...
Serial.println("Sunucu başladı."); //Ekrana ...yazar. 
 
Serial.println(WiFi.localIP()); //Kullanılacak IP adresi ekrana yazdırılır. Bu IP aynı zamanda sunucuya bağlanmak için kullanılmaktadır. 
if (basinc.begin()) //Eğer BM180 sensörü başarılı şekilde çalışmaya başlarsa...
Serial.println("BMP180 sensörü çalışmaya başladı."); //Ekrana ... yazdırılır. 
else
{
Serial.println("BMP180 sensör hatası\n\n"); //Eğer BM180 sensörü düzgün şekilde çalışmazsa ekrana ...yazdırılır. 
while(1); // Program durur. 
}
delay(1000);
}
 
void loop() {
  
float nem = dht.readHumidity(); //Nem değeri okunur. 
float sicaklikC = dht.readTemperature(); // Celsius cinsinden sıcaklık okunur. 
float sicaklikF = dht.readTemperature(true); // Fahrenayt cinsinden sıcaklık okunur. (Fahrenheit = true)
  
status = basinc.startPressure(3); //Sensörden basınç değerinin okunması başlatılır. 
//Burda parametre bekleme süresiyle ilgilidir. 0=en hızlı okuma, 3=en uzun bekleme
if (status != 0)
{
// Okuma işleminin tamamlanması beklenir. 
delay(status);

status = basinc.getPressure(P,T); //Sensörden P ve T değerleri elde edilir. 
if (status != 0)
{
Serial.print("Mutlak basınç: "); //Mutlak basınç değeri 2 ondalıklı olmak üzere seri porta yazdırılır. 
Serial.print(P,2);
Serial.print(" mb, "); //Yazdırılan değer mb cinsindendir. 
Serial.print(P*0.0295333727,2); //Değer aynı zamanda Hg cinsine dönüştürülerek yazdırılır. 
Serial.println(" Hg"); 
 
basinc0 = basinc.sealevel(P,yukseklik); // Ankara, TR, deniz seviyesi 875 metre
Serial.print("Göreceli deniz seviyesi basıncı: ");
Serial.print(basinc0,2);
Serial.print(" mb, "); //Hesaplanan göreceli deniz seviyesi basıncı mb ve Hg cinsinden yazdırılır. 
Serial.print(basinc0*0.0295333727,2);
Serial.println(" Hg");
}
}
Serial.print("Nem: "); //Nem ve sıcaklık değerleri (santigrad ve fahrenayt cinsinden) seri port ekrana yazdırılır. 
Serial.print(nem);
Serial.print(" %, ");
Serial.print("Sıcaklık: ");
Serial.print(sicaklikC);
Serial.print(" *C, ");
Serial.print(sicaklikF);
Serial.println("  F, ");
Serial.println("");

WiFiClient client = server.available(); // Sunucuya baglanılıp bağlanılmadığı kontrol edilir.
// Bu kısımda html kodları yardımıyla web istemci sayfası oluşturulur. 
client.println("HTTP/1.1 200 OK"); //Standart bir html sayfası oluşturulur.
client.println("Content-Type: text/html");
client.println("Connection: close");  // Sunucu bağlantısı, yanıtın tamamlanmasından sonra kapatılır. 
client.println("Refresh: 10");  // Web sayfası her 10 saniyede bir yenilenir. 
client.println();
client.println("<!DOCTYPE HTML>");
client.println("<html>");
//html sayfasının yazı tipi, sayfa ortalama özelliği, ve sayfa zemin rengi sarı olarak ayarlanır. 
client.println("<style>html { font-family: Cairo; text-align: center;color: black; background-color: PowderBlue;}");client.println("</style>");
client.println("</head>");
//h2 etiketiyle sayfanın ortasına bu yazı yazdırılır. 
client.println("<h2 style=color:red> NodeMCU METEROLOJI SUNUCU SAYFASI</h2>");
//Okunan nem değeri sayfanın ortasına yazdırılır. 
client.print("<p><h2>Nem: "); client.print(nem); client.println(" %</h2></p>");
//Okunan sıcaklık değeri sayfanın ortasına yazdırılır. (santigrad ve fahrenayt cinsinden)
client.print("<p><h2>Sicaklik: "); client.print(sicaklikC); client.print(" *C,  "); 
client.print(sicaklikF);client.println(" F </h2></p>");
//Okunan basınç değeri, sayfanın ortasına yazdırılır (mb cinsinde)
client.print("<p><h2>Basinc: "); client.print(basinc0,2); client.println(" mb </h2></p>");

client.println("</html>");
delay(4000); //4 saniye beklenir. 
}
