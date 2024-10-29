
#include "hmi_common.h"
#include "lcd12864_drv.h"

#define MENU_POS_X   7
#define MENU_POS_y   7
void DisplayMenuKey(void)
{

  LcdDrawString(MENU_POS_X,MENU_POS_y,(unsigned char *)"Menu");  
}


