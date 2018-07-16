#include <uvc.h>
#include <minrry.h>

static int Index = 0;
static char unsigned value[128] = {0};
FILE *fp = NULL;
static int IsIFrame = 0;

//说明：
//以下接口是针对UVC设备端的控制命令，只针对了一路码流，可以设置Index的值控制多路码流
//以下指令只配置了部分功能，详情配置请参考 "(UVC扩展命令).doc"

//设置编码器编码等级
int MinrryUvc_SetProfile(unsigned int handle)
{
	value[0] = Index;//Stream number
	value[1] = 1;//H.264 profile,0: Baseline,1: Main profile,2: High profile
	value[2] = 0;

	return MinrryUvc_SetPara(handle, XU_CONTROL_ENCODE_PROFILE, value, 3);
}

//设置码流的分辨率
int MinrryUvc_SetBuffSize(unsigned int handle)
{
	int w,h;

	printf("input width heigth exp:1920 1280\n");
	scanf("%d%d",&w,&h);

	value[0] = Index;
	value[1] = w & 0xff;
	value[2] = (w >> 8) & 0xff;
	value[3] = h & 0xff;
	value[4] = (h >> 8) & 0xff;

	return MinrryUvc_SetPara(handle, XU_CONTROL_ENCODE_VIDEOSIZE, value, 5);
}

//设置码率的码率
int MinrryUvc_SetBitsRate(unsigned int handle)
{
	int bitsrate;
	printf("input bitsrate/Kb exp:2048 == 2M\n");
	scanf("%d",&bitsrate);
	
	value[0] = Index;
	value[1] = 0;//CBR 
	value[2] = 10;
	value[3] = 20;
	value[4] = bitsrate & 0xff;//
	value[5] = (bitsrate >> 8) & 0xff;
	value[6] = (bitsrate >> 16) & 0xff;
	value[7] = (bitsrate >> 24) & 0xff;

	return MinrryUvc_SetPara(handle, XU_CONTROL_ENCODE_BITSRATE, value, 8);
}

//设置帧率
int MinrryUvc_SetFrameRate(unsigned int handle)
{
	int frame = 0;

	printf("input frame rate (1 - 30 / fps)\n");
	scanf("%d",&frame);

	value[0] = Index;
	value[1] = frame;

	return MinrryUvc_SetPara(handle, XU_CONTROL_ENCODE_FRAMERATE, value, 2);
}

//设置I帧间隔
int MinrryUvc_SetIdr(unsigned int handle)
{
	int idr = 0;

	printf("input idr (1-65535)\n");
	scanf("%d",&idr);

	value[0] = Index;
	value[1] = idr & 0xff;
	value[2] = (idr >> 8) & 0xff;

	return MinrryUvc_SetPara(handle, XU_CONTROL_ENCODE_IDR, value, 3);
}

//打开子码流
int MinrryUvc_OpenSubStream(unsigned int handle)
{
	value[0] = Index;
	value[1] = 1;
	return MinrryUvc_SetPara(handle, XU_CONTROL_SUB_ENCODE_STREAMER,value,2);
}

//云台控制
int MinrryUvc_PTZContrl(unsigned int handle)
{
	int pos = 0;
	printf("input ptz pos (0:stop,1:up,2:down,3:left)\n");
	scanf("%d", &pos);

	value[0] = 100;
	value[1] = pos;
	return MinrryUvc_SetPara(handle, XU_CONTROL_PTZ_CONTROL,value,2);
}

//云台控制2 这个是uvc标准的云台控制命令
int MinrryUvc_PTZContrlExt(unsigned int handle)
{
	int ret = 0;
	int pos[2] = {0};
	printf("input ptz pos Pan(-180 +180) and Tilt(-180  +180)\n");
	scanf("%d%d", &pos[0],&pos[1]);

	ret = MinrryUvc_SetParaExt(handle,V4L2_CID_PAN_ABSOLUTE, &pos[0],1);//水平移动

	ret = MinrryUvc_SetParaExt(handle,V4L2_CID_TILT_ABSOLUTE, &pos[1],1);//上下移动
	return ret;
}

//获取相机版本
int MinrryUvc_GetVersionsInfo(unsigned int handle)
{

	memset(value, 0x00, sizeof(value));

	if(MinrryUvc_GetPara(handle, XU_CONTROL_UPDATE, value, 22) < 0)
	{
		LOG_ERROR("MinrryUvc_GetVersionsInfo error");	
		return -1;
	}

	printf("version %d.%d.%d(%d.%d.%d)\n",value[1],value[2],value[3],
						value[4] << 8 | value[5],value[6], value[7]);	
	return 0;
}


int UvcRecvData(unsigned char *pBuff, int s32Size)
{
	if(pBuff[4]	== 0x67 || IsIFrame)//设置第一帧为I帧，不然保存的流vlc无法播放
	{
		if(fp)
			fwrite(pBuff, 1, s32Size, fp);
		IsIFrame = 1;	
	}

	return 0;
}

void Menu()
{
	printf("----------Menu-----------\n");
	printf("|---s-------start		 |\n");
	printf("|---e-------stop		 |\n");
	printf("|---i-----set Idr		 |\n");
	printf("|---p-----set profile	 |\n");
	printf("|---v-----set buffsize   |\n");
	printf("|---b-----set bits rate  |\n");
	printf("|---f-----set frame rate |\n");
	printf("|---c-----set buffsize	 |\n");
	printf("|---z-----pzt contrl   	 |\n");
	printf("|---g-----get version    |\n");
	printf("|---q-----exit        	 |\n");
	printf("--------------------------\n");
}

int main(int argc, char *argv[])
{
	char cmd = 0;
	unsigned int handle;

	if(argc < 3)
	{
		LOG_ERROR("input error exp:./uvc 0 /data/H.264");
		return -1;
	}

	fp = fopen(argv[2],"wb+");
	if(!fp)
	{
		LOG_ERROR("open %s error", argv[2]);
		return -1;
	}

	if(MinrryUvc_Init(&handle, atoi(argv[1])) < 0)
	{
		LOG_ERROR("MinrryUvc_Init error");	
		return -1;
	}

	MinrryUvc_SetListen(handle, UvcRecvData);

	while(1)
	{
		Menu();
		while((cmd = getchar()) == '\n');
		switch(cmd)
		{
			case 's':
				{
					if(MinrryUvc_StartPreview(handle) < 0)
					{
						LOG_ERROR("MinrryUvc_StartPreview error");	
						goto error;
					}	

				}break;
			case 'e':
				{
					if(MinrryUvc_StopPreview(handle) < 0)
					{
						LOG_ERROR("MinrryUvc_StartPreview error");	
						goto error;
					}
					IsIFrame = 0;

				}break;
			case 'i':
				{
					//idr
					MinrryUvc_SetIdr(handle);	
				}break;
			case 'p':
				{
					MinrryUvc_SetProfile(handle);	
					//profile
				}break;
			case 'v':
				{
					MinrryUvc_SetBuffSize(handle);	
					//buffsize
				}break;

			case 'b':
				{
					MinrryUvc_SetBitsRate(handle);	
					//bitsrate
				}break;
			case 'f':
				{
					MinrryUvc_SetFrameRate(handle);	
					//frame_rate
				}break;

			case 'c':
				{
					//open SubStream
					MinrryUvc_OpenSubStream(handle);	
				}break;

			case 'z':
				{
					//pzt contrl
					//MinrryUvc_PTZContrl(handle);	
					MinrryUvc_PTZContrlExt(handle);	
				}break;

			case 'g':
				{
					//open SubStream
					MinrryUvc_GetVersionsInfo(handle);	
				}break;

			case 'q':
				{
					MinrryUvc_StopPreview(handle);
					MinrryUvc_DeInit(&handle);
					return 0;
				}break;
		}
	}

	return 0;

error:
	MinrryUvc_StopPreview(handle);
	MinrryUvc_DeInit(&handle);
	fclose(fp);

	return -1;
}
