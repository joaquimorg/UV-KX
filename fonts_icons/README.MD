## Convert TTF to bdf in FontForge and then to u8g2 font files using bdfconv_2_22.exe

## w=5, h=5 ????
.\bdfconv_2_22.exe -v -f 1 -m "32-125" '.\Pentapixel-16.bdf' -o ../src/ui/font_5_tr.h  -n u8g2_font_5_tr

## w=3, h=5 ????
.\bdfconv_2_22.exe -v -f 1 -m "32-125" '.\Teeny-Tiny-Pixls-5.bdf' -o ../src/ui/font_5_tr.h  -n u8g2_font_5_tr

## w=5, h=7
.\bdfconv_2_22.exe -v -f 1 -m "32-125" '.\somybmp01_7-7.bdf' -o ../src/ui/font_5_tr.h  -n u8g2_font_5_tr


## w=7, h=8
.\bdfconv_2_22.exe -v -f 1 -m "32-125" '.\Uni0553-8.bdf' -o ../src/ui/font_8_tr.h  -n u8g2_font_8_tr

## w=8, h=8
.\bdfconv_2_22.exe -v -f 1 -m "32-125" '.\Uni0563-8.bdf' -o ../src/ui/font_8b_tr.h  -n u8g2_font_8b_tr

## w=8, h=11
.\bdfconv_2_22.exe -v -f 1 -m "32-125" '.\04b25-12.bdf' -o ../src/ui/font_10_tr.h  -n u8g2_font_10_tr

## w=6, h=9
.\bdfconv_2_22.exe -v -f 1 -m "48-57,43,45,44,46,58" '.\04b25-12.bdf' -o ../src/ui/font_10_tr.h  -n u8g2_font_10_tr

## w=11, h=12
.\bdfconv_2_22.exe -v -f 1 -m "48-57,43,45,44,46,58" '.\UpheavalPro-Regular-20.bdf' -o ../src/ui/font_bn_tn.h  -n u8g2_font_bn_tn

