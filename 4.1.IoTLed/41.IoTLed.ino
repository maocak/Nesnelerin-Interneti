#include <ESP8266WiFi.h>   // ESP8266 kütüphanesi eklenir.
#include <FirebaseArduino.h>   // Firebase kütüphanesi eklenir.

#define FIREBASE_HOST "leduygulama-f8029.firebaseio.com"  // Firebase içinde girilen proje ID ismi eklenir.
#define FIREBASE_AUTH "TDktbopnB7jK9XSuXbT49uwqvNuXVAxkgnkOSpAG"  // Firebase'in oluşturduğu gizli anahtar girilir.
#define WIFI_SSID "MAO"                   // Ev ya da işyerinde bulunan kablosuz ağ ismi(WiFi) girilir.  
#define WIFI_PASSWORD "furkanirem777"     //Wifi internet şifresi girilir. 

String fireStatus = "";   // Firebase'den okunacak olan led durumu string olarak atanır. 
int led = D3;  // NodeMCU kartının D3 dijital pini LED için atanır. 

void setup() {
Serial.begin(115200); //Seri port ekranına bağlantı sağlanır. 
delay(10); //1sn beklenir.
pinMode(led, OUTPUT);   //LED pini çıkış olarak tanımlanır.               
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);    //Kablosuz internet ağına bağlantı sağlanır. 
Serial.print("Bağlanıyor... ");
Serial.print(WIFI_SSID);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");

}
Serial.println();
Serial.print("Bağlandı ");
Serial.println(WIFI_SSID);
Serial.print("IP adresi : ");
Serial.println(WiFi.localIP());      //Yerel IP adresi gösterilir. 
Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // Firebase'e bağlantı sağlanır. 
Firebase.setString("LED_DURUM", "SONDUR");  //LED ilk durumu sönük olarak belirlenir.(program çalıştığında LED sönük durumdadır)
}

void loop() {
fireStatus = Firebase.getString("LED_DURUM");    // Firebase den girilen değer okunarak fireStatus a atanır. 
if (fireStatus == "YAK"|| fireStatus == "yak"){  // Firebase den girilen değer eğer YAK ya da yak ise...
Serial.println("Led yanıyor");                                        
digitalWrite(led, HIGH);     // LED yanar.
} 
else if (fireStatus == "SONDUR"||fireStatus == "sondur") {  // Firebase den girilen değer eğer SONDUR ya da sondur ise...
Serial.println("Led sönük");
digitalWrite(led, LOW);         // Led söner. 
}
else {
Serial.println("Yanlış veri! Lütfen YAK/yak ya da SONDUR/sondur yazınız"); //eğer farklı bir veri girilirse bu uyarı yazar.
}
}

