#include <Arduboy2.h>
Arduboy2 arduboy;
#include <EEPROM.h>
/*=========================================================
                         动态变量
  =========================================================*/
union uint
{
  int a;
  byte b[2];
};
bool player_move = false; //玩家是否移动
bool key_lock = false; //键盘锁
bool move_lock = false; //方向锁
bool dialog; //对话框
bool LA = false; //行动许可
bool DisplayInvert = false;
bool CN_text_BG = 0;
bool WOOPS = false; //世界崩坏开关
bool MoveTrue; //是否真移动
bool BEF; //是否完成眨眼动作
bool DrawKarmaB = true;  //是否绘制业力等级

int FPS, SFPS;
byte rbcr = 16;
byte Karma = 2; //业力值1-10  10:游戏结束
byte BF; //眨眼帧
byte ROOM; //当前房间号
byte DX, DY, CDX, CDY;
byte PMX, PMY; //玩家在地图中的位置 0:x 1:y
int CPDX, CPDY; //玩家朝向x和y轴
byte player_dyn = 0;     //玩家动态帧
byte PlayerD = 1;       //玩家方向
byte KeyBack = 255;     //按键返回
byte SBDPL[] = {2, 5, 9, 10, 11, 12, 13, 14, 20, 21, 22, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 39, 40, 43, 44, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 61}; //障碍物id
int Entity[2][3] = {    //实体坐标 以及ROOM
  {160, 32, 11},  //玩家出生点
  {128, 80, 20},  //回响
};
unsigned long Timer[5];  //时间列表 0 1 2 3FPS 4眨眼时间
//一次性业力列表 0-崩溃边缘 1-公园 2-小破庙 3-小溪边 4-床 5-酒 6-药 7-海岸 8-后花园 9-桃花源 EEPROM 127-158
/*=========================================================
                         常量
  =========================================================*/
#define FPST 1000
#define BlinkEyesTime 5000
#define dialog_cool_time 150
#define mobile_frame_time 150
#define key_cool_time 20
#define player_move_cool_time 20



/*=========================================================
                         位图
  =========================================================*/
const uint8_t OOPS[] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x07, 0xfb, 0xfa, 0xfa, 0xba, 0x7b, 0xbb, 0xfb, 0xfb,
                                0xfb, 0xfb, 0xbb, 0x7b, 0xbb, 0xfb, 0xfb, 0xfb, 0xfb, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xbe, 0xbf, 0xde, 0xdf, 0xdb,
                                0xdb, 0xdf, 0xde, 0xdf, 0xde, 0xbf, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfe, 0xfd, 0xfd, 0xfd, 0x9d, 0x8d, 0x9d, 0xfd, 0xfd,
                                0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0x7e, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfd, 0xf8, 0xf8, 0xff, 0xcf, 0x8f, 0xcf, 0xff, 0x78,
                                0x78, 0x7d, 0x7f, 0xff, 0xef, 0xc7, 0xef, 0xff, 0xfe, 0xfc, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00,
                               };

/*
   方块材质
   0  报错
   1  土地
   2  水-1
   3  水-2
   4  草
   5  石头
   6  路
   7  花
   8  麦田
   9  门
   10 砖块
   11 窗户
   12 柱
   13 瓦
   14 灯-1 墙灯
   15 灯-2 路灯
*/
const uint8_t Block_0[] PROGMEM = {0x55, 0x88, 0x55, 0x22, 0x55, 0x88, 0x55, 0x22, 0xaa, 0xff, 0xaa, 0xff, 0xaa, 0xff, 0xaa, 0xff,
                                   0xaa, 0xff, 0xaa, 0xff, 0xaa, 0xff, 0xaa, 0xff, 0x55, 0x88, 0x55, 0x22, 0x55, 0x88, 0x55, 0x22,
                                  };
const uint8_t Block_2[] PROGMEM = {0xe3, 0x26, 0x24, 0x74, 0x9c, 0x84, 0x8c, 0x93, 0x50, 0x70, 0x5c, 0x87, 0x0c, 0x08, 0x88, 0xfc,
                                   0xb5, 0x4a, 0x44, 0x44, 0x44, 0x44, 0x46, 0xa3, 0x1c, 0x34, 0x44, 0xc2, 0xc5, 0xc9, 0xb8, 0x84,
                                  };
const uint8_t Block_3[] PROGMEM = {0xc4, 0x43, 0x66, 0xd4, 0x1c, 0x04, 0x0c, 0x14, 0x53, 0x70, 0x5c, 0x87, 0x0c, 0x08, 0x88, 0xfc,
                                   0x3b, 0x84, 0x48, 0x48, 0x49, 0x49, 0x4d, 0xa7, 0x14, 0x0c, 0x74, 0xc4, 0xc7, 0xc5, 0xc8, 0xb8,
                                  };
const uint8_t Block_4[] PROGMEM = {0xff, 0xfe, 0xff, 0xef, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xbf, 0xff, 0xff,
                                   0xff, 0x7f, 0x7f, 0xfb, 0xf7, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xb7, 0xf7, 0xff, 0xfb, 0xff, 0xff,
                                  };
const uint8_t Block_5[] PROGMEM = {0xdf, 0x4e, 0x9f, 0x6f, 0x6f, 0x6f, 0xaf, 0xcf, 0x6f, 0xef, 0xdf, 0xbb, 0x7f, 0xff, 0xbf, 0xff,
                                   0xff, 0x60, 0x1e, 0xbf, 0x3b, 0x9c, 0xb1, 0xaf, 0xbf, 0xbb, 0xb3, 0xbd, 0xde, 0xa9, 0x97, 0xff,
                                  };
const uint8_t Block_6[] PROGMEM = {0x77, 0xb3, 0xcd, 0xee, 0x77, 0x3b, 0xdd, 0xec, 0x73, 0xba, 0xdd, 0xce, 0x37, 0x3b, 0xdc, 0xee,
                                   0x75, 0xbb, 0xdd, 0xce, 0x37, 0xbb, 0xdc, 0xee, 0x77, 0xb3, 0xcd, 0xee, 0x77, 0x3b, 0xdc, 0xee,
                                  };
const uint8_t Block_7[] PROGMEM = {0xff, 0xff, 0xff, 0x5f, 0x3f, 0x5f, 0xab, 0x89, 0x13, 0x89, 0xab, 0x7f, 0xbf, 0xbf, 0x7f, 0xbf,
                                   0xff, 0xff, 0xff, 0xff, 0xfe, 0xfd, 0xea, 0xd0, 0x81, 0xd0, 0xea, 0xfb, 0xfb, 0xfc, 0xff, 0xff,
                                  };
const uint8_t Block_8[] PROGMEM = {0xff, 0x12, 0x25, 0x12, 0xff, 0x12, 0x25, 0x12, 0xff, 0x12, 0x25, 0x12, 0xff, 0x12, 0x25, 0x12,
                                   0xff, 0xe1, 0x02, 0xe1, 0xff, 0xe1, 0x02, 0xe1, 0xff, 0xe1, 0x02, 0xe1, 0xff, 0xe1, 0x02, 0xe1,
                                  };
const uint8_t Block_9[] PROGMEM = {0xff, 0x01, 0xfe, 0xfe, 0xe6, 0xca, 0xc2, 0xca, 0xc2, 0xca, 0xc2, 0xe6, 0x7e, 0xfe, 0x01, 0xff,
                                   0xff, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7e, 0x7f, 0x80, 0xff,
                                  };
const uint8_t Block_10[] PROGMEM = {0x83, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0x38, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
                                    0x83, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0x38, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
                                   };
const uint8_t Block_11[] PROGMEM = {0xff, 0x07, 0xfb, 0xfd, 0xfd, 0xfd, 0xfd, 0x01, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfb, 0x07, 0xff,
                                    0xff, 0xe0, 0xde, 0xbe, 0xbe, 0xbe, 0xbe, 0x80, 0xbe, 0xbe, 0xbe, 0xbe, 0xbe, 0xde, 0xe0, 0xff,
                                   };
const uint8_t Block_12[] PROGMEM = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   };
const uint8_t Block_13[] PROGMEM = {0xfe, 0x7d, 0xbd, 0xdb, 0xc7, 0xbf, 0x7f, 0xff, 0xfe, 0x7d, 0xbd, 0xdb, 0xc7, 0xbf, 0x7f, 0xff,
                                    0xfe, 0x7d, 0xbd, 0xdb, 0xc7, 0xbf, 0x7f, 0xfe, 0xfe, 0x7d, 0xbd, 0xdb, 0xc7, 0xbf, 0x7f, 0xfe,
                                   };
const uint8_t Block_14[] PROGMEM = {0x38, 0xfb, 0xbb, 0x5b, 0xdb, 0x4b, 0x5b, 0x5b, 0xbb, 0xfb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
                                    0x38, 0xbb, 0xbb, 0xbc, 0xb9, 0xa2, 0xa9, 0xbc, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
                                   };
const uint8_t Block_15[] PROGMEM = {0x3f, 0xdf, 0xef, 0xef, 0xdf, 0x33, 0x2d, 0x1e, 0x1e, 0x2d, 0x33, 0xdf, 0xef, 0xef, 0xdf, 0x3f,
                                    0xff, 0xfe, 0xfd, 0xfd, 0xfe, 0xff, 0x0f, 0xe0, 0xe0, 0x0f, 0xff, 0xfe, 0xfd, 0xfd, 0xfe, 0xff,
                                   };
const uint8_t Block_17[] PROGMEM = {0x07, 0xf3, 0x1b, 0x5b, 0xf3, 0x9b, 0x73, 0x9b, 0xf3, 0xdb, 0x13, 0xfb, 0xd3, 0x1b, 0xd3, 0x07,
                                    0xf0, 0xe7, 0xec, 0xe5, 0xef, 0xe4, 0xef, 0xe4, 0xef, 0xe5, 0xec, 0xe5, 0xef, 0xec, 0xe7, 0xf0,
                                   };
const uint8_t Block_18[] PROGMEM = {0xff, 0xff, 0xff, 0x1f, 0x5f, 0x0f, 0x6f, 0x07, 0xd7, 0xc3, 0xdb, 0x01, 0xf5, 0x70, 0x76, 0x00,
                                    0x01, 0x7d, 0x7d, 0x00, 0x2f, 0x07, 0x37, 0x00, 0x6b, 0x61, 0x6d, 0x60, 0x7a, 0x78, 0x7b, 0x00,
                                   };
const uint8_t Block_19[] PROGMEM = {0x00, 0x00, 0x00, 0x8f, 0xcf, 0xcf, 0x4f, 0x6f, 0x2f, 0x77, 0x17, 0x3b, 0xcb, 0xdd, 0xdd, 0x01,
                                    0x02, 0xf7, 0xf1, 0xf1, 0x00, 0xfd, 0xfc, 0x00, 0xff, 0xff, 0xff, 0x00, 0xdf, 0x0f, 0xef, 0x00,
                                   };
const uint8_t Block_20[] PROGMEM = {0x59, 0xc6, 0xd6, 0x14, 0x31, 0xc3, 0xeb, 0xe9, 0x2c, 0xad, 0x4c, 0x65, 0x63, 0x0b, 0xd8, 0x1b,
                                    0xb0, 0x0e, 0x66, 0x71, 0x22, 0x8c, 0x3e, 0x3c, 0x4c, 0x31, 0xeb, 0x07, 0xb3, 0xb8, 0x8d, 0x30,
                                   };
const uint8_t Block_21[] PROGMEM = {0xff, 0x1f, 0xef, 0xd7, 0xf7, 0xf7, 0xb7, 0xd7, 0xb7, 0xb7, 0xf7, 0xf7, 0xd7, 0xef, 0x1f, 0xff,
                                    0xff, 0x00, 0x7f, 0xde, 0xec, 0xda, 0xff, 0xec, 0xe9, 0xfa, 0xdf, 0xea, 0xfd, 0x7f, 0x00, 0xff,
                                   };
const uint8_t Block_22[] PROGMEM = {0xff, 0x00, 0xff, 0xfb, 0x1d, 0x0d, 0xc7, 0xe7, 0xe7, 0xf7, 0xed, 0x1d, 0xfb, 0xff, 0x00, 0xff,
                                    0xff, 0x00, 0xff, 0x95, 0xb6, 0xec, 0xf8, 0xf9, 0xf9, 0xf9, 0xec, 0xb6, 0x95, 0xff, 0x00, 0xff,
                                   };
const uint8_t Block_23[] PROGMEM = {0xfa, 0xfa, 0xfa, 0xfa, 0xfa, 0x3a, 0xda, 0xea, 0xe2, 0xea, 0xda, 0x3a, 0xfa, 0xfa, 0xfa, 0xfa,
                                    0xff, 0xff, 0xff, 0xff, 0xf9, 0xf4, 0xf6, 0xf4, 0xf4, 0xf6, 0xf6, 0xf4, 0xf9, 0xff, 0xff, 0xff,
                                   };
const uint8_t Block_24[] PROGMEM = {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
                                    0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0x01, 0xfd, 0xfd, 0x01, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd,
                                   };
const uint8_t Block_25[] PROGMEM = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfe, 0xfe, 0x00, 0xfa, 0xfa, 0xfa, 0xfa, 0xfa, 0xfa,
                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   };
const uint8_t Block_26[] PROGMEM = {0xfa, 0xfa, 0xfa, 0xfa, 0xfa, 0xfa, 0x00, 0xfe, 0xfe, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   };
const uint8_t Block_27[] PROGMEM = {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
                                    0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd,
                                   };
const uint8_t Block_29[] PROGMEM = {0x0f, 0x70, 0x7e, 0x70, 0x77, 0x00, 0x7e, 0x70, 0x77, 0x70, 0x7e, 0x00, 0x77, 0x70, 0x7e, 0x70,
                                    0x00, 0x77, 0x77, 0x77, 0x70, 0x77, 0x07, 0x77, 0x77, 0x77, 0x70, 0x77, 0x77, 0x07, 0x77, 0x77,
                                   };
const uint8_t Block_30[] PROGMEM = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xee, 0xee, 0xee, 0xee, 0xe1, 0xff, 0xff, 0xff, 0xff,
                                    0x07, 0xeb, 0xed, 0xee, 0xe0, 0xee, 0x0e, 0xee, 0xee, 0xee, 0xe0, 0xee, 0x0e, 0xed, 0xeb, 0x07,
                                   };
const uint8_t Block_31[] PROGMEM = {0x07, 0xfb, 0xfd, 0x6e, 0xfe, 0x6e, 0xfe, 0x00, 0xfe, 0x6e, 0xfe, 0x6e, 0xfd, 0xfb, 0x07, 0xff,
                                    0x00, 0x3f, 0x7f, 0x5b, 0x7f, 0x5b, 0x7f, 0x00, 0x7f, 0x5b, 0x7f, 0x5b, 0x7f, 0x3f, 0x00, 0xff,
                                   };
const uint8_t Block_32[] PROGMEM = {0x00, 0xaa, 0xdd, 0xdd, 0xaa, 0x77, 0x55, 0x77, 0xee, 0xaa, 0xee, 0x55, 0xbb, 0xbb, 0x55, 0x00,
                                    0x00, 0xaa, 0xdd, 0xdd, 0xaa, 0x77, 0x55, 0x77, 0xee, 0xaa, 0xee, 0x55, 0xbb, 0xbb, 0x55, 0x00,
                                   };
const uint8_t Block_33[] PROGMEM = {0xeb, 0xab, 0xab, 0x6b, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0x6b, 0xab, 0xab, 0xeb,
                                    0xff, 0xbf, 0xb3, 0xb7, 0xbe, 0xbd, 0xdb, 0xe7, 0xe7, 0xdb, 0xbd, 0xbe, 0xb7, 0xb3, 0xbf, 0xff,
                                   };
const uint8_t Block_34[] PROGMEM = {0x00, 0xfe, 0xfe, 0xe6, 0xd2, 0xca, 0xca, 0xca, 0xc2, 0xc2, 0xc2, 0xe2, 0xe6, 0xfe, 0xfe, 0x00,
                                    0x00, 0x7f, 0x47, 0x3b, 0x25, 0x01, 0x4d, 0x1d, 0x15, 0x35, 0x4d, 0x1d, 0x2b, 0x47, 0x7f, 0x00,
                                   };
const uint8_t Block_35[] PROGMEM = {0x00, 0xfe, 0xfe, 0xe6, 0xd2, 0xca, 0xca, 0xca, 0xc2, 0xc2, 0xc2, 0xe2, 0xe6, 0xfe, 0xfe, 0x00,
                                    0x00, 0x7f, 0x47, 0x13, 0x2d, 0x05, 0x05, 0x59, 0x0d, 0x21, 0x25, 0x0d, 0x1b, 0x47, 0x7f, 0x00,
                                   };
const uint8_t Block_36[] PROGMEM = {0x7f, 0x9b, 0xe5, 0xc9, 0xd5, 0xad, 0xb9, 0xbd, 0xbd, 0xbd, 0xbd, 0xdd, 0xdd, 0xe5, 0x8b, 0x7f,
                                    0xf8, 0xc5, 0x0b, 0x17, 0x2f, 0x1f, 0x3f, 0x1f, 0x3f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3f, 0xc7, 0xf8,
                                   };
const uint8_t Block_37[] PROGMEM = {0xfe, 0x01, 0x01, 0x0f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x15, 0x11, 0x0f, 0x01, 0x02, 0x04, 0xf8,
                                    0x7f, 0x80, 0xfe, 0x81, 0x81, 0x95, 0x95, 0x95, 0x95, 0x95, 0x95, 0x81, 0x81, 0xfe, 0x80, 0x7f,
                                   };
const uint8_t Block_38[] PROGMEM = {0xff, 0x7f, 0x3f, 0x5f, 0x6f, 0x77, 0x7b, 0x75, 0x75, 0x7b, 0x77, 0x6f, 0x5f, 0x3f, 0x7f, 0xff,
                                    0x80, 0x7f, 0x5b, 0x5b, 0x5b, 0x5b, 0x5b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7f, 0x7f, 0x7f, 0x7f, 0x80,
                                   };
const uint8_t Block_39[] PROGMEM = {0x01, 0xfd, 0x01, 0xb9, 0xb9, 0xb9, 0xb9, 0xb9, 0xb9, 0xb9, 0xb9, 0x01, 0xfd, 0x01, 0xf5, 0xf3,
                                    0x00, 0xff, 0x02, 0xfa, 0xfa, 0xb6, 0xd6, 0xf6, 0xf6, 0xfa, 0xf2, 0x06, 0x0f, 0x00, 0xff, 0xff,
                                   };
const uint8_t Block_40[] PROGMEM = {0x00, 0xe1, 0x1e, 0xff, 0xef, 0xd7, 0xef, 0xff, 0xff, 0xef, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff,
                                    0xf0, 0xeb, 0xe2, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf4, 0xeb, 0xe7,
                                   };
const uint8_t Block_41[] PROGMEM = {0xff, 0x7f, 0x3f, 0x5f, 0x6f, 0x77, 0x7b, 0x75, 0x75, 0x7b, 0x77, 0x6f, 0x5f, 0x3f, 0x7f, 0xff,
                                    0x80, 0x7b, 0x47, 0x17, 0x43, 0x78, 0x12, 0x6d, 0x6d, 0x12, 0x74, 0x4f, 0x27, 0x53, 0x6f, 0x80,
                                   };
const uint8_t Block_42[] PROGMEM = {0x03, 0xfd, 0x35, 0x35, 0xb5, 0xb5, 0xb5, 0x35, 0xfd, 0xb5, 0xbd, 0x75, 0xfd, 0x03, 0xef, 0x1f,
                                    0x80, 0x7f, 0x40, 0x50, 0x5c, 0x44, 0x5c, 0x40, 0x7f, 0x6b, 0x6b, 0x4f, 0x5b, 0x5a, 0x7f, 0x80,
                                   };
const uint8_t Block_43[] PROGMEM = {0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0x8b, 0x6b, 0x2b, 0x9b, 0x7b,
                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0x5f, 0x1f, 0xdf, 0xdf, 0xdf, 0xdf, 0xd9, 0xd6, 0xd5,
                                   };
const uint8_t Block_44[] PROGMEM = {0x9b, 0x2b, 0x4b, 0xbb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff,
                                    0xd6, 0xd9, 0xdf, 0xdf, 0xdf, 0xdf, 0xdf, 0x1f, 0x5f, 0x1f, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   };
const uint8_t Block_45[] PROGMEM = {0xff, 0xff, 0xff, 0xff, 0x43, 0xbd, 0x5a, 0x66, 0x66, 0x5a, 0xbd, 0x83, 0x7f, 0xff, 0xff, 0xff,
                                    0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff,
                                   };
const uint8_t Block_46[] PROGMEM = {0xff, 0xff, 0xff, 0x03, 0xf3, 0xeb, 0xdb, 0xbb, 0x7b, 0x6b, 0x9b, 0xeb, 0xfb, 0x7b, 0xbb, 0xdb,
                                    0xff, 0xff, 0xff, 0x00, 0xff, 0xfa, 0xfd, 0xfd, 0xfe, 0xff, 0xf3, 0xfb, 0xef, 0x1f, 0xff, 0xff,
                                   };
const uint8_t Block_47[] PROGMEM = {0xff, 0xff, 0xff, 0x03, 0xf3, 0xeb, 0xdb, 0xbb, 0x7b, 0x6b, 0x9b, 0xeb, 0xfb, 0x7b, 0xbb, 0xdb,
                                    0xff, 0xff, 0xff, 0x00, 0xff, 0xfa, 0xfd, 0xfd, 0xfe, 0xff, 0xf3, 0xfb, 0xef, 0x1f, 0xff, 0xff,
                                   };
const uint8_t Block_48[] PROGMEM = {0xff, 0xff, 0xff, 0x00, 0xff, 0x75, 0xbb, 0xdb, 0xe7, 0x6f, 0x99, 0x7b, 0xfe, 0xef, 0xdf, 0xbf,
                                    0xff, 0xff, 0xff, 0xf8, 0xf0, 0xed, 0xdd, 0xd9, 0xd1, 0xc1, 0xc5, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd,
                                   };
const uint8_t Block_49[] PROGMEM = {0xdb, 0xbb, 0x7b, 0xfb, 0xfb, 0xeb, 0x9b, 0x6b, 0x7b, 0xbb, 0xdb, 0xeb, 0xf3, 0x03, 0x07, 0x0f,
                                    0xff, 0xff, 0x1f, 0xef, 0xfb, 0xf3, 0xff, 0xff, 0xfe, 0xfd, 0xfd, 0xfa, 0xff, 0x00, 0xfa, 0xf8,
                                   };
const uint8_t Block_50[] PROGMEM = {0xbf, 0xdf, 0xef, 0xfe, 0xff, 0x7b, 0x99, 0x6f, 0xe7, 0xdb, 0xbb, 0x75, 0xff, 0x00, 0xff, 0xff,
                                    0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xf9, 0xe5, 0xcc, 0xdc, 0xd0, 0xc1,
                                   };
const uint8_t Block_53[] PROGMEM = {0x7f, 0x7f, 0x67, 0x5b, 0x3b, 0xed, 0xbd, 0xd5, 0xdb, 0x67, 0x1f, 0xff, 0x7f, 0x7f, 0x7f, 0x7f,
                                    0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfc, 0xfd, 0xfc, 0xfc, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd,
                                   };
const uint8_t Block_54[] PROGMEM = {0x7f, 0x7f, 0x7f, 0xff, 0x01, 0xfe, 0x0a, 0x6a, 0x0a, 0x5a, 0xbe, 0x51, 0x9b, 0x03, 0xff, 0x7f,
                                    0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfd, 0xfd, 0xfd,
                                   };
const uint8_t Block_55[] PROGMEM = {0x00, 0xfe, 0xc2, 0xf6, 0xfe, 0xe2, 0xde, 0xe6, 0xde, 0xe2, 0xfe, 0xfe, 0x3e, 0x8e, 0xf6, 0xf6,
                                    0x00, 0xbf, 0x3f, 0xff, 0x33, 0xef, 0xdf, 0x03, 0xdf, 0xe7, 0xff, 0xff, 0x00, 0xff, 0x1f, 0x0f,
                                   };
const uint8_t Block_56[] PROGMEM = {0x00, 0xfe, 0xe0, 0xce, 0x9e, 0xbf, 0xbf, 0xbe, 0xb9, 0x33, 0x6f, 0x6f, 0x00, 0xff, 0xfe, 0xfc,
                                    0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x00, 0x7e, 0x7f, 0x7f, 0x7f,
                                   };
const uint8_t Block_57[] PROGMEM = {0x8e, 0x3e, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0x3e, 0xc6, 0xfa, 0xfa, 0xfa, 0xf6, 0x0e, 0xfe, 0x00,
                                    0x2f, 0xde, 0x38, 0xfb, 0x7b, 0x1b, 0x0d, 0x2e, 0xdf, 0x1f, 0xff, 0xff, 0x07, 0xf8, 0xff, 0x00,
                                   };
const uint8_t Block_58[] PROGMEM = {0xfd, 0xfc, 0xf7, 0xf7, 0xff, 0xfe, 0xfc, 0xfd, 0xfc, 0xfe, 0xff, 0x03, 0xf8, 0xff, 0xff, 0x00,
                                    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x70, 0x4f, 0x3f, 0x1f, 0x00,
                                   };
const uint8_t Block_59[] PROGMEM = {0xff, 0x1f, 0x1f, 0xcf, 0xd7, 0x01, 0xec, 0x1e, 0xf4, 0x96, 0x32, 0x06, 0xfa, 0xfa, 0xfa, 0xf8,
                                    0xff, 0xff, 0x7f, 0x02, 0xdc, 0xc8, 0x3e, 0xc3, 0x37, 0xc0, 0x74, 0x20, 0x9f, 0xbf, 0x3f, 0xbf,
                                   };
const uint8_t Block_60[] PROGMEM = {0xff, 0xf8, 0x42, 0x80, 0xbf, 0x80, 0x5f, 0x50, 0x57, 0x4b, 0x4c, 0x4d, 0x4c, 0x4c, 0x4d, 0x4d,
                                    0xff, 0xff, 0x9c, 0x63, 0x5c, 0x31, 0x83, 0xfd, 0xf2, 0xf6, 0xfb, 0xf5, 0xf5, 0xf2, 0xf5, 0xf2,
                                   };
const uint8_t Block_61[] PROGMEM = {0xf8, 0xfa, 0xfa, 0xfa, 0x06, 0x32, 0x96, 0xf4, 0x1e, 0xec, 0x01, 0xd7, 0xcf, 0x1f, 0x1f, 0xff,
                                    0xbf, 0x3f, 0xbf, 0x9f, 0x20, 0x74, 0xc0, 0x37, 0xc7, 0x3e, 0xc8, 0xdc, 0x02, 0x7f, 0xff, 0xff,
                                   };
const uint8_t Block_62[] PROGMEM = {0x4d, 0x4d, 0x4c, 0x4c, 0x4d, 0x4c, 0x4b, 0x57, 0x50, 0x5f, 0x80, 0xbf, 0x80, 0x42, 0xf8, 0xff,
                                    0xf2, 0xf5, 0xf2, 0xf5, 0xf5, 0xfb, 0xf6, 0xf2, 0xfd, 0x83, 0x31, 0x5c, 0x63, 0x9c, 0xff, 0xff,
                                   };
/*
  const uint8_t Block_[] PROGMEM = {};
*/
#define BNUM 62 //方块个数
const unsigned char *Block[BNUM] = {
  Block_0, Block_2, Block_3, Block_4, Block_5,
  Block_6, Block_7, Block_8, Block_9, Block_10,
  Block_11, Block_12, Block_13, Block_14, Block_15,
  Block_10, Block_17, Block_18, Block_19, Block_20,
  Block_21, Block_22, Block_23, Block_24, Block_25,
  Block_26, Block_27, Block_0, Block_29, Block_30,
  Block_31, Block_32, Block_33, Block_34, Block_35,
  Block_36, Block_37, Block_38, Block_39, Block_40,
  Block_41, Block_42, Block_43, Block_44, Block_45,
  Block_0, Block_47, Block_48, Block_49, Block_50,
  Block_0, Block_27, Block_53, Block_54, Block_55,
  Block_56, Block_57, Block_58, Block_59, Block_60,
  Block_61, Block_62,
};
/*
   业力符文 1-10 普通符文11-18
   const uint8_t Karma_[] PROGMEM = {};
   const uint8_t Rune_[] PROGMEM = {};
*/
const uint8_t Karma_1[] PROGMEM = {0x40, 0x41, 0x42, 0x44, 0x48, 0x50, 0x60, 0x40,};
const uint8_t Karma_2[] PROGMEM = {0xc2, 0xb4, 0x8c, 0x82, 0x81, 0x8e, 0xb0, 0xc0,};
const uint8_t Karma_3[] PROGMEM = {0xc2, 0x34, 0x0c, 0x02, 0x81, 0x8e, 0xb0, 0xc0,};
const uint8_t Karma_4[] PROGMEM = {0xff, 0x11, 0x11, 0x11, 0x81, 0x81, 0x81, 0xff,};
const uint8_t Karma_5[] PROGMEM = {0xff, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff,};
const uint8_t Karma_6[] PROGMEM = {0xff, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x9f,};
const uint8_t Karma_7[] PROGMEM = {0xff, 0x91, 0x91, 0x91, 0xf1, 0x91, 0x91, 0x9f,};
const uint8_t Karma_8[] PROGMEM = {0xff, 0x91, 0x92, 0x94, 0x98, 0x90, 0x80, 0x00,};
const uint8_t Karma_9[] PROGMEM = {0xff, 0x90, 0x90, 0x90, 0x90, 0x90, 0x80, 0x00,};
const uint8_t Karma_10[] PROGMEM = {0x00, 0x20, 0x1e, 0x10, 0x14, 0x10, 0x00, 0x00,};
const uint8_t Rune_1[] PROGMEM = {0x00, 0x22, 0x22, 0x3e, 0x02, 0x02, 0x3e, 0x00,};
const uint8_t Rune_2[] PROGMEM = {0x00, 0x70, 0x08, 0x0e, 0x30, 0x40, 0x30, 0x0e,};
const uint8_t Rune_3[] PROGMEM = {0x10, 0x28, 0x28, 0x54, 0x44, 0xff, 0x02, 0x02,};
const uint8_t Rune_4[] PROGMEM = {0x00, 0x18, 0x08, 0x7d, 0x48, 0x48, 0x78, 0x00,};
const uint8_t Rune_5[] PROGMEM = {0x00, 0x0c, 0x10, 0x11, 0x3c, 0x52, 0x42, 0x3c,};
const uint8_t Rune_6[] PROGMEM = {0x3c, 0x42, 0x89, 0x8d, 0x81, 0xa1, 0x42, 0x3c,};
const uint8_t Rune_7[] PROGMEM = {0x00, 0x00, 0xff, 0x42, 0x24, 0x24, 0x42, 0x00,};
const uint8_t Rune_8[] PROGMEM = {0x00, 0xff, 0x0a, 0x14, 0x28, 0x50, 0xff, 0x00,};
const unsigned char *Rune[18] = {Karma_1, Karma_2, Karma_3, Karma_4, Karma_5, Karma_6, Karma_7, Karma_8, Karma_9, Karma_10, Rune_1, Rune_2, Rune_3, Rune_4, Rune_5, Rune_6, Rune_7, Rune_8,};
/*
  玩家
*/
const uint8_t Man_U_0[] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xfe, 0x7c, 0x7c, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x02, 0x03, 0x01, 0x02, 0x1f, 0xbc, 0x3e, 0x3d, 0xbc, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  };
const uint8_t Man_U_1[] PROGMEM = {0x40, 0x20, 0x20, 0xc0, 0x00, 0x7c, 0xfe, 0x7c, 0x7c, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x08, 0x05, 0x1a, 0xba, 0x3b, 0x3d, 0x3c, 0x9f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  };
const uint8_t Man_U_2[] PROGMEM = {0x00, 0x00, 0x00, 0x40, 0x80, 0x7c, 0xfe, 0x7c, 0x7c, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x03, 0x04, 0x04, 0x02, 0x1c, 0x3d, 0xba, 0x3b, 0xbc, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  };
const uint8_t Man_U_3[] PROGMEM = {0x40, 0x20, 0x20, 0xc0, 0x00, 0x7c, 0xfe, 0x7c, 0x7c, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x08, 0x05, 0x1a, 0xba, 0x3b, 0x3d, 0xbc, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  };
const uint8_t Man_U_4[] PROGMEM = {0x00, 0x00, 0x00, 0x40, 0x80, 0x7c, 0xfe, 0x7c, 0x7c, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x03, 0x04, 0x04, 0x02, 0x1c, 0xbd, 0x3a, 0x3b, 0xbc, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  };
const uint8_t Man_D_0[] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x4e, 0x7c, 0x4c, 0x7e, 0x7c, 0x90, 0x50, 0x40, 0x80, 0x80,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xbf, 0x3f, 0x3f, 0xbf, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  };
const uint8_t Man_D_1[] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x4e, 0x7c, 0x4c, 0x7e, 0x7c, 0x90, 0x60, 0x50, 0x10, 0x20,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xbf, 0x3f, 0x3f, 0x3f, 0x9f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  };
const uint8_t Man_D_2[] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x4e, 0x7c, 0x4c, 0x7e, 0x7c, 0xa0, 0x10, 0x00, 0xc0, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x3f, 0xbf, 0x3f, 0xbf, 0x1f, 0x00, 0x01, 0x01, 0x00, 0x00,
                                  };
const uint8_t Man_D_3[] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x4e, 0x7c, 0x4c, 0x7e, 0x7c, 0x90, 0x60, 0x50, 0x10, 0x20,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xbf, 0x3f, 0x3f, 0xbf, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  };
const uint8_t Man_D_4[] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x4e, 0x7c, 0x4c, 0x7e, 0x7c, 0xa0, 0x10, 0x00, 0xc0, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xbf, 0x3f, 0x3f, 0xbf, 0x1f, 0x00, 0x01, 0x01, 0x00, 0x00,
                                  };
const uint8_t Man_L_0[] PROGMEM = {0x00, 0x00, 0x7c, 0x4e, 0x78, 0x48, 0x7c, 0x7e, 0x80, 0x40, 0x40, 0x40, 0x40, 0x20, 0x00, 0x00,
                                   0x00, 0x00, 0x1f, 0xbf, 0x3f, 0x3f, 0xbf, 0x1f, 0x01, 0x02, 0x04, 0x04, 0x04, 0x03, 0x00, 0x00,
                                  };
const uint8_t Man_L_1[] PROGMEM = {0x00, 0x00, 0x7c, 0x4e, 0x78, 0x48, 0x78, 0x7c, 0x86, 0x40, 0x20, 0x20, 0x30, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x9f, 0x3f, 0x3f, 0x3f, 0x3f, 0x9f, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x0e, 0x00,
                                  };
const uint8_t Man_L_2[] PROGMEM = {0x00, 0x00, 0x7c, 0x4e, 0x7a, 0x48, 0x7c, 0x7e, 0x80, 0x40, 0x40, 0x40, 0x40, 0x20, 0x00, 0x00,
                                   0x00, 0x00, 0x1f, 0xbf, 0x3f, 0x3f, 0x3f, 0x9f, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x00,
                                  };
const uint8_t Man_L_3[] PROGMEM = {0x00, 0x00, 0x7c, 0x4e, 0x78, 0x48, 0x78, 0x7c, 0x86, 0x40, 0x20, 0x20, 0x30, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x1f, 0xbf, 0x3f, 0x3f, 0xbf, 0x1f, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x0e, 0x00,
                                  };
const uint8_t Man_L_4[] PROGMEM = {0x00, 0x00, 0x7c, 0x4e, 0x7a, 0x48, 0x7c, 0x7e, 0x80, 0x40, 0x40, 0x40, 0x40, 0x20, 0x00, 0x00,
                                   0x00, 0x00, 0x1f, 0xbf, 0x3f, 0x3f, 0xbf, 0x1f, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x00,
                                  };

const uint8_t Man_R_0[] PROGMEM = {0x00, 0x00, 0x20, 0x40, 0x40, 0x40, 0x40, 0x80, 0x7e, 0x7c, 0x48, 0x78, 0x4e, 0x7c, 0x00, 0x00,
                                   0x00, 0x00, 0x03, 0x04, 0x04, 0x04, 0x02, 0x01, 0x1f, 0xbf, 0x3f, 0x3f, 0xbf, 0x1f, 0x00, 0x00,
                                  };
const uint8_t Man_R_1[] PROGMEM = {0x00, 0x00, 0x00, 0x30, 0x20, 0x20, 0x40, 0x80, 0x7e, 0x7c, 0x48, 0x7a, 0x4e, 0x7c, 0x00, 0x00,
                                   0x00, 0x0e, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x9f, 0x3f, 0x3f, 0x3f, 0x3f, 0x9f, 0x00, 0x00,
                                  };
const uint8_t Man_R_2[] PROGMEM = {0x00, 0x00, 0x20, 0x40, 0x40, 0x40, 0x40, 0x86, 0x7c, 0x78, 0x48, 0x78, 0x4e, 0x7c, 0x00, 0x00,
                                   0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x9f, 0x3f, 0x3f, 0x3f, 0x3f, 0x1f, 0x80, 0x00,
                                  };
const uint8_t Man_R_3[] PROGMEM = {0x00, 0x00, 0x00, 0x30, 0x20, 0x20, 0x40, 0x80, 0x7e, 0x7c, 0x48, 0x7a, 0x4e, 0x7c, 0x00, 0x00,
                                   0x00, 0x0e, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x1f, 0xbf, 0x3f, 0x3f, 0xbf, 0x1f, 0x00, 0x00,
                                  };
const uint8_t Man_R_4[] PROGMEM = {0x00, 0x00, 0x20, 0x40, 0x40, 0x40, 0x40, 0x86, 0x7c, 0x78, 0x48, 0x78, 0x4e, 0x7c, 0x00, 0x00,
                                   0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x1f, 0xbf, 0x3f, 0x3f, 0xbf, 0x1f, 0x00, 0x00,
                                  };
const unsigned char *T_Man_direction[8][4] = {
  {Man_U_3, Man_U_0, Man_U_4, Man_U_0},
  {Man_U_1, Man_U_0, Man_U_2, Man_U_0},
  {Man_D_3, Man_D_0, Man_D_4, Man_D_0},
  {Man_D_1, Man_D_0, Man_D_2, Man_D_0},
  {Man_L_3, Man_L_0, Man_L_4, Man_L_0},
  {Man_L_1, Man_L_0, Man_L_2, Man_L_0},
  {Man_R_3, Man_R_0, Man_R_4, Man_R_0},
  {Man_R_1, Man_R_0, Man_R_2, Man_R_0}
};


/*=========================================================
                         地图
                         第一个参数为房间号 第二三个参数为xy
  =========================================================*/
const PROGMEM byte MAP[21][16][16] = {
  //Room 0
  20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 8 , 8 , 5 , 4 , 1 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 5 , 5 , 5 , 8 , 8 , 8 , 1 , 4 , 4 , 1 , 20 , 20 , 20 , 20 , 20 , 20 , 2 , 2 , 8 , 8 , 8 , 8 , 20 , 20 , 20 , 4 , 1 , 20 , 20 , 20 , 20 , 5 , 5 , 2 , 8 , 4 , 8 , 8 , 8 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 5 , 8 , 1 , 1 , 4 , 4 , 8 , 5 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 5 , 8 , 1 , 21 , 1 , 4 , 1 , 20 , 20 , 5 , 6 , 6 , 6 , 20 , 20 , 20 , 8 , 8 , 1 , 22 , 6 , 6 , 6 , 6 , 6 , 6 , 1 , 20 , 20 , 20 , 20 , 20 , 8 , 1 , 1 , 6 , 6 , 1 , 15 , 1 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 8 , 4 , 4 , 6 , 1 , 1 , 12 , 1 , 1 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 8 , 4 , 5 , 6 , 4 , 1 , 20 , 1 , 1 , 1 , 20 , 20 , 20 , 20 , 20 , 8 , 8 , 4 , 1 , 6 , 4 , 1 , 1 , 1 , 1 , 13 , 1 , 20 , 38 , 20 , 8 , 8 , 8 , 1 , 6 , 6 , 1 , 11 , 1 , 10 , 33 , 10 , 33 , 13 , 1 , 20 , 8 , 8 , 8 , 4 , 6 , 1 , 1 , 13 , 33 , 13 , 9 , 10 , 1 , 10 , 1 , 20 , 8 , 8 , 1 , 4 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 1 , 33 , 1 , 1 , 20 , 8 , 1 , 4 , 4 , 6 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
  //Room 1
  20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 4 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 1 , 1 , 1 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 4 , 1 , 6 , 6 , 6 , 19 , 20 , 20 , 20 , 20 , 20 , 5 , 5 , 5 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 1 , 1 , 5 , 1 , 5 , 5 , 5 , 4 , 1 , 20 , 4 , 1 , 4 , 4 , 4 , 1 , 1 , 4 , 1 , 1 , 1 , 4 , 4 , 4 , 1 , 6 , 6 , 6 , 5 , 5 , 6 , 6 , 5 , 5 , 6 , 6 , 2 , 6 , 6 , 6 , 6 , 20 , 20 , 20 , 5 , 20 , 20 , 20 , 20 , 20 , 20 , 2 , 2 , 2 , 7 , 7 , 7 , 20 , 20 , 20 , 2 , 2 , 2 , 2 , 2 , 2 , 20 , 2 , 2 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 2 , 2 , 2 , 2 , 2 , 2 , 20 , 20 , 20 , 20 , 20 , 2 , 2 , 20 , 20 , 0, 20 , 20 , 2 , 2 , 2 , 20 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 20 , 20 , 2 , 2 , 2 , 20 , 20 , 20 , 20 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 55 , 57 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 , 56 , 58 ,
  //Room 2
  20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 7 , 7 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 2 , 2 , 2 , 2 , 7 , 1 , 20 , 20 , 5 , 5 , 20 , 5 , 20 , 20 , 20 , 2 , 2 , 5 , 1 , 7 , 1 , 7 , 1 , 1 , 1 , 5 , 20 , 5 , 5 , 5 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 5 , 1 , 1 , 20 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 30 , 30 , 30 , 30 , 6 , 30 , 30 , 30 , 30 , 6 , 8 , 8 , 8 , 8 , 20 , 20 , 29 , 29 , 29 , 29 , 6 , 29 , 29 , 29 , 29 , 6 , 8 , 7 , 4 , 8 , 20 , 20 , 10 , 11 , 11 , 10 , 6 , 10 , 11 , 11 , 10 , 6 , 8 , 4 , 7 , 1 , 8 , 20 , 10 , 10 , 10 , 10 , 6 , 10 , 10 , 10 , 10 , 6 , 8 , 8 , 1 , 7 , 8 , 20 , 10 , 14 , 9 , 14 , 6 , 10 , 14 , 9 , 14 , 6 , 6 , 6 , 8 , 4 , 8 , 8 , 1 , 1 , 6 , 4 , 6 , 4 , 4 , 6 , 4 , 4 , 4 , 6 , 6 , 8 , 1 , 8 , 1 , 1 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 1 , 4 , 6 , 8 , 8 , 8 , 7 , 4 , 7 , 1 , 7 , 1 , 7 , 1 , 7 , 1 , 7 , 1 , 6 , 8 , 1 , 1 , 1 , 7 , 1 , 7 , 1 , 7 , 1 , 7 , 1 , 7 , 1 , 7 , 6 , 1 , 1 , 1 ,
  //Room 3
  20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 2 , 2 , 2 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 1 , 2 , 2 , 2 , 2 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 1 , 1 , 1 , 2 , 2 , 2 , 8 , 20 , 20 , 20 , 8 , 8 , 20 , 20 , 1 , 1 , 21 , 1 , 7 , 1 , 2 , 2 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 1 , 1 , 22 , 1 , 1 , 1 , 2 , 2 , 8 , 4 , 4 , 4 , 4 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 20 , 20 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 1 , 1 , 1 , 1 , 1 , 20 , 20 , 20 , 8 , 8 , 8 , 8 , 8 , 1 , 4 , 6 , 6 , 4 , 4 , 1 , 1 , 1 , 20 , 20 , 8 , 8 , 8 , 8 , 5 , 4 , 8 , 8 , 6 , 4 , 5 , 1 , 1 , 1 , 1 , 20 , 8 , 8 , 8 , 4 , 8 , 8 , 8 , 8 , 6 , 6 , 4 , 1 , 1 , 1 , 20 , 20 , 8 , 8 , 8 , 4 , 8 , 8 , 8 , 8 , 8 , 6 , 5 , 5 , 1 , 1 , 1 , 20 , 8 , 21 , 4 , 8 , 8 , 8 , 8 , 8 , 8 , 6 , 1 , 4 , 4 , 1 , 1 , 20 , 20 , 27 , 4 , 4 , 27 , 27 , 27 , 27 , 20 , 6 , 6 , 4 , 1 , 1 , 1 , 20 , 7 , 4 , 7 , 26 , 7 , 4 , 7 , 4 , 12 , 8 , 6 , 4 , 1 , 1 , 1 , 20 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 7 , 12 , 8 , 6 , 1 , 1 , 1 , 20 , 20 , 7 , 4 , 7 , 4 , 7 , 4 , 7 , 4 , 20 , 8 , 6 , 1 , 20 , 20 , 20 , 20 ,
  //Room 4
  20 , 8 , 8 , 4 , 4 , 6 , 36 , 13 , 4 , 7 , 1 , 7 , 1 , 33 , 33 , 33 , 20 , 1 , 1 , 8 , 4 , 6 , 36 , 4 , 7 , 4 , 13 , 4 , 7 , 10 , 10 , 10 , 20 , 32 , 2 , 1 , 4 , 6 , 15 , 33 , 33 , 33 , 33 , 33 , 1 , 10 , 10 , 10 , 2 , 32 , 2 , 2 , 20 , 6 , 12 , 10 , 10 , 10 , 10 , 10 , 7 , 10 , 9 , 10 , 2 , 32 , 2 , 2 , 12 , 6 , 20 , 10 , 10 , 10 , 11 , 10 , 1 , 1 , 6 , 1 , 2 , 32 , 2 , 2 , 12 , 6 , 4 , 10 , 9 , 10 , 10 , 10 , 1 , 6 , 6 , 1 , 2 , 32 , 2 , 2 , 12 , 6 , 4 , 7 , 6 , 8 , 8 , 8 , 7 , 6 , 4 , 1 , 2 , 32 , 2 , 1 , 12 , 6 , 6 , 6 , 6 , 15 , 8 , 8 , 7 , 4 , 15 , 1 , 2 , 32 , 2 , 1 , 20 , 1 , 6 , 6 , 1 , 12 , 1 , 8 , 7 , 4 , 12 , 1 , 2 , 32 , 2 , 1 , 1 , 1 , 4 , 6 , 1 , 20 , 1 , 6 , 6 , 6 , 6 , 6 , 2 , 32 , 2 , 2 , 1 , 4 , 5 , 6 , 6 , 6 , 6 , 6 , 1 , 1 , 1 , 1 , 2 , 1 , 1 , 1 , 4 , 8 , 8 , 5 , 8 , 6 , 1 , 29 , 27 , 27 , 27 , 29 , 1 , 1 , 4 , 4 , 4 , 8 , 8 , 8 , 5 , 6 , 1 , 25 , 28 , 28 , 28 , 12 , 8 , 8 , 5 , 5 , 5 , 8 , 5 , 8 , 8 , 6 , 1 , 46 , 28 , 28 , 28 , 12 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 5 , 6 , 1 , 25 , 28 , 28 , 28 , 12 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 5 , 6 , 1 , 29 , 27 , 27 , 27 , 29 ,
  //Room 5
  20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 33 , 33 , 33 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 36 , 36 , 36 , 20 , 20 , 20 , 10 , 10 , 10 , 20 , 20 , 20 , 20 , 20 , 1 , 1 , 1 , 1 , 1 , 1 , 20 , 20 , 10 , 9 , 10 , 20 , 20 , 20 , 20 , 20 , 29 , 30 , 29 , 30 , 29 , 1 , 1 , 20 , 45 , 4 , 4 , 20 , 20 , 20 , 20 , 20 , 10 , 10 , 10 , 10 , 10 , 1 , 1 , 20 , 4 , 4 , 6 , 33 , 33 , 20 , 20 , 20 , 10 , 10 , 11 , 10 , 10 , 1 , 1 , 33 , 33 , 33 , 6 , 10 , 10 , 20 , 20 , 20 , 10 , 10 , 10 , 10 , 10 , 1 , 1 , 10 , 10 , 10 , 6 , 10 , 10 , 7 , 12 , 7 , 33 , 33 , 33 , 33 , 33 , 1 , 1 , 10 , 9 , 10 , 6 , 10 , 10 , 7 , 12 , 1 , 10 , 10 , 10 , 10 , 10 , 1 , 1 , 6 , 6 , 6 , 6 , 10 , 10 , 7 , 12 , 1 , 10 , 11 , 10 , 10 , 10 , 1 , 1 , 1 , 4 , 4 , 6 , 6 , 6 , 7 , 20 , 1 , 10 , 10 , 9 , 10 , 10 , 1 , 1 , 1 , 33 , 33 , 33 , 33 , 6 , 6 , 6 , 1 , 7 , 1 , 6 , 7 , 1 , 6 , 6 , 4 , 10 , 10 , 10 , 10 , 36 , 6 , 6 , 6 , 6 , 1 , 6 , 1 , 7 , 6 , 8 , 4 , 10 , 10 , 11 , 10 , 36 , 6 , 7 , 1 , 6 , 6 , 6 , 7 , 1 , 6 , 8 , 4 , 10 , 9 , 10 , 10 , 6 , 6 , 45 , 43 , 44 , 45 , 6 , 6 , 6 , 6 , 8 , 1 , 4 , 6 , 6 , 6 , 1 , 7 , 1 , 7 , 1 , 7 , 1 , 7 , 6 , 8 , 8 ,
  //Room 6
  8 , 8 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 6 , 8 , 8 , 8 , 4 , 4 , 4 , 1 , 30 , 1 , 30 , 1 , 30 , 1 , 4 , 4 , 6 , 6 , 8 , 8 , 4 , 1 , 1 , 1 , 29 , 29 , 29 , 29 , 29 , 1 , 1 , 1 , 1 , 6 , 8 , 8 , 4 , 30 , 1 , 30 , 10 , 10 , 10 , 10 , 10 , 30 , 1 , 30 , 1 , 4 , 1 , 8 , 4 , 29 , 29 , 29 , 29 , 29 , 29 , 29 , 29 , 29 , 29 , 29 , 4 , 4 , 4 , 1 , 4 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 4 , 15 , 4 , 15 , 4 , 10 , 14 , 11 , 14 , 10 , 10 , 10 , 14 , 11 , 14 , 10 , 4 , 12 , 4 , 12 , 4 , 10 , 10 , 10 , 10 , 15 , 31 , 15 , 10 , 10 , 10 , 10 , 4 , 6 , 6 , 6 , 4 , 4 , 4 , 4 , 7 , 12 , 32 , 12 , 7 , 4 , 4 , 4 , 4 , 6 , 6 , 6 , 4 , 4 , 15 , 4 , 4 , 7 , 32 , 7 , 4 , 4 , 4 , 15 , 4 , 6 , 6 , 8 , 4 , 4 , 12 , 4 , 7 , 4 , 32 , 4 , 7 , 4 , 4 , 12 , 4 , 6 , 6 , 8 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 8 , 8 , 8 , 8 , 6 , 6 , 15 , 6 , 6 , 15 , 6 , 6 , 15 , 6 , 6 , 15 , 8 , 8 , 5 , 4 , 6 , 8 , 12 , 1 , 1 , 12 , 1 , 1 , 12 , 1 , 1 , 12 , 8 , 8 , 8 , 4 , 6 , 8 , 1 , 8 , 8 , 1 , 1 , 1 , 1 , 1 , 1 , 8 , 8 , 8 , 8 , 8 , 6 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 ,
  //Room 7
  4 , 7 , 4 , 7 , 4 , 7 , 4 , 7 , 4 , 7 , 6 , 5 , 8 , 8 , 20 , 20 , 7 , 4 , 7 , 4 , 7 , 4 , 7 , 4 , 6 , 6 , 6 , 8 , 8 , 5 , 8 , 20 , 4 , 7 , 4 , 7 , 4 , 6 , 6 , 6 , 6 , 21 , 8 , 5 , 1 , 1 , 1 , 20 , 7 , 4 , 7 , 4 , 7 , 6 , 1 , 1 , 1 , 1 , 1 , 5 , 1 , 1 , 20 , 20 , 4 , 7 , 4 , 7 , 21 , 6 , 1 , 15 , 1 , 2 , 2 , 2 , 2 , 2 , 28 , 28 , 15 , 4 , 15 , 6 , 6 , 6 , 1 , 12 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 28 , 12 , 7 , 12 , 6 , 8 , 1 , 20 , 2 , 2 , 2 , 2 , 2 , 28 , 2 , 28 , 2 , 6 , 6 , 6 , 6 , 6 , 15 , 12 , 2 , 2 , 2 , 2 , 21 , 28 , 28 , 21 , 2 , 15 , 6 , 15 , 6 , 6 , 12 , 12 , 2 , 2 , 2 , 2 , 28 , 24 , 28 , 22 , 20 , 12 , 8 , 12 , 6 , 6 , 20 , 20 , 2 , 2 , 1 , 4 , 12 , 28 , 1 , 12 , 28 , 8 , 8 , 6 , 6 , 6 , 1 , 12 , 2 , 2 , 4 , 4 , 21 , 4 , 28 , 28 , 20 , 8 , 15 , 6 , 6 , 6 , 15 , 12 , 2 , 2 , 2 , 1 , 22 , 28 , 24 , 22 , 20 , 8 , 12 , 6 , 6 , 6 , 12 , 20 , 2 , 2 , 2 , 1 , 1 , 1 , 4 , 24 , 28 , 8 , 8 , 6 , 6 , 6 , 1 , 12 , 2 , 2 , 2 , 2 , 28 , 1 , 4 , 21 , 20 , 8 , 8 , 15 , 6 , 6 , 1 , 12 , 2 , 2 , 2 , 2 , 2 , 2 , 4 , 22 , 20 , 8 , 8 , 12 , 6 , 6 , 1 , 20 , 2 , 2 , 2 , 2 , 2 , 2 , 6 , 22 , 20 ,
  //ROOM 8
  8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 6 , 8 , 8 , 8 , 8 , 4 , 4 , 8 , 4 , 4 , 4 , 4 , 8 , 8 , 8 , 8 , 6 , 8 , 8 , 8 , 8 , 8 , 8 , 4 , 8 , 8 , 8 , 8 , 8 , 8 , 4 , 8 , 6 , 8 , 8 , 8 , 4 , 8 , 8 , 4 , 1 , 1 , 1 , 1 , 4 , 4 , 4 , 8 , 6 , 8 , 8 , 4 , 4 , 8 , 8 , 1 , 10 , 13 , 13 , 13 , 13 , 13 , 10 , 8 , 6 , 8 , 8 , 6 , 8 , 8 , 8 , 1 , 10 , 13 , 13 , 13 , 13 , 13 , 10 , 8 , 6 , 8 , 8 , 6 , 8 , 8 , 8 , 4 , 12 , 10 , 10 , 10 , 10 , 10 , 12 , 8 , 6 , 8 , 8 , 4 , 8 , 8 , 8 , 4 , 12 , 10 , 11 , 10 , 11 , 10 , 12 , 4 , 6 , 8 , 8 , 6 , 8 , 8 , 8 , 4 , 12 , 10 , 10 , 9 , 10 , 10 , 12 , 4 , 6 , 8 , 13 , 4 , 4 , 8 , 8 , 4 , 4 , 4 , 7 , 6 , 7 , 4 , 4 , 5 , 6 , 8 , 8 , 4 , 4 , 8 , 8 , 8 , 4 , 1 , 4 , 6 , 6 , 6 , 6 , 6 , 6 , 8 , 8 , 6 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 1 , 1 , 1 , 1 , 8 , 6 , 8 , 8 , 6 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 4 , 8 , 6 , 8 , 8 , 6 , 6 , 6 , 6 , 8 , 8 , 8 , 4 , 4 , 4 , 4 , 8 , 1 , 6 , 6 , 6 , 6 , 8 , 8 , 8 , 8 , 8 , 4 , 4 , 8 , 4 , 4 , 8 , 1 , 6 , 8 , 8 , 8 , 8 , 4 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 6 , 8 , 4 , 4 , 4 , 4 , 8 ,
  //Room 9
  8 , 8 , 4 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 1 , 6 , 4 , 1 , 8 , 4 , 8 , 4 , 8 , 8 , 8 , 8 , 4 , 4 , 8 , 8 , 1 , 6 , 1 , 4 , 8 , 8 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 4 , 4 , 8 , 6 , 4 , 33 , 8 , 8 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 4 , 15 , 1 , 6 , 4 , 10 , 8 , 4 , 10 , 13 , 13 , 13 , 13 , 13 , 13 , 10 , 4 , 12 , 1 , 6 , 4 , 10 , 8 , 8 , 10 , 13 , 13 , 13 , 13 , 13 , 13 , 10 , 5 , 4 , 1 , 6 , 4 , 10 , 8 , 4 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 9 , 5 , 1 , 6 , 4 , 4 , 8 , 8 , 12 , 10 , 11 , 10 , 10 , 11 , 10 , 12 , 6 , 5 , 1 , 6 , 6 , 6 , 8 , 8 , 12 , 10 , 10 , 10 , 10 , 10 , 10 , 12 , 5 , 1 , 1 , 6 , 5 , 15 , 8 , 8 , 12 , 10 , 10 , 9 , 14 , 10 , 10 , 12 , 5 , 1 , 6 , 6 , 1 , 12 , 8 , 4 , 12 , 10 , 10 , 6 , 4 , 15 , 1 , 1 , 6 , 6 , 6 , 1 , 1 , 1 , 8 , 4 , 12 , 10 , 10 , 6 , 4 , 12 , 1 , 6 , 6 , 1 , 1 , 2 , 2 , 2 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 1 , 2 , 2 , 2 , 2 , 2 , 1 , 5 , 4 , 4 , 1 , 4 , 4 , 4 , 1 , 1 , 2 , 2 , 2 , 2 , 2 , 2 , 1 , 1 , 4 , 1 , 1 , 4 , 7 , 4 , 2 , 2 , 2 , 2 , 2 , 2 , 1 , 1 , 1 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 1 , 4 , 4 , 1 ,
  //Room 10
  8 , 8 , 8 , 6 , 8 , 8 , 8 , 8 , 8 , 4 , 10 , 10 , 10 , 10 , 10 , 10 , 8 , 8 , 4 , 6 , 33 , 33 , 33 , 33 , 7 , 8 , 10 , 11 , 14 , 10 , 11 , 14 , 33 , 33 , 15 , 6 , 10 , 10 , 10 , 10 , 1 , 15 , 10 , 10 , 10 , 9 , 9 , 10 , 11 , 10 , 12 , 6 , 10 , 11 , 11 , 10 , 23 , 26 , 6 , 6 , 6 , 6 , 6 , 6 , 10 , 14 , 4 , 6 , 10 , 10 , 10 , 10 , 36 , 12 , 6 , 36 , 36 , 4 , 7 , 7 , 9 , 10 , 4 , 6 , 10 , 9 , 10 , 10 , 1 , 12 , 6 , 6 , 36 , 1 , 5 , 5 , 6 , 6 , 6 , 6 , 6 , 6 , 33 , 33 , 33 , 33 , 33 , 6 , 36 , 1 , 2 , 5 , 33 , 33 , 33 , 33 , 33 , 6 , 10 , 10 , 10 , 10 , 10 , 6 , 6 , 1 , 2 , 5 , 10 , 23 , 23 , 11 , 10 , 4 , 10 , 11 , 10 , 10 , 10 , 6 , 1 , 36 , 2 , 20 , 10 , 17 , 17 , 10 , 10 , 4 , 10 , 10 , 10 , 9 , 10 , 6 , 15 , 7 , 2 , 2 , 6 , 6 , 6 , 6 , 6 , 4 , 4 , 6 , 6 , 6 , 1 , 1 , 12 , 2 , 2 , 2 , 7 , 1 , 1 , 7 , 1 , 1 , 2 , 2 , 2 , 2 , 2 , 32 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 5 , 5 , 2 , 32 , 2 , 5 , 2 , 2 , 2 , 2 , 5 , 5 , 5 , 5 , 5 , 2 , 2 , 2 , 2 , 32 , 2 , 2 , 2 , 2 , 5 , 5 , 1 , 1 , 1 , 21 , 2 , 5 , 5 , 5 , 5 , 32 , 5 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 4 , 4 , 4 , 1 , 4 , 1 , 6 , 7 , 5 , 8 , 8 ,
  //Room 11
  8 , 1 , 1 , 1 , 6 , 1 , 1 , 2 , 2 , 2 , 2 , 2 , 2 , 6 , 22 , 20 , 1 , 6 , 6 , 6 , 6 , 8 , 4 , 2 , 2 , 2 , 2 , 2 , 1 , 4 , 22 , 20 , 8 , 6 , 20 , 8 , 8 , 4 , 2 , 2 , 2 , 2 , 1 , 4 , 4 , 1 , 2 , 20 , 7 , 6 , 5 , 8 , 2 , 2 , 2 , 2 , 2 , 1 , 24 , 27 , 24 , 2 , 5 , 20 , 8 , 6 , 20 , 7 , 2 , 2 , 2 , 2 , 2 , 2 , 5 , 20 , 20 , 20 , 20 , 20 , 1 , 6 , 1 , 7 , 2 , 2 , 2 , 2 , 2 , 7 , 20 , 19 , 6 , 21 , 7 , 20 , 8 , 6 , 15 , 8 , 2 , 2 , 2 , 2 , 2 , 5 , 5 , 6 , 6 , 6 , 7 , 20 , 6 , 6 , 12 , 5 , 2 , 2 , 2 , 2 , 20 , 7 , 20 , 5 , 5 , 4 , 7 , 20 , 8 , 4 , 20 , 1 , 5 , 2 , 2 , 2 , 20 , 5 , 20 , 20 , 20 , 5 , 5 , 20 , 2 , 1 , 1 , 4 , 1 , 2 , 2 , 2 , 20 , 20 , 20 , 13 , 20 , 20 , 20 , 20 , 2 , 2 , 2 , 7 , 2 , 2 , 2 , 2 , 5 , 20 , 20 , 13 , 13 , 20 , 5 , 5 , 2 , 5 , 2 , 2 , 2 , 5 , 2 , 2 , 2 , 5 , 25 , 23 , 23 , 26 , 20 , 5 , 2 , 2 , 2 , 1 , 2 , 5 , 5 , 2 , 2 , 21 , 12 , 4 , 1 , 12 , 20 , 5 , 2 , 1 , 21 , 1 , 1 , 2 , 2 , 2 , 5 , 1 , 1 , 1 , 4 , 5 , 20 , 20 , 4 , 8 , 4 , 4 , 1 , 2 , 2 , 4 , 8 , 1 , 4 , 1 , 1 , 4 , 5 , 20 , 4 , 4 , 8 , 8 , 4 , 8 , 4 , 8 , 8 , 6 , 1 , 4 , 4 , 5 , 5 , 20 ,
  //ROOM 12
  4 , 4 , 4 , 8 , 8 , 8 , 8 , 8 , 1 , 6 , 1 , 8 , 8 , 1 , 2 , 2 , 4 , 7 , 4 , 4 , 4 , 8 , 8 , 1 , 1 , 6 , 1 , 8 , 1 , 2 , 2 , 2 , 8 , 4 , 4 , 8 , 8 , 8 , 6 , 6 , 6 , 6 , 1 , 1 , 2 , 2 , 2 , 2 , 8 , 8 , 8 , 8 , 4 , 1 , 6 , 4 , 4 , 4 , 1 , 2 , 2 , 2 , 2 , 2 , 8 , 8 , 4 , 5 , 1 , 1 , 6 , 1 , 4 , 1 , 2 , 2 , 2 , 2 , 7 , 7 , 8 , 8 , 4 , 4 , 4 , 1 , 6 , 1 , 1 , 2 , 2 , 2 , 2 , 7 , 1 , 8 , 8 , 4 , 8 , 1 , 1 , 7 , 6 , 1 , 2 , 2 , 2 , 2 , 1 , 1 , 8 , 8 , 8 , 8 , 1 , 7 , 1 , 1 , 4 , 2 , 2 , 2 , 2 , 2 , 1 , 1 , 8 , 8 , 1 , 1 , 1 , 1 , 1 , 1 , 16 , 2 , 2 , 2 , 2 , 2 , 1 , 8 , 8 , 8 , 2 , 2 , 2 , 2 , 2 , 2 , 16 , 2 , 2 , 5 , 2 , 2 , 1 , 8 , 5 , 8 , 2 , 2 , 2 , 2 , 2 , 2 , 16 , 2 , 5 , 4 , 2 , 2 , 1 , 8 , 8 , 8 , 2 , 2 , 2 , 2 , 2 , 1 , 6 , 7 , 4 , 4 , 2 , 2 , 1 , 8 , 8 , 6 , 2 , 2 , 2 , 2 , 1 , 1 , 6 , 6 , 1 , 16 , 16 , 16 , 16 , 6 , 6 , 6 , 8 , 8 , 2 , 2 , 1 , 1 , 6 , 1 , 4 , 2 , 2 , 2 , 1 , 1 , 8 , 8 , 8 , 8 , 5 , 4 , 1 , 6 , 6 , 4 , 8 , 4 , 2 , 2 , 1 , 7 , 8 , 8 , 8 , 8 , 8 , 4 , 4 , 1 , 1 , 8 , 8 , 4 , 2 , 2 , 1 , 1 , 1 , 8 ,
  //ROOM 13
  2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 4 , 7 , 1 , 1 , 7 , 2 , 2 , 2 , 2 , 1 , 4 , 4 , 1 , 1 , 1 , 4 , 4 , 1 , 8 , 8 , 1 , 2 , 4 , 1 , 1 , 1 , 1 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 1 , 1 , 1 , 1 , 6 , 1 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 7 , 1 , 4 , 4 , 6 , 8 , 8 , 8 , 7 , 1 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 1 , 4 , 1 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 1 , 4 , 7 , 1 , 6 , 8 , 4 , 8 , 4 , 1 , 1 , 1 , 7 , 8 , 1 , 7 , 1 , 4 , 7 , 4 , 6 , 8 , 8 , 2 , 2 , 2 , 2 , 4 , 1 , 8 , 8 , 1 , 1 , 4 , 4 , 5 , 6 , 8 , 2 , 2 , 4 , 4 , 2 , 2 , 1 , 8 , 8 , 8 , 1 , 1 , 4 , 4 , 6 , 8 , 2 , 2 , 5 , 4 , 2 , 2 , 2 , 1 , 8 , 8 , 1 , 6 , 6 , 6 , 6 , 8 , 8 , 2 , 2 , 2 , 7 , 2 , 2 , 1 , 8 , 8 , 6 , 6 , 1 , 8 , 6 , 8 , 8 , 4 , 4 , 2 , 2 , 2 , 2 , 1 , 8 , 8 , 6 , 5 , 1 , 1 , 6 , 8 , 8 , 8 , 8 , 8 , 2 , 2 , 7 , 1 , 8 , 8 , 1 , 8 , 4 , 8 , 6 , 6 , 6 , 6 , 6 , 8 , 4 , 1 , 8 , 8 , 8 , 8 , 8 , 8 , 4 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 7 , 8 , 8 , 8 , 1 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 1 ,
  //Room 14
  8 , 8 , 8 , 13 , 13 , 13 , 13 , 13 , 13 , 13 , 13 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 13 , 13 , 10 , 23 , 10 , 10 , 23 , 10 , 13 , 13 , 8 , 8 , 8 , 8 , 8 , 8 , 13 , 10 , 2 , 2 , 16 , 16 , 2 , 2 , 10 , 13 , 8 , 8 , 8 , 8 , 8 , 8 , 13 , 10 , 7 , 2 , 16 , 16 , 2 , 7 , 10 , 13 , 8 , 8 , 8 , 8 , 8 , 8 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 1 , 15 , 1 , 6 , 6 , 6 , 6 , 4 , 8 , 8 , 8 , 8 , 7 , 1 , 7 , 6 , 1 , 12 , 1 , 6 , 10 , 7 , 6 , 10 , 4 , 8 , 15 , 8 , 8 , 7 , 5 , 6 , 6 , 6 , 6 , 6 , 13 , 10 , 6 , 5 , 8 , 1 , 12 , 1 , 8 , 8 , 1 , 1 , 8 , 8 , 8 , 8 , 8 , 13 , 6 , 6 , 6 , 1 , 20 , 1 , 8 , 1 , 1 , 8 , 1 , 8 , 8 , 8 , 8 , 1 , 1 , 8 , 8 , 1 , 1 , 1 , 8 , 1 , 8 , 8 , 1 , 8 , 8 , 8 , 8 , 1 , 8 , 8 , 8 , 8 , 6 , 8 , 8 , 4 , 8 , 4 , 4 , 4 , 1 , 1 , 1 , 1 , 20 , 20 , 8 , 6 , 6 , 8 , 8 , 4 , 1 , 4 , 8 , 8 , 8 , 8 , 1 , 8 , 20 , 8 , 7 , 6 , 8 , 20 , 8 , 4 , 4 , 8 , 4 , 4 , 7 , 21 , 1 , 8 , 1 , 1 , 6 , 8 , 20 , 20 , 8 , 8 , 8 , 4 , 4 , 20 , 1 , 1 , 4 , 4 , 1 , 6 , 6 , 20 , 20 , 20 , 8 , 8 , 8 , 8 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 ,
  //Room 15
  7 , 4 , 8 , 13 , 13 , 8 , 8 , 4 , 8 , 6 , 8 , 8 , 8 , 8 , 8 , 20 , 4 , 4 , 8 , 8 , 13 , 4 , 4 , 4 , 8 , 6 , 8 , 8 , 8 , 8 , 20 , 20 , 8 , 25 , 23 , 8 , 8 , 4 , 4 , 8 , 6 , 15 , 8 , 8 , 8 , 4 , 20 , 20 , 8 , 12 , 8 , 8 , 6 , 6 , 6 , 6 , 6 , 12 , 1 , 8 , 8 , 8 , 20 , 20 , 6 , 6 , 6 , 6 , 6 , 1 , 1 , 1 , 8 , 20 , 1 , 8 , 4 , 4 , 20 , 20 , 8 , 1 , 1 , 1 , 1 , 8 , 8 , 1 , 1 , 1 , 1 , 4 , 4 , 20 , 20 , 20 , 1 , 1 , 8 , 8 , 8 , 4 , 8 , 8 , 1 , 23 , 26 , 4 , 2 , 20 , 20 , 20 , 1 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 12 , 8 , 2 , 2 , 2 , 20 , 1 , 13 , 8 , 4 , 8 , 8 , 4 , 4 , 8 , 8 , 8 , 8 , 2 , 8 , 20 , 20 , 1 , 8 , 8 , 8 , 4 , 8 , 4 , 8 , 8 , 21 , 8 , 2 , 4 , 21 , 4 , 20 , 1 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 8 , 22 , 4 , 8 , 4 , 22 , 4 , 20 , 1 , 1 , 1 , 1 , 8 , 1 , 1 , 1 , 8 , 1 , 24 , 24 , 24 , 1 , 1 , 20 , 20 , 20 , 4 , 13 , 8 , 13 , 8 , 1 , 1 , 4 , 12 , 13 , 12 , 1 , 4 , 20 , 20 , 20 , 20 , 4 , 4 , 8 , 8 , 8 , 4 , 21 , 25 , 23 , 26 , 21 , 4 , 20 , 20 , 20 , 20 , 20 , 13 , 4 , 20 , 2 , 1 , 22 , 20 , 15 , 20 , 22 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 2 , 2 , 4 , 4 , 1 , 4 , 1 , 20 , 20 , 20 ,
  //Room 16
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0, 0, 0, 10 , 10 , 41 , 10 , 46 , 41 , 20 , 20 , 20 , 20 , 46 , 46 , 10 , 0, 0, 0, 10 , 39 , 43 , 10 , 46 , 46 , 20 , 34 , 34 , 20 , 36 , 36 , 10 , 0, 0, 0, 11 , 40 , 1 , 10 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 36 , 10 , 0, 0, 0, 10 , 1 , 1 , 10 , 1 , 47 , 51 , 51 , 51 , 51 , 49 , 1 , 10 , 0, 0, 0, 10 , 1 , 1 , 10 , 1 , 48 , 53 , 52 , 52 , 54 , 50 , 1 , 10 , 0, 0, 0, 10 , 45 , 1 , 1 , 1 , 1 , 45 , 45 , 1 , 45 , 1 , 1 , 10 , 0, 0, 0, 10 , 10 , 10 , 10 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 10 , 0, 0, 0, 10 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 10 , 10 , 10 , 10 , 0, 0, 0, 10 , 1 , 36 , 33 , 33 , 33 , 33 , 33 , 33 , 1 , 18 , 10 , 10 , 0, 0, 0, 10 , 1 , 42 , 10 , 41 , 41 , 10 , 38 , 10 , 1 , 1 , 10 , 10 , 0, 0, 0, 10 , 1 , 7 , 10 , 10 , 10 , 10 , 10 , 10 , 7 , 1 , 10 , 10 , 0, 0, 0, 10 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 10 , 10 , 0, 0, 0, 10 , 10 , 10 , 11 , 10 , 10 , 17 , 10 , 10 , 11 , 10 , 10 , 10 , 0,
  //Room 17
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0, 0, 0, 10 , 46 , 46 , 46 , 46 , 46 , 46 , 46 , 46 , 46 , 46 , 46 , 10 , 0, 0, 0, 10 , 46 , 46 , 46 , 46 , 55 , 57 , 46 , 46 , 46 , 46 , 46 , 10 , 0, 0, 0, 10 , 52 , 52 , 52 , 36 , 56 , 58 , 36 , 52 , 52 , 52 , 52 , 10 , 0, 0, 0, 10 , 7 , 7 , 7 , 1 , 1 , 1 , 1 , 7 , 7 , 7 , 7 , 10 , 0, 0, 0, 10 , 1 , 1 , 1 , 47 , 51 , 51 , 51 , 49 , 1 , 1 , 1 , 10 , 0, 0, 0, 10 , 1 , 1 , 45 , 48 , 52 , 53 , 52 , 50 , 45 , 1 , 1 , 10 , 0, 0, 0, 10 , 1 , 1 , 1 , 1 , 1 , 45 , 1 , 1 , 1 , 1 , 1 , 10 , 0, 0, 0, 10 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 10 , 10 , 10 , 10 , 0, 0, 0, 10 , 1 , 33 , 33 , 33 , 33 , 33 , 33 , 33 , 19 , 1 , 10 , 10 , 0, 0, 0, 10 , 1 , 38 , 41 , 38 , 41 , 41 , 38 , 38 , 36 , 1 , 10 , 10 , 0, 0, 0, 10 , 1 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 1 , 1 , 10 , 10 , 0, 0, 0, 10 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 10 , 10 , 0, 0, 0, 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0,
  //Room 18
  0, 20 , 0, 0, 0, 20 , 0, 20 , 0, 20 , 0, 0, 0, 20 , 0, 0, 0, 20 , 0, 0, 0, 20 , 20 , 20 , 0, 20 , 0, 20 , 0, 20 , 0, 0, 0, 20 , 0, 0, 0, 20 , 0, 20 , 0, 20 , 0, 20 , 0, 20 , 0, 0, 0, 20 , 20 , 20 , 0, 20 , 0, 20 , 0, 0, 20 , 0, 20 , 0, 0, 0, 0, 0, 0, 0, 0, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0, 0, 0, 0, 0, 0, 0, 0, 0 , 0 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 0, 0, 0, 0, 0, 0 , 0 , 0 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 0, 0, 0, 0, 0 , 20 , 20 , 20 , 20 , 20 , 20 , 36 , 20 , 20 , 20 , 20 , 20 , 0, 0, 0, 20 , 20 , 20 , 20 , 20 , 36 , 36 , 36 , 20 , 20 , 20 , 20 , 20 , 0, 0, 0, 20 , 20 , 20 , 20 , 36 , 1 , 1 , 1 , 36 , 20 , 20 , 20 , 20 , 0, 0, 0, 20 , 20 , 20 , 20 , 36 , 1 , 36 , 36 , 20 , 20 , 20 , 20 , 20 , 0, 0, 0, 20 , 20 , 20 , 20 , 36 , 1 , 1 , 1 , 36 , 20 , 20 , 20 , 20 , 0, 0, 0, 20 , 20 , 20 , 10 , 36 , 1 , 36 , 36 , 10 , 20 , 20 , 20 , 20 , 0, 0, 0, 20 , 20 , 20 , 10 , 36 , 1 , 1 , 1 , 10 , 20 , 20 , 20 , 20 , 0, 0, 0, 20 , 20 , 20 , 10 , 36 , 1 , 36 , 36 , 10 , 20 , 20 , 20 , 20 , 0, 0, 0, 20 , 20 , 20 , 10 , 10 , 17 , 10 , 10 , 10 , 20 , 20 , 20 , 20 , 0,
  //Room 19
  20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 1 , 20 , 8 , 8 , 18 , 20 , 20 , 20 , 20 , 20 , 8 , 18 , 20 , 20 , 20 , 20 , 1 , 1 , 8 , 8 , 8 , 8 , 20 , 20 , 20 , 20 , 8 , 8 , 20 , 20 , 20 , 20 , 1 , 20 , 20 , 1 , 20 , 8 , 20 , 20 , 20 , 20 , 20 , 8 , 1 , 20 , 20 , 20 , 4 , 20 , 20 , 1 , 20 , 1 , 1 , 20 , 20 , 20 , 20 , 1 , 4 , 20 , 20 , 28 , 4 , 28 , 1 , 1 , 20 , 1 , 2 , 1 , 20 , 20 , 20 , 1 , 20 , 20 , 4 , 4 , 1 , 20 , 28 , 1 , 20 , 1 , 2 , 20 , 20 , 20 , 20 , 1 , 20 , 4 , 4 , 28 , 1 , 20 , 28 , 4 , 20 , 4 , 2 , 20 , 20 , 20 , 20 , 4 , 20 , 20 , 20 , 28 , 1 , 20 , 1 , 1 , 20 , 4 , 2 , 20 , 20 , 20 , 20 , 4 , 4 , 20 , 20 , 28 , 4 , 1 , 1 , 20 , 20 , 4 , 2 , 2 , 20 , 20 , 20 , 20 , 4 , 20 , 20 , 20 , 4 , 20 , 20 , 20 , 20 , 1 , 2 , 2 , 20 , 20 , 20 , 20 , 1 , 20 , 20 , 20 , 4 , 20 , 20 , 8 , 1 , 1 , 2 , 2 , 20 , 20 , 20 , 1 , 1 , 1 , 7 , 20 , 1 , 20 , 20 , 20 , 1 , 2 , 2 , 20 , 20 , 20 , 20 , 12 , 4 , 12 , 4 , 1 , 4 , 1 , 1 , 1 , 1 , 2 , 2 , 20 , 20 , 20 , 12 , 1 , 4 , 1 , 20 , 20 , 20 , 1 , 1 , 2 , 2 , 2 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 , 20 ,
  //Room 20
  10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 11 , 11 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 11 , 10 , 10 , 10 , 10 , 11 , 11 , 10 , 10 , 10 , 10 , 11 , 10 , 10 , 10 , 14 , 10 , 14 , 10 , 14 , 10 , 10 , 10 , 10 , 14 , 10 , 14 , 10 , 14 , 10 , 51 , 51 , 51 , 51 , 51 , 51 , 51 , 59 , 61 , 51 , 51 , 51 , 51 , 51 , 51 , 51 , 10 , 10 , 6 , 1 , 6 , 1 , 6 , 60 , 62 , 6 , 1 , 6 , 1 , 6 , 10 , 10 , 10 , 10 , 6 , 1 , 6 , 1 , 6 , 6 , 6 , 6 , 1 , 6 , 1 , 6 , 10 , 10 , 10 , 10 , 6 , 1 , 6 , 1 , 1 , 1 , 1 , 1 , 1 , 6 , 1 , 6 , 10 , 10 , 10 , 10 , 6 , 1 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 1 , 6 , 10 , 10 , 10 , 10 , 6 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 6 , 10 , 10 , 1 , 1 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 29 , 29 , 29 , 29 , 29 , 29 , 29 , 1 , 1 , 1 , 29 , 29 , 29 , 29 , 29 , 29 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 1 , 1 , 1 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 1 , 32 , 1 , 10 , 10 , 10 , 10 , 10 , 10 ,
};

/*=========================================================
                          对话
  =========================================================*/
#define MESNUM 59 //文本数量
#define MesFL 32  //文本条数
//byte MesF[MesFL]; //对应对话进行条数
PROGMEM const char mes0[] =   {"The front was...\n"};
PROGMEM const char mes1[] =   {"It's really unstable here!\n"};
PROGMEM const char mes2[] =   {"It was a dangerous place. It looked like...crash\n"};
PROGMEM const char mes3[] =   {"I finally got out\n"};
PROGMEM const char mes4[] =   {"There is a flower on it\n"};
PROGMEM const char mes5[] =   {"When the altar was activated but did not hesitate to jump in\n"};
PROGMEM const char mes6[] =   {"Although the prophet left this world\n"};
PROGMEM const char mes7[] =   {"But it gives us a way out\n"};
PROGMEM const char mes8[] =   {"You'll be forced to wake up again whether you want to or not\n"};
PROGMEM const char mes9[] =   {"Making futile efforts\n"};
PROGMEM const char mes10[] =  {"You can't escape because it's part of the world\n"};
PROGMEM const char mes11[] =  {"But the prophet found a solution\n"};
PROGMEM const char mes12[] =  {"Break the blockade with\n"};
PROGMEM const char mes13[] =  {"Leave the cycle of\n"};
PROGMEM const char mes14[] =  {"I'm tired. \n"};
PROGMEM const char mes15[] =  {"Find a place to rest\n"};
PROGMEM const char mes16[] =  {"Just the rock on the west side\n"};
PROGMEM const char mes17[] =  {"I can't walk any further\n"};
PROGMEM const char mes18[] =  {"It's a magnificent two-story building, and the doors are open\n"};
PROGMEM const char mes19[] =  {"It's a luxurious building with open doors.\n"};
PROGMEM const char mes20[] =  {"There's no way ahead. \n"};
PROGMEM const char mes21[] =  {"Don't go ahead.\n"};
PROGMEM const char mes22[] =  {"Purple flame\n"};
PROGMEM const char mes23[] =  {"This is not an ordinary flame.\n"};
PROGMEM const char mes24[] =  {"This is a special stove.\n"};
PROGMEM const char mes25[] =  {"I've seen it somewhere else.\n"};
PROGMEM const char mes26[] =  {"Inlaid spells make it fire\n"};
PROGMEM const char mes27[] =  {"If there really is no one, maybe we can get some sleep\n"};
PROGMEM const char mes28[] =  {"This is a poem\n"};
PROGMEM const char mes29[] =  {"It is written in hieroglyphics\n"};
PROGMEM const char mes30[] =  {"The first half describes the hardships of primitive life\n"};
PROGMEM const char mes31[] =  {"The second half is about the great nature\n"};
PROGMEM const char mes32[] =  {"Finally everyone left\n"};
PROGMEM const char mes33[] =  {"Newspaper!\n"};
PROGMEM const char mes34[] =  {"The country opens its corridors to the masses\n"};
PROGMEM const char mes35[] =  {"Newspaper decay\n"};
PROGMEM const char mes36[] =  {"Family photos\n"};
PROGMEM const char mes37[] =  {"A family of three who looked like me were having a picnic\n"};
PROGMEM const char mes38[] =  {"The world's resources are finite\n"};
PROGMEM const char mes39[] =  {"Should we give up the world?\n"};
PROGMEM const char mes40[] =  {"Getting out of the world is the solution, and the government is holding a referendum\n"};
PROGMEM const char mes41[] =  {"Rain world\n"};
PROGMEM const char mes42[] =  {"Slug?\n"};
PROGMEM const char mes43[] =  {"This is the outline of a meeting\n"};
PROGMEM const char mes44[] =  {"The building may have been a former government office\n"};
PROGMEM const char mes45[] =  {"Advanced examination reform document\n"};
PROGMEM const char mes46[] =  {"What do you think of mass suicide\n"};
PROGMEM const char mes47[] =  {"Use unstable particles to reduce population pressure\n"};
PROGMEM const char mes48[] =  {"How to prevent some government workers from committing mass suicide with their family members\n"};
PROGMEM const char mes49[] =  {"I'm not going there\n"};
PROGMEM const char mes50[] =  {"locked\n"};
PROGMEM const char mes51[] =  {"Dangerous house\n"};
PROGMEM const char mes52[] =  {"Danger, keep away\n"};
PROGMEM const char mes53[] =  {"No Wading\n"};
PROGMEM const char mes54[] =  {"It's time to go home"};
PROGMEM const uint8_t mes55[] =  { 0xf2, 0xeb, 0xec, 0xed, 0x8a, };
PROGMEM const uint8_t mes56[] =  { 0xf2, 0x16, 0x17, 0x09, 0xee, 0x94, 0x4e, 0x25, };
PROGMEM const char mes57[] =  {"wine\n"};
PROGMEM const char mes58[] =  {"water\n"};

/*                                 1     2     3     4      5     6     7    8     9      10    11   12    13    14     15   X
  PROGMEM const uint8_t mes[] =
*/
const unsigned char *MES[MESNUM] = {
  mes0, mes1, mes2, mes3, mes4, mes5, mes6, mes7, mes8, mes9, mes10,
  mes11, mes12, mes13, mes14, mes15, mes16, mes17, mes18, mes19, mes20,
  mes21, mes22, mes23, mes24, mes25, mes26, mes27, mes28, mes29, mes30,
  mes31, mes32, mes33, mes34, mes35, mes36, mes37, mes38, mes39, mes40,
  mes41, mes42, mes43, mes44, mes45, mes46, mes47, mes48, mes49, mes50,
  mes51, mes52, mes53, mes54, mes55, mes56, mes57, mes58,
};
#define ETNUM 119 //事件个数
//事件触发房间和目标房间 {,,},  0-x 1-y 2-事件类型
/*
   事件类型: 0传送 1自动对话  {触发房间，第二属性，事件类型} 2业力
    传送第二属性为目标房间
    对话第二属性为对话在 “对话进行”列表的位置
    业力第二属性为增加业力等级或减少业力等级 0为减少 1为增加
*/
const PROGMEM byte ETRoom[ETNUM][3] = {
  {9, 8, 0},
  {8, 9, 0},
  {8, 12, 0},
  {12, 8, 0},
  {12, 13, 0},
  {12, 13, 0},
  {13, 12, 0},
  {13, 12, 0},
  {13, 14, 0},
  {14, 13, 0},
  {14, 1, 2},     //公园魔法阵残余业力
  {14, 14, 0},
  {14, 15, 0},
  {15, 14, 0},
  {15, 11, 0},
  {11, 15, 0},
  {11, 7, 0},
  {7, 11, 0},
  {7, 11, 0},
  {11, 7, 0},
  {7, 0, 2},  //崩塌区块离开时候触发
  {7, 11, 0},
  {11, 0, 1},
  {6, 7, 0},
  {6, 7, 0},
  {7, 6, 0},
  {9, 10, 0},
  {10, 9, 0},
  {10, 11, 0},
  {11, 10, 0},
  {10, 6, 0},
  {6, 10, 0},
  {11, 1, 1},
  {15, 1, 2}, //小破庙
  {12, 1, 2}, //小溪边
  {15, 2, 1}, //小破庙
  {15, 3, 1}, //小破庙
  {15, 4, 1}, //小破庙
  {12, 6, 1}, //有点累了
  {12, 7, 1}, //走不动了
  {8, 8, 1}, //有一栋房子门没有锁
  {8, 9, 1}, //前面没有路了
  {8, 16, 0},
  {16, 8, 0},
  {16, 1, 2},
  {16, 16, 0},
  {16, 10, 1},
  {16, 11, 1},
  {16, 12, 1},
  {16, 13, 1},
  {16, 14, 1},
  {16, 15, 1},
  {16, 16, 1},
  {16, 17, 0},
  {17, 16, 0},
  {17, 17, 1},
  {17, 18, 1},
  {17, 19, 1},
  {17, 20, 1},
  {17, 21, 1},
  {17, 22, 1},
  {17, 23, 1},
  {8, 4, 0},
  {4, 8, 0},
  {4, 5, 0},
  {5, 4, 0},
  {4, 0, 0},
  {0, 4, 0},
  {4, 24, 1},
  {4, 25, 1},
  {4, 25, 1},
  {5, 6, 0},
  {6, 5, 0},
  {5, 9, 0},
  {9, 5, 0},
  {5, 18, 0},
  {18, 5, 0},
  {18, 0, 2},
  {18, 1, 2},
  {0, 26, 1},
  {0, 27, 1},
  {0, 1, 0},
  {1, 0, 0},
  {0, 1, 0},
  {1, 0, 0},
  {1, 19, 0},
  {19, 1, 0},
  {1, 2, 0},
  {2, 1, 0},
  {2, 6, 0},
  {6, 2, 0},
  {2, 3, 0},
  {3, 2, 0},
  {3, 7, 0},
  {7, 3, 0},
  {3, 1, 2},
  {7, 27, 1},
  {3, 28, 1},
  {7, 29, 1},
  {19, 245, 0},
  {245, 246, 0},
  {246, 247, 0},
  {247, 248, 0},
  {248, 249, 0},
  {249, 250, 0},
  {250, 251, 0},
  {251, 252, 0},
  {252, 253, 0},
  {253, 254, 0},
  {254, 255, 0},
  {255, 20, 0},
  {11, 1, 2},
  {6, 20, 0},
  {20, 6, 0},
  {18, 30, 1}, //酒
  {18, 31, 1}, //水
  {10, 11, 0},
  {11, 11, 0},
  {11, 9, 2},
};
/*事件触发坐标
   如果事件类型为传送第二属性为目标坐标
   对话类型第二属性{文本编号开始,文本编号结束}
   业力类型第二属性{变化多少业力,业力记忆编号}
    {{,}, {,}},
*/
const PROGMEM byte ETXY[ETNUM][2][2] = {
  {{0, 12}, {15, 12}},
  {{15, 12}, {0, 12}},
  {{9, 15}, {9, 0}},
  {{9, 0}, {9, 15}},
  {{15, 11}, {0, 11}},
  {{15, 12}, {0, 12}},
  {{0, 11}, {15, 11}},
  {{0, 12}, {15, 12}},
  {{15, 5}, {0, 5}},
  {{0, 5}, {15, 5}},
  {{7, 14}, {1, 1}}, //公园魔法阵残余业力
  {{7, 14}, {7, 2}},
  {{15, 4}, {0, 4}},
  {{0, 4}, {15, 4}},
  {{9, 0}, {9, 15}},
  {{9, 15}, {9, 0}},
  {{4, 0}, {3, 15}},
  {{3, 15}, {4, 0}},
  {{4, 15}, {4, 0}},
  {{13, 0}, {13, 15}},
  {{13, 15}, {1, 0}},//崩塌区块离开时候触发
  {{13, 15}, {2, 14}},
  {{13, 1}, {0, 1}},
  {{15, 7}, {0, 7}},
  {{15, 8}, {0, 7}},
  {{0, 7}, {15, 7}},
  {{15, 7}, {0, 6}},
  {{0, 6}, {15, 7}},
  {{15, 3}, {0, 7}},
  {{0, 7}, {15, 3}},
  {{3, 0}, {3, 15}},
  {{3, 15}, {3, 0}},
  {{2, 14}, {2, 4}},
  {{11, 14}, {1, 2}}, //小破庙
  {{3, 14}, {1, 3}}, //小溪边
  {{12, 10}, {5, 7}}, //小破庙
  {{9, 11}, {8, 10}}, //小破庙
  {{9, 15}, {11, 13}}, //小破庙
  {{12, 12}, {14, 15}},
  {{9, 12}, {16, 17}},
  {{4, 10}, {18, 19}},
  {{12, 11}, {20, 21}},
  {{4, 9}, {8, 15}},
  {{8, 15}, {4, 9}},
  {{4, 5}, {3, 4}},
  {{4, 5}, {3, 6}},
  {{9, 5}, {22, 22}},
  {{10, 5}, {23, 26}},
  {{5, 8}, {27, 27}},
  {{8, 8}, {28, 32}},
  {{11, 8}, {33, 35}},
  {{4, 5}, {36, 37}},
  {{3, 12}, {38, 40}},
  {{12, 11}, {12, 11}},
  {{11, 11}, {11, 11}},
  {{7, 6}, {41, 41}},
  {{8, 6}, {42, 42}},
  {{8, 9}, {43, 44}},
  {{4, 14}, {45, 45}},
  {{6, 14}, {46, 46}},
  {{9, 14}, {47, 47}},
  {{10, 14}, {48, 48}},
  {{9, 0}, {9, 15}},
  {{9, 15}, {9, 0}},
  {{15, 9}, {0, 9}},
  {{0, 9}, {15, 9}},
  {{5, 0}, {5, 15}},
  {{5, 15}, {5, 0}},
  {{10, 13}, {49, 49}},
  {{8, 6}, {50, 50}},
  {{14, 4}, {50, 50}},
  {{15, 11}, {0, 11}},
  {{0, 11}, {15, 11}},
  {{13, 15}, {13, 0}},
  {{13, 0}, {13, 15}},
  {{2, 4}, {7, 15}},
  {{7, 15}, {2, 4}},
  {{9, 9}, {1, 5}},
  {{9, 11}, {2, 6}},
  {{15, 12}, {51, 51}},
  {{6, 8}, {52, 52}},
  {{15, 6}, {0, 6}},
  {{0, 6}, {15, 6}},
  {{15, 3}, {0, 3}},
  {{0, 3}, {15, 3}},
  {{5, 3}, {1, 2}},
  {{2, 2}, {4, 3}},
  {{15, 6}, {0, 6}},
  {{0, 6}, {15, 6}},
  {{12, 15}, {12, 0}},
  {{12, 0}, {12, 15}},
  {{15, 6}, {0, 6}},
  {{0, 6}, {15, 6}},
  {{10, 15}, {10, 0}},
  {{10, 0}, {10, 15}},
  {{12, 3}, {1, 7}},
  {{9, 3}, {52, 52}},
  {{10, 5}, {53, 53}},
  {{13, 13}, {54, 54}},
  {{11, 1}, {7, 7}},

  {{15, 0}, {7, 7}},
  {{15, 15}, {7, 7}},
  {{0, 15}, {7, 7}},
  {{15, 15}, {7, 7}},
  {{15, 0}, {7, 7}},
  {{15, 0}, {7, 7}},
  {{15, 0}, {7, 7}},
  {{15, 0}, {7, 7}},
  {{0, 15}, {7, 7}},
  {{15, 15}, {7, 7}},
  {{0, 0}, {0, 11}},
  {{3, 10}, {1, 8}},
  {{6, 8}, {8, 15}},
  {{8, 15}, {6, 8}},
  {{9, 9}, {57, 57}},
  {{9, 11}, {58, 58}},
  {{5, 15}, {13, 6}},
  {{11, 5}, {11, 12}},
  {{13, 6}, {1, 9}},
};
/*事件触发方向以及其他属性 {,},
   如果触发方向为255则忽略方向限定
   传送类型 0-触发方向 1-目标方向
   对话类型 0-触发方向 1-是否重复触发
   业力类型 0-触发方向 1-是否重复触发
*/
const PROGMEM byte ETPC[ETNUM][2] = {
  {2, 2},
  {3, 3},
  {1, 1},
  {0, 0},
  {3, 3},
  {3, 3},
  {2, 2},
  {2, 2},
  {3, 3},
  {2, 2},
  {0, 0}, //公园魔法阵残余业力
  {0, 1},
  {3, 3},
  {2, 2},
  {0, 0},
  {1, 1},
  {0, 0},
  {1, 1},
  {1, 1},
  {0, 0},
  {1, 0},//崩塌区块离开时候触发
  {1, 1},
  {0, 0},
  {3, 3},
  {3, 3},
  {2, 2},
  {3, 3},
  {2, 2},
  {3, 3},
  {2, 2},
  {0, 0},
  {1, 1},
  {1, 0},
  {0, 0}, //小破庙
  {2, 0}, //小溪边
  {3, 1},
  {0, 1},
  {0, 1},
  {2, 0},
  {2, 0},
  {0, 0},
  {0, 0},
  {0, 0},
  {1, 1},
  {2, 0},
  {2, 1},
  {0, 1},
  {0, 1},
  {2, 1},
  {0, 1},
  {0, 1},
  {0, 1},
  {3, 1},
  {3, 3},
  {2, 2},
  {0, 1},
  {0, 1},
  {0, 1},
  {0, 1},
  {0, 1},
  {0, 1},
  {0, 1},
  {0, 0},
  {1, 1},
  {3, 3},
  {2, 2},
  {0, 0},
  {1, 1},
  {3, 1},
  {0, 1},
  {0, 1},
  {3, 3},
  {2, 2},
  {1, 1},
  {0, 0},
  {0, 0},
  {1, 1},
  {3, 0},
  {3, 0},
  {0, 1},
  {0, 1},
  {3, 3},
  {2, 2},
  {3, 3},
  {2, 2},
  {3, 3},
  {3, 3},
  {3, 3},
  {2, 2},
  {1, 1},
  {0, 0},
  {3, 3},
  {2, 2},
  {1, 1},
  {0, 0},
  {255, 0},
  {255, 1},
  {0, 1},
  {255, 0},
  {255, 1},
  {255, 1},
  {255, 1},
  {255, 1},
  {255, 0},
  {255, 1},
  {255, 1},
  {255, 0},
  {255, 0},
  {255, 1},
  {255, 0},
  {255, 3},
  {255, 0},
  {0, 0},
  {1, 1},
  {3, 255},
  {3, 255},
  {0, 0},
  {2, 1},
  {255, 0},
};
/*====================================================================
                             软重启函数
  ====================================================================*/
void(* resetFunc) (void) = 0; //制造重启命令
/*====================================================================
                             只循环一次
  ====================================================================*/
void setup()
{
  arduboy.boot();
  SetTextColor(0);
  arduboy.invert(DisplayInvert);
  //Serial.begin(115200);
  ROOM = Entity[0][2];
  draw();
  Eload();
}
/*====================================================================
                             主程序
  ====================================================================*/
void loop()
{
  FixedUpdate();
  Update() ;
}
void FixedUpdate()
{
  if (!key_lock) {
    key();
  }
  logic();
}
void Update()
{
  draw();

  FPS++;

}
/*====================================================================
                             逻辑
  ====================================================================*/
void logic()
{
  /*
     检测按键返回值 对相应方向进行移动障碍物判断
  */
  if (KeyBack < 4) {
    SBDP();
    player_move = true;
  } else {
    player_move = false;
  }
  switch (KeyBack) {
    case 0:
      if (!move_lock) {
        PlayerD = 0;
      }
      break;
    case 1:
      if (!move_lock) {
        PlayerD = 1;
      }
      break;
    case 2:
      if (!move_lock) {
        PlayerD = 2;
      }
      break;
    case 3:
      if (!move_lock) {
        PlayerD = 3;
      }
      break;
    case 5:
      InfoMenu();
      break;
  }
  /*
     如果障碍物判断合法那么将会进行移动
  */
  if (player_move) {
    SBDP();
  }
  if (LA) {
    Entity[0][0] += CPDX;
    Entity[0][1] += CPDY;
    LA = false;
  }
  BF++;
  if (BF >= 5) BF = 0;
  /*
     检查业力不足 非法地图
  */
  if (Karma <= 0 || Karma > 10) ERst();
  if (ROOM >= 246 && Karma < ROOM - 245) {
    //drawText(0, 57, MES[56], pgm_read_byte(&MESleng[56]),1);
    arduboy.display();
    delay(1000);
    TP(14, 103, 31, 1);
  }
  /*
      通关前的混沌地图 符文引导模式
  */
  if (ROOM >= 245) {
    DrawKarmaB = false;
    if (abs(int((Entity[0][0] / 16) - pgm_read_byte(&ETXY[ROOM - 145][0][0]))) <= 7 && abs(int((Entity[0][1] / 16) - pgm_read_byte(&ETXY[ROOM - 145][0][1]))) <= 7) DrawKarmaB = true;
    if (ROOM == 255) EEPROM.update(512, 4);
  }

  /*
     通关
  */
  if (rbcr >= 64) {
    MBlur();
    EEPROM.update(512, 3);
    if (EEPROM.read(513) == 3) resetFunc();
  }
  if (EEPROM.read(512) != EEPROM.read(513)) EEPROM.update(513, EEPROM.read(512));  //确保存档状态码可靠性
  /*
     检查存档状态码
  */

}

/*=========================================================
                      事件
  =========================================================*/
/*
  事件类型: 0传送 1自动对话 2触发性对话 3业力增加 4业力减少
*/
void Event() {
  PMX = Entity[0][0] / 16;
  PMY = Entity[0][1] / 16;
  for (byte TPN = 0; TPN < ETNUM; TPN++) { //遍历事件
    /*
      自动型事件
    */
    if (pgm_read_byte(&ETRoom[TPN][0]) == ROOM) {
      if (pgm_read_byte(&ETPC[TPN][0]) == PlayerD || pgm_read_byte(&ETPC[TPN][0]) == 255) {
        if (pgm_read_byte(&ETXY[TPN][0][0]) == PMX && pgm_read_byte(&ETXY[TPN][0][1]) == PMY ) {
          switch (pgm_read_byte(&ETRoom[TPN][2])) {
            case 0:
              //符合目标传送门跳转条件
              TP(pgm_read_byte(&ETRoom[TPN][1]), pgm_read_byte(&ETXY[TPN][1][0]) * 16, pgm_read_byte(&ETXY[TPN][1][1]) * 16, pgm_read_byte(&ETPC[TPN][1]));
              Esave();
              break;
            case 1:  //自动型对话
              /* pgm_read_byte(&)  byte() F(" ,")
                 当前句    MesI
                 开始      pgm_read_byte(&ETXY[MesI][1][0])
                 结束      pgm_read_byte(&ETXY[MesI][1][1])
                 长度      pgm_read_byte(&MESleng[当前句])
                 文本内容  MES[MesI]
                 对话进度  MesF[当前句]
              */

              key();
              if (millis() >= dialog_cool_time + Timer[2] && KeyBack == 4 && pgm_read_byte(&ETPC[TPN][1]) && pgm_read_byte(&ETXY[TPN][1][0]) + EEPROM.read(pgm_read_byte(&ETRoom[TPN][1]) + 11) > pgm_read_byte(&ETXY[TPN][1][1])) {
                //当前对话为可重复触发类型，重置触发状态
                EEPROM.update(pgm_read_byte(&ETRoom[TPN][1]) + 11, 0);
                Timer[2] = millis();
              } else {
                byte MesI = pgm_read_byte(&ETXY[TPN][1][0]) + EEPROM.read(pgm_read_byte(&ETRoom[TPN][1]) + 11);
                if (MesI <= pgm_read_byte(&ETXY[TPN][1][1])) {
                  /*
                    arduboy.println(MesI);
                    arduboy.println(pgm_read_byte(&ETXY[TPN][1][0]));
                    arduboy.println(MesF[pgm_read_byte(&ETRoom[TPN][1])]);
                    arduboy.println(pgm_read_byte(&MESleng[MesI]));
                    arduboy.println();
                    arduboy.println();
                  */
                  //drawText(1, 56, MES[MesI], pgm_read_byte(&MESleng[MesI]),1);
                  PrintMes(MesI);
                  if (millis() >= dialog_cool_time + Timer[2]) {
                    Timer[2] = millis();
                    key();
                    if (KeyBack == 4) EEPROM.update(pgm_read_byte(&ETRoom[TPN][1]) + 11, EEPROM.read(pgm_read_byte(&ETRoom[TPN][1]) + 11) + 1) ;
                  }
                }
              }
              break;
            /*
               业力事件

               业力记忆表     KarmaB[]
               业力减或增     pgm_read_byte(&ETRoom[TPN][1])
               变化多少业力   pgm_read_byte(&ETXY[TPN][1][0])
               业力记忆表编号 pgm_read_byte(&ETXY[TPN][1][1])
               是否重复触发   pgm_read_byte(&ETP[TPN][1])
            */
            case 2:
              if (EEPROM.read(pgm_read_byte(&ETXY[TPN][1][1]) + 127) == false || pgm_read_byte(&ETPC[TPN][1]) == 1) {
                char KC = pgm_read_byte(&ETXY[TPN][1][0]);
                if (pgm_read_byte(&ETRoom[TPN][1]) == 0) KC = -KC;
                EEPROM.update(pgm_read_byte(&ETXY[TPN][1][1]) + 127, 1);
                KarmaCutscenes(0);
                DrawKarma(KC);
                DrawKarma(0);
                Esave();
                // Eload();
                delay(1000);
              }
              break;
          }
        }
      } /*else {

        key();
        switch (KeyBack) {
          case 4: //按下A键
            if (pgm_read_byte(&ETXY[TPN][0][0]) == (Entity[0][1] + 8  * CPDY) / 16 && pgm_read_byte(&ETXY[TPN][0][1]) == (Entity[0][0] + 8 * CPDX) / 16 && pgm_read_byte(&ETRoom[TPN][2]) == 2) {
              //手动触发对话
            }
            break;
        }
      }*/
    }
  }
}
/*
   场景虚化
*/
void Blur(int sx, int sy, int ex, int ey, byte f) {
  for (byte y = 0; y < 64; y++) {
    for (byte x = 0; x < 128; x++) {
      if (x % 2 == y % 2 && x % 2 == 0 && x >= sx && x <= ex && y >= sy && y <= ey) arduboy.drawPixel(x + (f > 0 && f < 3), y + (f > 1), 0);
    }
  }
}
void MBlur() {
  for (byte f = 0; f < 4; f++) {
    Blur(0, 0, 127, 64, f);
    arduboy.display();
    delay(250);
  }
}
/*=========================================================
                     绘图
  =========================================================*/
void draw()
{
  arduboy.clear();
  DrawMap();
  draw_player(55, 23);
  if (ROOM == Entity[1][2] && EEPROM.read(512) == 4) draw_Reverberation(Entity[1][0] - Entity[0][0] / 2, Entity[1][1] - Entity[0][1] / 2);
  // draw_Reverberation(64, 32);
  if (DrawKarmaB) DrawRune(0, 49, Karma - 1);
  Event();
  /*
    arduboy.setCursor(0, 0);
    arduboy.print(EEPROM.read(512));

    drawFPS();
    arduboy.println(abs(int((Entity[0][0] / 16) - pgm_read_byte(&ETXY[ROOM - 145][0][0]))));
    arduboy.println(abs(int((Entity[0][1] / 16) - pgm_read_byte(&ETXY[ROOM - 145][0][1]))));
    arduboy.println(ROOM);
  */
  arduboy.display();

}
/*
   显示符文
*/
void DrawRune(int x, int y, byte K)
{
  arduboy.fillCircle(x + 7, y + 7, 7, 1);
  drawCircle(x + 7, y + 7, 7, 0, 0);
  arduboy.drawBitmap(x + 4, y + 3, Rune[K], 8, 8, 0);
}
/*
   业力条
*/
void DrawKarma(char KC)
{
  //KC 业力变动范围 -1 0 1
  if (Karma + KC > 0) Karma += KC;
  if (Karma > 10) Karma = 10;
  for (char KCY = 14 * KC;;) {
    arduboy.clear();
    DrawMap();
    draw_player(55, 23);
    for (byte i = 0; i < 6; i++) {
      if (Karma + 1 - i >= 0 && Karma + 1 - i < 10) {
        DrawRune(8, -7 + i * 15 - KCY, Karma + 1 - i);
        if (i != 2) Blur(8, -7 + i * 15 - KCY, 22, +7 + i * 15 - KCY, 1);
      }
    }
    //当前业力符文外圈浮动效果
    drawCircle(15, 30, 10 + player_dyn, 1, 0);
    drawCircle(15, 30, 11 + player_dyn, 0, 0);
    drawCircle(15, 30, 12 + player_dyn, 1, 0);
    arduboy.display();
    if (KC < 0) KCY++; else if (KC > 0) KCY--;
    if (KCY == 0) break;
  }
  KC = 0;
}
/*
  业力符文过场动画  BKS=t 则按B键可退出
*/
void KarmaCutscenes(bool BKS)
{
  for (byte KF = 49; KF > 24; KF--) {
    if (BKS) {
      key();
      if (KeyBack != 5) break;
    }
    arduboy.clear();
    DrawMap();
    draw_player(55, 23);
    DrawRune(map(KF, 49, 24, 0, 7), KF, Karma - 1);
    arduboy.display();
  }
}
/*
   地图场景
*/
void DrawMap()
{
  PMX = Entity[0][0] / 16;
  PMY = Entity[0][1] / 16;
  int MapReadLimit[4]; //显示地图缓存读取范围 防止溢出死循环
  MapReadLimit[0] = PMX - 4;
  MapReadLimit[1] = PMX + 5;
  MapReadLimit[2] = PMY - 2;
  MapReadLimit[3] = PMY + 3;
  if (MapReadLimit[0] < 0) {
    CDX = 15 * (-MapReadLimit[0]);
  } else {
    CDX = 0;
  }
  if (MapReadLimit[2] < 0) {
    CDY = 15 * (-MapReadLimit[2]);
  } else {
    CDY = 0;
  }
  DX = CDX;
  DY = CDY;
  for (byte i = 0; i < 4; i++) {
    if (MapReadLimit[i] < 0) {
      MapReadLimit[i] = 0;
    } else if (MapReadLimit[i] > 15) {
      MapReadLimit[i] = 15;
    }
  }
  byte CCDX, CCDY;
  for (byte MapReadY = MapReadLimit[2]; MapReadY <= MapReadLimit[3]; MapReadY++) {
    for (byte MapReadX = MapReadLimit[0]; MapReadX <= MapReadLimit[1]; MapReadX++) {
      CCDX = Entity[0][0] - PMX * 16;
      CCDY = Entity[0][1] - PMY * 16;
      switch (pgm_read_byte(&MAP[ROOM][MapReadY][MapReadX])) {
        case 1:
          arduboy.fillRect(DX - CCDX, DY - CCDY, 16, 16, 1);
          break;
        case 2:
          arduboy.drawBitmap(DX - CCDX, DY - CCDY, Block[map(player_dyn, 0, 2, 0, 1) + !WOOPS] , 16, 16, 1); //动态水
          break;
        case 28:
          arduboy.drawBitmap(DX - CCDX, DY - CCDY, Block[random(0, BNUM - 1)] , 16, 16, 1);
          break;
        case 34:
          arduboy.drawBitmap(DX - CCDX, DY - CCDY, Block[map(player_dyn, 0, 2, 0, 1) + 33] , 16, 16, 1);
          break;
        case 46:
          arduboy.fillRect(DX - CCDX, DY - CCDY, 16, 16, 1);
          break;
        case 51:
          arduboy.fillRect(DX - CCDX, DY - CCDY, 16, 16, 1);
          arduboy.drawFastHLine(DX - CCDX, DY - CCDY + 2, 16, 0);
          arduboy.drawFastHLine(DX - CCDX, DY - CCDY + 4, 16, 0);
          break;
        default :
          arduboy.drawBitmap(DX - CCDX, DY - CCDY, Block[pgm_read_byte(&MAP[ROOM][MapReadY][MapReadX]) - 1], 16, 16, 1);
          break;
      }
      DX += 16;
    }
    DX = CDX;
    DY += 16;
  }
}
/*
    回响
*/
void draw_Reverberation(int x, int y)
{
  for (byte cr = 1; cr < rbcr; cr += 1) {
    drawCircle(x, y, cr, 0, cr * (255 / rbcr));
  }
  PMX = Entity[0][0] / 16;
  PMY = Entity[0][1] / 16;
  if (Entity[0][0] / 16 == Entity[1][0] / 16 && Entity[0][1] / 16 == Entity[1][1] / 16) rbcr += 4;
}
/*
   玩家
*/
void draw_player(byte x, byte y)
{
  //玩家皮肤
  arduboy.drawBitmap(x , y, T_Man_direction[PlayerD * 2 + player_move][player_dyn], 16, 16, 0);
  //眨眼控制
  if (PlayerD != 0) {
    y += 4;
    switch (PlayerD) {
      case 1:
        x += 6;
        break;
      case 2:
        x += 3;
        break;
      case 3:
        x += 10;
        break;
    }
    for (byte i = 0; i <= 1; i++) {
      arduboy.fillRect(x + 2 * i, y, 1, 2, 1);
    }
    if (millis() >= BlinkEyesTime + Timer[4] && player_dyn == 0 || !BEF) {
      if (player_dyn == 0) BEF = false;
      //arduboy.drawPixel
      for (byte i = 0; i <= 1; i++) {
        arduboy.fillRect(x + 2 * i, y, 1, abs(int(2 - player_dyn)), 0);
      }
      if (player_dyn == 2) {
        Timer[4] = millis();
        BEF = true;
      }
    }
  }
  /*
    下一动态帧
  */
  if (millis() >= mobile_frame_time + Timer[0]) {  //移动帧时间
    Timer[0] = millis();   //重置移动帧计时器
    player_dyn++; //下一个动态帧
    if (player_dyn >= 4) {
      player_dyn = 0;
    }
  }
}
/*
   游戏内信息菜单 持续B键调出
*/
void InfoMenu()
{
  KarmaCutscenes(1);
  while (KeyBack == 5) {
    key();
    DrawKarma(0);


    arduboy.display();
  }
}
/*
   扫描按键
*/
void key()
{
  if (millis() >= key_cool_time + Timer[1]) {
    Timer[1] = millis();   //重置移动帧计时器
    /*
        0  1  2  3  4  5
        ↑ ↓← →  A  B
    */
    KeyBack = 255;
    if (arduboy.pressed(UP_BUTTON)) {
      KeyBack = 0;
    }
    if (arduboy.pressed(DOWN_BUTTON)) {
      KeyBack = 1;
    }
    if (arduboy.pressed(LEFT_BUTTON)) {
      KeyBack = 2;
    }
    if (arduboy.pressed(RIGHT_BUTTON)) {
      KeyBack = 3;
    }
    if (arduboy.pressed(A_BUTTON)) {
      KeyBack = 4;
    }
    if (arduboy.pressed(B_BUTTON)) {
      KeyBack = 5;
    }
    if (arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON)) {
      KeyBack = 6;
    }
  }
}

/*
   传送系统
*/
void TP(byte TPRoom, byte x, byte y, byte TD) {
  ROOM = TPRoom;
  PMX = x / 16;
  PMY = y / 16;
  Entity[0][0] = x;
  Entity[0][1] = y;
  PlayerD = TD;
  MBlur();
}
/*
   障碍物检查
*/
void SBDP()
{
  //设置前一格方向向量
  switch (PlayerD) {
    case 0:
      CPDX = 0;
      CPDY = -1;
      break;
    case 1:
      CPDX = 0;
      CPDY = 1;
      break;
    case 2:
      CPDX = -1;
      CPDY = 0;
      break;
    case 3:
      CPDX = 1;
      CPDY = 0;
      break;
  }
  LA = true;
  if (Entity[0][0] + CPDX < 0 || Entity[0][0] + CPDX >= 247 || Entity[0][1] + CPDY < 0 || Entity[0][1] + CPDY >= 247) {
    LA = false;
  } else {
    if (Entity[0][0] / 16 != ((Entity[0][0] + CPDX) / 16) || Entity[0][1] / 16 != ((Entity[0][1] + CPDY) / 16) ) {
      byte length = sizeof(SBDPL) / sizeof(SBDPL[0]);
      for (byte i = 0; i < length; i++) {
        if (pgm_read_byte(&MAP[ROOM][(Entity[0][1] + 8  * CPDY) / 16][(Entity[0][0] + 8 * CPDX) / 16]) == SBDPL[i] && ROOM < 245) {
          LA = false;
        }
        // Serial.println(F("LA False"));
        //  Serial.println((Entity[0][0] + 4CPDX) / 16 );
        // Serial.println((Entity[0][1] + 4CPDY) / 16 );
        // Serial.println(pgm_read_byte(&MAP[ROOM][(Entity[0][1] + CPDY) / 16][(Entity[0][0] + CPDX) / 16]));
        //  Serial.println(length);
      }
    }
  }
}
/*
   加强版arduboy画圆
*/
void drawCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color, uint8_t GC)
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  if (random(0, 255) >= GC) arduboy.drawPixel(x0, y0 + r, color);
  if (random(0, 255) >= GC) arduboy.drawPixel(x0, y0 - r, color);
  if (random(0, 255) >= GC) arduboy.drawPixel(x0 + r, y0, color);
  if (random(0, 255) >= GC)  arduboy.drawPixel(x0 - r, y0, color);

  while (x < y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    if (random(0, 255) >= GC) arduboy.drawPixel(x0 + x, y0 + y, color);
    if (random(0, 255) >= GC) arduboy.drawPixel(x0 - x, y0 + y, color);
    if (random(0, 255) >= GC) arduboy.drawPixel(x0 + x, y0 - y, color);
    if (random(0, 255) >= GC) arduboy.drawPixel(x0 - x, y0 - y, color);
    if (random(0, 255) >= GC) arduboy.drawPixel(x0 + y, y0 + x, color);
    if (random(0, 255) >= GC) arduboy.drawPixel(x0 - y, y0 + x, color);
    if (random(0, 255) >= GC) arduboy.drawPixel(x0 + y, y0 - x, color);
    if (random(0, 255) >= GC) arduboy.drawPixel(x0 - y, y0 - x, color);
  }
}
/*
  void drawText(uint8_t x, uint8_t y, const uint8_t *mes, uint8_t cnt, bool dda)
  {
  uint8_t pb;
  uint8_t page;
  uint8_t screen_start = 0;
  uint8_t screen_end = 128;
  if (x < screen_start) {
    x = screen_start;
  }
  for (uint8_t i = 0; i < cnt; i++) {
    pb = pgm_read_byte_near(mes + i);
    switch (pb) {
      case MISAKI_FONT_F1_PAGE:
        page = MISAKI_FONT_F1_PAGE;
        continue;
      case MISAKI_FONT_F2_PAGE:
        page = MISAKI_FONT_F2_PAGE;
        continue;
    }
    if (!page) {
      continue;
    }
    arduboy.fillRect(x, y - 1, 8, 9, 1); //白底
    switch (page) {
      case MISAKI_FONT_F1_PAGE:
        if (pb > MISAKI_FONT_F1_SIZE) {
          continue;
        }
        if ((x + 4) > screen_end) {
          x = screen_start;
          y = y + 8;
        }
        arduboy.drawBitmap(x, y,  misaki_font_f1[ pb ], MISAKI_FONT_F1_W, 8, 0);
        arduboy.drawBitmap(x + 3, y, misaki_font_0x00, 1, 8, 0);
        x = x + 4;
        break;
      case MISAKI_FONT_F2_PAGE:
        if (pb > MISAKI_FONT_F2_SIZE) {
          continue;
        }
        if ((x + 8) > screen_end) {
          x = screen_start;
          y = y + 8;
        }

        arduboy.drawBitmap(x, y,  misaki_font_f2[ pb ], MISAKI_FONT_F2_W, 8, 0);
        arduboy.drawBitmap(x + 7, y, misaki_font_0x00, 1, 8, 0);
        x = x + 8;
        break;
    }
    if ((x + 8) > screen_end) {
      x = screen_start;
      y = y + 8;
    }
    if (dda) drawDownArrow(x, y);
  }
  }
*/
void drawDownArrow(byte ddax, byte dday) {
  arduboy.setCursor(ddax, dday + player_dyn);
  arduboy.print(char(31));
}
void SetTextColor(bool color) {
  arduboy.setTextColor(color);
  arduboy.setTextBackground(!color);
}
void drawFPS()
{
  arduboy.println(SFPS);
  if (millis() >= FPST + Timer[3]) {
    Timer[3] = millis();
    SFPS = FPS;
    FPS = 0;
  }
}
/*
  union ESaveData
  {
  byte Karma;
  int Entity[1][3];
  byte MesF[30];
  };
*/
void Esave() {
  int EAddress = 0;
  for (byte i = 0; i < 5; i++) {
    EEPROM.update(EAddress, pgm_read_byte(&mes55[i]));  //写入署名数据
    EAddress++;
  }
  EEPROM.update(5, Karma); //业力
  EWUint(6, Entity[0][0]);
  EWUint(8, Entity[0][1]);
  EEPROM.update(10, ROOM);
  /*
    for (byte i = EAddress; i < EAddress + MesFL; i++) {
    EEPROM.update(i, MesF[i - 6]);
    }
    EAddress += MesFL;
  */
}
void Eload() {
  byte EData;
  for (byte i = 0; i < 5; i++) {
    EData = EEPROM.read(i);
    if (i < 5 && EData != pgm_read_byte(&mes55[i])) ERst();
  }
  if (EEPROM.read(512) != EEPROM.read(513)) ERst();
  if (EEPROM.read(512) != 2 && EEPROM.read(512) != 4) {
    UpdateROM();
    }
  for (int i = 0; i < 1024; i++) {
    EData = EEPROM.read(i);
    switch (i) {
      case 5:
        Karma = EData;
        break;
    }
    /*
      Serial.print(F("# "));
      Serial.print(i);
      Serial.print(F(" : "));
      Serial.println(EData);
    */
  }
  Entity[0][0] = ERUint(6);
  Entity[0][1] = ERUint(8);
  ROOM = EEPROM.read(10);
}
int ERUint(byte address)
{
  uint ERDuint;
  for (byte i = 0; i < 2; i++) {
    ERDuint.b[i] = EEPROM.read(address + i);
    if (i == 1) {
      delay(1);
      return ERDuint.a;
    }
  }
}
void EWUint(byte address, int RSint)
{
  uint ERDuint;
  for (byte i = 0; i < 2; i++) {
    ERDuint.a = RSint;
    EEPROM.update(address + i, ERDuint.b[i]);
  }
}
void drawOOPS() {
  arduboy.clear();
  for (byte y = 0; y < 2; y++) {
    for (byte x = 0; x < 4; x++) arduboy.drawBitmap(x * 32, y * 32, OOPS, 32, 32, 1);
  }
  arduboy.display();
}

void ERst() {
  drawOOPS();
  arduboy.display();
  delay(1000);
  SetTextColor(0);
  arduboy.println(F("SAV CHECK ERROR!"));
  SetTextColor(1);
  for (byte i = 0; i < 5; i++) {
    arduboy.println( byte(EEPROM.read(i)) + String("!=") + pgm_read_byte(&mes55[i]));
  }
  arduboy.display();
  delay(5000);
  arduboy.clear();
  drawOOPS();
  SetTextColor(0);
  arduboy.println(F("CLEAR EEPROM"));
  arduboy.display();
  for (int i = 0; i < 1024; i++) {
    EEPROM.update(i, 0);
  }
  arduboy.println(F("SETUP EEPROM"));
  arduboy.display();
  Esave();
  EEPROM.update(5, 2); //业力
  EWUint(6, 191);  //玩家x
  EWUint(8, 32);   //玩家y
  EEPROM.update(10, 11);  //载入地图
  EEPROM.update(512, 1); //游戏状态
  EEPROM.update(513, 1);
  arduboy.println(F("REBOOT"));
  arduboy.display();
  delay(1500);
  while (1) {
    resetFunc();
  }
}
/*
   提示更新固件
*/
void UpdateROM() {
  arduboy.clear();
  //arduboy.invert(1);
  arduboy.fillRect(0, 0, 128, 64, 1);
  arduboy.drawBitmap(55, 7, Block_37, 16, 16, 0);
  arduboy.setCursor(4, 39);
  arduboy.print(F("Please upload Part"));
  arduboy.setCursor(120, 39);
  arduboy.print(EEPROM.read(512));
  arduboy.display();
  while (1) {}
}
void PrintMes(byte MesNum) {
  arduboy.fillRect(0, 0, 128, 64, 1);
  arduboy.setCursor(0, 48);
  bool ie = false;
  byte i = 0;
  for (byte y = 0; y < 4; y++) {
    for (byte x = 0; x < 21; x++) {
      arduboy.setCursor(x * 6, y * 8);
      if (char(pgm_read_byte_near(MES[MesNum] + i)) == 10) ie = true;
      arduboy.print(char(pgm_read_byte_near(MES[MesNum] + i)));
      i++;
      if (ie) break;
    }
    arduboy.println("");
    if (ie) break;
  }
  arduboy.display();
}