/*
 * @note:	FabLab NKNU T3103 T3103 DHT11 + DHT22 + DS18B20 + WiFi
 * 			with all comments for most important codes
 * @author:	ccchuang
 * @since:	20170209
 * @version:
 * 		v20170209:
 * 			嘗試以工作坊鄭伯壎老師指導的程式寫作方法 coding
 *			import WiFi IoT codes
 * @operation:
 * 		Due to lack of power supply from NB thru USB cable,
 * 		please follow up the following procedures to renew your codes on Arduino board
 * 			1. Board: unplug DC power cable
 *			2. Board: switch off ESP8266 button
 *			3. Board: plug USB cable
 *			4. Board: upload codes from NB
 *			5. LCD:	  confirm the version is the same as your codes
 *			6. Board: unplug USB cable
 *			7. Board: switch on ESP8266 button
 *			8. Board: plug DC power cable
 *			9. LCD: check whether data processing okay?
 *			10. Web: check ThingSpeack channel for data uploading entries
 * @reference:
 * 		1. FabLab NKNU T3103 IoT 控制板由高師大自造基地提供
 * 		2. FabLab NKNU T3103 IoT 與 DHT11 sensor 範例由高師大自造基地工作坊提供
 *		
 */

const String sVersion = "v20170302-01";			// 本程式之版本序號，我們必須每次改版後更新此版本序號
int iDataRecord = 1;    							//設定顯示至LCD之資料列序號

/*
 * 宣告I2C LCD顯示螢幕之必要常數設定值
 * @note:
 * 		此處之LCD顯示螢幕為2行，16字元/行
 *		LCD 1602的2004轉接板，常見兩種不同的晶片，如果是PCF8574T，位址是0X27，如果是PCF8574AT，位址是0X3F。
 *		make sure you set the right adress. here are the most common ones.
 *		PCF8574 = 0x20, PCF8574A = 0x38, PCF8574AT = 0x3F.
 *		可先run i2c_scanner.ino 找出I2C位址
 */
#include <Wire.h>	// Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>	// 引入I2C LCD 函示庫 NewliquidCrystal_1.3.4.zip，from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads

// Set the LCD I2C address
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);	// 設定LCD螢幕型號+長+高 (0x27)
//LiquidCrystal_I2C lcd(0x27, 16, 2);	// 很奇怪，這種寫法螢幕無字元顯示

/*
 * FabLab NKNU 範例提供的函示庫，待測
 */
//#include "LiquidCrystal_I2C_AvrI2C.h"	// LCD螢幕函式庫

//LiquidCrystal_I2C_AvrI2C lcd(0x3f, 15, 2); 		// 設定LCD螢幕型號+長+高 (0x3F)
/*
 * FabLab NKNU 範例提供的函示庫，待測 註解結束
 */

/*
 * DHT溫溼度感測器之必要常數設定值
 */
#include "DHT.h"	// 引入 DHT11/DHT22 函示庫，from https://github.com/adafruit/DHT-sensor-library
#define DHT11APIN	2	// 第一個DHT11訊號腳接D2
#define DHT11BPIN	3	// 第二個DHT11訊號腳接D3
//#define DHT11CPIN	4	// 第二個DHT11訊號腳接D3
#define DHTTYPE11 DHT11	// 感測器為 DHT11

#define DHT22APIN	4	// DHT22訊號腳接D3
//#define DHT22BPIN	2	// 第二個DHT22訊號腳接D3
//#define DHT22CPIN	4	// 第二個DHT22訊號腳接D3
#define DHTTYPE22 DHT22	// 感測器為 DHT22

// 運用程式庫建立DHTxx 物件
DHT dht11A(DHT11APIN, DHTTYPE11);	// 設定第一個DHT11感測器名稱為dht11A
DHT dht11B(DHT11BPIN, DHTTYPE11);	// 設定第二個DHT11感測器名稱dht11B
DHT dht22A(DHT22APIN, DHTTYPE22);	// 設定第一個DHT22感測器名稱dht22A

/*
 * DS18B20 溫度計之必要常數設定值
 * @note:
 * 		DS18B20 的VCC與Signal腳位間必須接一4.7K歐姆電阻
 *		多個溫度計感測並聯即可
 *		只需一個腳位就可多溫度計感測
 */
#include <OneWire.h>	//from http://playground.arduino.cc/Learning/OneWire
#include <DallasTemperature.h>	//from https://github.com/milesburton/Arduino-Temperature-Control-Library

#define ONE_WIRE_BUS	5 // Arduino D3接到1-Wire裝置-DS18B20 溫度計
#define NUMBER_OF_DS18B20	2	// 並聯的 DS18B20 溫度計數目

// 運用程式庫建立DS18B20 溫度計物件
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

/*
 * 宣告暫存DHT感測器資料的一維陣列
 */
#define DHT_DATA_NUMBER 	6			// 所有DHTxx感測器數目x2 
float fDHTData[DHT_DATA_NUMBER];// 暫存DHTxx感測器資料用

/*
 * 宣告暫存DS18B20感測器資料的一維陣列
 */
#define DS18B20_DATA_NUMBER 	2			// 所有DS18B20感測器數目 
float fDS18B20Data[DS18B20_DATA_NUMBER];// 暫存DS18B20感測器資料用

/*
 * 宣告暫存所有感測器資料的一維陣列
 */
#define DATA_NUMBER 8		// 每次抓取感測器數值的數量，6 = DHT_DATA_NUMBER + DS18B20_DATA_NUMBER 
float fData[DATA_NUMBER];// 暫存所有感測器資料用

/*
 * 抓DHTxx感測器資料
 */
void getDHTData() {

	fDHTData[0] = dht11A.readTemperature();	// 溫度資料
	fDHTData[1] = dht11A.readHumidity();		// 濕度資料
	fDHTData[2] = dht11B.readTemperature();	// 溫度資料
	fDHTData[3] = dht11B.readHumidity();		// 濕度資料
	fDHTData[4] = dht22A.readTemperature();	// 溫度資料
	fDHTData[5] = dht22A.readHumidity();		// 濕度資料

	// 檢查是否有測到資料
//	for (int i = 0; i < DHT_DATA_NUMBER; i++) {
//	if (isnan(fDHTData[i])) {	// 任一資料未測到
//		Serial.println("Failed to read from DHT sensor!");	//顯示錯誤訊息
//    return;
//      }
//		}
	}

/*
 * 抓DS18B20感測器資料
 */
void getDS18B20Data() {	// one-wire可並聯多感測器，iNumberOfSensors設定並聯的DS18B20溫度計數目

	DS18B20.requestTemperatures();		//讀取one-wire通道的所有DS18B20溫度計感測器
	
	for (int i = 0; i < DS18B20_DATA_NUMBER; i++) {
		fDS18B20Data[i] = DS18B20.getTempCByIndex(i);	//讀取DS18B20溫度計感測器資料，依序寫入fDt[]陣列中

		// 檢查是否有測到資料
//		if (isnan(fDS18B20Data[i])) {	// 任一資料未測到
//			Serial.println("Failed to read from DS18B20 sensor!");	//顯示錯誤訊息
//      return;
//			}
		}
	}

/*
 * 宣告ESP8266之Wi-Fi之必要常數設定值
 */
const String WIFI_SSID = "E1F01-2.4G";					// Wi-Fi名稱
const String WIFI_PWD = "59655965"; 				// Wi-Fi密碼

/*
 * 宣告ThingSpeak之必要常數設定值
 */
const String THING_SPEAK_IP = "184.106.153.149"; 	// ThingSpeak網址IP
const String THING_SPEAK_WRITE_KEY = "WNYIV1HXUCSAKDD2"; 			// ThingSpeak辨識碼: Write API Key (channel name: NKNU_FabLab)

/*
 * 將設定訊息字串送出至ESP8266，並確認Arduino是否有回饋
 * 如果有回應，則在LCD螢幕顯示"OK"，並回傳真
 * 如果無回應，則在LCD螢幕顯示"Loading..."，並回傳偽
 */
boolean ChatWiFi(String sStr) {
	boolean bRetFlag;				// 設定回傳旗標

	Serial.println(sStr);			// 串列埠: 輸出字串sStr

	lcd.clear();  					// LCD: 清除螢幕
	lcd.setCursor(0, 0);  			// LCD: 游標移到左上角
	lcd.print(sStr);  				// LCD: 顯示Arduino傳給ESP8266的指令(字串A)
	lcd.setCursor(0, 1);  			// LCD: 游標移到左下角

	if (Serial.find("OK")) {		// 串列埠: 若串列埠攔截到OK字串
		lcd.print("OK");			// LCD: 輸出字串
		bRetFlag = true;			// 設定回傳旗標為真
	} else {						// 串列埠: 若串列埠沒有攔截到OK字串
		lcd.print("Loading...");	// LCD: 輸出字串
		bRetFlag = false;			// 設定回傳旗標為偽
	}
	delay(1000);					// 等候1秒

	return(bRetFlag);				// 回傳旗標值，真表示成功，否則為不成功
}

void setup() {
	/*
	 * 設定串列埠，並確認是否通暢
	 */
	Serial.begin(9600);				// 串列埠: 設定串列通訊鮑率(Baud Rate: 每秒傳送幾位元)
	while (!Serial); 				// 串列埠: 等候直到連接上串列通訊埠，只能被使用再USB通訊埠連線上

	/*
	 * 設定LCD，並輸出開始畫面+版本資訊
	 */
	lcd.begin(16,2); 				// LCD: 設定LCD尺寸
	lcd.backlight(); 				// LCD: 開啟背光
 	lcd.cursor(); 					// LCD: 顯示游標
 	lcd.blink(); 					// LCD: 游標閃爍
	lcd.setCursor(0, 0);  			// LCD: 游標移到左上角
	lcd.print("NKNU Maker Proj.");	// LCD: 輸出NKNU字串
	lcd.setCursor(0, 1);  			// LCD: 游標移到左下角
	lcd.print(sVersion);			// LCD: 輸出版本序號字串
	delay(3000);					// 等候3秒

	/*
	 * 確認ESP8266 Wi-Fi通訊是否正常
	 */
	while (!ChatWiFi("AT"));		// 將設定指令字串送至ESP8266，並確認Arduino是否收到回饋

	/*
	 * 若網路正常，則LCD顯示歡迎畫面，準備開始工作
	 */
	lcd.clear();  					// LCD: 清除螢幕
	lcd.setCursor(0, 0);  			// LCD: 游標移到左上角
	lcd.print("FabLab-NKNU-IOT");  	// LCD: 輸出字串
	lcd.setCursor(0, 1);  			// LCD: 游標移到左下角
	lcd.print("Ready...");			// LCD: 輸出字串
	delay(3000);					// 等候3秒

	/*
	 * 啟動感測器
	 */
	dht11A.begin(); // 初始化第一個DHT11感測器
	dht11B.begin(); // 初始化第一個DHT11感測器
	dht22A.begin(); // 初始化第一個DHT22感測器
	DS18B20.begin(); // 初始化DS18B20溫度計感測器

	/*
	 * LCD顯示準備送出資料訊息
	 */
	lcd.clear();  					// LCD: 清除螢幕
	lcd.setCursor(0, 0);  			// LCD: 游標移到第一列始(左到右顯示)
	lcd.print("8 Datas/Row");		// LCD: 輸出指令字串內容
	lcd.setCursor(0, 1);  			// LCD: 游標移到第二列始(左到右顯示)
	lcd.print("READY, GO...");		// LCD: 輸出指令字串內容
	delay(1000);					// 等候1秒
	
	}

void loop() {
	/*
	 * 抓DHTxx 感測器資料
	 */
	getDHTData();

	/*
	 * 抓DS18B20感測器資料
	 */
	getDS18B20Data();

	/*
	 * 把所有感測器資料放入fData[]中
	 */
	for (int i = 0; i < DHT_DATA_NUMBER; i++) {	
		fData[i] = fDHTData[i];
		}
	
	for (int i = 0; i < DS18B20_DATA_NUMBER; i++) {	
		fData[(i + DHT_DATA_NUMBER)] = fDS18B20Data[i];
		}
	
 
	/*
	 * 上傳ThingSpeak的變數
	 */
	String	sCmd = "";				// 儲存指令之用
	String	sData = "";				// 儲存資料之用
	int	iDataLen = 0;				// 儲存資料長度

	sCmd = "AT+RST";				// Wi-Fi: 重置ESP8266，Response: OK
	while (!ChatWiFi(sCmd));		// Wi-Fi: 將設定訊息字串sCmd送出至ESP8266，並確認Arduino是否有回饋

	/*
	 * 設定ESP8266模式
	 * AT+CWMODE=1，STA
	 * AT+CWMODE=2，AP
	 * AT+CWMODE=3，AP+STA 
	 * STA 模式 : Station 模式, 即無線網卡模式, 可連接到 AP, 不接受連入
	 * AP 模式 : Access Point 模式, 即無線基地台模式, 接受其他 WiFi 終端連入
	 */
	sCmd = "AT+CWMODE=1";			// Wi-Fi: 設定ESP8266為Station型態							
	Serial.println(sCmd);			// 串列埠: 輸出指令字串
	lcd.setCursor(0, 0);  			// LCD: 游標移到左上角

									// Wi-Fi: 準備指令字串，令ESP8266去串接需要的Wi-Fi
	sCmd = "AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_PWD + "\"";
	while (!ChatWiFi(sCmd));		// Wi-Fi: 將設定訊息字串sCmd送出至ESP8266，並確認Arduino是否有回饋

	/*
	 * 設定TCP/UDP Connections
	 * AT+CIPMUX=0，single
	 * AT+CIPMUX=1，multi
	 */
	sCmd = "AT+CIPMUX=0";			// Wi-Fi: 設定ESP8266為單路連接
	while (!ChatWiFi(sCmd));		// Wi-Fi: 將設定訊息字串sCmd送出至ESP8266，並確認Arduino是否有回饋

									// Wi-Fi: 準備指令字串，以連線至ThingSpeak
	sCmd = "AT+CIPSTART=\"TCP\",\"" + THING_SPEAK_IP + "\",80";
	while (!ChatWiFi(sCmd));		// Wi-Fi: 將設定指令字串送出至ESP8266，並確認Arduino是否有回饋

	/*
	 * 準備要上傳至ThingSpeak網站的資料字串內容
	 * Example:
	 * 		http://api.thingspeak.com/update?api_key=9X6QXEDGFO7WY9C8&field1=24.00&field2=58.00
	 * 		GET /update?api_key=9X6QXEDGFO7WY9C8&field1=24.00&field2=58.00
	 */
	sData = "GET /update?api_key=" + THING_SPEAK_WRITE_KEY;
		for (int i = 0; i < DATA_NUMBER; i++) {
		sData = sData + "&field" + String(i + 1) + "=";
		sData = sData +fData[i];
	}

	/*
	 * 首先，計算需要上傳ThinkSpeak資料字串的總長度
	 * 實際字串長度=(資料字串長度)+(2個字元長度)，必須在後面加上2個跳行字元 \r\n
	 * 		\r表示carriage return
	 *  	\n表示new line
	 */
	iDataLen = sData.length() + 2;

	/*
	 * 然後，上傳指令字串ThinkSpeak網站，並告知稍後要上傳資料字串的總長度，且顯示在LCD上
	 */
	sCmd = "AT+CIPSEND=" + String(iDataLen);	// 設定指令字串內容=(指令字串)+(實際字串長度)
	Serial.println(sCmd);  			// 告知ThinkSpeak網站稍後想要傳送字串的長度

	lcd.clear();  					// LCD: 清除螢幕
	lcd.setCursor(0, 0);  			// LCD: 游標移到左上角
	lcd.print(sCmd);				// LCD: 輸出指令字串內容
	delay(1000);					// 等候1秒

	/*
	 * 最後，上傳資料字串到ThinkSpeak網站
	 */
	Serial.println(sData);  		// 傳送資料字串sData到ThingSpeak
	
	/*
	 * 顯示資料字串在LCD上第二列
	 */
	lcd.clear();  					// LCD: 清除螢幕
	lcd.setCursor(0, 0);  			// LCD: 游標移到第一列始
	lcd.print(iDataRecord);					// 第一列顯示資料列數
	lcd.setCursor(0, 1);  			// LCD: 游標移到第二列始
	for (int i = 0; i < DATA_NUMBER; i++) {		//輸出資料，以,號隔開
		lcd.print(fData[i]);
		lcd.print(",");
		}

	delay(15000);					// 等候15秒，如此會每15秒測量上傳一次
	
	iDataRecord++;					//資料列序號累加
}

