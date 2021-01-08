#include <LiquidCrystal.h> //LCD ekran için kütüphane koda eklenir.
const int RS = 12, enable = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(RS, enable, d4, d5, d6, d7); //LCD pin uçları tanımlanır.
#include <SoftwareSerial.h> //Softwareserial kütüphanesi koda eklenir.
float pulse = 0;
float sicaklik = 0; //sicaklik değişkenine 0 değeri atanır. 
SoftwareSerial ser(9,10); //RX, TX bağlantıları pin10 ve pin9 dijital pinlere bağlanır.
String apiKey = "62XDFC5V3U7H94ZH"; //thingspeak'ten elde edilen API anahtarı yazılır. 

// Değişkenler
int nabizPin = A0; // Nabız sensörü data kablosu A0 analog pine bağlanır.
int nabizvarPin = 8 ; // Her kalp atışında yanma sönme yapan pin no
int nabizyokPin = 13; // Kalp atışı olmadığında yanma sönme yapan pin no
int nabizyokoran = 0; // Nabız olmadığında orana göre atan değer 0 olarak atanır. 

// Kodda kesme kullanıldığı için, aşağıdaki değişkenler geçici (volatile) değişken türünde atanmıştır. 
volatile int KAH; // Kalp atış hızı değişkeni (Analog 0'dan gelen değeri tuar, 2ms'de bir yenilenir) 
volatile int sinyal; // Gelen ham verileri tutar.
volatile int zamanaralik = 600; // Kalp atışları arasındaki zamanı tutan değişken. 
volatile boolean Pulse = false; // Kullanıcının canlı kap atışı algılandığında "True" değeri alır, diğer durumda "False" olur.
volatile boolean atis = false; // Arduino bir kalp atışı tespit ettiğinde "True" değeri alır. 

// Bu kısımda seri çıkış ayarlanır. İhtiyaca göre değerler değiştirilebilir. 
static boolean seriportgorsel = true; // Varsayılan olarak 'false' ayarlanır. Arduino Seri Monitör ASCII Görsel Nabzı görmek için 'true' değeri ayarlanır. 
volatile int zaman[10]; // En son alınan 10 zamanaralik değerini tutar.
volatile unsigned long orneksayac = 0; // kalp atış zamanlamasını belirlemek için kullanılır. 
volatile unsigned long sonatiszaman = 0; //  "zamanaralik" değerini bulmak için kullanılır. 
volatile int enyuksek = 512; // Nabız atışlarındaki en yüksek değeri bulmak için kullanılır. 
volatile int endusuk = 512; // Nabız atışlarındaki en küçük değeri bulmak için kullanılır 
volatile int anlik = 525; // Nabız atışlarındaki anlık değeri bulmak için kullanılır. 
volatile int genlik = 100; // Nabız dalga formunun genliğini tutmak için kullanılır. 
volatile boolean ilkkalpatis = true; // Zaman dizisini beslemek(seed) için kullanılır, böylece mantıklı bir KAH değeri ile başlanmış olunur.
volatile boolean ikincikalpatis = false; // Zaman dizisini beslemek(seed) için kullanılır, böylece mantıklı bir KAH değeri ile başlanmış olunur.

void setup()
{
lcd.begin(16, 2); //LCD ekran başlatılır.
pinMode(nabizvarPin,OUTPUT); // kalp atışına yanıp sönecek olan pin çıkış olarak tanımlanır.
pinMode(nabizyokPin,OUTPUT); // Nabız olmadığında yanıp sönecek pin çıkış olarak tanımlanır. 
Serial.begin(115200); // Arduino seriport veri akış hızı belirlenir. 

kesmefonksiyonu(); // Her 2 ms'de bir kalp atış sensöründen sinyal okumak için kullanılan fonksiyon. 

// Eğer, Kalp atış sensörü, Arduino kartının sağladığı gerilimden daha küçük bir voltajta çalıştırılıyorsa bu kod satırı
// aktif hale getirilir, ve karttaki A-ref pinine kullanılacak gerilim kaynağı bağlanır. 
// analogReference(EXTERNAL);

lcd.clear(); //Ekran temizlenir
lcd.setCursor(0,0);
lcd.print("  Hasta Takip");
lcd.setCursor(0,1);
lcd.print("     Sistemi ");
delay(4000);
lcd.clear();
lcd.setCursor(0,0);
lcd.print("Baslatiliyor....");
delay(5000);
lcd.clear();
lcd.setCursor(0,0);
lcd.print("Veri aliniyor....");
ser.begin(9600);
ser.println("AT");
delay(1000);
ser.println("AT+GMR");
delay(1000);
ser.println("AT+CWMODE=1");
delay(1000);
ser.println("AT+CWQAP");
delay(1000);
ser.println("AT+RST");
delay(5000);
ser.println("AT+CIPMUX=1");
delay(1000);

String cmd="AT+CWJAP=\"MAOCU\",\"gnzl3979\"";
ser.println(cmd);
delay(1000);
ser.println("AT+CIFSR");
delay(1000);
}

// loop fonksiyonu içinde okunan sıcaklık ve KAH değerleri devamlı olarak ESP8266 modülüne, ordanda sunucuya yollanır.
void loop()
{
seriportcikis();
if (atis == true) // Arduino kalp atışı algıladığında atis değeri true olur. 
{
nabizyokoran = 255; // 'nabizyokoran' değişkenine 255 atanarak, kalp atışı ile LEDin yanıp sönmesi sağlanır. 
kalpatisseriportcikis(); // Bir kalp atışı olduğunda, fonksiyona yollanır. 
atis = false; // atis değerine "false" atanarak sıfırlanır.  
}
ledsoldurma(); // LED'in solma etkisini oluşturan fonksiyon çağrılır. 
delay(20); // 20msaniye beklenir. 
read_sicaklik(); //Sıcaklık değeri okuyan fonksiyon çağrılır. 
esp_8266(); //ESP8266 fonksiyonu çağrılır. 
}

void ledsoldurma()
{
nabizyokoran -= 15; // LED solukluk değeri atanır. 
nabizyokoran = constrain(nabizyokoran,0,255); // LED değerinin negatif sayılar alması önlenir. 
analogWrite(nabizyokPin,nabizyokoran); // LED değeri okunan nabiz değerine göre soluklaştırılır. 
}

void kesmefonksiyonu()
{
// Her 2mS'de bir kesinti yapmak için Timer2'yi başlatır.
TCCR2A = 0x02; // Dijital pinler 3 ve 11'de PWM'yi devre dışı bırakır ve CTC moduna sokar.
TCCR2B = 0x06; // Bu, WGM22(bit kaydediciler) bitlerini ayarlayarak sayım sırasına başlar, ön ölçekleme 256 dır. 
OCR2A = 0X7C; // 500Hz zamanlama için, en üst sayma sayısı 124 olarak atanır. 
TIMSK2 = 0x02; // Timer2 ve OCR2A arasındaki kesme eşleşmesi etkin hale getirilir. 
sei(); // Kesme işlemini yapan fonksiyonun ismidir. Tüm global kesmelerin etkinleştirildiğinden emin olunur. 
}

void seriportcikis()// Seri veri çıkışının nasıl olacağına karar verilir. 
{ 
if (seriportgorsel == true) //seriportgorsel değeri eğer "true" ise...
{
arduinoSerialMonitorVisual('-', sinyal); // ...seri port görüntüleyici yapan fonksiyona gider. 
}
else //...diğer türlü...
{
seriportaveriyolla('S', sinyal); // ...'S' öneki kullanılarak seriportaveriyolla fonksiyonuna gider. 
}
}

void kalpatisseriportcikis()
{
if (seriportgorsel == true) // Seri port ekran çalıştırmak için kullanılan fonksiyon eğer "true" ise...
{
Serial.print("Kalp atışı hızı:"); //"Kalp atış hızı:" yazdırılır. 
Serial.println(KAH); //Sensörden okunan değer yazdırılır. 
}
else //...eğer false ise...
{
seriportaveriyolla('B',KAH); // Kalp atış hızı 'B' önek yardımıyla fonksiyona yollanır.
seriportaveriyolla('Q',zamanaralik); // 'Q' öneki ile atımlar arasındaki zaman fonksiyona yollanır. 
}
}

void arduinoSerialMonitorVisual(char sembol, int veri )
{
const int sensorMin = 0; // Minimum sensör değeri. 
const int sensorMax = 1024; // Maksimum sensör değeri. 
int sensorokuma = veri; // Sensör aralığı 12 seçenekle eşleştirilir. 
int aralik = map(sensorokuma, sensorMin, sensorMax, 0, 11);
// Sensörden okunan değerin düştüğü aralığa göre, seri port ekranında yazdırılan ASCII karakterler case ile belirlenmiştir. 
switch (aralik)
{
case 0:
Serial.println(""); //12 seçeneğe göre, yazdırılacak olan ASCII karakterler şu şekildedir:
break;
case 1:
Serial.println("---");
break;
case 2:
Serial.println("------");
break;
case 3:
Serial.println("---------");
break;
case 4:
Serial.println("------------");
break;
case 5:
Serial.println("--------------|-");
break;
case 6:
Serial.println("--------------|---");
break;
case 7:
Serial.println("--------------|-------");
break;
case 8:
Serial.println("--------------|----------");
break;
case 9:
Serial.println("--------------|----------------");
break;
case 10:
Serial.println("--------------|-------------------");
break;
case 11:
Serial.println("--------------|-----------------------");
break;
}
}

void seriportaveriyolla(char sembol, int veri ) //Kullanılan önek ler ile seriport ekrana veri yollamanın yapıldığı fonksiyon.
{
Serial.print(sembol);
Serial.println(veri);
}

ISR(TIMER2_COMPA_vect) //Timer2(dahili zamanlayıcı) 124 olarak sayıldığında tetiklenir.
{
cli(); // Bu tetikleme yapılırken kesmeler devre dışı bırakılır. 
sinyal = analogRead(nabizPin); // Kalp atış sensöründen okunan veri, sinyal isimli değişkene atanır. 
orneksayac += 2; // Bu değişkenle, zaman mS cinsinden takip edilir. 
int N = orneksayac - sonatiszaman; // gürültüyü önlemek için son vuruştan bu yana geçen zaman izlenir. 
// Nabız atış dalgasının en yüksek ve en düşük noktası hesaplanır. 
if(sinyal < anlik && N > (zamanaralik/5)*3) // En son zamanaralik değerinin 3/5'i kadar beklenerek çift nabız okuma gürültülerinin önüne geçilir. 
{
if (sinyal < endusuk) // endusuk değişkeni, kalp atış dalgasında yer alan en düşük nabız değeridir. 
{
endusuk = sinyal; // Nabız dalgasındaki en düşük değer takip edilir. 
}
}
if(sinyal > anlik && sinyal > enyuksek)// anlik durum, gürültü oluşmasını engeller. 
{ 
enyuksek = sinyal; // enyuksek değişkeni, kalp atış dalgasında yer alan en yüksek nabız değeridir.(Nabız dalgasındaki en düşük değer takip edilir.)
} 

if (N > 250) // sinyal her nabız attığında yükselir. Eğer N değeri 250'den fazla ise...
{ 
if ( (sinyal > anlik) && (Pulse == false) && (N > (zamanaralik/5)*3) ) // Yüksek frekanslı gürültüden kaçınılır. 
{
Pulse = true; // Nabız olduğunda, değişken True değeri alır. 
digitalWrite(nabizvarPin,HIGH); // Pin13'e bağlı LED yanar. 
zamanaralik = orneksayac - sonatiszaman; // Kalp atışları arasındaki zaman mS cinsinden ölçülür. 
sonatiszaman = orneksayac; // Bir sonraki kalp atışı için zaman takip edilir. 

if(ikincikalpatis) //Eğer 2.kalp atışı gerçekleşirse(eğer ikincikalpatis == TRUE ise)
{  
ikincikalpatis = false; // ikincikalpatis değişkenine "false" atanarak, değer temizlenir. 
for(int i=0; i<=9; i++) // Başlangıçta gerçekçi bir KAH değeri elde etmek için toplanan değerler(koşu toplamı) beslenir. 
{
zaman[i] = zamanaralik;
}
}
if(ilkkalpatis) // Eğer ilk defa kalp atışı belirlenirse( if ilkkalpatis == TRUE ise)
{
ilkkalpatis = false; //  ilkkalpatis değeri temizlenir. 
ikincikalpatis = true; // 2.kalp atışı ayarlanır. 
sei(); // Kesmeler tekrar etkinleştirilir. 
return; // zamanaralik değeri burada göz ardı edilebilir. 
}
//Son 10 zamanaralık değerinin toplamı tutulur. 
word toplanandeger = 0; // toplanandeger değişkeni temizlenir. 
for(int i=0; i<=8; i++)
{ 
zaman[i] = zaman[i+1]; // zaman dizisinde yer alan verile kaydırılır, ve en eski zamanaralik değeri atılır. 
toplanandeger += zaman[i]; // En eski 9 zamanaralık değeri toplanır. 
}
zaman[9] = zamanaralik; // en son zamanaralik değeri zaman dizisine eklenir. 
toplanandeger += zaman[9]; // en son zamanaralik değeri toplanandeger a eklenir. 
toplanandeger /= 10; // En son 10 zamanaralik değerinin ortalaması alınır. 
KAH = 60000/toplanandeger; // Dakikada kaç kalp atışı olduğu hesaplanarak KAH'a atanır. 
atis = true; // atis değişkenine "true" atanır. 
pulse = KAH; //Ortalamadan elde edilen KAH değeri, pulse değişkenine atanır. 
}
}

if (sinyal < anlik && Pulse == true)//Eğer değerler aşağı doğru giderse, kalp atışı yavaşlar. 
{ 
digitalWrite(nabizvarPin,LOW); // Bu durumda LED söner. 
Pulse = false; // Pulse değeri sıfırlanır, böylece tekrar ölçüm hazır hale getirilir. 
genlik = enyuksek - endusuk; // Nabız dalgasının genliği hesaplanır. 
anlik = genlik/2 + endusuk; // Genliğin yüzde 50'si alınarak anlik değere atanır. 
enyuksek = anlik; // Değerler bir sonraki ölçüm için sıfırlanır. 
endusuk = anlik;
}
if (N > 2500) //Kalp atışı olmadan 2.5 saniye geçerse...
{  
anlik = 512; // anlik değere, var olan değer olan 512 atanır. 
enyuksek = 512; // Var olan değer, 512, tekrar atanır. 
endusuk = 512; // endusuk değişkenine de, var olan değer olan 512 atanır. 
sonatiszaman = orneksayac; //  sonatiszaman değeri güncellenir. 
ilkkalpatis = true; // Gürültüyü önlemek için ilkkalpatis değerine "true" atanır. 
ikincikalpatis = false; // Kalp atışı algılandığında ikincikalpatis değeri "false" atanır. 
}
sei(); // Kesmeler tekrar aktif hale getirilir. 
}// ISR fonksiyonun sonu...

void esp_8266() //Sıcaklık ve KAH değerleri thingspeak programına yollanır. 
{
// TCP bağlantısı AT+CIPSTART=4,"TCP","184.106.153.149",80 kullanılarak...
String cmd = "AT+CIPSTART=4,\"TCP\",\""; //...CIPSTART komutu ile sunucuya bağlanılır. 
cmd += "184.106.153.149"; // Sunucunun (api.thingspeak.com) IP adresi yazılır. 
cmd += "\",80"; //Port numarası seçilir. 
ser.println(cmd); //Sunucuya yollanacak değer, ESP8266 modülüne yollanır. 
Serial.println(cmd); //Bağlantı linki seriport ekrana yazdırılır. 
if(ser.find("Hata")) //Eğer sunucu bağlantısında bir Hata tespi edilirse...
{
Serial.println("AT+CIPSTART hata"); //...seriport ekrana uyarı mesajı yazdırılır.
return; //Değer geri döndürülerek bağlantı işlemi tekrarlanır. 
}
String getStr = "GET /update?api_key="; //thinspeak bağlantısı için kullanılan API anahtarı atanır. 
getStr += apiKey;
getStr +="&field1="; //Birinci alan sıcaklık değeri için atanır. 
getStr +=String(sicaklik);
getStr +="&field2=";
getStr +=String(pulse); //2.alan, KAH (kalp atış hızı) için atanır. 
getStr += "\r\n\r\n";
cmd = "AT+CIPSEND=4,";  // Veri uzunluğu yollanır. 
cmd += String(getStr.length());
ser.println(cmd); //Oluşturulan cmd değeri, ESP8266 modülününe yollanır. 
Serial.println(cmd); //Değer aynı zamanda seriport ekrana yazdırılır. 
delay(1000); //1 saniye beklenir. 
ser.print(getStr); //Okunan değerler(sıcaklık ve KAH), ESP8266 modülüne yollanır. 
Serial.println(getStr); //thingspeak değerleri güncellerken 15 saniyeye ihtiyacı vardır. 
delay(3000);
}

void read_sicaklik()//Bu fonksiyon içinde okunan sıcaklık ve kalp atış değeri LCD ekrana yazdırılır.
{
int sicaklikdeger = analogRead(A1); //A1 den okunan değer,sicaklikdeger isimli değişkene atanır.. 
float milivolt = (sicaklikdeger/1024.0)*5000; //Okunan değer, milivolt cinsinden dönüştürülerek mv'ye atanır. 
sicaklik = milivolt/10; //Milivolt değer, sıcaklık değerine dönüştürülür. 
Serial.print("Sıcaklık:");  
Serial.println(sicaklik); //Seriport ekranına okunan sıcaklık değeri yazdırılır.
lcd.clear(); //LCD ekran temizlenir.
lcd.setCursor(0,0); //LCD ilk satıra Kalp atis yazdırılır. 
lcd.print("Kalp atis:");
lcd.setCursor(11,0);
lcd.print(KAH); //Kalp atış değeri yazdırılır. 
lcd.setCursor(0,1);
lcd.print("Sicak.:"); //2.satıra yazdırılır. 
lcd.setCursor(8,1);
lcd.print(sicaklik);//LM35'ten okunan değer yazdırılır. 
lcd.setCursor(14,1);
lcd.print("C");
delay(2000); //LM35 her 2sn'de bir okuma yaptığı için beklenir. 
}
