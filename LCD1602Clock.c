/***************************************************************************/
//LCD1602��Ƭ���������������ӹ��ܣ�
//���ߣ������D
//ʱ�䣺2011��5��8��
//�汾��1.0
/***************************************************************************/
#include<reg52.h>

#define uint unsigned int
#define uchar unsigned char

//LCD1602���Ŷ���
sbit rs=P2^7;   
sbit rd=P2^6;
sbit lcdcs=P2^5;
sbit lcdbacklight1 = P2^1;
sbit lcdbacklight0 = P2^0;
uchar backlight = 2;
//DS18B20����
uchar count,s1num,tempint,f,tempth,settemp; //�¶��������ֺ�С������;
int tempdf,c;
sbit TMDAT=P1^6; //DS18B20������
//DS1302����
uchar nian,yue,ri,shi,fen,miao,zhou;
sbit ACC0=ACC^0;
sbit ACC7=ACC^7;
sbit T_RST=P2^2;//ds1302-5
sbit T_IO=P2^3;//ds1302-6
sbit T_CLK=P2^4;//ds1302-7
//��ʾ������
uchar lcd1602_one[]="2011-05-08 6 MT1";
uchar lcd1602_two[]=" 11:08:23  30^C ";
//����״̬��־λ
uchar status = 0; //0:�������� 1:����� 2:�µ��� 3:�յ��� 4:�ܵ��� 
				      //5:ʱ���� 6:�ֵ��� 7:����� 8:���������Ƿ��� 9:����ʱ����
						//10:���ӷ����� 11:logo��һλ���� 12:logo�ڶ�λ���� 13:logo����λ����
						//20:�������� 21:����ģʽ
//��������
sbit MODE = P3^2;
sbit UP = P3^3;
sbit DOWN = P3^4;
//ÿ���������� �������ڵ���
code uchar days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
//��������ʱ��ǰ���� ���ڳ�ʱ��û�����ò���ʱʱ�ӽ���������ʱ״̬
uchar key_click_sec,now_sec;
//����������ʾ������
uchar alarm_on[] = "OFF";
uchar alarm_time[]="07:10";
//�������û�����
code uchar alarm_set_one[]="Alarm:";
code uchar alarm_set_two[]=" Logo:";
//������ʾ������
code uchar alarm_one[]="*****??:??******";
code uchar alarm_two[]="*****Alarm******";
//�����ñ�������
sbit BEEP = P1^4;
code uint cyc[]={1800,1600,1440,1351,1200,1079,960};//����1-7�İ�������
code uchar tone[]={13,15,16,16,15,16,13,12,12,13,15,16,16,15,16,13,13, 13,15,16,
16,15,16,13,12,12,15,13,12,13,12,11,12,6,6,12,15,13,12,6,6,15,13,12,13,12,11,
12,6,5,6,0xff};// ������������衷�ļ��ױ�
code uchar time[]={8,8,8,4,4,8,8,12,4,8,8,8,4,4,8,16,8,8,8,8,4,4,8,8,12,4,8,8,4,
4,4,4,8,24,8,24,8,24,8,16,8,8,8,4,4,4,4,8,16,8,32};// ���ı�
uchar H0,L0,cnt;

/***************��ʱ����******************/ 
void delay(uint ms)
{
	uint i,j;
	for (j=0;j<ms;j++)
		for (i=0;i<120;i++);
}

//*************************LCD1602��غ���************************************
//****************************************************************************

/***************LCD1602д�����******************/ 
void write_command(uchar command)
{
	rs=0;
	P0=command;
	lcdcs=1;
	lcdcs=0;
}
/***************LCD1602д���ݺ���******************/ 
void write_data(uchar data0)
{
	rs=1;
	P0=data0;
	lcdcs=1;
	lcdcs=0;	
}
/***************��ʼ��LCD1602����******************/ 
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
/***************LCD1602ʱ����ʾ����******************/ 
void display_1602()
{
	uchar j;
	write_command(0x80); //Һ����ʾλ��
	delay(5);
	for (j=0;j<sizeof(lcd1602_one)-1;j++)
	{
		write_data(lcd1602_one[j]);
		delay(5);
	} 	
	write_command(0x80+0x40 ); //Һ����ʾλ��
	delay(5);
	for (j=0;j<sizeof(lcd1602_two)-1;j++)
	{
		write_data(lcd1602_two[j]);
		delay(5);
	} 
}
/***************LCD1602����������ʾ����******************/ 
void display_1602_alarm_set()
{
	uchar j;
	write_command(0x80); //Һ����ʾλ��
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
	write_command(0x80+0x40 ); //Һ����ʾλ��
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
/***************LCD1602������ʾ����******************/ 
void display_1602_alarm()
{
	uchar j;
	write_command(0x80); //Һ����ʾλ��
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
 	
	write_command(0x80+0x40); //Һ����ʾλ��
	delay(5);
	for (j=0;j<sizeof(alarm_two)-1;j++)
	{
		write_data(alarm_two[j]);
		delay(5);
	} 
}

//*************************DS18B20��غ���************************************
//****************************************************************************

/***************��ʼ��DS18B20�ӳ���******************/ 
void set_ds18b20()
{
	while(1)
	{ 
		uchar dela,flag;
		flag=0;
		TMDAT=1;
		dela=1;
		while(--dela);
		TMDAT=0; //�������õ͵�ƽ
		dela=250;
		while(--dela); //�͵�ƽ����500us
		TMDAT=1; //�������øߵ�ƽ
		dela=30;
		while(--dela); //�ߵ�ƽ����60us
		while(TMDAT==0) //�ж�DS18B20�Ƿ񷢳��͵�ƽ�ź�
		{
			dela=210; //DS18B20��Ӧ����ʱ420us
			while(--dela);
			if(TMDAT) //DS18B20�����ߵ�ƽ��ʼ���ɹ�������
			{
				flag=1; //DS18B20��ʼ���ɹ���־		
				break;
			}
		}
		if(flag) //��ʼ���ɹ�������ʱ480us��ʱ��Ҫ��
		{
			dela=240;
			while(--dela);
			break;
		}
	}
}
/***************��DS18B20�ӳ���******************/ 
void read_ds18b20()
{
	uchar dela,i,j,k,temp,temph,templ;
	j=3; //��2λ�ֽ�����
	do
	{
		for(i=8;i>0;i--) //һ���ֽڷ�8λ��ȡ
		{
			temp>>=1; //��ȡ1λ����1λ
			TMDAT=0; //�������õ͵�ƽ
			dela=1;
			while(--dela);
			TMDAT=1; //�������øߵ�ƽ
			dela=4;
			while(--dela); //��ʱ8us
			if(TMDAT) //��ȡ1λ����
				temp|=0x80;		    
			dela=25; //��ȡ1λ���ݺ���ʱ50us
			while(--dela);
		}
		if(j==3)                      
			templ=temp; //��ȡ�ĵ�һ�ֽڴ�templ
		if(j==2) 
		  	temph=temp; //��ȡ�ĵڶ��ֽڴ�temph
		if(j==1)  
		  	tempth=temp; //��ȡ�ĵ�3�ֽڴ�tempth   TH��ֵ         
	}while(--j);
	f=0;
	if((temph&0xf8)!=0x00) //���¶�Ϊ���Ĵ����Զ����Ʋ���Ĵ���
	{
		f=1; //Ϊ���¶�f��1
		temph=~temph;
		templ=~templ;        
		k=templ+1;
		templ=k;
		if(k>255)
		{
			temph++;
		}
	}
	tempdf=templ&0x0f; //����ȡ������ת�����¶�ֵ���������ִ�tempint,С�����ִ�tempdf
	c=(tempdf*625);
	tempdf=c;
	templ>>=4;
	temph<<=4;
	tempint=temph|templ; //���ֽںϲ�Ϊһ���ֽ�
}
/***************дDS18B20�ӳ���******************/ 
void write_ds18b20(uchar command)
{
	uchar dela,i;
	for(i=8;i>0;i--) //��һ�ֽ�����һλһλд��
	{
		TMDAT=0; //�������õ͵�ƽ
		dela=6; //��ʱ12us
		while(--dela);
		TMDAT=command&0x01; //�����ݷ�������������
		dela=25; //��ʱ50us
		while(--dela);
		command=command>>1; //׼��������һλ����
		TMDAT=1; //������һλ���ݣ��������øߵ�ƽ
	}
}
/**********DS18B20����¶� ������ʾ������********/ 
void get_temperature()
{
	set_ds18b20(); //��ʼ��DS18B20
	write_ds18b20(0xcc); //������ROMƥ������
	write_ds18b20(0x44); //���¶�ת������
	delay(5);
	set_ds18b20();
	write_ds18b20(0xcc); //������ROMƥ������
	write_ds18b20(0xbe); //�������¶�����
	read_ds18b20(); //���������¶����ݱ��浽tempint��tempdf��
	lcd1602_two[12] = 0x30+tempint%10;
	lcd1602_two[11] = 0x30+tempint%100/10;	 
}

//*************************DS1302��غ���*************************************
//****************************************************************************

/************DS1302��д�����(������)*******************/ 
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
/*************DS1302����ȡ�������½��أ�*****************/
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
/**********DS1302:д�����ݣ����͵�ַ����д���ݣ�********/ 
void write_1302(uchar addr,uchar da)
{
	T_RST=0; //ֹͣ����
	T_CLK=0;                                 
	T_RST=1; //���¹���
	write_byte(addr); //д���ַ
	write_byte(da);
	T_RST=0;
	T_CLK=1;
}
/**********DS1302:��ȡ���ݣ����͵�ַ���ٶ����ݣ�********/
uchar read_1302(uchar addr)
{
   uchar temp;
   T_RST=0; //ֹͣ����
   T_CLK=0;  
   T_RST=1; //���¹���
   write_byte(addr); //д���ַ
   temp=read_byte();
   T_RST=0;
   T_CLK=1; //ֹͣ����
   return(temp);
}
/**********DS1302:��ȡ��ǰ�¶� ������ʾ������***********/
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

//*****************************�жϴ�����***********************************
//****************************************************************************

/**********��ʱ��0�ж����ڲ������׷���***********/
void cntint0(void) interrupt 1 
{	
	TH0=H0;
	TL0=L0;
	BEEP=~BEEP; // BEEP�������ź�����ţ�BEEP���࣬��������
}
/**********��ʱ��1�ж����ڲ���������ʱ***********/
void cntint1(void) interrupt 3
{	
	cnt++; // ������ֵΪ0�����Բ��ø�ֵ
}

//*****************************����������***********************************
//****************************************************************************

/**********����ɨ����������ʱ��***********/
void time_set_key_scan()
{
	//������
	if(status ==1)
	{
		write_command(0x80 + 0x03); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//������
	else if(status ==2)
	{
		write_command(0x80 + 0x06); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}								
	}
	//������
	else if(status ==3)
	{
		uchar nowMonthDays = days[(lcd1602_one[5]-0x30)*10+(lcd1602_one[6]-0x30)-1];
		write_command(0x80 + 0x09); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//������
	else if(status ==4)
	{
		write_command(0x80 + 0x0b); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//ʱ����
	else if(status ==5)
	{
		write_command(0x80 + 0x40 + 0x02); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//������
	else if(status ==6)
	{
		write_command(0x80 + 0x40 + 0x05); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//������
	else if(status ==7)
	{
		write_command(0x80 + 0x40 + 0x08); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
}
/**********����ɨ��������������***********/
void alarm_key_scan()
{
	//��������ģʽ
	if(status == 8)
	{
		write_command(0x80 + 0x08); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
		if(UP==0)
		{
			delay(10);
			if(UP==0)
			{
				if(alarm_on[2] == 'N') //���ӿ�
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
		if(DOWN==0)
		{
			delay(10);
			if(DOWN==0)
			{
				if(alarm_on[2] == 'N') //���ӿ�
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//����ʱ����
	else if(status == 9)
	{
		write_command(0x80 + 0x0b); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}
	//���ӷ�����
	else if(status == 10)
	{
		write_command(0x80 + 0x0e); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}	
}
/**********����ɨ����������logo***********/
void logo_key_scan()
{
	//logo��һλ����
	if(status == 11)
	{
		write_command(0x80 + 0x40 + 0x06); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}	
	//logo�ڶ�λ����
	else if(status == 12)
	{
		write_command(0x80 + 0x40 + 0x07); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}	
	//logo����λ����
	else if(status == 13)
	{
		write_command(0x80 + 0x40 + 0x08); //�����ʾλ��
		delay(5);
		write_command(0x0d);
		delay(5);				
		//UP��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!UP);	
		}
		//DOWN��������
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
				//��ȡ��ǰʱ������
				key_click_sec = read_1302(0x81);
				key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);			
			}
			while(!DOWN);	
		}						
	}	
}
/**********����ɨ����������ģʽ����***********/
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
			//��ȡ��ǰʱ������
			key_click_sec = read_1302(0x81);
			key_click_sec = (key_click_sec/16)*10+(key_click_sec%16);
		}
		while(!MODE);	
	}								
	if(status == 0)
	{
		//DOWN�������� ���ڱ������
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

//*************************������*********************************************
//****************************************************************************
void main()
{
	//�����ñ�������
	uchar  i,j,a,t;
	uint b;
	//��Һ����ʾ^C��Ϊ���Ϸ���
	lcd1602_two[13] = 0xdf;
	//��ʼ��Һ��
	init_1602();
	//��ʼ��DS1302
	if(read_1302(0xff) == 0xaa)//1302δ����ʼ��
	{	
		//��ȡalarmֵ
		alarm_time[4] = 0x30 + read_1302(0xd1)%16;
		alarm_time[3] = 0x30 + read_1302(0xd1)/16;
		alarm_time[1] = 0x30 + read_1302(0xd3)%16;
		alarm_time[0] = 0x30 + read_1302(0xd3)/16;
		if(read_1302(0xc5) == 0x00)// ���ӹ�
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
		write_1302(0x8e,0x00); //WP=0 д����
		write_1302(0x90,0x00); //�رճ�繦��
		write_1302(0x80,0x00);// miao
		write_1302(0x82,0x27);// fen
		write_1302(0x84,0x20);// shi
		write_1302(0x86,0x09);// ri
		write_1302(0x88,0x05);// yue
		write_1302(0x8c,0x11);// nian
		write_1302(0x8a,0x01);// zhou
		write_1302(0x8e,0x80); //WP=1 д����	
	}
	lcdbacklight0 = 1;				
	lcdbacklight1 = 0;				
	while(1)
	{
		//ɨ��mode������û�б�����
		mode_key_scan();
		//�û�һ��ʱ��û�в���ʱ�ָ�������ʱ
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
		//������ʱ״̬
		if(status == 0)
		{
			get_temperature();
			get_1302time();
			display_1602();
			//�������
			if(alarm_on[2] == 'N') //���ӿ�
			{
				if(lcd1602_two[1]==alarm_time[0]
					&&lcd1602_two[2]==alarm_time[1]
					&&lcd1602_two[4]==alarm_time[3]
					&&lcd1602_two[5]==alarm_time[4]
					&&lcd1602_two[7]=='0'
					&&lcd1602_two[8]=='0') //����ʱ�䵽
				{
					status = 21;							
				}
			}
		}
		//��������
		else if(status ==20)
		{
			//д��ʱ�䵽DS1302
			write_1302(0x8e,0x00);//WP=0 д����
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
			write_1302(0xfe,0xaa);// �ж�1302�Ƿ񱻳�ʼ��
			write_1302(0x8e,0x80); //WP=1 д����
			//LCD1602
			write_command(0x0c); //ȡ��Һ����˸			
			status = 0;
		}
		//����ģʽ
		else if(status == 21)
		{
			display_1602_alarm();
			for(j=0;j<2;j++) //���������� һ����
			{
				TMOD=0x11;
				EA=1;//�����ж�
				ET0=1;
				ET1=1;
				cnt=0;
				TR1=1;
				i = 0;
				while(1)
				{
					//ɨ�谴�� ���а��������˳�����
					if(DOWN==0)
					{
						delay(10);
						if(DOWN==0)
						{
							break;
						}
						while(!DOWN);
					}										
					t=tone[i]; //������
					if(t==0xff) break; //0xff�ǽ�����
					if(t!=0) //0����ֹ��	
					{	
						b=cyc[t%10-1]; //���ݻ������ף������������
						if(t<10) b=b*2; //���ǵͰ˶����ף����������ӱ�
						if(t>20) b=b/2; //���Ǹ߰˶����ף�������������
						H0=(65536-b)/256; //���ݰ�������������T0��ֵ�ĸ��ֽں͵��ֽ�
						L0=(65536-b)%256;
						TR0=1; //������ʱ��0����
					}
					cnt=0;
					a=time[i]; //������
					while(a>cnt)
					{
						//ɨ�谴�� ���а��������˳�����
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
					for(b=0;b<1000;b++); //�Լ���ʱ����ǿ�����
				}
				EA = 0;
			}
			BEEP=1; //�ر�����
			status = 0;			
		}
		time_set_key_scan();
		alarm_key_scan();
		logo_key_scan();
	}
}
