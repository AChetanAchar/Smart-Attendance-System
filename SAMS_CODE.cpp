#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <FS.h>
#include <SPIFFS.h>
#include <time.h>
#include <map>

const char* ssid = "VAC";
const char* password = "SonuSammuSmitha";

LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

#define TOTAL_STUDENTS 40
int totalPresent = 0;
const char* attendanceFile = "/AtriaAttendance.csv";

struct AttendanceRecord {
  time_t loginTime = 0;
  time_t logoutTime = 0;
};

std::map<String, AttendanceRecord> attendanceRecords;

String getNameByUSN(String usn) {
  if (usn == "1AT24CD001") return "Raghu";
  if (usn == "1AT24CD002") return "Chetan Achar";
  if (usn == "1AT24CD003") return "Yathin";
  if (usn == "1AT24CD004") return "Chinmay";
  if (usn == "1AT24CD005") return "Manya";
  if (usn == "1AT24CD006") return "Mufeez";
  if (usn == "1AT24CD007") return "Akshay";
  if (usn == "1AT24CD008") return "Niranjan";
  if (usn == "1AT24CD009") return "Sneha";
  if (usn == "1AT24CD010") return "Shankar";
  if (usn == "1AT24CD011") return "Subhashinee";
  if (usn == "1AT24CD012") return "Manish";
  if (usn == "1AT24CD013") return "Hema";
  if (usn == "1AT24CD014") return "Chinmai";
  if (usn == "1AT24CD015") return "Praveen";
  if (usn == "1AT24CD016") return "Keerthana";
  if (usn == "1AT24CD017") return "Likith";
  if (usn == "1AT24CD018") return "Mithun";
  if (usn == "1AT24CD019") return "Jashwanth";
  if (usn == "1AT24CD020") return "Ganesh";
  if (usn == "1AT24CD021") return "Nethraj";
  if (usn == "1AT24CD022") return "Hemanth";
  if (usn == "1AT24CD023") return "Yash";
  if (usn == "1AT24CD024") return "Nithin";
  if (usn == "1AT24CD025") return "Pooja";
  if (usn == "1AT24CD026") return "Siddharth";
  if (usn == "1AT24CD027") return "Lakshmi";
  if (usn == "1AT24CD028") return "Rahul";
  if (usn == "1AT24CD029") return "Swati";
  if (usn == "1AT24CD030") return "Mohan";
  if (usn == "1AT24CD031") return "Tanya";
  if (usn == "1AT24CD032") return "Deepak";
  if (usn == "1AT24CD033") return "Isha";
  if (usn == "1AT24CD034") return "Vikram";
  if (usn == "1AT24CD035") return "Nandini";
  if (usn == "1AT24CD036") return "Abhishek";
  if (usn == "1AT24CD037") return "Shruti";
  if (usn == "1AT24CD038") return "Vijay";
  if (usn == "1AT24CD039") return "Gayathri";
  if (usn == "1AT24CD040") return "Sanjay";
  return "Unknown";
}

time_t getMidnight(time_t now) {
  struct tm *tm_now = localtime(&now);
  tm_now->tm_hour = 0;
  tm_now->tm_min = 0;
  tm_now->tm_sec = 0;
  return mktime(tm_now);
}

void handleAttendance() {
  if (!server.hasArg("usn")) {
    lcd.clear();
    lcd.print("Missing USN");
    server.send(400, "text/plain", "Missing USN");
    return;
  }

  String usn = server.arg("usn");
  String name = getNameByUSN(usn);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(usn.length() > 16 ? usn.substring(0, 16) : usn);
  lcd.setCursor(0, 1);
  lcd.print(name.length() > 16 ? name.substring(0, 16) : name);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    server.send(500, "text/plain", "Time error");
    return;
  }
  time_t now = mktime(&timeinfo);
  time_t midnight = getMidnight(now);

  AttendanceRecord &record = attendanceRecords[usn];

  File file = SPIFFS.open(attendanceFile, FILE_APPEND);
  if (!file) {
    server.send(500, "text/plain", "File error");
    return;
  }

  if (file.size() == 0) {
    file.println("USN,Name,Type,Timestamp");
  }

  char timeStr[25];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

  if (record.loginTime < midnight) {
    // No login today → mark login
    record.loginTime = now;
    record.logoutTime = 0;  // reset logout for new day
    file.printf("%s,%s,Login,%s\n", usn.c_str(), name.c_str(), timeStr);
    server.send(200, "text/plain", "Login recorded for: " + name);
    Serial.printf("Login marked: %s - %s at %s\n", usn.c_str(), name.c_str(), timeStr);
  } else if (record.logoutTime < record.loginTime) {
    // Logged in but no logout → mark logout
    record.logoutTime = now;
    file.printf("%s,%s,Logout,%s\n", usn.c_str(), name.c_str(), timeStr);
    server.send(200, "text/plain", "Logout recorded for: " + name);
    Serial.printf("Logout marked: %s - %s at %s\n", usn.c_str(), name.c_str(), timeStr);
  } else {
    server.send(200, "text/plain", "Already logged out today: " + name);
  }

  file.close();
}

void handleDownload() {
  File file = SPIFFS.open(attendanceFile, FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=AtriaAttendance.csv");
  server.sendHeader("Connection", "close");
  server.streamFile(file, "text/csv");
  file.close();
}

void handleClearCSV() {
  File file = SPIFFS.open(attendanceFile, FILE_WRITE);
  if (!file) {
    lcd.clear();
    lcd.print("Clear Failed");
    Serial.println("Failed to open file for clearing");
    server.send(500, "text/plain", "Failed to clear file");
    return;
  }
  file.close();

  totalPresent = 0;
  attendanceRecords.clear();

  lcd.clear();
  lcd.print("CSV cleared");

  Serial.println("Attendance CSV file cleared.");
  server.send(200, "text/plain", "Attendance CSV file cleared");
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());
  delay(2000);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    lcd.clear();
    lcd.print("FS Mount Fail");
    while (true) delay(1000);
  }

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");

  server.on("/attendance", handleAttendance);
  server.on("/download", handleDownload);
  server.on("/clear", handleClearCSV);
  server.begin();

  Serial.println("Server started at IP: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.print("Ready to Scan");
}

void loop() {
  server.handleClient();
}