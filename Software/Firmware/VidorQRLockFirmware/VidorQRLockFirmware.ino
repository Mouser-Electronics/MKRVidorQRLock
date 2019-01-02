///////////////////////////////////////////////////////////////////////
//File:     VidorQRLockFirmware.ino
//Author:   MB Parks
//
//Description: Proof-of-concept for MKR Vidor 4000-based QR Code lock
//mechanism leveraging a OV5647 camera.
//
//Open Source License:
///////////////////////////////////////////////////////////////////////

#include <VidorGraphics.h>
#include <VidorCamera.h>

VidorCamera vcam;

const int MAXDIM = 10;

////////////////////////////////////////////
const int lockPin = 11;
const int ledPin = LED_BUILTIN;
static int counter = 0;
static unsigned long startMillis = 0;
unsigned long currentMillis = 0;
const unsigned long period = 8000;
static bool lockEngaged = true;
/////////////////////////////////////////////

static uint16_t x[QR_PT_DET_NUM], y[QR_PT_DET_NUM];

struct qrPtn {
  uint16_t x[QR_PT_DET_NUM];
  uint16_t y[QR_PT_DET_NUM];
};

static qrPtn qrBufferPtn[MAXDIM];

uint16_t count = 0, last;

void setup() {

  pinMode(lockPin, OUTPUT);
  digitalWrite(lockPin, LOW);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  Serial.begin(9600);

  // wait for the serial monitor to open,
  // if you are powering the board from a USB charger remove the next line
  while (!Serial);
  
  Serial.println(F("Welcome to the ROBIN v1.0...system initializing..."));

  if (!FPGA.begin()) {
    Serial.println(F("WARNING: FPGA Initialization failed!"));
    while (1) {}
  }
  /**
    begin() enable the I2C communication and initialize the display for the camera
  */
  if (!vcam.begin()) {
    Serial.println(F("WARNING: Camera begin failed!"));
    while (1) {}
  }
  /**
      qrrec.begin(); enable the QR code recognition
  */
  vcam.qrrec.begin();
  delay(4000);
  Serial.println(F("System initialized. Door locked. Awaiting QR code..."));
}

void loop()  {
  /**
     qrrec.readQRCode(); get, if available, the coordinates of the QR code in the screen
  */
  vcam.qrrec.readQRCode();
  for (int i = 0; i < QR_PT_DET_NUM; i++) {
    currentMillis = millis();
    if (vcam.qrrec.qr.pt[i].valid) {
      x[i] = (vcam.qrrec.qr.pt[i].xs + vcam.qrrec.qr.pt[i].xe) / 2;
      y[i] = (vcam.qrrec.qr.pt[i].ys + vcam.qrrec.qr.pt[i].ye) / 2;
      vcam.vgfx.Cross(x[i], y[i], 65535);

      if (currentMillis - startMillis >= period) {
        qrDetectHandler();
      }
    }

    if (lockEngaged == false) {
      resetLock();
    }
  }

  last = count % MAXDIM;
  for (int i = 0; i < QR_PT_DET_NUM; i++) {
    vcam.vgfx.Cross(qrBufferPtn[last].x[i], qrBufferPtn[last].y[i], 0, 0);
    qrBufferPtn[last].x[i] = x[i];
    qrBufferPtn[last].y[i] = y[i];
  }
  count++;
}


/////////////////////////////////////////////////////////////////////////////////////////////
void qrDetectHandler() {
  lockEngaged = false;
  digitalWrite(ledPin, LOW);
  digitalWrite(lockPin, HIGH);
  Serial.print(F("ALERT: QR detected. Lock opening..."));
  Serial.println(counter++);
  if (counter > 10000) {
    counter = 0;
  }
  startMillis = currentMillis;
}

void resetLock() {
  if (currentMillis - startMillis >= period) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(lockPin, LOW);
    startMillis = currentMillis;
    lockEngaged = true;
    Serial.println(F("Lock closed."));
  }
}
