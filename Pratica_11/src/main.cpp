#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#define LED 13

const char* host = "esp32";
const char* ssid = "NOME_DA_REDE";
const char* password = "SENHA_DA_REDE";

int contador_ms = 0;

WebServer server(80);

const char* loginIndex = 
  "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
      "<tr>"
        "<td colspan=2>"
          "<center><font size=4><b>ESP32 - identifique-se</b></font></center>"
          "<br>"
        "</td>"
        "<br><br>"
      "</tr>"
      "<tr>"
        "<td>Login:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
      "</tr>"
      "<br><br>"
      "<tr>"
        "<td>Senha:</td>"
        "<td><input type='Password' size=25 name='pwd'><br></td>"
        "<br><br>"
      "</tr>"
      "<tr>"
          "<td><input type='submit' onclick='check(this.form)' value='Identificar'></td>"
      "</tr>"
    "</table>"
  "</form>"
  "<script>"
    "function check(form) {"
      "if(form.userid.value=='admin' && form.pwd.value=='admin') {"
        "window.open('/serverIndex')"
      "}"
      "else {"
        "alert('Login ou senha inválidos')"
      "}"
    "}"
"</script>";

const char* serverIndex = 
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>Progresso: 0%</div>"
  "<script>"
    "$('form').submit(function(e) {"
      "e.preventDefault();"
      "var form = $('#upload_form')[0];"
      "var data = new FormData(form);"
      " $.ajax({"
        "url: '/update',"
        "type: 'POST',"
        "data: data,"
        "contentType: false,"
        "processData:false,"
        "xhr: function() {"
          "var xhr = new window.XMLHttpRequest();"
          "xhr.upload.addEventListener('progress', function(evt) {"
            "if (evt.lengthComputable) {"
              "var per = evt.loaded / evt.total;"
              "$('#prg').html('Progresso: ' + Math.round(per*100) + '%');"
            "}"
          "}, false);"
          "return xhr;"
        "},"
        "success:function(d, s) {"
          "console.log('Sucesso!')" 
        "},"
        "error: function (a, b, c) {}"
      "});"
    "});"
  "</script>";

void setup(void) {
  Serial.begin(9600);

  pinMode(LED, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Conectado a rede wi-fi ");
  Serial.println(ssid);
  Serial.print("IP obtido: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin(host)) {
    Serial.println("Erro ao configurar mDNS. O ESP32 vai reiniciar em 1s...");
    delay(1000);
    ESP.restart();        
  }

  Serial.println("mDNS configurado e inicializado;");

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
        Update.printError(Serial);
      }
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    }
    else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Sucesso no update de firmware: %u\nReiniciando ESP32...\n", upload.totalSize);
      }
      else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

void loop() {
  server.handleClient();
  delay(1);
  contador_ms++;
  if (contador_ms >= 1000) {   
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    delay(1000);
    contador_ms = 0;
  }
}
