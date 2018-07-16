#ifndef __MINRRY_UVC_H__
#define __MINRRY_UVC_H__
#include <uvc.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>


typedef int (*UvcPostData)(unsigned char *pBuff, int u32Size);

typedef struct 
{
	int mFd;
	int u32Heigth;
	int u32Width;
	int u32Pix;
	int Status;
	int UvcIsStart;
	pthread_t pid;
	UvcPostData pPostData;
	mMapBuffers MmBuff;
	FrameData   Frame;
}S_MINRRY_UVC_ATTR;

#ifdef __cplusplus
extern "C" {
#endif

//初始化UVC
//pHandle传入的句柄地址
//mID 打开的video设备，比如0 就是打开/dev/video0
extern int MinrryUvc_Init(unsigned int *pHandle, int mID);

//去初始化
extern void MinrryUvc_DeInit(unsigned int *pHandle);

//设置回调函数，该函数应用于UVC上报获取的数据
extern void MinrryUvc_SetListen(unsigned int Handle, UvcPostData pFunction);

//明日UVC扩展SET命令，控制命令详情请参考 extion uint commands(UVC扩展命令).doc
extern int MinrryUvc_SetPara(unsigned int Handle,unsigned char selector,unsigned char *value,int size);
//明日UVC扩展GET命令，目前该函数只支持获取版本信息 select为0x09
extern int MinrryUvc_GetPara(unsigned int Handle,uint8_t selector,uint8_t *value,int size);
//uvc控制云台标准命令
extern int MinrryUvc_SetParaExt(unsigned int Handle,int selector,int *value,int size);

//开启预览
extern int MinrryUvc_StartPreview(unsigned int Handle);
//关闭预览
extern int MinrryUvc_StopPreview(unsigned int Handle);


#ifdef __cplusplus
	}
#endif

#endif
