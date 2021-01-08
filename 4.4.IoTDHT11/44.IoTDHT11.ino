#include <ESP8266WiFi.h>        // ESP8266 kütüphanesi eklenir.
#include <FirebaseArduino.h>    // Firebase kütüphanesi eklenir.
#include <DHT.h>                // Dht11 kütüphanesi eklenir.

#define FIREBASE_HOST "dht11-b61a8.firebaseio.com"     // Firebase içinde yer alan proje ID ismi eklenir.
#define FIREBASE_AUTH "c8WFqahVHBXy8y25XPVcg0qPfWJ6zELHP6b65dEx"   // Firebase'in oluşturduğu gizli anahtar girilir.
#define WIFI_SSID "MAO"                    // Ev ya da işyerinde bulunan kablosuz ağ ismi(WiFi) girilir. 
#define WIFI_PASSWORD "furkanirem777"      //Wifi internet şifresi girilir.
 
#define DHTPIN D4      // NodeMCU kartının D4 dijital pini DHT11 için atanır.
#define DHTTYPE DHT11  // Sensör türü DHT11 ya da DHT22 olarak seçilir. 
DHT dht(DHTPIN, DHTTYPE);   //pin çıkışı ve türü tanımlanan DHT sensörü için nesne oluşturulur.                                                   

void setup() {
Serial.begin(115200); //seri veri iletişimi başlatılır. 
delay(1000);  //1 saniye beklenir.               
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  //Kablosuz internet ağına bağlantı sağlanır.
Serial.print("Bağlanılıyor ");
Serial.print(WIFI_SSID);
while (WiFi.status() != WL_CONNECTED) {
Serial.print(".");
delay(500);
}
Serial.println();
Serial.print("Bağlandı "); //Bağlantı sorunsuz şekilde sağlanırsa, bağlantı adı ve IP adresi seri port ekrana yazdırılır.
Serial.println(WIFI_SSID);
Serial.print("IP adresi : ");
Serial.println(WiFi.localIP());                                            //print local IP address
Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   // Firebase'e bağlantı sağlanır.
dht.begin();    //DHT11 sensöründen veri okuma işlemi başlatılır. 
}

void loop() { 
float h = dht.readHumidity();    // Sensörden okunan nem değeri h isimli değişkene atanır. 
float t = dht.readTemperature(); // Sensörden okunan sıcaklık değeri  t isimli değişkene atanır. 
    
if (isnan(h) || isnan(t)) {      // Sensörden veri okuma ile ilgili bir sıkıntı oluşursa, ekrana hata uyarısı verir. 
Serial.println(F("DHT sensöründen veri okuma hatası!"));
return;
}
  
Serial.print("Nem: ");  Serial.print(h);
String nem = String(h) + String("%");    //Nem değeri tam sayıdan, stringe dönüştürülür.  
Serial.print("%  Sıcaklık: ");  Serial.print(t);  Serial.println("°C ");
String sicaklik = String(t) + String("°C"); //Sıcaklık değeri tam sayıdan, stringe dönüştürülür. 
delay(4000); //Değerlerin yazımı 4 saniye de bi gerçekleşir. 
  
Firebase.pushString("/DHT11/Nem", nem);     //Okunan nem değeri Firebase veri tabanına "nem yolu" içinde yazdırılır. 
Firebase.pushString("/DHT11/Sicaklik", sicaklik);   //Okunan sıcaklık değeri Firebase veri tabanına "sıcaklık yolu" içinde yazdırılır. 
   
}
