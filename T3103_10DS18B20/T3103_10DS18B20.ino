/*
 * @note:	FabLab NKNU T3103 DHT11 + DHT22 + DS18B20
 * 			with all comments for most important codes
 * @author:	ccchuang
 * @since:	20170209
 * @version:
 * 		v20170209:
 * 			嘗試以工作坊鄭伯壎老師指導的程式設計方法 coding
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

const String sVersion = "v20170324-01";			// 本程式之版本序號，我們必須每次改版後更新此版本序號
int iDataRecord = 1;    //設定寫出至serial視窗之資料列序號
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
//LiquidCrystal_I2C lcd(0x3F, 16, 2);	// 設定LCD螢幕型號+長+高

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
/*
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
*/
/*
 * DS18B20 溫度計之必要常數設定值
 * @note:
 * 		DS18B20 的VCC與Signal腳位間必須接一4.7K歐姆電阻
 *		多個溫度計感測並聯即可
 *		只需一個腳位就可多溫度計感測
 */
#include <OneWire.h>	//from http://playground.arduino.cc/Learning/OneWire
#include <DallasTemperature.h>	//from https://github.com/milesburton/Arduino-Temperature-Control-Library

#define ONE_WIRE_BUS	2 // Arduino D4接到1-Wire裝置-DS18B20 溫度計
#define NUMBER_OF_DS18B20	10	// 並聯的 DS18B20 溫度計數目

// 運用程式庫建立DS18B20 溫度計物件
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

/*
 * 宣告暫存DHT感測器資料的一維陣列
 */
/*
#define DHT_DATA_NUMBER 	6			// 所有DHTxx感測器數目x2 
float fDHTData[DHT_DATA_NUMBER];// 暫存DHTxx感測器資料用
*/
/*
 * 宣告暫存DS18B20感測器資料的一維陣列
 */
#define DS18B20_DATA_NUMBER 	10			// 所有DS18B20感測器數目 
float fDS18B20Data[DS18B20_DATA_NUMBER];// 暫存DS18B20感測器資料用

/*
 * 宣告暫存所有感測器資料的一維陣列
 */
#define DATA_NUMBER 10		// 每次抓取感測器數值的數量，4 = 1xDHT + 1xDHT22 + 2xDS18B20 
float fData[DATA_NUMBER];// 暫存所有感測器資料用

/*
 * 抓DHTxx感測器資料
void getDHTData() {

	fDHTData[0] = dht11A.readTemperature();	// 溫度資料
	fDHTData[1] = dht11A.readHumidity();		// 濕度資料
	fDHTData[2] = dht11B.readTemperature();	// 溫度資料
	fDHTData[3] = dht11B.readHumidity();		// 濕度資料
	fDHTData[4] = dht22A.readTemperature();	// 溫度資料
	fDHTData[5] = dht22A.readHumidity();		// 濕度資料

	// 檢查是否有測到資料
	for (int i = 0; i < DHT_DATA_NUMBER; i++) {
	if (isnan(fDHTData[i])) {	// 任一資料未測到
		Serial.println("Failed to read from DHT sensor!");	//顯示錯誤訊息
    return;
			}
		}
	}
 */

/*
 * 抓DS18B20感測器資料
 */
void getDS18B20Data() {	// one-wire可並聯多感測器，iNumberOfSensors設定並聯的DS18B20溫度計數目

	DS18B20.requestTemperatures();		//讀取one-wire通道的所有DS18B20溫度計感測器
	
	for (int i = 0; i < DS18B20_DATA_NUMBER; i++) {
		fDS18B20Data[i] = DS18B20.getTempCByIndex(i);	//讀取DS18B20溫度計感測器資料，依序寫入fDt[]陣列中

		// 檢查是否有測到資料
		if (isnan(fDS18B20Data[i])) {	// 任一資料未測到
			Serial.println("Failed to read from DS18B20 sensor!");	//顯示錯誤訊息
      return;
			}
		}
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
	 * 啟動感測器
	dht11A.begin(); // 初始化第一個DHT11感測器
  dht11B.begin(); // 初始化第一個DHT11感測器
	dht22A.begin(); // 初始化第一個DHT22感測器
   */
	DS18B20.begin(); // 初始化DS18B20溫度計感測器
	
	/*
	 * LCD顯示準備送出資料訊息
	 */
	lcd.clear();  					// LCD: 清除螢幕
	lcd.setCursor(0, 0);  			// LCD: 游標移到第一列始(左到右顯示)
	lcd.print("10 Datas/Row");		// LCD: 輸出指令字串內容
	lcd.setCursor(0, 1);  			// LCD: 游標移到第二列始(左到右顯示)
	lcd.print("READY, GO...");		// LCD: 輸出指令字串內容
	delay(1000);					// 等候1秒
	
	/*
	 * Serial視窗顯示準備送出資料訊息
	 */
	Serial.println("CLEAR");	// 輸出指令字串內容
	Serial.println("10 Datas/Row");	// 輸出指令字串內容
	Serial.println("READY, GO...");	// 輸出指令字串內容
	delay(1000);					// 等候1秒
	}

void loop() {
	/*
	 * 抓DHTxx 感測器資料
	 */
//	getDHTData();

	/*
	 * 抓DS18B20感測器資料
	 */
	getDS18B20Data();

	/*
	 * 把所有感測器資料放入fData[]中
	for (int i = 0; i < DHT_DATA_NUMBER; i++) {	
		fData[i] = fDHTData[i];
		}
   */
	
	for (int i = 0; i < DS18B20_DATA_NUMBER; i++) {	
		fData[(i)] = fDS18B20Data[i];
		}
	
	/*
	 * 準備要寫出資料的資料字串內容
	 */
	String	sData = "";				// 儲存資料之用

	for (int i = 0; i < DATA_NUMBER; i++) {
		sData = sData + fData[i] + ",";		//資料間以,隔開，如此將serial視窗資料直接複製貼上到文字檔，即可存成.cvs檔，供EXCEL處理
	}

	/*
	 * LCD顯示
	 */
	lcd.clear();  					// LCD: 清除螢幕
	lcd.setCursor(0, 0);  			// LCD: 游標移到第一列始(左到右顯示)
	lcd.print(iDataRecord);			// LCD: 輸出資料列數
	lcd.setCursor(0, 1);  			// LCD: 游標移到第一列始(左到右顯示)
	lcd.print(sData);				// LCD: 輸出資料字串sData

	/*
	 * 最後，將資料字串寫到Serial視窗，且顯示在LCD上
	 */
	//寫到Serial視窗
	Serial.print(iDataRecord);  // 傳送資料列序號到Serial視窗
	Serial.print(",");
	Serial.println(sData);  		// 傳送資料字串sData到Serial視窗
	
	delay(1000);					// 等候1秒，如此會每一秒測量一次
	
	iDataRecord++;					//資料列序號累加
	}

