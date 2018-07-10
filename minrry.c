#include <uvc.h>
#include <minrry.h>

#define UVC_MAX_DEVICE   4

static S_MINRRY_UVC_ATTR *sUvcAttrs[UVC_MAX_DEVICE] = {NULL};

static int MinrryUvc_GetFreeUvcById()
{
	int i = 0;
	for(i = 0; i < UVC_MAX_DEVICE; i++)
	{
		if(sUvcAttrs[i] == NULL)
			break;
	}	

	if(i == UVC_MAX_DEVICE)
		return -1;

	return i;
}

static void *MinrryUvc_GetBuff(void *arg)
{
	int ret = 0;
	int fd = 0;
	S_MINRRY_UVC_ATTR *sUvcAttr = (S_MINRRY_UVC_ATTR *)arg;

	fd = sUvcAttr->mFd;

	//UvcDevice_SetIdr(fd, 0);
	//UvcDevice_SetIdr(fd, 1);
	//UvcDevice_SetIdr(fd, 2);

	while(!sUvcAttr->Status)
	{
		ret = UvcDevice_DQBuffers(fd, &(sUvcAttr->Frame), &(sUvcAttr->MmBuff));
		if(ret != 0)
		{
			LOG_ERROR("UvcDevice_DQBuffers error");
			return NULL;
		}		

		//TODO
		if(sUvcAttr->pPostData)
			sUvcAttr->pPostData(sUvcAttr->Frame.data, sUvcAttr->Frame.length);

		ret = UvcDevice_QBuffers(fd, &(sUvcAttr->Frame));
		if(ret != 0)
		{
			LOG_ERROR("UvcDevice_QBuffers error");
			return NULL;
		}
	}

	return NULL;
}

int MinrryUvc_Init(unsigned int *pHandle, int mID)
{
	int fd = 0;
	int index = MinrryUvc_GetFreeUvcById(); 
	if(pHandle == NULL || mID < 0)
	{
		LOG_ERROR("this para is Invalid");
		return -1;
	}	

	if(index == -1)
		return -1;

	S_MINRRY_UVC_ATTR *sUvcAttr = (S_MINRRY_UVC_ATTR *)malloc(sizeof(S_MINRRY_UVC_ATTR));
	memset(sUvcAttr, 0x00, sizeof(S_MINRRY_UVC_ATTR));

	if(sUvcAttr == NULL)
	{
		LOG_ERROR("MinrryUvc_Init error");
		return -1;
	}

	fd = UvcDevice_Open(mID);
	if(fd < 0)
	{
		LOG_ERROR("UvcDevice_Open error /dev/video%d",mID);
		return -1;
	}

	sUvcAttr->u32Width = 1920;
	sUvcAttr->u32Heigth = 1280;
	sUvcAttr->u32Pix = V4L2_PIX_FMT_H264;
	sUvcAttr->mFd = fd;

	sUvcAttrs[index] = sUvcAttr;

	*pHandle = (unsigned int)index;

	return 0;
}

void MinrryUvc_DeInit(unsigned int *pHandle)
{
	if(*pHandle >= UVC_MAX_DEVICE)
	{
		LOG_ERROR("this hande dev not open");
		return;
	}

	S_MINRRY_UVC_ATTR *sUvcAttr = sUvcAttrs[*pHandle];

	if(sUvcAttr->mFd)
		close(sUvcAttr->mFd);

	free(sUvcAttr);
	sUvcAttr = sUvcAttrs[*pHandle] = NULL;
	*pHandle = 0;
}

void MinrryUvc_SetListen(unsigned int Handle, UvcPostData pFunction)
{
	if(Handle >= UVC_MAX_DEVICE)
	{
		LOG_ERROR("this hande dev not open");
		return;
	}

	S_MINRRY_UVC_ATTR *sUvcAttr = sUvcAttrs[Handle];
	sUvcAttr->pPostData = pFunction;
}

int MinrryUvc_SetPara(unsigned int Handle,unsigned char selector,unsigned char *value, int size)
{
	if(Handle > UVC_MAX_DEVICE)
	{
		LOG_ERROR("this hande dev not open");
		return -1;
	}

	S_MINRRY_UVC_ATTR *sUvcAttr = sUvcAttrs[Handle];

	switch(selector)
	{
		case XU_CONTROL_ENCODE_VIDEOSIZE:
			{
				sUvcAttr->u32Width = value[1] | (value[2]<<8);
				sUvcAttr->u32Heigth = value[3] | (value[4] << 8);
			}break;

		case XU_CONTROL_ENCODE_CODEC:
			{
				char codec = value[1];
				switch(codec)
				{
					case 5:
						{
							sUvcAttr->u32Pix = V4L2_PIX_FMT_H264;
						}break;

					case 7:
						{
							//sUvcAttr->u32Pix = V4L2_PIX_FMT_H265;
						}break;
				}
			}break;
	}

	return UvcDevice_SetPara(sUvcAttr->mFd, UVC_EXTENSION_UNIT_ID, selector, value, size);
}

int MinrryUvc_GetPara(unsigned int Handle,unsigned char selector,unsigned char *value, int size)
{
	if(Handle > UVC_MAX_DEVICE)
	{
		LOG_ERROR("this hande dev not open");
		return -1;
	}

	S_MINRRY_UVC_ATTR *sUvcAttr = sUvcAttrs[Handle];

	return UvcDevice_GetPara(sUvcAttr->mFd, UVC_EXTENSION_UNIT_ID, selector, value, size);
}

int MinrryUvc_StartPreview(unsigned int Handle)
{
	int fd = 0;
	int ret = -1;

	if(Handle >= UVC_MAX_DEVICE)
	{
		LOG_ERROR("this hande dev not open");
		return -1;
	}

	S_MINRRY_UVC_ATTR *sUvcAttr = sUvcAttrs[Handle];

	fd = sUvcAttr->mFd;

	if(sUvcAttr->UvcIsStart)
		return 0;

	ret = UvcDevice_SetFormat(fd, sUvcAttr->u32Width, sUvcAttr->u32Heigth, sUvcAttr->u32Pix);
	if(ret < 0)
	{
		LOG_ERROR("UvcDevice_SetFormat error fd = %d, w = %d, h = %d ,pix = %d",fd, sUvcAttr->u32Width, sUvcAttr->u32Heigth, sUvcAttr->u32Pix);

//		return -1;
	}

	ret = UvcDevice_Mmap(fd, &(sUvcAttr->MmBuff));
	if(ret < 0)
	{
		LOG_ERROR("UvcDevice_Mmap error fd = %d ret = %d",fd, ret);
		return -1;
	}
	
	ret = UvcDevice_StreamOn(fd);
	if(ret != 0)
	{
		LOG_ERROR("UvcDevice_StreamOn error fd = %d ret = %d", fd, ret);
		return -1;
	}

	ret = UvcDevice_CleanFrame(fd, &sUvcAttr->MmBuff);
	if(ret != 0)
	{
		LOG_ERROR("call UvcDevice_CleanFrame error");
		return -1;
	}

	sUvcAttr->UvcIsStart = 1;
	sUvcAttr->Status = 0;
	pthread_create(&(sUvcAttr->pid), NULL, MinrryUvc_GetBuff, sUvcAttr);

	return 0;

}

int MinrryUvc_StopPreview(unsigned int Handle)
{
	void *status;
	int ret = 0;	
	if(Handle >= UVC_MAX_DEVICE)
	{
		LOG_ERROR("this hande dev not open");
		return -1;
	}

	S_MINRRY_UVC_ATTR *sUvcAttr = sUvcAttrs[Handle];

	if(!sUvcAttr->UvcIsStart)
		return 0;

	sUvcAttr->Status = 1;
	sUvcAttr->UvcIsStart = 0;
	pthread_join(sUvcAttr->pid, &status);

	ret = UvcDevice_StreamOff(sUvcAttr->mFd);
	if(ret < 0)
	{
		LOG_ERROR("UvcDevice_StreamOff error %d", sUvcAttr->mFd);
	}

	UvcDevice_UnMmap(sUvcAttr->mFd, &(sUvcAttr->MmBuff));

	return 0;
}

