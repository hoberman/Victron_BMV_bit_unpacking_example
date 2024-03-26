// This is an example of how to unpack the "Aux Input" and "Battery Current" fields
// from Victron BMV devices' extra manufacturer data BLE broadcasts. Note that this
// is not complete, as it does not include code to actually receive and unencrypt the
// Victron-provided BLE extra manufacturer data. You will need to integrate the sample
// coding below into a larger, more functional program.
// 
// This is related to the code in my Victron BLE decoding example:
//
// https://github.com/hoberman/Victron_BLE_Advertising_example/blob/main/Victron_BLE_Advertising_example.ino
//
//   Information on the "extra manufacturer data" that we're picking up from Victron SmartSolar
//   BLE advertising beacons can be found at:
//  
//     https://community.victronenergy.com/storage/attachments/48745-extra-manufacturer-data-2022-12-14.pdf
//  
// Thanks, Victron, for providing both the beacon and the documentation on its contents!
//
//
// Note that I don't have a BMV; this is only meant as an example for how to decode the
// value once you have the data bytes from the BMV. Actual integration into your own code
// is left as an exercise for the reader.
//
// Also: Since I don't have a BMV, I can't test this to see if it actually works with real data.
// If it turns out that I have this all wrong, please provide me some feeback via a Github issue
// report. Thanks!

// This is used to generate example data for decoding.
int byte0ExampleCounter = 0x00;

void setup() {
  delay(1000);  // I tested this on an Arduino Pro Micro. I need this delay() here
                // or subsequent attempts to upload new code might not work.
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.println("Reset.");
  Serial.println();
  Serial.print("Source file: ");
  Serial.println(__FILE__);
  Serial.print(" Build time: ");
  Serial.println(__TIMESTAMP__);
  Serial.println();
  Serial.println();
}

void loop() {
  // The "Aux Input" and "Battery Current" data from the BVM is held in three bytes
  // that come from the struct that holds extra manufacturer data (after decryption).
  // The 24 bits from the BVM - starting at bit 80 - are laid out as follows:
  //
  //    AAsBBBBB  CCCCCCCC  DDDDDDDD
  //
  // Where:
  //    AA - two bits for Aux Input data.
  //      sBBBBB - a sign bit plus the upper-most 5 bits of the Battery Current data
  //    CCCCCCCC - middle 8 bits of Battery Current data
  //    DDDDDDDD - lower 8 bits of Battery Current data
  //
  // In this example I have them defined here individually, but in actual usage these will
  // come from the struct.

  uint8_t byte0;
  uint8_t byte1;
  uint8_t byte2;

  // For this example I am simulating Battery Current data by filling in the byte0 value with
  // a counter.
  byte0 = byte0ExampleCounter;

  // Extract the two-bit "Aux Input" value so we could use that if we wanted. This is FYI only
  // for this example code.
  int auxInput = (byte0 & 0xc0) >> 6;

  // Strip the auxInput bits out of our byte0 data:
  byte0 = byte0 & 0x3f;

  // Construct a 32-bit value out of 00000000 00sBBBBBB CCCCCCCC DDDDDDDD. I need to explicitly cast the byte
  // values to long so the compiler doesn't complain about the shifts:
  long batteryCurrentCount = ((long)byte0 << 16) | ((long)byte1 << 8) | ((long)byte2);

  // But this is supposed to be a signed value, so we're going to synthesize an 'extension' of
  // the uppermost (sign) bit that came from our 0x3f-masked byte0 data. We'll see if that bit is set
  // and then use it to set (or not set) the forced-zero bits of the 32-bit long signed value.
  if (batteryCurrentCount & 0x00200000) {
    // The 'sign' bit of the six-bit-wide byte0 value is set, so extend it to the top 10 bits
    // to make the 32-bit batteryCurrentCount into a proper signed value.
    batteryCurrentCount = batteryCurrentCount | 0xffc00000;
  }

  // Now turn this into a float and multiply by 0.001 - per the Victron doc - to get the actual current in amps.
  float batteryCurrentAmps = float(batteryCurrentCount) * 0.001;

  // Note that because our byte1 and byte2 are always zero in this example code, the
  // resultant Battery Current values displayed will range from (roughly) +/-2048. If we
  // filled in those lower two bytes with actual data then the full range would be (roughly)
  // +/-4096 as described in the Victron doc.

  Serial.print("byte0=");
  Serial.print(byte0, HEX);
  Serial.print("    BatteryCurrentAmps=");
  Serial.println(batteryCurrentAmps);

  // Now increment our example counter so we can see the decoded value change. When the counter increments from
  // 0x1f t0 0x20, that sets the "S" bit and so the value will go negative.
  byte0ExampleCounter = (byte0ExampleCounter + 1) & 0x3f;

  delay(1000);
}
