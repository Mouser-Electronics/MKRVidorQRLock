#include <VidorGraphics.h>
#include <VidorCamera.h>

VidorCamera vcam;

#define MAXDIM 10

static uint16_t x[QR_PT_DET_NUM], y[QR_PT_DET_NUM];
static int counter = 0;

struct qrPtn {
  uint16_t x[QR_PT_DET_NUM];
  uint16_t y[QR_PT_DET_NUM];
};

static qrPtn qrBufferPtn[MAXDIM];

uint16_t count = 0, last;

void setup() {
  Serial.begin(9600);

  // wait for the serial monitor to open,
  // if you are powering the board from a USB charger remove the next line
  //while (!Serial);

  if (!FPGA.begin()) {
    Serial.println("Initialization failed!");
    while (1) {}
  }
  /**
    begin() enable the I2C communication and initialize the display for the camera
  */
  if (!vcam.begin()) {
    Serial.println("Camera begin failed");
    while (1) {}
  }
  /**
      qrrec.begin(); enable the QR code recognition
  */
  vcam.qrrec.begin();
  delay(4000);
  Serial.println("Power ON");
}

void loop()  {
  /**
     qrrec.readQRCode(); get, if available, the coordinates of the QR code in the screen
  */
  vcam.qrrec.readQRCode();
  for (int i = 0; i < QR_PT_DET_NUM; i++) {
    if (vcam.qrrec.qr.pt[i].valid) {
      x[i] = (vcam.qrrec.qr.pt[i].xs + vcam.qrrec.qr.pt[i].xe) / 2;
      y[i] = (vcam.qrrec.qr.pt[i].ys + vcam.qrrec.qr.pt[i].ye) / 2;
      vcam.vgfx.Cross(x[i], y[i], 65535);
      Serial.print("QR Code detected...");
      Serial.println(counter++);
      if (counter > 100000) {
        counter = 0;
      }
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
