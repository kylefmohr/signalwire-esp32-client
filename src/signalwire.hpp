#ifndef __SignalWire__
#define __SignalWire__

// From https://github.com/TwilioDevEd/twilio_esp8266_arduino_example

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "base64.h"
#include "url_coding.hpp"
#include "digicert.h"

class SignalWire {
public:
        SignalWire(
                const char* account_sid_in, 
                const char* auth_token_in,
                const char* ca_crt_in = DigiCertGlobalRootCA_crt
        )
                : account_sid(account_sid_in)
                , auth_token(auth_token_in)
                , ca_crt(ca_crt_in)
        {}
        // Empty destructor
        ~SignalWire() = default; 

        bool send_message(
                const String& to_number,
                const String& from_number,
                const String& message_body,
                const String& space_url,
                String& response,
                const String& picture_url = ""
        );

private:
        // Account SID and Auth Token come from the SignalWire console.
        // See: https://twilio.com/console for more.

        // Used for the username of the auth header
        String account_sid;
        // Used for the password of the auth header
        String auth_token;
        // Root CA cert, so that Twilio's cert chain can be verified
        const char* ca_crt;

        // Utilities
        static String _get_auth_header(
                const String& user, 
                const String& password
        );
        
};
#endif
