#include <pgmspace.h>

const char HTTPHEAD[] PROGMEM ="<!DOCTYPE HTML><html><head><meta content= \"text/html; charset=ISO-8859-1 \"http-equiv=\"content-type\"><meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\"><title>Gateway</title><style>"; 

const char HTTPBODYSTYLE[] PROGMEM = "body { background-color: #0067B3 ; font-family: Arial, Helvetica, Sans-Serif; Color: #FFFFFF; }input[type=text], select {width: 100%;padding: 12px 20px;margin: 8px 0;display: inline-block;border: 1px solid #ccc;border-radius: 4px;box-sizing: border-box;}</style></head>";

const char HTTPBODY[] PROGMEM = "<body><center>";

const char HTTPCONTENTSTYLE[] PROGMEM = "<h3 style=\"color:#FFFFFF; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique\">GATEWAY</h3><label style=\"color:#FFFFFF;font-family: Verdana;font-size: 27px;padding-top: 5px;padding-bottom: 10px;\">Configure Device Settings</label><br><br>";

const char HTTPDEVICE[] PROGMEM = "<label style=\"color:#FFFFF;font-family: Verdana;font-size: 27px;padding-top: 5px;padding-bottom: 10px;\">{s}</label><br><br>";

const char HTTPFORM[] PROGMEM = "<FORM action=\"/\" method=\"post\">";

const char HTTP_CONTENT1_START[] PROGMEM= "<div style=\"padding-left:100px;text-align:left;display:inline-block;min-width:150px;\"><a href=\"#pass\" onclick=\"c(this)\" style=\"text-align:left\">{v}</a></div>&nbsp&nbsp&nbsp <div style=\"display:inline-block;min-width:260px;\"><span class=\"q\" style=\"text-align:right\">{r}%</span></div><br>";

const char HTTP_CONTENT2_START[] PROGMEM= "<P ><label style=\"font-family:Times New Roman\">SSID</label><br><input maxlength=\"30px\" id=\"ssid\" type=\"text\" name=\"ssid\" placeholder='Enter WiFi SSID' style=\"width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498db; border-radius: 4px; box-sizing:border-box\" ><br></P>";

const char HTTPLABLE1[] PROGMEM = "<P><label style=\"font-family:Times New Roman\">SSID</label><br><input maxlength=\"30px\" type='text' id=\"ssid\" name=\"ssid\" placeholder='SSID' style=\"width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #cdab1e; border-radius: 4px; box-sizing:border-box\" required;><br></P>";

const char HTTPLABLE2[] PROGMEM = "<P><label style=\"font-family:Times New Roman\">Passkey</label><br><input maxlength=\"30px\" type = \"text\" name=\"passkey\"  placeholder = \"Passkey\" style=\"width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #cdab1e; border-radius: 4px; box-sizing:border-box\" required;><br></P>";

const char HTTPLABLE3[] PROGMEM =  "<P><label style=\"font-family:Times New Roman\">Sensor Nodes</label><br>";

const char HTTPNODESEL[] PROGMEM = "<select name=\"sensor_select\" id=\"sensor_select_id\" style=\"border=2px; width: 400px; margin: 8px 0; padding: 5px 10px; display: inline-block;  border : 2px solid #cdab1e; border-radius: 4px; box-sizing:border-box\"><option value=\"1\" id=\"1\">1</option><option value=\"2\" id=\"2\">2</option><option value=\"3\" id=\"3\">3</option><option value=\"4\" id=\"4\">4</option><option value=\"5\" id=\"5\">5</option><option value=\"6\" id=\"6\">6</option><option value=\"7\" id=\"7\">7</option><option value=\"8\" id=\"8\">8</option><option value=\"9\" id=\"9\">9</option><option value=\"10\" id=\"10\">10<br>";

const char HTTPSUBMIT[] PROGMEM = "<INPUT type=\"submit\"> <style>input[type=\"submit\"]{background-color: #3498DB; border: none;color: white;padding:15px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}</style>";

const char HTTPCLOSEFORM[] PROGMEM = "</FORM>";

const char HTTPSCRIPT[] PROGMEM= "<script>function c(l){document.getElementById('ssid').value=l.innerText||l.textContent;document.getElementById('pass').focus();}</script>";

const char HTTPCLOSE[] PROGMEM = "</center></body>";

