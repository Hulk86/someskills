/*************************************************************************
	> File Name: baseShm.cpp
	> Author: abc2514671
	> Mail: abc2514671@163.com 
	> Created Time: Thu 17 Aug 2017 11:47:46 AM CST
 ************************************************************************/

#include "thepublic.h"
#include "baseShm.h"
#include <errno.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>

//gcc -wAll -m64 -g3 -fPIC -c obaseShm.cpp -o obaseShm.o -I./

//g++ -Wall -m64 -g3 -fPIC -c baseShm.cpp -o baseShm.o -I./

OShmBase::~OShmBase()
{
	//移除地址
	shmdt(m_pvShmAddr);
}

//功能：创建共享内存
//参数：
//返回值：
int OShmBase::iCreate(int p_iShmKey,int p_iLen)
{
	int l_iRet;
	if(m_iShmId != INVALID_VALUE)
	{	
		//如果已经打开了共享内存,删除旧的，创建新的。
		shmctl(m_iShmId,IPC_RMID,NULL);
		m_iShmId = INVALID_VALUE;
	}
	m_iShmId = shmget(p_iShmKey, p_iLen, IPC_CREAT | 0666 | IPC_EXCL);
	if(m_iShmId == -1)
	{
		m_iShmId = INVALID_VALUE;
		if(errno == EEXIST)
		{	
			//已经存在,删除
			//已经存在，得到这个信号灯集
			l_iRet = shmget(p_iShmKey, 0, 0666);
			if (l_iRet < 0)
			{
				ErrorLog(ERR_SHM_GET, "[key=%d, errno=%d, errmsg=%s]", p_iShmKey, errno, strerror(errno));
				return ERR_SHM_GET;
			}

			shmctl(l_iRet,IPC_RMID,NULL);
			l_iRet = shmget(p_iShmKey, p_iLen, IPC_CREAT | 0666 | IPC_EXCL);
			if(l_iRet == -1)
			{
				ErrorLog(ERR_SHM_CREATE, "[key=%d, errno=%d, errmsg=%s]", p_iShmKey, errno, strerror(errno));
				return ERR_SHM_CREATE;
			}
		}
		else
		{	
			ErrorLog(ERR_SHM_CREATE, "[key=%d, errno=%d, errmsg=%s]", p_iShmKey, errno, strerror(errno));
			return ERR_SHM_CREATE;
		}
		
		m_iShmId = l_iRet;
	}
  
	m_iShmKey = p_iShmKey;
	m_iShmLen = p_iLen;
	//added by hxj
	//绑定地址
	if(m_pvShmAddr == NULL)
	{
		m_pvShmAddr = shmat(m_iShmId, NULL, 0);
		if(m_pvShmAddr == (void *)-1 )
		{
			ErrorLog(ERR_SHM_BIND, "[key=%d, errno=%d, errmsg=%s]", p_iShmKey, errno, strerror(errno));
			return ERR_SHM_BIND;
		}
	}
	//end add
	return 0;
}

//得到一个共享内存
int OShmBase::iGetShm(int p_iShmKey)
{
	if(m_iShmId != INVALID_VALUE)
	{	
		return 0;
	}
	m_iShmId = shmget(p_iShmKey, 0, 0666);
  if(m_iShmId == -1)
  {
  	 m_iShmId = INVALID_VALUE;
  	 ErrorLog(ERR_SHM_GET, "[key=%d, errno=%d, errmsg=%s]", p_iShmKey, errno, strerror(errno));
     return ERR_SHM_GET;
	}
  m_iShmKey = p_iShmKey;
  //得到共享内存的长度
  struct shmid_ds l_shm;
  
  if(shmctl(m_iShmId,IPC_STAT,&l_shm) == -1)
  {
  	ErrorLog(ERR_SHM_STATUS, "[key=%d, errno=%d, errmsg=%s]", p_iShmKey, errno, strerror(errno));
  	return ERR_SHM_STATUS;
	}
	m_iShmLen = l_shm.shm_segsz;
	//绑定地址
	if(m_pvShmAddr == NULL)
 	{
		m_pvShmAddr = shmat(m_iShmId, NULL, 0);
		if(m_pvShmAddr == (void *)-1 )
		{
			ErrorLog(ERR_SHM_BIND, "[key=%d, errno=%d, errmsg=%s]", p_iShmKey, errno, strerror(errno));
  	return ERR_SHM_STATUS;
		}
	}
  return 0;
}


//功能：写数据
//参数：p_iPos [共享内存中的偏移位置]
//返回：
int OShmBase::iWriteData(const void *p_pacData,const int p_iLen,int p_iPos)
{
	void *l_sAddr = NULL;
	int l_iRet;
	//added by hxj
	l_sAddr = m_pvShmAddr;
	//end add

	//printf("start %d,%d\n",(int)now.time,(int)now.millitm);
	//判断共享内存的大小
	if(p_iPos < 0 || p_iLen <= 0)
	{
		ErrorLog(ERR_SHMM_FORMAT, "[data len=%d, pos=%d]", p_iLen, p_iPos);
		return ERR_SHMM_FORMAT;
	}
	if(p_iLen >= m_iShmLen || p_iPos + p_iLen >= m_iShmLen)
	{
		ErrorLog(ERR_SHMM_FULL, "[data len=%d, max=%d]", p_iPos+p_iLen, m_iShmLen);
		return ERR_SHMM_FULL;
	}
	
  memcpy((void *)((char *)l_sAddr + p_iPos),p_pacData,p_iLen);
  

  return 0;
}	


//功能：读共享内存数据
//参数：p_pacData,读出的数据缓冲区
//	    p_iLen 是需要读出的长度。
//			p_iPos[共享内存位置的偏移位置]
//返回：读出的长度值。[<=0]失败。[>0]成功
//      如果成功。
int OShmBase::iReadData(void *p_pacData,int p_iDstLen,int p_iReadLen,int p_iPos)
{
	int l_iLen = p_iReadLen + p_iPos;//偏移长度和读出的数据之和
	int l_iRemLen = m_iShmLen - p_iPos; //剩余的和
	void *l_sAddr = NULL;
	int l_iRet;
	
	//added by hxj
	l_sAddr = m_pvShmAddr;
	//end add
	//l_sAddr = (void *)m_uiAddr;
	if(p_iPos < 0 || l_iLen <= 0 || p_iDstLen <= 0)
	{
		ErrorLog(ERR_SHMM_FORMAT, "[data len=%d, pos=%d]", l_iLen, p_iPos);
		return ERR_SHMM_FORMAT;
	}
	
	if(p_iPos >= m_iShmLen)
	{
		ErrorLog(ERR_SHMM_FULL, "[data len=%d, max=%d]", p_iPos, m_iShmLen);
		return ERR_SHMM_FULL;
	}
	
	if(p_iReadLen <= 0 || (p_iReadLen > 0 && l_iLen >= m_iShmLen))
	//读出剩余全部数据
	{
		if(p_iDstLen <= l_iRemLen) //目标字串大小不足
		{
			ErrorLog(ERR_SHMM_BUF_LESS, "[dest=%d, read=%d]", p_iDstLen, l_iRemLen);
			return ERR_SHMM_BUF_LESS;
		}
	
		//解锁
		memcpy(p_pacData,(void *)((char *)l_sAddr + p_iPos),l_iRemLen);
	
		return l_iRemLen;
	}
	else //读出部分数据
	{
		if(p_iReadLen > p_iDstLen) //目标字串大小不足
		{
			ErrorLog(ERR_SHMM_BUF_LESS, "[dest=%d, read=%d]", p_iDstLen, p_iReadLen);
			return ERR_SHMM_BUF_LESS;
		}

		//复制字串
		memcpy(p_pacData,(void *)((char *)l_sAddr + p_iPos),p_iReadLen);
		
		return p_iReadLen;
	}		
	return -1;
}

//清空某一区域数据
int OShmBase::iClearData(int p_iStartpos,int p_iLen,char p_iClearChar)
{
	int l_iLen = 0;
	void *l_sAddr = NULL;
	int l_iRet;
	
	//added by hxj
	l_sAddr = m_pvShmAddr;
	//end add
	l_iLen = p_iLen;
	if(l_iLen <= 0 || p_iStartpos < 0)
	{
		ErrorLog(ERR_SHMM_FORMAT, "[data len=%d, pos=%d]", l_iLen, p_iStartpos);
		return ERR_SHMM_FORMAT;
	}
	if(p_iStartpos >= m_iShmLen)
	{
		ErrorLog(ERR_SHMM_FULL, "[data len=%d, max=%d]", p_iStartpos, m_iShmLen);
		return ERR_SHMM_FULL;
	}
	
	if(p_iLen <= 0) //当作全部清空
	{
		l_iLen = m_iShmLen - p_iStartpos;
	}
	if(l_iLen + p_iStartpos > m_iShmLen)
		l_iLen = m_iShmLen - p_iStartpos;
	
	//写数据
	memset((void *)((unsigned char *)l_sAddr + p_iStartpos),p_iClearChar,l_iLen);

	return 0;
}


//删除共享内存
int OShmBase::iDelShm()
{
	int 	l_iRet;
	#ifdef __SOLARIS__
	l_iRet = shmctl(m_iShmId,0,(struct shmid_ds *)IPC_RMID);
	#else
	l_iRet = shmctl(m_iShmId,0,IPC_RMID);
	#endif
	if(l_iRet == -1)
	{
		ErrorLog(ERR_SHMM_CLOSE, "[errno=%d, errmsg=%d]", errno, strerror(errno));
		return ERR_SHMM_CLOSE;
	}
	m_iShmId = INVALID_VALUE;
	m_iShmLen = INVALID_VALUE;
	m_iShmKey = INVALID_VALUE;
	
	return l_iRet;
}
