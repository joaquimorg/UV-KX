#pragma once

#define batt_50_width 13
#define batt_50_height 5
static unsigned char batt_50_bits[] = {
   0xff, 0x0f, 0x01, 0x18, 0x01, 0x18, 0x01, 0x18, 0xff, 0x0f };

#define charging_width 5
#define charging_height 5
static unsigned char charging_bits[] = {
   0x0a, 0x1f, 0x1f, 0x0e, 0x04 };

#define mmeter_width 54
#define mmeter_height 8
static unsigned char mmeter_bits[] = {
   0x80, 0x81, 0x83, 0x83, 0x83, 0x83, 0x39, 0x11, 0x01, 0x82, 0x00, 0x82,
   0x02, 0x29, 0x1b, 0x81, 0x83, 0x03, 0x82, 0x03, 0x29, 0x15, 0x01, 0x02,
   0x02, 0x02, 0x02, 0x29, 0x91, 0x93, 0x93, 0x13, 0x12, 0x92, 0x3b, 0x11,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x91, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0x3b };

#define smeter_width 74
#define smeter_height 10
static unsigned char smeter_bits[] = {
   0x80, 0x81, 0x83, 0x83, 0x83, 0x03, 0xb8, 0x03, 0xa8, 0x03, 0x1e, 0x01,
   0x82, 0x00, 0x82, 0x02, 0xa0, 0x02, 0xa8, 0x02, 0x01, 0x81, 0x83, 0x03,
   0x82, 0x03, 0xb8, 0x02, 0xb8, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
   0x88, 0x02, 0xa0, 0x02, 0x81, 0x93, 0x93, 0x13, 0x12, 0x12, 0xb9, 0x13,
   0xa1, 0x03, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0xbb,
   0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0x03, 0x10, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00 };
  
