/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			Delay函数的设计
 *
 *
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.07      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/
#ifndef _DRIVER_DELAY_H
#define _DRIVER_DELAY_H

#include <stdint.h>

void udelay(int us);
void mdelay(int ms);
uint64_t system_get_ns(void);


#endif /* _DRIVER_TIMER_H */

