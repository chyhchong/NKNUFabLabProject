/*
 * @note:	NANO + DS18B20 + DS1302 RTC + microSD Card
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

const String sVersion = "v20170326-01";			// 本程式之版本序號，我們必須每次改版後更新此版本序號

int iDataRecord = 1;    //設定寫出至serial視窗之資料列序號

/*
 * I2C LCD 顯示螢幕之必要常數設定值
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
//LiquidCrystal_I2C lcd(0x3F, 16, 2);	// 設定LCD螢幕型號+長+高，另一種寫法

/*
 * DS1302 RTC 時鐘模組設定
 * @ note:
 *		1. DS1302 
 *			RST --> D5
 *			DAT --> D6
 *			CLK --> D7
 *			VCC --> +5V
 *			GND --> GND
 */
#include <DS1302.h>						//引入函式庫，DS1302.h 函數載點  http://www.rinkydinkelectronics.com/library.php?id=5

//-------宣告腳位-------//
const int kCePin   = 5;  				//RST  DS1302 RST 接D5腳位
const int kIoPin   = 6;  				//DAT  DS1302 DAT 接D6腳位
const int kSclkPin = 7;  				//CLK  DS1302 CLK 接D7腳位

//-------宣告物件-------//
DS1302 rtc(kCePin, kIoPin, kSclkPin);	//建立物件，名稱設為 rtc

// Init a Time-data structure
Time nowTime;									//建立時間資料結構物件 ，名稱設為 nowTime


/*
 * DS18B20 溫度計之必要常數設定值
 * @note:
 *			DS18B20 的VCC與Signal腳位間必須接一4.7K歐姆電阻
 *			多個溫度計感測並聯即可
 *			只需一個腳位就可多溫度計感測
 *			VCC(紅線) --> +5V
 *			GND(黑線，灰線) --> GND
 *			DATA(黃線) --> D2
 *
 */
// 引入函式庫
#include <OneWire.h>	//from http://playground.arduino.cc/Learning/OneWire
#include <DallasTemperature.h>	//from https://github.com/milesburton/Arduino-Temperature-Control-Library

#define ONE_WIRE_BUS	2 // Arduino D2 接到1-Wire裝置-DS18B20 溫度計
#define NUMBER_OF_DS18B20	10	// 並聯的 DS18B20 溫度計數目

// 運用程式庫建立DS18B20 溫度計物件
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

/*
 * SD, microSD 設置
 *
 * note@
 *		1. SD card attached to SPI bus as follows:
 *		MOSI - pin 11
 *  	MISO - pin 12
 *  	CLK - pin 13
 *  	CS - pin 10 ，chipSelect 腳位，也有接 D4 腳位，
 *		2. SD.begin(chipSelect) 啟動
 */
// 引入函式庫
#include <SPI.h>				//內建，for SPI Bus
#include <SD.h>					//內建，for SD Card

//-------宣告腳位-------//
const int chipSelect = 10;						//SD card module CS 接D10腳位

//-------宣告檔名變數-------//
String DataFileName;						//儲存資料資檔名，由 RTC 抓時間當檔名，xxhyymzz.csv，xx為時，yy為分，zz為秒

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
 * 抓DS18B20感測器資料
 */
void getDS18B20Data() {	// one-wire可並聯多感測器，iNumberOfSensors設定並聯的DS18B20溫度計數目

	DS18B20.requestTemperatures();		//讀取one-wire通道的所有DS18B20溫度計感測器
	
	for (int i = 0; i < DS18B20_DATA_NUMBER; i++) {
		fDS18B20Data[i] = DS18B20.getTempCByIndex(i);	//讀取DS18B20溫度計感測器資料，依序寫入fDS18B20Data[]陣列中

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
	 * 設定RTC狀態，不停止計時，寫入保護
	 */
	rtc.halt(false);				//不停止計時，要設定時間寫入時，設為 true
	rtc.writeProtect(true);			//寫入保護，要設定時間寫入時，設為 false

	/*
	 * 連結初始 SD Card
	 */
	Serial.print("Initializing SD card...");	//列印連結初始訊息

	// SD.begin(chipSelect) 檢查sd卡是否存在，且可被連結初始
	if (!SD.begin(chipSelect)) {				
		Serial.println("Card failed, or not present");	//無法使用sd card 訊息
		// don't do anything more:
		return;
		}
	Serial.println("card initialized.");				//可以使用sd card 訊息

	/*
	 * 以RTC的時間設定sd card寫入資料檔檔名為hhmmss.csv
	*/
	nowTime = rtc.getTime();
	DataFileName = String(nowTime.hour) + "h" + String(nowTime.min) + "m" + String(nowTime.sec) + ".csv";

	/*
	 * 啟動溫度感測器
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
	 * 寫入sd card中
	 */
	// 開啟檔案，設為可寫入
	// 一次只能開啟一個檔案，其他檔案要先 dataFile.close(); 關閉
	File dataFile = SD.open(DataFileName, FILE_WRITE);		//開啟DataFileName(=hhmmss.csv)為可寫入

	// 開啟成功，寫入資料
	if (dataFile) {
		dataFile.print(iDataRecord);
		dataFile.print(",");
		dataFile.println(sData);
		dataFile.close();
		}
	else {													// 開啟失敗，顯示訊息
		Serial.print("error opening ");
		Serial.println(DataFileName);
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

