#include <DHT.h>     // DHT11 sensörünün kütüphanesi koda eklenir. 
#include <LiquidCrystal.h> //LCD ekran için kütüphane koda eklenir.
const int RS = 12, enable = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(RS, enable, d4, d5, d6, d7); //LCD pin uçları tanımlanır.

#include<Timer.h> //Timer.h kütüphanesi koda eklenir. 
Timer t; //Timer(zamanlayıcı) nesnesi oluşturulur.
#include <SoftwareSerial.h> //Softwareserial kütüphanesi koda eklenir. 
SoftwareSerial Serial1(2, 3); //RX, TX bağlantıları pin3 ve pin2 dijital pinlere bağlanır.

#define DHT11pin 8 //DHT11 sensör 8 nolu dijital pine atanır.
#define veri 13 //zamanlayıcının kontrol edileceği pin 13 olarak belirlenir.
#define DHTTYPE DHT11  // Sensör türü DHT11 ya da DHT22 olarak seçilir. 
DHT dht(DHT11pin, DHTTYPE);   //pin çıkışı ve türü tanımlanan DHT sensörü için nesne oluşturulur.   

char *api_anahtar="Q5EOSH1ND9Z0MWUS"; //ThingSpeak'dan elde edilen Write API numarası yazılır. 
static char postUrl[150]; //Statik char kullanılmasının nedeni, postURL değişkeninin 150 ile sınırlı kalmasını sağlamak içindir.
int nem,sicaklik; //Nem ve sıcaklık değerleri integer olarak atanır. 
void httpGet(String ip, String path, int port=80); //httpGet fonksiyonu yardımıyla yollanacak veriler için,
//ip adresi, veri yolu ve port numarası String olarak atanır. 

void setup()
{
lcd.begin(16, 2); //LCD ekran başlatılır. 
lcd.clear(); //Ekran temizlenir
lcd.print(" Nem&Sicaklik"); //İlk satıra yazdırılır. 
lcd.setCursor(0,1);
lcd.print("  Uygulamasi "); //2.satıra yazdırılır. 
delay(4000); //4 saniye beklenir. 
lcd.clear(); //Ekran temizlenir. 
lcd.print("DHT11&Thingspeak "); //İlk satıra yazdırılır. 
lcd.setCursor(0,1);
lcd.print("   NIT "); //2.satıra yazdırılır. 
delay(4000); //4 saniye beklenir. 
Serial1.begin(9600); // ESP8266 modülü veri akış hızı 9600 olarak belirlenir. 
Serial.begin(9600); //Arduino seri port veri akış hızı 9600 olarak belirlenir. 
lcd.clear(); //LCD ekran temizlenir. 
lcd.print("Wifi baglaniyor"); //LCD ekrana yazdırılır. 
lcd.setCursor(0,1);
lcd.print("Lutfen bekleyin...."); //2.satıra yazdırılır. 
Serial.println("Wifi baglaniyor...."); //Seri port ekranına yazdırılır. 
connect_wifi("AT",1000); //ESP8266 modülüne AT komutu yollanarak, modülün hazır olup olmadığı kontrol edilir. 
//Modül hazırsa, seri port ekrana OK yazılır.1 saniye beklenir.
connect_wifi("AT+CWMODE=1",1000); //ESP8266, STA moduna alınır. Seri port ekranda OK yazar. 1 saniye beklenir.
connect_wifi("AT+CWQAP",1000);   //ESP8266 herhangi bir ağa bağlıysa çıkmak için kullanılır. Seri port ekranda OK yazar. 1 saniye beklenir.
connect_wifi("AT+RST",5000); //ESP8266 modülü resetler ve kullanıma hazır hale getirir. 5 saniye bekler. Seri port ekranda OK yazar. 
connect_wifi("AT+CWJAP=\"NetMASTER Uydunet-9167\",\"7f6dbd25e1fec512\"",10000); // Kullanılacak Wifi(Kablosuz ağ) adı ve şifresi yazılmalıdır. 
//1 dakika beklenir. 
Serial.println("Wifi bağlandı."); //Bağlantı başarılı ise, seri port ekrana yazdırılır. 
lcd.clear(); //LCD ekran temizlenir. 
lcd.print("Wifi baglandi."); //LCD ekrana aynı yazı yazdırılır. 
pinMode(veri, OUTPUT);
delay(2000);
t.oscillate(veri, 1000, LOW); //13 nolu pin üzerinden veri, her 1 saniyede salınımı(osciallete) geçerek geçişe hazır hale getirilir. 
t.every(20000, send2server); //Zamanlayıcı kütüphanesi kullanarak thinkspeak veri tabanına veri yollama işi 2 dak. bir gerçekleşir. 
}

void loop()//Void loop () fonksiyonunda sıcaklık ve nem okur ve ardından LCD'de bu değerler gösterilir. 
{

dht.begin(); //DHT11 sensörü başlatılır. 
lcd.setCursor(0,0); 
lcd.print("Nem: "); //LCD ekranın ilk satırına yazdırılır. 
nem=dht.readHumidity(); //Okunan nem değeri, nem isimli değişkene atanır. 
lcd.print(nem);   // Okunan değer, LCD ekrana yazdırılır. 
lcd.print(" %       "); //Nem değerinden sonra yüzdelik işareti yazdırılır. 
lcd.setCursor(0,1); //2.satıra yazdırılır. 
lcd.print("Sicaklik:");
sicaklik=dht.readTemperature(); //Sensörden okunan sıcaklık değeri, sicaklik isimli değişkene atanır. 
lcd.print(sicaklik);   // Okunan değer, LCD ekrana yazdırılır.
lcd.print(" C   "); //
delay(1000);
t.update(); //Zamanlayıcı nesnesi güncellenierek, okunan verilerin thingspeak'a yollanması sağlanır. 
}

void send2server() //Verileri sunucuya göndermek için void send2server () fonksiyonu kullanılır. 
//Send2server fonksiyonu, her 20 saniyede bir çağrı yapan bir zamanlayıcı kesme servisi rutinidir. 
//Güncelleme işlevini çağırdığımızda, zamanlayıcı kesme servis rutini çağrılır.
{
char sicaklikStr[8]; //sicaklikStr isminde bir değişken tanımlanır. 
char nemStr[8]; //nemStr isminde bir değişken tanımlanır. 
dtostrf(sicaklik, 5, 3, sicaklikStr); //veritabanına değişken yollanırken float türünden, char türünü dönüştürülür.
dtostrf(nem, 5, 3, nemStr); //5 yollanacak verinin toplam uzunluğu, 3 ise .dan sonraki ondalık kısmıdır. 
sprintf(postUrl, "update?api_key=%s&field1=%s&field2=%s",api_anahtar,nemStr,sicaklikStr); //nem ve sicaklik değerleri thingspeak de oluşturulan
//field1 ve field2 isimli alanları Yazma anahtarı yardımıyla postURL fonksiyonu aracılığı ile yollanır. 
httpGet("184.106.153.149", postUrl, 80); //thingspeak programının ip adresi doğru olarak girilmelidir. 
}

void httpGet(String ip, String path, int port) //Nem ve sıcaklık değerleri thingspeak programına yollanır. 
{
int resp;
String atHttpGetCmd = "GET /"+path+" HTTP/1.0\r\n\r\n"; // GET komutu ile GET https://api.thingspeak.com/update?api_key=Q5EOSH1ND9Z0MWUS&field1=0
//kullanarak field1 ve field2 alanlarına veri yazma işlemi hazır hale getirilir. 
String atTcpPortConnectCmd = "AT+CIPSTART=\"TCP\",\""+ip+"\","+port+""; //CIPSTART komutu ile sunucuya bağlanılır. 
connect_wifi(atTcpPortConnectCmd,1000); //Kablosuz ağ yardımıyla her 1 saniyede port üzerinden veri yollama işlemi başlatılır. 
int len = atHttpGetCmd.length();  //Yollanacak verinin boyutu belirlenir. 
String atSendCmd = "AT+CIPSEND="; //CIPSEND yardımıyla yollanacak karakterin uzunluğu atSendCmd isimli bir değişkene atanır. 
atSendCmd+=len;
connect_wifi(atSendCmd,1000); //Uzunluğu belirlenen karakterler connect_wifi fonksiyonuna yollanır.
connect_wifi(atHttpGetCmd,1000); //Karakter uzunluğu sayı olarak connect_wifi fonksiyonuna yollanır. 
}

void connect_wifi(String cmd, int t) //AT komut çalıştırma ve veri yollama işlemleri bu fonksiyon aracılığıyla yapılır. 
{
int i=0;
while(1) //Veri yollama işleminin gösterilmesi sürekli olarak istendiği için while(1) kullanılmıştır.
{
lcd.clear(); //LCD ekran, temizlenir. 
lcd.print(cmd); //Yollanan karakterler LCD ekranda yazdırılır. 
Serial.println(cmd); //Yollanan veriler seriport ekranda yazdırılır. 
Serial1.println(cmd); //Serial1 vasıtasıyla ESP8266 modülüne de veriler yollanır. 
while(Serial1.available()) //Eğer, ESP8266 modülüyle ilgili bi sıkıntı yoksa... 
{
if(Serial1.find("OK")) //ESP8266 modülünden OK döndürülerek seri port ekrana yollanır. 
i=8; //i değişkenine 8 atanır. 
}
delay(t); //Yukardan AT komutuyla gelen süre kadar beklenir. 
if(i>5) //AT komutlarının çalıştırılması 5 kere denenir. AT komutu başarısız olursa Hata mesajı yazar. 
break;
i++; //Sayaç bir artırılarak bağlantı işlemi tekrar kontrol edilir. 
}
if(i==8) //Eğer sayaç numarası tekrar 8 olursa, veri yollanır...
{
Serial.println("OK"); //ESP8266'a Tx üzerinden veri yollanması başarılı bir şekilde yapılırsa, Seri port ekrana OK yazar. 
lcd.setCursor(0,1);
lcd.print("OK"); //Aynı şekilde veri yollanması başarılı olursa, LCD ekrana OK yazar. 
}
else 
{
Serial.println("Hata"); // Eğer veri yollanması ile ilgili bir hata oluşursa seri port ekranında Hata yazar. 
lcd.setCursor(0,1);
lcd.print("Hata"); //Veri yollanması ile ilgili bir hata oluşursa LCD ekranında Hata yazar.
}
}
