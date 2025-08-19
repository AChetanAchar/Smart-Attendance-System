# Smart Attendance Monitoring System (IoT Project)

## 📌 Overview
This project is an **IoT-based Smart Attendance Monitoring System** developed using **ESP32**, **WiFi**, and **LCD Display**.  
It automates the attendance process by recording **login and logout times** of students and storing them in a **CSV file** on ESP32 (SPIFFS).  
Data can be downloaded via a web interface for easy monitoring.

## ⚡ Features
- ESP32 Web Server with REST APIs  
- Real-time clock synchronization using NTP  
- Login/Logout detection with timestamps  
- Data stored in CSV (`AtriaAttendance.csv`)  
- LCD feedback for student USN and name  
- Option to download or clear attendance records  

## 🛠️ Components Used
- ESP32 Microcontroller  
- 16x2 LCD (I2C)  
- WiFi Network  
- (Optional) RFID/QR Scanner  

## 🚀 How It Works
1. ESP32 connects to WiFi and hosts a web server.  
2. User scans/enters USN → ESP32 marks **Login** or **Logout**.  
3. Records are saved in `AtriaAttendance.csv` on ESP32 (SPIFFS).  
4. Endpoints available:  
   - `/attendance?usn=ID` → Mark attendance  
   - `/download` → Download CSV file  
   - `/clear` → Reset CSV file  

## 📄 Example CSV Output