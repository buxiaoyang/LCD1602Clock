/***************************************************************************/
//LCD1602单片机万年历（带闹钟功能）
//作者：卜晓旸
//时间：2011年5月8日
//版本：1.0
/***************************************************************************/
#include<reg52.h>

#define uint unsigned int
#define uchar unsigned char

//LCD1602引脚定义
sbit rs=P2^7;   
sbit rd=P2^6;
sbit lcdcs=P2^5;
sbit lcdbacklight1 = P2^1;
sbit lcdbacklight0 = P2^0;
uchar backlight = 2;
//DS18B20定义
uchar count,s1num,tempint,f,tempth,settemp; //温度整数部分和小数部分;
int tempdf,c;
sbit TMDAT=P1^6; //DS18B20数据线
//DS1302定义
uchar nian,yue,ri,shi,fen,miao,zhou;
sbit ACC0=ACC^0;
sbit ACC7=ACC^7;
sbit T_RST=P2^2;//ds1302-5
sbit T_IO=P2^3;//ds1302-6
sbit T_CLK=P2^4;//ds1302-7
//显示缓冲区
uchar lcd1602_one[]="2011-05-08 6 MT1";
uchar lcd1602_two[]=" 11:08:23  30^C ";
//运行状态标志位
uchar status = 0; //0:正常运行 1:年调节 2:月调节 3:日调节 4:周调节 
				      //5:时调节 6:分调节 7:秒调节 8:闹钟设置是否开启 9:闹钟时设置
						//10:闹钟分设置 11:logo第一位设置 12:logo第二位设置 13:logo第三位设置
						//20:保存设置 21:闹钟模式
//按键定义
sbit MODE = P3^2;
sbit UP = P3^3;
sbit DOWN = P3^4;
//每月天数定义 用于日期调整
code uchar days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
//按键按下时当前秒数 用于长时间没有设置操作时时钟进入正常走时状态
uchar key_click_sec,now_sec;
//闹钟设置显示缓冲区
uchar alarm_on[] = "OFF";
uchar alarm_time[]="07:10";
//闹钟设置缓冲区
code uchar alarm_set_one[]="Alarm:";
code uchar alarm_set_two[]=" Logo:";
//闹钟显示缓冲区
code uchar alarm_one[]="*****??:??******";
code uchar alarm_two[]="*****Alarm******";
//闹钟用变量定义
sbit BEEP = P1^4;
code uint cyc[]={1800,1600,1440,1351,1200,1079,960};//音阶1-7的半周期数
code uchar tone[]={13,15,16,16,15,16,13,12,12,13,15,16,16,15,16,13,13, 13,15,16,
16,15,16,13,12,12,15,13,12,13,12,11,12,6,6,12,15,13,12,6,6,15,13,12,13,12,11,
12,6,5,6,0xff};// 乐曲《康定情歌》的简谱表
code uchar time[]={8,8,8,4,4,8,8,12,4,8,8,8,4,4,8,16,8,8,8,8,4,4,8,8,12,4,8,8,4,
4,4,4,8,24,8,24,8,24,8,16,8,8,8,4,4,4,4,8,16,8,32};// 节拍表
uchar H0,L0,cnt;

/***************延时函数******************/ 
void delay(uint ms)
{
	uint i,j;
	for (j=0;j<ms;j++)
		for (i=0;i<120;i++);
}

//*************************LCD1602相关函数************************************
//****************************************************************************

/***************LCD1602写命令函数******************/ 
void write_command(uchar command)
{
	rs=0;
	P0=command;
	lcdcs=1;
	lcdcs=0;
}
/***************LCD1602写数据函数******************/ 
void write_data(uchar data0)
{
	rs=1;
	P0=data0;
	lcdcs=1;
	lcdcs=0;	
}
/***************初始化LCD1602函数******************/ 
void init_1602()
{
	rd=0;
	delay(15);
	write_command(0x38);
	delay(5);
	write_command(0x0c);
	delay(5);
	write_command(0x06);
	delay(5);
} 
/***************LCD1602时钟显示函数******************/ 
void display_1602()
{
	uchar j;
	write_command(0x80); //液晶显示位置
	delay(5);
	for (j=0;j<sizeof(lcd1602_one)-1;j++)
	{
		write_data(lcd1602_one[j]);
		delay(5);
	} 	
	write_command(0x80+0x40 ); //液晶显示位置
	delay(5);
	for (j=0;j<sizeof(lcd1602_two)-1;j++)
	{
		write_data(lcd1602_two[j]);
		delay(5);
	} 
}
/***************LCD1602闹钟设置显示函数******************/ 
void display_1602_alarm_set()
{
	uchar j;
	write_command(0x80); //液晶显示位置
	delay(5);
	for (j=0;j<sizeof(alarm_set_one)-1;j++)
	{
		write_data(alarm_set_one[j]);
		delay(5);
	} 	
	for (j=0;j<sizeof(alarm_on)-1;j++)
	{
		write_data(alarm_on[j]);
		delay(5);
	}
	write_data(' ');
	delay(5);
	for (j=0;j<sizeof(alarm_time)-1;j++)
	{
		write_data(alarm_time[j]);
		delay(5);
	} 	
	write_data(' ');
	delay(5);	
	write_command(0x80+0x40 ); //液晶显示位置
	delay(5);
	for (j=0;j<sizeof(alarm_set_two)-1;j++)
	{
		write_data(alarm_set_two[j]);
		delay(5);
	} 
	write_data(lcd1602_one[13]);
	delay(5);
	write_data(lcd1602_one[14]);
	delay(5);
	write_data(lcd1602_one[15]);
	delay(5);
	for (j=0;j<7;j++)
	{
		write_data(' ');
		delay(5);
	} 	
}
/***************LCD1602闹钟显示函数******************/ 
void display_1602_alarm()
{
	uchar j;
	write_command(0x80); //液晶显示位置
	delay(5);
	for (j=0;j<5;j++)
	{
		write_data(alarm_one[j]);
		delay(5);
	}
	for (j=0;j<sizeof(alarm_time)-1;j++)
	{
		write_data(alarm_time[j]);
		delay(5);
	} 
	for (j=10;j<sizeof(alarm_one)-1;j++)
	{
		write_data(alarm_one[j]);
		delay(5);
	}
 	
	write_command(0x80+0x40); //液晶显示位置
	delay(5);
	for (j=0;j<sizeof(alarm_two)-1;j++)
	{
		write_data(alarm_two[j]);
		delay(5);
	} 
}

//*************************DS18B20相关函数************************************
//****************************************************************************

/***************初始化DS18B20子程序******************/ 
void set_ds18b20()
{
	while(1)
	{ 
		uchar dela,flag;
		flag=0;
		TMDAT=1;
		dela=1;
		while(--dela);
		TMDAT=0; //数据线置低电平
		dela=250;
		while(--dela); //低电平保持500us
		TMDAT=1; //数据线置高电平
		dela=30;
		while(--dela); //高电平保持60us
		while(TMDAT==0) //判断DS18B20是否发出低电平信号
		{
			dela=210; //DS18B20响应，延时420us
			while(--dela);
			if(TMDAT) //DS18B20发出高电平初始化成功，返回
			{
				flag=1; //DS18B20初始化成功标志		
				break;
			}
		}
		if(flag) //初始化成功，再延时480us，时序要求
		{
			dela=240;
			while(--dela);
			break;
		}
	}
}
/***************读DS18B20子程序******************/ 
void read_ds18b20()
{
	uchar dela,i,j,k,temp,temph,templ;
	j=3; //读2位字节数据
	do
	{
		for(i=8;i>0;i--) //一个字节分8位读取
		{
			temp>>=1; //读取1位右移1位
			TMDAT=0; //数据线置低电平
			dela=1;
			while(--dela);
			TMDAT=1; //数据线置高电平
			dela=4;
			while(--dela); //延时8us
			if(TMDAT) //读取1位数据
				temp|=0x80;		    
			dela=25; //读取1位数据后延时50us
			while(--dela);
		}
		if(j==3)                      
			templ=temp; //读取的第一字节存templ
		if(j==2) 
		  	temph=temp; //读取的第二字节存temph
		if(j==1)  
		  	tempth=temp; //读取的第3字节存tempth   TH的值         
	}while(--j);
	f=0;
	if((temph&0xf8)!=0x00) //若温度为负的处理，对二进制补码的处理
	{
		f=1; //为负温度f置1
		temph=~temph;
		templ=~templ;        
		k=templ+1;
		templ=k;
		if(k>255)
		{
			temph++;
		}
	}
	tempdf=templ&0x0f; //将读取的数据转换成温度值，整数部分存tempint,小数部分存tempdf
	c=(tempdf*625);
	tempdf=c;
	templ>>=4;
	temph<<=4;
	tempint=temph|templ; //两字节合并为一个字节
}
/***************写DS18B20子程序******************/ 
void write_ds18b20(uchar command)
{
	uchar dela,i;
	for(i=8;i>0;i--) //将一字节数据一位一位写入
	{
		TMDAT=0; //数据线置低电平
		dela=6; //延时12us
		while(--dela);
		TMDAT=command&0x01; //将数据放置在数据线上
		dela=25; //延时50us
		while(--dela);
		command=command>>1; //准备发送下一位数据
		TMDAT=1; //发送完一位数据，数据线置高电平
	}
}
/**********DS18B20获得温度 更新显示缓冲区********/ 
void get_temperature()
{
	set_ds18b20(); //初始化DS18B20
	write_ds18b20(0xcc); //发跳过ROM匹配命令
	write_ds18b20(0x44); //发温度转换命令
	delay(5);
	set_ds18b20();
	write_ds18b20(0xcc); //发跳过ROM匹配命令
	write_ds18b20(0xbe); //发出读温度命令
	read_ds18b20(); //将读出的温度数据保存到tempint和tempdf处
	lcd1602_two[12] = 0x30+tempint%10;
	lcd1602_two[11] = 0x30+tempint%100/10;	 
}

//*************************DS1302相关函数*************************************
//****************************************************************************

/************DS1302：写入操作(上升沿)*******************/ 
void write_byte(uchar da)
{
	uchar i;
	ACC=da;
	for(i=8;i>0;i--)
	{ 
		T_IO=ACC0;
		T_CLK=0;     
		T_CLK=1;
		ACC=ACC>>1;
	}
}
/*************DS1302：读取操作（下降沿）*****************/
uchar read_byte(void)
{
	uchar i;
	for(i=0;i<8;i++)
	{
		ACC=ACC>>1;
		T_CLK = 1;
		T_CLK = 0;
		ACC7 = T_IO;
	}
	return(ACC);
}
/**********DS1302:写入数据（先送地址，再写数据）********/ 
void write_1302(uchar addr,uchar da)
{
	T_RST=0; //停止工作
	T_CLK=0;                                 
	T_RST=1; //重新工作
	write_byte(addr); //写入地址
	write_byte(da);
	T_RST=0;
	T_CLK=1;
}
/**********DS1302:读取数据（先送地址，再读数据）********/
uchar read_1302(uchar addr)
{
   uchar temp;
   T_RST=0; //停止工作
   T_CLK=0;  
   T_RST=1; //重新工作
   write_byte(addr); //写入地址
   temp=read_byte();
   T_RST=0;
   T_CLK=1; //停止工作
   return(temp);
}
/**********DS1302:获取当前温度 更新显示缓冲区***********/
void get_1302time()
{
	miao = read_1302(0x81);
	fen = read_1302(0x83);
	shi = read_1302(0x85);
	ri = read_1302(0x87);
	yue = read_1302(0x89);
	nian = read_1302(0x8d);
	zhou = read_1302(0x8b);
	lcd1602_two[8] = 0x30+miao%16;
   lcd1602_two[7] = 0x30+miao/16;	 
	lcd1602_two[5] = 0x30+fen%16;
   lcd1602_two[4] = 0x30+fen/16;	 
	lcd1602_two[2] = 0x30+shi%16;
   lcd1602_two[1] = 0x30+shi/16;	 
	lcd1602_one[9] = 0x30+ri%16;
   lcd1602_one[8] = 0x30+ri/16;	 
	lcd1602_one[6] = 0x30+yue%16;
   lcd1602_one[5] = 0x30+yue/16;	 
	lcd1602_one[3] = 0x30+nian%16;
   lcd1602_one[2] = 0x30+nian/16;
	lcd1602_one[11] = 0x30+zhou%16;	 
}

//*****************************中断处理函数***********************************
//****************************************************************************

/**********定时器0中断用于产生音阶方波***********/
void cntint0(void) interrupt 1 
{	
	TH0=H0;
	TL0=L0;
	BEEP=~BEEP; // BEEP是音乐信号输出脚，BEEP反相，产生方波
}
/**********定时器1中断用于产生节拍延时***********/
void cntint1(void) interrupt 3
{	
	cnt++; // 计数初值为0，所以不用赋值
}

//*****************************按键处理函数***********************************
//****************************************************************************

/**********按键扫描用于设置时间***********/
void time_set_key_scan()
{
	//年设置
	if(status ==1)
	{
		write_command(0x80 + 0x03); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_one[3] ++;
				if(lcd1602_one[3] > '9')
				{
					lcd1602_one[3] = '0';
					lcd1602_one[2] ++;
					if(lcd1602_one[2] > '9')
					{
						lcd1602_one[2] = '0';							
					}	
				}
				display_1602();
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_one[3] --;
				if(lcd1602_one[3] < '0')
				{
					lcd1602_one[3] = '9';
					lcd1602_one[2] --;
					if(lcd1602_one[2] < '0')
					{
						lcd1602_one[2] = '9';							
					}	
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//月设置
	else if(status ==2)
	{
		write_command(0x80 + 0x06); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_one[6] ++;
				if(lcd1602_one[6] > '9' && lcd1602_one[5] == '0')
				{
					lcd1602_one[6] = '0';
					lcd1602_one[5] ++;
				}
				if(lcd1602_one[6] > '2' && lcd1602_one[5] == '1')
				{
					lcd1602_one[6] = '1';
					lcd1602_one[5] = '0';
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_one[6] --;
				if(lcd1602_one[6] < '0' && lcd1602_one[5] == '1')
				{
					lcd1602_one[6] = '9';
					lcd1602_one[5] --;
				}
				if(lcd1602_one[6] < '1' && lcd1602_one[5] == '0')
				{
					lcd1602_one[6] = '2';
					lcd1602_one[5] = '1';
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}								
	}
	//日设置
	else if(status ==3)
	{
		uchar nowMonthDays = days[(lcd1602_one[5]-0x30)*10+(lcd1602_one[6]-0x30)-1];
		write_command(0x80 + 0x09); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_one[9] ++;
				if(lcd1602_one[9] > '9')
				{
					lcd1602_one[9] = '0';
					lcd1602_one[8] ++;
					if(lcd1602_one[8] > '9')
					{
						lcd1602_one[8] = '0';							
					}	
				}
				if((lcd1602_one[8]-0x30)*10+(lcd1602_one[9]-0x30) > nowMonthDays)
				{
					lcd1602_one[9] = '1';
					lcd1602_one[8] = '0';	
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_one[9] --;
				if(lcd1602_one[9] < '0')
				{
					lcd1602_one[9] = '9';
					lcd1602_one[8] --;
					if(lcd1602_one[8] < '0')
					{
						lcd1602_one[8] = '9';							
					}	
				}
				if((lcd1602_one[8]-0x30)*10+(lcd1602_one[9]-0x30) < 1)
				{
					lcd1602_one[9] = nowMonthDays%10 + 0x30;
					lcd1602_one[8] = nowMonthDays/10 + 0x30;;	
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//周设置
	else if(status ==4)
	{
		write_command(0x80 + 0x0b); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_one[11] ++;
				if(lcd1602_one[11] > '7')
				{
					lcd1602_one[11] = '1';
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_one[11] --;
				if(lcd1602_one[11] < '1')
				{
					lcd1602_one[11] = '7';
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//时设置
	else if(status ==5)
	{
		write_command(0x80 + 0x40 + 0x02); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_two[2] ++;
				if(lcd1602_two[2] > '9')
				{
					lcd1602_two[2] = '0';
					lcd1602_two[1] ++;
					if(lcd1602_two[1] > '9')
					{
						lcd1602_two[1] = '0';							
					}	
				}
				if((lcd1602_two[1]-0x30)*10+(lcd1602_two[2]-0x30) > 23)
				{
					lcd1602_two[2] = '0';
					lcd1602_two[1] = '0';	
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_two[2] --;
				if(lcd1602_two[1] != '0' && lcd1602_two[2] < '0')
				{
					lcd1602_two[2] = '9';
					lcd1602_two[1] --;
					if(lcd1602_two[1] < '0')
					{
						lcd1602_two[1] = '9';							
					}	
				}
				if(lcd1602_two[1] == '0' && lcd1602_two[2] < '0')
				{
					lcd1602_two[2] = '3';
					lcd1602_two[1] = '2';	
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//分设置
	else if(status ==6)
	{
		write_command(0x80 + 0x40 + 0x05); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_two[5] ++;
				if(lcd1602_two[5] > '9')
				{
					lcd1602_two[5] = '0';
					lcd1602_two[4] ++;
					if(lcd1602_two[4] > '9')
					{
						lcd1602_two[4] = '0';							
					}	
				}
				if((lcd1602_two[4]-0x30)*10+(lcd1602_two[5]-0x30) > 59)
				{
					lcd1602_two[5] = '0';
					lcd1602_two[4] = '0';	
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_two[5] --;
				if(lcd1602_two[4] != '0' && lcd1602_two[5] < '0')
				{
					lcd1602_two[5] = '9';
					lcd1602_two[4] --;
					if(lcd1602_two[4] < '0')
					{
						lcd1602_two[4] = '9';							
					}	
				}
				if(lcd1602_two[4] == '0' && lcd1602_two[5] < '0')
				{
					lcd1602_two[5] = '9';
					lcd1602_two[4] = '5';	
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//秒设置
	else if(status ==7)
	{
		write_command(0x80 + 0x40 + 0x08); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_two[8] ++;
				if(lcd1602_two[8] > '9')
				{
					lcd1602_two[8] = '0';
					lcd1602_two[7] ++;
					if(lcd1602_two[7] > '9')
					{
						lcd1602_two[7] = '0';							
					}	
				}
				if((lcd1602_two[7]-0x30)*10+(lcd1602_two[8]-0x30) > 59)
				{
					lcd1602_two[8] = '0';
					lcd1602_two[7] = '0';	
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_two[8] --;
				if(lcd1602_two[7] != '0' && lcd1602_two[8] < '0')
				{
					lcd1602_two[8] = '9';
					lcd1602_two[7] --;
					if(lcd1602_two[7] < '0')
					{
						lcd1602_two[7] = '9';							
					}	
				}
				if(lcd1602_two[7] == '0' && lcd1602_two[8] < '0')
				{
					lcd1602_two[8] = '9';
					lcd1602_two[7] = '5';	
				}
				display_1602();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
}
/**********按键扫描用于设置闹钟***********/
void alarm_key_scan()
{
	//闹钟设置模式
	if(status == 8)
	{
		write_command(0x80 + 0x08); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				if(alarm_on[2] == 'N') //闹钟开
				{
					alarm_on[0] = 'O';	
					alarm_on[1] = 'F';	
					alarm_on[2] = 'F';	
				}
				else
				{
					alarm_on[0] = ' ';	
					alarm_on[1] = 'O';	
					alarm_on[2] = 'N';						
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				if(alarm_on[2] == 'N') //闹钟开
				{
					alarm_on[0] = 'O';	
					alarm_on[1] = 'F';	
					alarm_on[2] = 'F';	
				}
				else
				{
					alarm_on[0] = ' ';	
					alarm_on[1] = 'O';	
					alarm_on[2] = 'N';						
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//闹钟时设置
	else if(status == 9)
	{
		write_command(0x80 + 0x0b); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				alarm_time[1] ++;
				if(alarm_time[1] > '9')
				{
					alarm_time[1] = '0';
					alarm_time[0] ++;
					if(alarm_time[0] > '9')
					{
						alarm_time[0] = '0';							
					}	
				}
				if((alarm_time[0]-0x30)*10+(alarm_time[1]-0x30) > 23)
				{
					alarm_time[1] = '0';
					alarm_time[0] = '0';	
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				alarm_time[1] --;
				if(alarm_time[0] != '0' && alarm_time[1] < '0')
				{
					alarm_time[1] = '9';
					alarm_time[0] --;
					if(alarm_time[0] < '0')
					{
						alarm_time[0] = '9';							
					}	
				}
				if(alarm_time[0] == '0' && alarm_time[1] < '0')
				{
					alarm_time[1] = '3';
					alarm_time[0] = '2';	
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//闹钟分设置
	else if(status == 10)
	{
		write_command(0x80 + 0x0e); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				alarm_time[4] ++;
				if(alarm_time[4] > '9')
				{
					alarm_time[4] = '0';
					alarm_time[3] ++;
					if(alarm_time[3] > '9')
					{
						alarm_time[3] = '0';							
					}	
				}
				if((alarm_time[3]-0x30)*10+(alarm_time[4]-0x30) > 59)
				{
					alarm_time[4] = '0';
					alarm_time[3] = '0';	
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				alarm_time[4] --;
				if(alarm_time[3] != '0' && alarm_time[4] < '0')
				{
					alarm_time[4] = '9';
					alarm_time[3] --;
					if(alarm_time[3] < '0')
					{
						alarm_time[3] = '9';							
					}	
				}
				if(alarm_time[3] == '0' && alarm_time[4] < '0')
				{
					alarm_time[4] = '9';
					alarm_time[3] = '5';	
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}	
}
/**********按键扫描用于设置logo***********/
void logo_key_scan()
{
	//logo第一位设置
	if(status == 11)
	{
		write_command(0x80 + 0x40 + 0x06); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_one[13] ++;
				if(lcd1602_one[13] > 'z')
				{
					lcd1602_one[13] = '0';
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_one[13] --;
				if(lcd1602_one[13] < '0')
				{
					lcd1602_one[13] = 'z';
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}	
	//logo第二位设置
	else if(status == 12)
	{
		write_command(0x80 + 0x40 + 0x07); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_one[14] ++;
				if(lcd1602_one[14] > 'z')
				{
					lcd1602_one[14] = '0';
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_one[14] --;
				if(lcd1602_one[14] < '0')
				{
					lcd1602_one[14] = 'z';
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}	
	//logo第三位设置
	else if(status == 13)
	{
		write_command(0x80 + 0x40 + 0x08); //光标显示位置
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP按键处理
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				lcd1602_one[15] ++;
				if(lcd1602_one[15] > 'z')
				{
					lcd1602_one[15] = '0';
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN按键处理
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				lcd1602_one[15] --;
				if(lcd1602_one[15] < '0')
				{
					lcd1602_one[15] = 'z';
				}
				display_1602_alarm_set();			
				//获取当前时钟秒数
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}	
}
/**********按键扫描用于设置模式调节***********/
void mode_key_scan()
{ 	
	if(MODE==0)
	{
		delay(10);
		if(MODE==0)
		{
			status++;
			if(status>7 && status<11)
			{
				display_1602_alarm_set();
			}
			if(status>13)
			{
				status = 20;
			}
			//获取当前时钟秒数
			key_click_sec = read_1302(0x81);
			key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);
		}
		while(!MODE);	
	}								
	if(status == 0)
	{
		//DOWN按键处理 用于背光控制
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				backlight ++;
				if(backlight > 2)
				{
					backlight = 0;
				}				
				if(backlight == 0)
				{
					lcdbacklight0 = 1;				
					lcdbacklight1 = 1;
				}
				else if(backlight == 1)
				{
					lcdbacklight0 = 0;				
					lcdbacklight1 = 1;				
				}
				else
				{
					lcdbacklight0 = 1;				
					lcdbacklight1 = 0;				
				}
			}
			while(!UP);	
		}						
	}
}

//*************************主函数*********************************************
//****************************************************************************
void main()
{
	//闹钟用变量定义
	uchar  i,j,a,t;
	uint b;
	//将液晶显示^C改为摄氏符号
	lcd1602_two[13] = 0xdf;
	//初始化液晶
	init_1602();
	//初始化DS1302
	if(read_1302(0xff) == 0xaa)//1302未被初始化
	{	
		//读取alarm值
		alarm_time[4] = 0x30 + read_1302(0xd1)%16;
		alarm_time[3] = 0x30 + read_1302(0xd1)/16;
		alarm_time[1] = 0x30 + read_1302(0xd3)%16;
		alarm_time[0] = 0x30 + read_1302(0xd3)/16;
		if(read_1302(0xc5) == 0x00)// 闹钟关
		{
			alarm_on[0] = 'O';
			alarm_on[1] = 'F';
			alarm_on[2] = 'F';
		}
		else
		{
			alarm_on[0] = ' ';
			alarm_on[1] = 'O';
			alarm_on[2] = 'N';	
		}
		lcd1602_one[13] = read_1302(0xc7);
		lcd1602_one[14] = read_1302(0xc9);
		lcd1602_one[15] = read_1302(0xcb);
	}
	else
	{
		write_1302(0x8e,0x00); //WP=0 写操作
		write_1302(0x90,0x00); //关闭充电功能
		write_1302(0x80,0x00);// miao
		write_1302(0x82,0x27);// fen
		write_1302(0x84,0x20);// shi
		write_1302(0x86,0x09);// ri
		write_1302(0x88,0x05);// yue
		write_1302(0x8c,0x11);// nian
		write_1302(0x8a,0x01);// zhou
		write_1302(0x8e,0x80); //WP=1 写保护	
	}
	lcdbacklight0 = 1;				
	lcdbacklight1 = 0;				
	while(1)
	{
		//扫描mode按键有没有被按下
		mode_key_scan();
		//用户一段时间没有操作时恢复正常走时
		if(status != 0 && status!= 20 && status!= 21)
		{
			now_sec = read_1302(0x81);
			now_sec = (now_sec/16)*10+(now_sec%16);	
			if(now_sec < key_click_sec)
			{
				now_sec += 60;		
			}
			if(now_sec - key_click_sec > 30)
			{
				status = 20;		
			}		
		}
		//正常走时状态
		if(status == 0)
		{
			get_temperature();
			get_1302time();
			display_1602();
			//检测闹钟
			if(alarm_on[2] == 'N') //闹钟开
			{
				if(lcd1602_two[1]==alarm_time[0]
					&&lcd1602_two[2]==alarm_time[1]
					&&lcd1602_two[4]==alarm_time[3]
					&&lcd1602_two[5]==alarm_time[4]
					&&lcd1602_two[7]=='0'
					&&lcd1602_two[8]=='0') //闹钟时间到
				{
					status = 21;							
				}
			}
		}
		//保存设置
		else if(status ==20)
		{
			//写入时间到DS1302
			write_1302(0x8e,0x00);//WP=0 写操作
			write_1302(0x80,(lcd1602_two[7]-0x30)*16+(lcd1602_two[8]-0x30));// miao
			write_1302(0x82,(lcd1602_two[4]-0x30)*16+(lcd1602_two[5]-0x30));// fen
			write_1302(0x84,(lcd1602_two[1]-0x30)*16+(lcd1602_two[2]-0x30));// shi
			write_1302(0x86,(lcd1602_one[8]-0x30)*16+(lcd1602_one[9]-0x30));// ri
			write_1302(0x88,(lcd1602_one[5]-0x30)*16+(lcd1602_one[6]-0x30));// yue
			write_1302(0x8c,(lcd1602_one[2]-0x30)*16+(lcd1602_one[3]-0x30));// nian
			write_1302(0x8a,lcd1602_one[11]-0x30);// zhou
			write_1302(0xd0,(alarm_time[3]-0x30)*16+(alarm_time[4]-0x30));// alarm fen
			write_1302(0xd2,(alarm_time[0]-0x30)*16+(alarm_time[1]-0x30));// alarm shi
			if(alarm_on[2] == 'F')
			{
				write_1302(0xc4,0x00);// alarm is_off
			}
			else
			{
				write_1302(0xc4,0x01);// alarm is_on
			}
			write_1302(0xc6,lcd1602_one[13]);// logo 1
			write_1302(0xc8,lcd1602_one[14]);// logo 2
			write_1302(0xca,lcd1602_one[15]);// logo 3
			write_1302(0xfe,0xaa);// 判断1302是否被初始化
			write_1302(0x8e,0x80); //WP=1 写保护
			//LCD1602
			write_command(0x0c); //取消液晶闪烁			
			status = 0;
		}
		//闹钟模式
		else if(status == 21)
		{
			display_1602_alarm();
			for(j=0;j<2;j++) //放两遍音乐 一分钟
			{
				TMOD=0x11;
				EA=1;//开总中断
				ET0=1;
				ET1=1;
				cnt=0;
				TR1=1;
				i = 0;
				while(1)
				{
					//扫描按键 如有按键按下退出闹钟
					if(DOWN==0)
					{
						delay(10);
						if(DOWN==0)
						{
							break;
						}
						while(!DOWN);
					}										
					t=tone[i]; //读音调
					if(t==0xff) break; //0xff是结束符
					if(t!=0) //0是休止符	
					{	
						b=cyc[t%10-1]; //根据基本音阶，求出半周期数
						if(t<10) b=b*2; //若是低八度音阶，半周期数加倍
						if(t>20) b=b/2; //若是高八度音阶，半周期数减半
						H0=(65536-b)/256; //根据半周期数，计算T0初值的高字节和低字节
						L0=(65536-b)%256;
						TR0=1; //启动定时器0发音
					}
					cnt=0;
					a=time[i]; //读节拍
					while(a>cnt)
					{
						//扫描按键 如有按键按下退出闹钟
						if(DOWN==0)
						{
							delay(10);
							if(DOWN==0)
							{								
								break;
							}
							while(!DOWN);
						}										
					}
					TR0=0;
					i++;
					for(b=0;b<1000;b++); //稍加延时，增强节奏感
				}
				EA = 0;
			}
			BEEP=1; //关闭喇叭
			status = 0;			
		}
		time_set_key_scan();
		alarm_key_scan();
		logo_key_scan();
	}
}
