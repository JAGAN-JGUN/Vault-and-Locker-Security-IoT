#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <time.h>

#define WIFI_SSID "Your Wifi Name"
#define WIFI_PASSWORD "Your Wifi Pwd"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define IMAP_HOST "imap.gmail.com"
#define IMAP_PORT 993

#define AUTHOR_EMAIL "Your ESP email ID"
#define AUTHOR_PASSWORD "Your ESP email Pwd"

#define RECIPIENT_EMAIL "Your client Email ID"

#define NTP "pool.ntp.org"
#define GMTOFF 5.5

SMTPSession smtp;
IMAPSession imap;

Session_Config smtp_config, imap_config;
IMAP_Data imap_data;
int msg_uid;

void Delay(unsigned long interval);

void SendMail(String sub, String bod);

void smtpCallback(SMTP_Status status);
void imapCallback(IMAP_Status status);
void Actuation();

const int irSensorAPin = 13;
const int irSensorBPin = 14;
const int L1 = 15;
const int L2 = 4;
const int L3 = 5;

int count = 0;

void setup() {
  pinMode(irSensorAPin, INPUT);
  pinMode(irSensorBPin, INPUT);
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    Delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  configTime(GMTOFF * 3600, 0, NTP);

  MailClient.networkReconnect(true);


  smtp.debug(1);
  imap.debug(1);

  smtp.callback(smtpCallback);
  imap.callback(imapCallback);

  smtp_config.server.host_name = SMTP_HOST;
  smtp_config.server.port = SMTP_PORT;
  smtp_config.login.email = AUTHOR_EMAIL;
  smtp_config.login.password = AUTHOR_PASSWORD;
  smtp_config.login.user_domain = "";

  smtp_config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  smtp_config.time.gmt_offset = GMTOFF;
  smtp_config.time.day_light_offset = 0;

  imap_config.server.host_name = IMAP_HOST;
  imap_config.server.port = IMAP_PORT;
  imap_config.login.email = AUTHOR_EMAIL;
  imap_config.login.password = AUTHOR_PASSWORD;

  imap_data.fetch.uid.clear();
  imap_data.fetch.number.clear();
  imap_data.fetch.sequence_set.string.clear();

  imap_data.download.header = false;
  imap_data.download.text = false;
  imap_data.download.html = false;
  imap_data.download.attachment = false;
  imap_data.download.inlineImg = false;
  imap_data.enable.text = true;
  imap_data.enable.recent_sort = true;
  imap_data.enable.download_status = true;
  imap_data.limit.search = 1;
  imap_data.limit.msg_size = 512;

  if (!imap.connect(&imap_config, &imap_data))
  {
    MailClient.printf("Connection error, Error Code: %d, Reason: %s\n", imap.errorCode(), imap.errorReason().c_str());
    return;
  }

  if (imap.isAuthenticated())
    Serial.println("Successfully logged in.");
  else
    Serial.println("Connected with no Auth.");

  if (!imap.selectFolder(F("INBOX")))
  {
      MailClient.printf("Folder selection error, Error Code: %d, Reason: %s\n", imap.errorCode(), imap.errorReason().c_str());
      return;
  }
}

void loop() {
  time_t now = time(nullptr);
  int sensorAValue = digitalRead(irSensorAPin);
  int sensorBValue = digitalRead(irSensorBPin);
  String sub, bod;

  if(!imap.listen()) return;

  if(imap.folderChanged())
  {
    SelectedFolderInfo sFolder = imap.selectedFolder();
    
    imap.stopListen();

    msg_uid = imap.getUID(sFolder.pollingStatus().messageNum);

    MailClient.addFlag(&imap, msg_uid, F("\\Seen"), false, false);

    imap_data.fetch.uid = msg_uid;

    MailClient.readMail(&imap, false);

    return;
  }

  if (sensorAValue == LOW) {
    int i = 0;
    while(i < 10)
    {
      sensorBValue = digitalRead(irSensorBPin);
      if(sensorBValue == LOW)
      {
        count++;
        digitalWrite(L1,HIGH);
        digitalWrite(L2,HIGH);
        Serial.println("Entity Entered. Count: ");
        Serial.println(count);
        sub = "Obstacle Detected";
        bod = "Entry Detected by ESP32.\n\nTime: ";
        bod += String(ctime(&now));
        bod += "\n\nReply \'YES\' if you are aware. If not, reply \'NO\'.";
        SendMail(sub, bod);
        Delay(500);
        return;
      }
      Delay(1000);
      i++;
    }
  }
  else if(sensorBValue == LOW) {
    int i = 0;
    while(i < 10)
    {
      sensorAValue = digitalRead(irSensorAPin);
      if(sensorAValue == LOW)
      {
        count--;
        if(count <= 0)
        {
          count = 0;
          digitalWrite(L1,LOW);
          digitalWrite(L2,LOW);
        }
        Serial.println("Entity Exited. Count: ");
        Serial.println(count);
        Delay(2000);
        return;
      }
      Delay(1000);
      i++;
    }
  }
}

void SendMail(String sub, String bod)
{
  SMTP_Message message;

  message.sender.name = F("ESP32");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F(sub.c_str());
  message.addRecipient(F("Client"), RECIPIENT_EMAIL);

  char buffer[50];
  message.text.content = bod;
  message.text.charSet = F("utf-8");

  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  while(!smtp.connect(&smtp_config)){
    Serial.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    Delay(2000);
  }

  Serial.println("\nLogging in");
  while(!smtp.isLoggedIn()){
    Serial.println(".");
    Delay(1000);
  }
  if (smtp.isAuthenticated())
    Serial.println("\nSuccessfully logged in.");
  else
    Serial.println("\nConnected with no Auth.");

  if (!MailClient.sendMail(&smtp, &message))
    Serial.printf("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  
  smtp.closeSession();
}

void smtpCallback(SMTP_Status status){
  Serial.println(status.info());

  if (status.success()){

    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      SMTP_Result result = smtp.sendingResult.getItem(i);

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      Serial.printf("Recipient: %s\n", result.recipients.c_str());
      Serial.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    smtp.sendingResult.clear();
  }
}

void imapCallback(IMAP_Status status)
{
  String sub, bod;
  Serial.println(status.info());

  if (status.success())
  {
    if(imap.headerOnly())
    {
      msg_uid = imap.data().msgItems[0].UID;
    } 
    else
    {
      Actuation();
    }
  }
}

void Actuation()
{
  IMAP_MSG_Item msg = imap.data().msgItems[0];
  String sub, bod;
  String RE(msg.from);
  if(RE.indexOf(RECIPIENT_EMAIL) != -1)
    if (strlen(msg.text.content))
    {
      String str = msg.text.content;
      str.remove(str.indexOf("\n") - 1);
      Serial.printf("%s",str);
      if(str == "YES")
      {
        digitalWrite(L2,LOW);
      }
      if(str == "NO")
      {
        digitalWrite(L3,HIGH);
        sub = "Alarm Turned On";
        bod = "To Turn alarm off, Reply \'OFF\'.";
        SendMail(sub, bod);
      }
      if(str == "OFF")
      {
        digitalWrite(L2,LOW);
        digitalWrite(L3,LOW);
        Serial.println("Alarm Turned Off");
      }
    }
}

void Delay(unsigned long interval)
{
  unsigned long CT = millis();
  while(millis() - CT < interval);
}