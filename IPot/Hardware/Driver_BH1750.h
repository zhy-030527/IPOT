/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			BH1750光照传感器初始化配置
 *
 *
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.07      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#ifndef _DRIVER_BH1750_H
#define _DRIVER_BH1750_H

#include <stdint.h>
#include <stdio.h>

//电源与复位指令
#define		BH1750_Power_Down				0x00		//掉电模式
#define		BH1750_Power_ON					0x01		//上电等待测量命令
#define		BH1750_Reset					0x07		//复位数据寄存器

//连续测量模式
#define		BH1750_H_Resolution_Mode		0x10		//(高分辨率 1 lx)
#define		BH1750_H_Resolution_Mode2		0x11		//(高分辨率 0.5 lx)
#define		BH1750_L_Resolution_Mode		0x13		//(低分辨率 4 lx)

//单次测量模式
#define		BH1750_OH_Resolution_Mode		0x20		//(高分辨率 1 lx)
#define		BH1750_OH_Resolution_Mode2		0x21        //(高分辨率 0.5 lx)
#define		BH1750_OL_Resolution_Mode		0x23        //(低分辨率 4 lx)

#define 	BH1750_TIMEOUT     				500
#define 	BH1750_I2C_ADDR					(0x23<<1)		//寄存器地址


void BH1750_Test(void *params);
int BH1750_Init(void);

#endif

