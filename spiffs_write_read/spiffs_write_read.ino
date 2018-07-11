#include "FS.h"

void setup()
{
  Serial.begin(9600);

  // always use this to "mount" the filesystem
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);

  // this opens the file "f.txt" in read-mode
  File f = SPIFFS.open("/cl_conf.txt", "r");
  
  if (!f) {
    Serial.println("File doesn't exist yet. Creating it");

    // open the file in write mode
    File f = SPIFFS.open("/cl_conf.txt", "w");
    if (!f) {
      Serial.println("file creation failed");
    }
    // now write two lines in key/value style with  end-of-line characters
    f.println("msi_wifi");
    f.println("11111111");
  } else {
    // we could open the file
    while(f.available()) {
      //Lets read line by line from the file
      String line = f.readStringUntil('n');
      Serial.println(line);
    }

  }
  f.close();
}

void loop() {
  // nothing to do for now, this is just a simple test

}
