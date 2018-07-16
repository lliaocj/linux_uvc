#include <uvc.h>
#include <sys/ioctl.h>

static int backfd = 0;
static int backpix = 0;

int xioctl(int fd, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fd, request, arg);
	} while (-1 == r && EINTR == errno);


	return r;
}

int UvcDevice_Open(int Id)
{
	int fd = 0;
	char path[64] = {0};

	sprintf(path, "/dev/video%d", Id);

	fd = open(path, O_RDWR);
	if(fd < 0)
	{
		LOG_ERROR("open uvc %s error", path);	

		return -1;
	}

	return fd;
}

int UvcDevice_Close(int fd)
{
	if(fd)
		close(fd);

	return 0;
}

int UvcDevice_SetFormat(int fd, int w, int h, int pix)
{
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = w;
	format.fmt.pix.height = h;
	format.fmt.pix.pixelformat = pix;
	format.fmt.pix.field = V4L2_FIELD_INTERLACED;
	
#if 0
	if(fd == backfd && pix == backpix)
		return 0;
#endif

	if(xioctl(fd, VIDIOC_S_FMT, &format) != 0)
	{
		LOG_ERROR("set VIDIOC_S_FMT error %s",strerror(errno));

		return -1;
	}

	backfd = fd;
	backpix = pix;

	return 0;
}

int UvcDevice_GetFormat(int fd, int *w, int *h, int *pix)
{
	struct v4l2_format format;

	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(xioctl(fd, VIDIOC_G_FMT, &format) != 0)
	{
		LOG_ERROR("set format error %s",strerror(errno));

		return -1;
	}

	*w = format.fmt.pix.width;
	*h = format.fmt.pix.height;
	*pix = format.fmt.pix.pixelformat;

	return 0;
}

int UvcDevice_ReqBuffers(int fd, struct v4l2_requestbuffers *pBuff)
{
	memset(pBuff, 0x00, sizeof(struct v4l2_requestbuffers));

	pBuff->count = 4;
	pBuff->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pBuff->memory = V4L2_MEMORY_MMAP;

	if(xioctl(fd, VIDIOC_REQBUFS, pBuff) != 0)
	{
		LOG_ERROR("set VIDIOC_REQBUFS error %s",strerror(errno));

		return -1;
	}

	return 0;
}

int UvcDevice_QueBuffers(int fd, struct v4l2_buffer *pBuff, int index)
{
	memset(pBuff, 0x00, sizeof(struct v4l2_buffer));

	pBuff->index = index;
	pBuff->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pBuff->memory = V4L2_MEMORY_MMAP;

	if(xioctl(fd, VIDIOC_QUERYBUF, pBuff) != 0)
	{
		LOG_ERROR("set VIDIOC_QUERYBUF error %s",strerror(errno));

		return -1;
	}

	return 0;
}

int UvcDevice_Mmap(int fd, mMapBuffers *pMmBuff)
{
	int ret = 0;
	int index = 0;
	struct v4l2_requestbuffers req_buff;
	struct v4l2_buffer v4l2_buff;

	ret = UvcDevice_ReqBuffers(fd, &req_buff);
	if(ret != 0)
	{
		LOG_ERROR("call UvcDevice_ReqBuffers error\n");
		return -1;
	}

	pMmBuff->_buff = (v_buffers * )malloc(req_buff.count * sizeof(v_buffers));
	memset(pMmBuff->_buff, 0x00, sizeof(req_buff.count * sizeof(v_buffers)));

	for(index = 0; index < (int )req_buff.count; index ++)
	{
		ret = UvcDevice_QueBuffers(fd, &v4l2_buff, index);		
		if(ret != 0)
		{
			LOG_ERROR("call UvcDevice_QueBuffers error");
			goto error1;
		}

		pMmBuff->_buff[index].size = v4l2_buff.length;
		pMmBuff->_buff[index].start = mmap(NULL, v4l2_buff.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, v4l2_buff.m.offset);
		if(pMmBuff->_buff[index].start == MAP_FAILED)
		{
			LOG_ERROR("mmap error");
			goto error2;
		}

		if(xioctl(fd, VIDIOC_QBUF, &v4l2_buff) != 0)
		{
			LOG_ERROR("set VIDIOC_QBUF error %s",strerror(errno));
			goto error3;
		}
	}	

	pMmBuff->total = req_buff.count;

	return 0;

error3:
	index ++;
error2:
	for(ret = 0; ret < index; ret ++)
		munmap(pMmBuff->_buff[ret].start, pMmBuff->_buff[ret].size);
error1:
	free(pMmBuff->_buff);
	pMmBuff->_buff = NULL;
	return -1;
}

int UvcDevice_UnMmap(int fd, mMapBuffers *pMmBuff)
{
	int index = 0;

	for(index = 0; index < (int)pMmBuff->total; index ++)
	{
		munmap(pMmBuff->_buff[index].start, pMmBuff->_buff[index].size);
	}	

	if(pMmBuff->_buff)
	{
		free(pMmBuff->_buff);
		pMmBuff->_buff = NULL;
	}

	return 0;
}

int UvcDevice_DQBuffers(int fd, FrameData *pFrame, mMapBuffers *pBuff)
{
	memset(&(pFrame->buff), 0x00, sizeof(struct v4l2_buffer));
	pFrame->buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pFrame->buff.memory = V4L2_MEMORY_MMAP;

	if(xioctl(fd, VIDIOC_DQBUF, &(pFrame->buff)) != 0)
	{
		LOG_ERROR("call VIDIOC_DQBUF error");
		return -1;
	}
	
	if(pFrame->buff.index < pBuff->total)
	{
		pFrame->data = pBuff->_buff[pFrame->buff.index].start;
		pFrame->length = pFrame->buff.bytesused;
#if 0
		//多路码流才需要用到下面的设置
	//	pFrame->data[0] = pFrame->buff.reserved >> 24;
		if(pFrame->data[0] >> 7 == 0x1)
		{
			pFrame->data[0] &= ~0x80;
		}
		else
		{
			pFrame->data[0] |= (0x1 << 7);
		}
#endif

	}
	return 0;
}

int UvcDevice_QBuffers(int fd, FrameData *pFrame)
{
	if(xioctl(fd, VIDIOC_QBUF, &(pFrame->buff)) != 0)
	{
		LOG_ERROR("call VIDIOC_DQBUF error");
		return -1;
	}

	return 0;
}

int UvcDevice_StreamOn(int fd)
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(xioctl(fd, VIDIOC_STREAMON, &type) != 0)
	{
		LOG_ERROR("set VIDIOC_STREAMON error %s",strerror(errno));
		return -1;
	}

	return 0;
}

int UvcDevice_StreamOff(int fd)
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(xioctl(fd, VIDIOC_STREAMOFF, &type) != 0)
	{
		LOG_ERROR("set VIDIOC_STREAMOFF error %s",strerror(errno));
		return -1;
	}

	return 0;
}

int UvcDevice_CleanFrame(int fd, mMapBuffers *pBuff)
{
	int index = 0;
	struct v4l2_buffer v4l2_buff;

	v4l2_buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buff.memory = V4L2_MEMORY_MMAP;

	for(index = 0; index < (int)pBuff->total; index ++)
	{
		if(xioctl(fd, VIDIOC_DQBUF, &v4l2_buff) != 0)
		{
			LOG_ERROR("set VIDIOC_DQBUF error %s",strerror(errno));
			return -1;
		}

		if(xioctl(fd, VIDIOC_QBUF, &v4l2_buff) != 0)
		{
			LOG_ERROR("set VIDIOC_QBUF error %s",strerror(errno));
			return -1;
		}
	}

	return 0;
}

int UvcDevice_SetPara(int fd, uint8_t unit,uint8_t selector,uint8_t *value,int size)
{
	struct v4l2_minrray_ext_ctr mext_ctr;

	mext_ctr.unit = unit;
	mext_ctr.selector = selector;
	mext_ctr.query = UVC_SET_CUR;
	mext_ctr.intfnum = UVC_INTF_CONTROL;

	mext_ctr.size = size;
	memset(mext_ctr.c,0,sizeof(mext_ctr.c));
	memcpy(mext_ctr.c,value,size);

	if(xioctl(fd, VIDIOC_MINRRAY_EXT_CTRLS, (void *)(&mext_ctr)) != 0)
	{
		LOG_ERROR("xioctl select %#x size %d %s\n",selector, size, strerror(errno));
		return -1;
	}

	return 0;
}

int UvcDevice_SetParaExt(int fd, int selector,int *mvalue_Absolute,int size)
{
	struct v4l2_ext_control control;
	struct v4l2_ext_controls controls;
	int ret;
	control.id = selector;
	control.value = mvalue_Absolute[0]*3600;
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = size;
	controls.controls = &control;
	ret = ioctl(fd, VIDIOC_S_EXT_CTRLS, &controls);
	if (ret < 0)
		return ret;

	return 0;

}

int UvcDevice_GetPara(int fd, uint8_t unit,uint8_t selector,uint8_t *value,int size)
{
	int	len = 0;
	struct v4l2_minrray_ext_ctr mext_ctr;

	mext_ctr.selector = selector;
	mext_ctr.query = UVC_GET_CUR;
	mext_ctr.intfnum = UVC_INTF_CONTROL;
	mext_ctr.unit = UVC_EXTENSION_UNIT_ID;                                                                                                                                                   
	mext_ctr.size = size;
	len = size;
	memset(mext_ctr.c,0,sizeof(mext_ctr.c));

	if(xioctl(fd, VIDIOC_MINRRAY_EXT_CTRLS, (void *)(&mext_ctr)) != 0)
	{
		LOG_ERROR("xioctl select %#x size %d %s\n",selector, size, strerror(errno));
		return -1;
	}

	memcpy(value,mext_ctr.c,len);

	return 0;
}

int UvcDevice_SetIdr(int fd, int index)
{
	unsigned char value[16] = {0};

	value[0] = index;
	value[1] = 0;

	return UvcDevice_SetPara(fd, UVC_EXTENSION_UNIT_ID, XU_CONTROL_ENCODE_IDR, value, 2);	
}

