#include "signalwire.hpp"

/*
   Send a SMS or MMS with the SignalWire REST API

   Inputs:
    - to_number : Number to send the message to
    - from_number : Number to send the message from
    - message_body : Text to send in the message (max 1600 characters)
    - space_url : URL of your SignalWire space
    - picture_url : (Optional) URL to an image

   Outputs:
    - response : Connection messages and Twilio responses returned to caller
    - bool (method) : Whether the message send was successful
*/
bool SignalWire::send_message(
  const String& to_number,
  const String& from_number,
  const String& message_body,
  const String& space_url,
  String& response,
  const String& picture_url)
{
  // Check the body is less than 1600 characters in length.  see:
  // https://support.twilio.com/hc/en-us/articles/223181508-Does-Twilio-support-concatenated-SMS-messages-or-messages-over-160-characters-
  // Note this is only checking ASCII length, not UCS-2 encoding; your
  // application may need to enhance this.
  if (message_body.length() > 1600) {
    response += "Message body must be 1600 or fewer characters.";
    response += "  You are attempting to send ";
    response += message_body.length();
    response += ".\r\n";
    return false;
  }

  // URL encode our message body & picture URL to escape special chars
  // such as '&' and '='
  String encoded_body = urlencode(message_body);

  // Use WiFiClientSecure class to create TLS 1.2 connection
  WiFiClientSecure client;
  client.setCACert(ca_crt);
  String host = space_url;
  const int   httpsPort = 443;

  // Use WiFiClientSecure class to create TLS connection
  Serial.print("connecting to ");
  Serial.println(host);

  //Serial.printf("Using cert '%s'\n", ca_crt);

  // Connect to SignalWire's REST API
  response += ("Connecting to host ");
  response += host;
  response += "\r\n";
  if (!client.connect(host, httpsPort)) {
    response += ("Connection failed!\r\n");
    return false;
  }

  // Attempt to send an SMS or MMS, depending on picture URL
  String post_data = "To=" + urlencode(to_number) + "&From=" + urlencode(from_number) + \
                     "&Body=" + encoded_body;
  if (picture_url.length() > 0) {
    String encoded_image = urlencode(picture_url);
    post_data += "&MediaUrl=" + encoded_image;
  }

  // Construct headers and post body manually
  String auth_header = _get_auth_header(account_sid, auth_token);
  String http_request = "POST /2010-04-01/Accounts/" +
                        String(account_sid) + "/Messages HTTP/1.1\r\n" +
                        auth_header + "\r\n" + "Host: " + host + "\r\n" +
                        "Cache-control: no-cache\r\n" +
                        "User-Agent: ESP8266 SignalWire Example\r\n" +
                        "Content-Type: " +
                        "application/x-www-form-urlencoded\r\n" +
                        "Content-Length: " + post_data.length() + "\r\n" +
                        "Connection: close\r\n" +
                        "\r\n" + post_data + "\r\n";

  response += ("Sending http POST: \r\n" + http_request);
  client.println(http_request);

  // Read the response into the 'response' string
  response += ("request sent");
  bool prev_was_empty = false;
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    // Two empty lines in a row and we'll assume it's done - otherwise this never terminates (?)
    if (line.length() == 0) {
      if (prev_was_empty) {
        break;
      }
      prev_was_empty = true;
    }
    response += (line);
    response += ("\r\n");
  }
  client.stop();
  response += ("closing connection");
  return true;
}

/* Private function to create a Basic Auth field and parameter */
String SignalWire::_get_auth_header(const String& user, const String& password) {
  size_t toencodeLen = user.length() + password.length() + 2;
  char toencode[toencodeLen];
  memset(toencode, 0, toencodeLen);
  snprintf(
    toencode,
    toencodeLen,
    "%s:%s",
    user.c_str(),
    password.c_str()
  );

  String encoded = base64::encode((uint8_t*)toencode, toencodeLen - 1);
  String encoded_string = String(encoded);
  int i = 0;

  // Strip newlines (after every 72 characters in spec)
  while (i < encoded_string.length()) {
    i = encoded_string.indexOf('\n', i);
    if (i == -1) {
      break;
    }
    encoded_string.remove(i, 1);
  }
  return "Authorization: Basic " + encoded_string;
}
