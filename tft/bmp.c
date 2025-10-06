#include "bmp.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"
/*  外设库  */
#include "U_USART.h"
/*  FATFS  */
#include "ff.h"

/*	原本的bmp.h/c的文件其实挺好的...
 *	怎么说....
 *	就是有点烂......
 *	以及.....在中秋节的晚上一个人在电脑前面写代码....
 *	烂透了......
 *	好烦......好烦....
 *	好烦.....
 *	糟糕透了....糟糕透了....
 *		——2025/10/6.19:41-秦羽
 */

FATFS fs;
const char SD_PATH[] = {"0:/cmr/"};
typedef struct{
	uint16_t bmp_sign;		//00 文件标识
	uint32_t file_size;		//02 用字节表示文件大小
	uint32_t reserved;		//06 保留 应该是0
	uint32_t data_offset;	//0A 数据偏移位置
	uint32_t header_size;	//0E 信息头长度
	uint32_t width;			//12 位图宽度，单位像素
	uint32_t height;		//16 位图高度，单位像素
	uint16_t planes;		//1A 位面数 总是1
	uint16_t bits_per_px;	//1C 每个像素的位数
	uint32_t compression;	//1E 压缩说明 0不压缩
	uint32_t data_size;		//22 用字节表示位图数据大小（4的倍数）
	uint32_t HResolution;	//26 像素/米表示水平分辨率
	uint32_t VResolution;	//2A 像素/米表示垂直分辨率
	uint32_t colors;		//2E 位图用的颜色数
	uint32_t important_cor; //32 重要颜色数
}bmp_head;

/**@brief  SD卡挂载
  *@add    SD卡的驱动文件好像有没什么用的引脚初始化，应该把这个初始化放在前面...
  *		   避免SDIO改变其他引脚...
  */
void Init_BMP(void)
{
	DIR dp;
	if(f_mount(&fs,"0:",1)!=FR_OK)
	{
		U_Printf("SD卡初始化异常,代码:%d \r\n",f_mount(&fs,"0:",1));
		return;
	}
	else if(f_opendir(&dp,SD_PATH)!=FR_OK)
	{
		U_Printf("未检测到文件夹[%s]位置,请创建文件夹后重试 \r\n",&SD_PATH[3]);
		return;
	}
	else
	{
		f_closedir(&dp);
		U_Printf("Init_BMP：正常挂载SD卡，初始化完成 \r\n");
	}
}
/**@brief  添加路径前缀
  *@param  front 	路径前缀
  *@param  path		路径位置，调用之后会在path前添加前缀
  *@retval void
  **/
void BMP_PathAdd_Front(const char* front,uint8_t* path)
{
	uint8_t index = 0;
	uint8_t temp_path[30];
	//复制路径
	for(;path[index]!='\0';index++)
	{
		temp_path[index]=path[index];
	}
	temp_path[index]=path[index];
	//找到
	for(index=0;front[index]!=0;index++)
	{
		path[index] = front[index];
	}
	for(int i=0;temp_path[i]!=0;i++)
	{
		path[index++] = temp_path[i];
	}
	path[index] = '\0';
}
/**@brief  添加路径后缀
  *@param  back 	路径后缀
  *@param  path		路径位置，调用之后会在path前添加后缀
  *@retval void
  **/
void BMP_PathAdd_Back(uint8_t* path,const char* back)
{
	uint8_t index = 0;
	for(;path[index]!='\0';index++);
	for(int i=0;back[i]!='\0';i++)
	{
		path[index++] = back[i];
	}
	path[index] = '\0';
}
/**@brief  读取一段数字
  *@param  path 要读取数字的字符串
  *@retval uint16_t 读取到的数字
  *@add	   会自动找到要读取的数字位置，不过只能读一个...
  */
uint16_t BMP_PathRead_Num(const char* path)
{
	uint16_t num = 0;
	uint8_t i=0;
	for(;(path[i]<'0'||path[i]>'9')&&i<100;i++);
	while(path[i]>='0'&&path[i]<='9')
	{
		num*=10;
		num+=(path[i]-'0');
		i++;
	}
	return num;
}
/**@brief  打印文件头信息
  *@param  bmp_infor 要打印的文件信息
  *@retval void
  */
static void BMP_PrintfInfor(bmp_head infor)
{
	U_Printf("现在打印文件信息: \r\n");
	U_Printf("文件标识:%h \r\n",infor.bmp_sign);
	U_Printf("文件大小:%d字节 \r\n",infor.file_size);
	U_Printf("保留位(0):%d \r\n",infor.reserved);
	U_Printf("数据偏移位置:%h \r\n",infor.data_offset);
	U_Printf("信息头长度:%d \r\n",infor.header_size);
	U_Printf("位图宽度:%dpx\t位图高度:%dpx \r\n",infor.width,infor.height);
	U_Printf("位面数(1):%d \r\n",infor.planes);
	U_Printf("像素位数:%d \r\n",infor.bits_per_px);
	U_Printf("位图数据大小:%d字节 \r\n",infor.data_size);
	U_Printf("水平分辨率:%dpx\t垂直分辨率:%dpx \r\n",infor.HResolution,infor.VResolution);
	U_Printf("用到的颜色数:%d \r\n",infor.colors);
	U_Printf("重要颜色数:%d \r\n",infor.important_cor);
}
/**@brief  读取文件头信息
  *@param  path 读取文件路径
  *@retval 文件头信息
  */
bmp_head BMP_ReadInfor(const char* path)
{
	bmp_head infor;
	FIL fp;
	//打开文件
	if(f_open(&fp,(const char*)path,FA_READ)!=FR_OK)
	{
		U_Printf("%s打开失败，状态:%d \r\n",path,f_open(&fp,(const char*)path,FA_READ));
		return infor;
	}
	//读取文件信息
	f_read(&fp,&infor.bmp_sign,sizeof(uint16_t),0);
	f_read(&fp,&infor.file_size,sizeof(uint32_t),0);
	f_read(&fp,&infor.reserved,sizeof(uint32_t),0);
	f_read(&fp,&infor.data_offset,sizeof(uint32_t),0);
	f_read(&fp,&infor.header_size,sizeof(uint32_t),0);
	f_read(&fp,&infor.width,sizeof(uint32_t),0);
	f_read(&fp,&infor.height,sizeof(uint32_t),0);
	f_read(&fp,&infor.planes,sizeof(uint16_t),0);
	f_read(&fp,&infor.bits_per_px,sizeof(uint16_t),0);
	f_read(&fp,&infor.compression,sizeof(uint32_t),0);
	f_read(&fp,&infor.data_size,sizeof(uint32_t),0);
	f_read(&fp,&infor.HResolution,sizeof(uint32_t),0);
	f_read(&fp,&infor.VResolution,sizeof(uint32_t),0);
	f_read(&fp,&infor.colors,sizeof(uint32_t),0);
	f_read(&fp,&infor.important_cor,sizeof(uint32_t),0);
	//关闭文件
	f_close(&fp);
	//打印文件信息(仅测试时，具体用到项目时....希望自己记得删)
	BMP_PrintfInfor(infor);
	return infor;
}
/**@brief  BMP数据读取前的预处理
  *@param  file_name	文件名(不包括文件夹位置和.bmp尾缀)
  *@param  width/height	读取到的长和宽
  *@param  data_offset	图像像素开始位置
  *@retval int8_t 		1正常读取/0文件打开失败
  */
static int8_t BMP_Read_ForeProcess(const char* file_name,FIL* fp,uint32_t* width,uint32_t* height,uint32_t* data_offset)
{
	//路径处理
	uint8_t path[50];
	for(int i=0;file_name[i]!='\0';i++)
	{
		path[i] = file_name[i];
		path[i+1] = '\0';
	}
	BMP_PathAdd_Front(SD_PATH,path);
	BMP_PathAdd_Back(path,".bmp");
	//打开文件
	if(f_open(fp,(const TCHAR*)path,FA_READ)!=FR_OK)
	{
		U_Printf("打开文件[%s]失败，代码:%d \r\n",file_name,f_open(fp,(const TCHAR*)path,FA_READ));
		return 0;
	}
	//读取数据
	f_lseek(fp,0x0A);
	f_read(fp,data_offset,sizeof(uint32_t),0);
	f_lseek(fp,0x12);
	f_read(fp,width,sizeof(uint32_t),0);
	f_read(fp,height,sizeof(uint32_t),0);
	U_Printf("data_offset:%h,width*height:%d*%d \r\n",*data_offset,*width,*height);
	return 1;
}
/**@brief  通过数据读取bmp图片数据
  *@param  file_name  	仅名字，不需要前缀和尾缀
  *@param  data			读取到的数据
  *@param  max_length	data能承受的最大rgb565像素数量
  *@retval void
  */
void BMP_Read_ByData(const char* file_name,uint16_t* data,uint32_t max_length)
{
	FIL fp;
	uint32_t data_offset,width,height;
	//预处理
	if(BMP_Read_ForeProcess(file_name,&fp,&width,&height,&data_offset)==0)
	{//处理异常
		return;
	}
	//获取数据
	f_lseek(&fp,data_offset);
	uint32_t rgb888;
	uint16_t rgb565;
	uint32_t circle_count = (max_length>width*height?width*height:max_length);
	for(uint32_t i=0;i<circle_count;i++)
	{
		//rgb565  ---- ---- rrrr rggg gggb bbbb
		//rgb888  rrrr r--- gggg gg-- bbbb b---
		f_read(&fp,(void*)&rgb888,sizeof(uint8_t)*3,0);
		rgb565 = ((rgb888&0xF80000)>>8);
		rgb565 |= ((rgb888&0xFC00)>>5);
		rgb565 |= ((rgb888&0xF8)>>3);
		data[i] = rgb565;
	}
	//关闭文件
	f_close(&fp);
}
/**@brief  通过函数处理数据
  *@param  file_name	仅名字，不需要前缀和尾缀
  *@param  void(*Func)(uint16_t)	rgb565像素处理函数,uint16_t->rgb565像素
  *@retval void
  */
void BMP_Read_ByFunc(const char* file_name,void(*Func)(uint16_t))
{
	FIL fp;
	uint32_t data_offset,width,height;
	//预处理
	if(BMP_Read_ForeProcess(file_name,&fp,&width,&height,&data_offset)==0)
	{//处理异常
		return;
	}
	//获取数据
	f_lseek(&fp,data_offset);
	uint32_t rgb888;
	uint16_t rgb565;
	for(uint32_t i=0;i<width*height;i++)
	{
		//rgb565  ---- ---- rrrr rggg gggb bbbb
		//rgb888  rrrr r--- gggg gg-- bbbb b---
		f_read(&fp,(void*)&rgb888,sizeof(uint8_t)*3,0);
		rgb565 = ((rgb888&0xF80000)>>8);
		rgb565 |= ((rgb888&0xFC00)>>5);
		rgb565 |= ((rgb888&0xF8)>>3);
		Func(rgb565);
	}
	//关闭文件
	f_close(&fp);
}
/**
  *@retval int8_t
  *@add	   文件头只是适配....
  *		   在f429板子上的相机项目(即320*240-qvga)尺寸的bmp
  */
int8_t BMP_Write_ForeProcess(const char* file_name,FIL* fp,uint16_t width,uint16_t height)
{
	//路径处理
	uint8_t path[50];
	for(int i=0;file_name[i]!='\0';i++)
	{
		path[i] = file_name[i];
		path[i+1] = '\0';
	}
	BMP_PathAdd_Front(SD_PATH,path);
	BMP_PathAdd_Back(path,".bmp");
	//打开文件
	if(f_open(fp,(const TCHAR*)path,FA_CREATE_ALWAYS|FA_WRITE)!=FR_OK)
	{
		U_Printf("打开文件[%s]失败，代码:%d \r\n",file_name,f_open(fp,(const TCHAR*)path,FA_CREATE_ALWAYS|FA_WRITE));
		return 0;
	}
	//写入文件头
	uint32_t data_dword = 0;
	uint16_t data_word = 0;
		//bmp文件标志:0x4D42
	data_word = 0x4D42;
	f_write(fp,&data_word,sizeof(uint16_t),0);
		//文件大小，以qvga(320*240)为准
	data_dword = 230538;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
		//保留位(0):0 
	data_dword = 0;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
		//数据偏移位置:0x8A 
	data_dword = 0x8A;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
		//信息头长度:124 
	data_dword = 124;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
		//位图宽度:320px	位图高度:240px 
	data_dword = width;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
	data_dword = height;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
		//位面数(1):1 
	data_word = 1;
	f_write(fp,&data_word,sizeof(uint16_t),0);
		//像素位数:24 
	data_word = 24;
	f_write(fp,&data_word,sizeof(uint16_t),0);
		//压缩方式:0
	data_dword = 0;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
		//位图数据大小:230400字节 
	data_dword = 230400;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
		//水平分辨率:0px	垂直分辨率:0px 
	data_dword = 0;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
	data_dword = 0;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
		//用到的颜色数:0 
	data_dword = 0;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
		//重要颜色数:0 
	data_dword = 0;
	f_write(fp,&data_dword,sizeof(uint32_t),0);
	//移动到可以写入像素的地方
	f_lseek(fp,0x8A);
	return 1;
}
void BMP_Write_ByData(const char* file_name,uint16_t* data,uint16_t width,uint16_t height)
{
	FIL fp;
	if(BMP_Write_ForeProcess(file_name,&fp,width,height)==0)
	{
		return;
	}
	uint8_t rgb888[3];
	uint32_t rgb565 = 0;
	for(int i=0;i<width*height;i++)
	{
		rgb565 = data[i];
		//rgb565  ---- ---- rrrr rggg gggb bbbb
		//rgb888  rrrr r--- gggg gg-- bbbb b---
		rgb888[0] = ((rgb565&0xF800)>>8);
		f_write(&fp,&rgb888[0],sizeof(uint8_t),0);
		rgb888[0] = ((rgb565&0x7E0)>>3);
		f_write(&fp,&rgb888[0],sizeof(uint8_t),0);
		rgb888[0] = ((rgb565&0x1F)<<3);
		f_write(&fp,&rgb888[0],sizeof(uint8_t),0);
	}
	//关闭文件
	f_close(&fp);
	U_Printf("bmp.c[BMP_Write_ByData]:写入[%s]完成 \r\n",file_name);
}
#include "TFT_ST7789V.h"
void Cmd_BMP(void)
{	
	TFT_SetCursor(0,0,300,200);
	BMP_ReadInfor("0:/cmr/aaa.bmp");
	TFT_SPI_Start();
	BMP_Read_ByFunc("aaa",TFT_Write16Data);
	TFT_SPI_Stop();
	
//	TFT_SetCursor(0,0,320,240);
//	TFT_SPI_Start();
//	BMP_Read_ByFunc("ciallo_qvga",TFT_Write16Data);
//	TFT_SPI_Stop();
//	BMP_ReadInfor("0:/cmr/ciallo_qvga.bmp");
	
	U_Printf("这里是BMP的指令 \r\n");
}








