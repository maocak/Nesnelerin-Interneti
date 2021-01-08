#include <ESP8266WiFi.h> //Öncelikle, Wi-Fi işlevlerine erişmek için ESP8266WiFi.h kütüphanesini kullanıyoruz.
 
const char* ssid = "MAO"; //Wi-fi kullanıcı ismi ve şifresi yazılır.
const char* password = "furkanirem777";
 
int led= D3; // LED için D3 digital pin kullanılmaktadır. 
WiFiServer server(80); //Sonra, WifiServer sunucu nesnesi oluşturulur. 
//Böylece wifi kütüphanesinde yer alan işlemleri yerine getirebiliriz. 
//80, HTTP için varsayılan bağlantı noktası olduğundan sunucu portunu dinlemek için kullanılır. 
 
void setup() {
Serial.begin(115200); //Seri veri iletişimi 115200 baud rate de başlatılır. 
delay(10);
 
pinMode(led, OUTPUT); //LED pini çıkış olarak atanır. 
digitalWrite(led, LOW); //LEDin ilk durumu LOW(sönük) yapılır. 
 
Serial.println();
Serial.print("Bağlanıyor... ");
Serial.println(ssid); //Wi-fi kullanıcı adı seri port ekrana yazdırılır. 
 
WiFi.begin(ssid, password); // Wifi bağlantısı sağlanır. 
 
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print("."); //Bağlantı sağlanana kadar seri port ekranda . karakteri yazar. 
}
Serial.println("");
Serial.println("Wi-Fi bağlantısı sağlandı."); //Wi-fi bağlantısı başarılı olursa ekrana uyarı yazısı yazar. 
 
server.begin();   // Sunucu başlatılır.
Serial.println("Web sunucu başladı.");
 
Serial.print("Web sunucuya bağlanmak için bu bağlantıyı kullanın  ");// baglantı saglandıktan sonra serial monitorde ip adresi gösterilir.
Serial.print("http://");
Serial.print(WiFi.localIP()); //Web sunucuya bağlanmak için kullanılacak ip adresi yazdırılır.
Serial.println("/");
}
 
void loop() {
WiFiClient client = server.available();// İstemciye baglanılıp bağlanılmadığı kontrol edilir.
if (!client) {
return;
}
 
Serial.println("Yeni istemci:"); // İstemciden veri yollanması beklenir.
while(!client.available()){
delay(1);
}
 
String request = client.readStringUntil('\r'); // Gelen veri okunur ve seriport ekrana yazdırılır.
Serial.println(request); 
client.flush(); //Arabellekte tüm giden karakterler gönderilinceye kadar bekler.
 
int deger = LOW; //Değer değişkeni ilk değer LOW atanır.
if (request.indexOf("/LEDYAK") != -1) { //Okunan değer eğer LEDYAK ise...
digitalWrite(led, HIGH); //LED yanar...
deger = HIGH; //değişkene HIGH atanır.
}
if (request.indexOf("/LEDSONDUR") != -1) { //Okunan değer eğer LEDSONDUR ise...
digitalWrite(led, LOW); //LED söner. 
deger = LOW; //değişkene LOW atanır.
}
 
// Bu kısımda html kodları yardımıyla web istemci sayfası oluşturulur. 
client.println("HTTP/1.1 200 OK"); //Standart bir html sayfası oluşturulur.
client.println("Content-Type: text/html");
client.println(""); 
client.println("<!DOCTYPE HTML><html>");
//html sayfasının yazı tipi, sayfa ortalama özelliği, ve sayfa zemin rengi sarı olarak ayarlanır. 
client.println("<style>html{ font-family: Cairo; text-align: center; background-color: yellow;}</style>"); 
client.println("<h2>NodeMCU LED uygulamasi</h2>"); //h2 etiketiyle sayfanın ortasına bu yazı yazdırılır. 
client.print("LED DURUM: "); 
 
if(deger == HIGH) {
client.print("<b>Yaniyor</b>"); //Eğer değer HIGH ise koyu renkli olarak Yaniyor yazar. 
} else {
client.print("<b>Sonuk</b>"); //Eğer değer LOW ise koyu renkli olarak Sonuk yazar. 
}
client.println("");

client.println("<p><a href=\"/LEDYAK\"\"><button>Ledi yak</button></a></p>"); //Ledi yak isminde bir buton oluşturulur.
client.println("<a href=\"/LEDSONDUR\"\"><button>Ledi sondur</button></a>");  //Ledi sondur isminde bir buton oluşturulur. 
client.println("</html>");
 
delay(1);
Serial.println("");
 
}
