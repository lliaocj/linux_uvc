#ifndef __UVC__H__
#define __UVC__H__

#include <stdio.h>
//#include <linux/videodev2.h>
#include <linux/videodev2.h>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
#include "unistd.h"
#include <fcntl.h>
#include <sys/mman.h>

#define UVC_SET_CUR                 0x01
#define UVC_GET_CUR                 0x81
#define UVC_TERMINAL_CONTROL	    2

//以下定义的宏如果内核中已经有该结构体的定义请关闭
#ifdef LINUX
/*fei add:2016-11-15*/
struct v4l2_ut10_ext_ctr{
	__u32 type;
	__u32 len;
	union{
		__u8 a;
		__u32 b;
		__u8 c[100];
	}u;
};
struct v4l2_minrray_ext_ctr{
	__u8 selector;
	__u8 query;
	__u8 intfnum;
	__u8 unit;
	__u16 size;
	__u8 c[100];
};
enum ut10_type{
	FILE_VERTICAL_HORIZONTROL_CTR = 1,
	STRING_SUBTITLE_CTR = 2,
	UPDATE_GET_CTR = 3,
	IDR_CTR = 4,
	PANTILT_RELATIVE_CTR = 5,//云台相对为控制类型
	ZOOM_RELATIVE_CTR = 6,
	UPDATE_SET_CTR = 7,
	ENCODE_PROPERTY = 8,//编码质量
	GET_ENCODE_PROPERTY = 9,

	FOCUS_MODE = 10,
	PRESET_CONTROL = 11,
	PTZ_MENU_CONTROL = 12,
	STRING_OSD_CONTROL = 13,
	DATE_OSD_CONTROL = 14,
	IDR_CONTROL = 15,
	ENCODER_PROFILE = 16,
	ENCODER_BITSTRATE = 17,
	ENCODER_VIDEO_SIZE = 18,
	ENCODER_FRAME_RATE = 19,
	SUB_ENCODER_STREAM= 20,
	UART_CMD = 21,
	ISP_CONTROL = 22,
};

/*Extension Unit ID*/
#define UVC_CAMERA_TERMINAL_ID  0x1
#define UVC_EXTENSION_UNIT_ID   0x3

/*Interface*/
#define UVC_INTF_CONTROL		0x0

/*Control Selector*/
#define XU_CONTROL_SEND_IDR		0x3
#define XU_CONTROL_UPDATE		0x9
#define XU_CONTROL_ENCODE_PROPERTY 0x4
#define XU_CONTROL_FOCUS_MODE   	0x6
#define XU_CONTROL_PRESET_CONTROL   0x7
#define XU_CONTROL_PTZ_CONTROL		0x8
#define XU_CONTROL_OSD_STRING 		0x0b
#define XU_CONTROL_OSD_DATE			0x0c
#define XU_CONTROL_ENCODE_IDR		0x0d
#define XU_CONTROL_ENCODE_PROFILE   0x0e
#define XU_CONTROL_ENCODE_BITSRATE  0x0f
#define XU_CONTROL_ENCODE_VIDEOSIZE 0x10
#define XU_CONTROL_ENCODE_FRAMERATE 0x11
#define XU_CONTROL_SUB_ENCODE_STREAMER 0x12
#define XU_CONTROL_UART_CMD			0x13
#define XU_CONTROL_ISP				0x14


#define UVC_UT10_EXT_CTR 	 _IOWR('V',104,struct v4l2_ut10_ext_ctr)
#define UVC_UT11_EXT_CTR_2   _IOWR('V',105,struct v4l2_ut10_ext_ctr)
#define UVC_UT11_EXTG_GET_CTR_2   _IOWR('V',106,struct v4l2_ut10_ext_ctr)
#define VIDIOC_MINRRAY_EXT_CTRLS _IOWR('V',107,struct v4l2_minrray_ext_ctr)
#endif
#define XU_CONTROL_ENCODE_CODEC				0x15


#define LOG_ERROR(fmt,ptr...) printf("\033[31mERROR:%s[%d]"fmt"\n\033[0m",__FUNCTION__,__LINE__,##ptr);


typedef unsigned char uint8_t;
typedef unsigned int  uint32_t;

typedef struct
{
	void *start;
	unsigned int size;
}v_buffers;

typedef struct 
{
	unsigned int total;
	v_buffers *_buff;
}mMapBuffers;

typedef struct 
{
	unsigned char *data;
	unsigned int  length;
	struct v4l2_buffer   buff;
}FrameData;

extern int UvcDevice_Open(int Id);
extern int UvcDevice_Close(int Id);

extern int UvcDevice_SetFormat(int fd, int w, int h, int pix);
extern int UvcDevice_GetFormat(int fd, int *w, int *h, int *pix);

extern int UvcDevice_ReqBuffers(int fd, struct v4l2_requestbuffers *pBuff);
extern int UvcDevice_QueBuffers(int fd, struct v4l2_buffer *pBuff, int index);

extern int UvcDevice_Mmap(int fd, mMapBuffers *pMmBuff);
extern int UvcDevice_UnMmap(int fd, mMapBuffers *pMmBuff);

extern int UvcDevice_DQBuffers(int fd, FrameData *pFrame, mMapBuffers *pBuff);
extern int UvcDevice_QBuffers(int fd, FrameData *pFrame);

extern int UvcDevice_StreamOn(int fd);
extern int UvcDevice_StreamOff(int fd);

extern int UvcDevice_CleanFrame(int fd, mMapBuffers *pBuff);

extern int UvcDevice_SetPara(int fd, uint8_t unit,uint8_t selector,uint8_t *value,int size);
extern int UvcDevice_SetParaExt(int fd, int selector,int *value,int size);
extern int UvcDevice_GetPara(int fd, uint8_t unit,uint8_t selector,uint8_t *value,int size);

extern int UvcDevice_SetIdr(int fd, int index);
#endif
