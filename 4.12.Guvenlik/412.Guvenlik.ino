#include <ESP8266WiFi.h>  //NodeMCU kartının kablosuz ağ üzerinden iletişimi yürütmesi için gerekli olan kütüphane koda eklenir. 
#include <BlynkSimpleEsp8266.h> //Blynk programına bağlanmak için gerekli olan kütüphan koda eklenir. 
#include <SoftwareSerial.h>
#include <SimpleTimer.h> //Zamanlama ile ilgili işleri yürütecek olan kütüphane koda eklenir.
 
char auth[] = "geuNceA2nzJ1LKpg05ral-x_6XKKdPSD"; //Blynk programında yollanan emailde bulunan Auth Token yazılır. 
char ssid[] = "***"; //Kullanılan kablosuz ağın ismi yazılır. 
char pass[] = "***";       //Kullanılan kablosuz ağın şifresi yazılır. 
 
SimpleTimer timer; //timer isminde bir nesne oluşturulur. 

void myTimerEvent()// Bu işlev, Arduino'nun her saniyede çalışma süresini Virtual Pin(1)'e gönderir. Uygulamada, Widget'ın okuma sıklığı PUSH olarak ayarlanmalıdır. 
//Bunun anlamı  Blynk Uygulaması'na ne sıklıkta veri gönderileceğini tanımlarsınız.
{
Blynk.virtualWrite(V1, millis() / 1000); // Herhangi bir değeri istediğiniz zaman gönderebilirsiniz. Saniyede 10'dan fazla değer gönderilmemelidir.
}
 
int pirsensor = D0; // PIR sensör, D0 pine bağlanır. 
int irsensor = D1;  // IR sensörü, D1 pine bağlanır. 
 
void setup()
{
Serial.begin(9600); //seri veri iletişimi başlatılır.
Blynk.begin(auth, ssid, pass); // Blynk uygulaması başlatılır. Burda Blynk Auth token, kablosuz ağ ismi ve şifresi belirtilir. 
pinMode(pirsensor, INPUT_PULLUP); //PIR sensörü pini giriş olarak tanımlanır. Burda içsel direnç kullanılmıştır. 
pinMode(irsensor, INPUT); //IR sensör pini giriş olarak tanımlanır. 
timer.setInterval(1000L,sensorvalue1); //Her 1sn'de çağrılan bir fonksiyon tanımlanır. 
 
}
 
void loop()
{
Blynk.run(); //Blynk programı çalıştırılır. 
timer.run(); // BlynkTimer  zamanlayıcısı çağrılır. 
}
 
void sensorvalue1()
{
 
if(digitalRead(pirsensor) == HIGH) //Eğer PIR sensörü HIGH olursa,...
{
Blynk.virtualWrite(V2,255 ); // V2'de tanımlı LED yanar. 
Blynk.virtualWrite(V4,"PIR sensöründe hareket tespit edildi!!!" ); //V4'te tanımlı terminale bu yazı yazdırılır. 
Blynk.notify("PIR sensöründe hareket tespit edildi!!!"); //Blynk programında, bu mesaj olarak gösterilir.
}
 
if(digitalRead(pirsensor) == LOW) //Eğer PIR sensörü LOW olursa,...
{
Blynk.virtualWrite(V2,0 ); //V2'de tanımlı LED söner. 
Blynk.virtualWrite(V4,"PIR Normal" );  //V4'te tanımlı terminale "PIR Normal" yazar.
}
 
if( digitalRead(irsensor) == LOW) //Eğer IR sensörü LOW olursa,...
{
Blynk.virtualWrite(V3,255 ); // V3'te tanımlı LED yanar. 
Blynk.virtualWrite(V4,"IR sensöründe hareket tespit edildi!!!" ); //V4'te tanımlı terminale bu yazı yazdırılır. 
Blynk.notify("IR sensöründe hareket tespit edildi!!!"); //Blynk programında, bu mesaj olarak gösterilir.
} 
 
if( digitalRead(irsensor) == HIGH) //Eğer IR sensörü HIGH olursa,...
{
Blynk.virtualWrite(V3,0 ); //V3'te tanımlı LED söner.
}
 
}
