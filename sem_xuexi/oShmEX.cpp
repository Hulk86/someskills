/*************************************************************************
	> File Name: oShmEX.cpp
	> Author: abc2514671
	> Mail: abc2514671@163.com 
	> Created Time: Thu 17 Aug 2017 01:37:08 PM CST
 ************************************************************************/


#include "oShmEX.h"
#include "oSem.h"
//#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/timeb.h>

//g++ -Wall -m64 -g3 -fPIC -c oShmEX.cpp -o oShmEX.o -I./
//g++ -w -m64 -g3 -fPIC -c oShmEX.cpp -o oShmEX.o -I./              //小w忽略所有警告
//g++ -Wextra -m64 -g3 -fPIC -c oShmEX.cpp -o oShmEX.o -I./ -ldl -lpthread

OShm::~OShm()
{
	//移除地址
	shmdt(m_pvShmAddr);
}

//功能：创建共享内存
//参数：
//返回值：
int OShm::iCreate(int p_iShmKey,int p_iLen)
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
			m_iShmId = shmget(p_iShmKey, p_iLen, IPC_CREAT | 0666 | IPC_EXCL);
			if(m_iShmId == -1)
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
int OShm::iGetShm(int p_iShmKey)
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


//设置信号灯集
int OShm::iSetSem(int p_iSemId)
{
	if(p_iSemId >= 0)//modified by hxj 2007-6-5 9:20
		m_iSemId = p_iSemId;//保存信号灯集
	else
		return ERR_SHM_INVALID_ID;
	return 0;
}


//功能：写数据
//参数：p_iPos [共享内存中的偏移位置]
//返回：
int OShm::iWriteData(const void *p_pacData,const int p_iLen,int p_iPos)
{
	struct timeb  now;
	void *l_sAddr = NULL;
	int l_iRet;
	//added by hxj
	l_sAddr = m_pvShmAddr;
	//end add
	ftime(&now);
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
	
	//加锁
	if((l_iRet = OSem::iLock(m_iSemId)) != 0)
	{
		ErrorLog(ERR_SHMM_LOCK, "[%s==>%s--->%d][errno=%d, errmsg=%d]", __FILE__,__FUNCTION__,__LINE__ , errno, strerror(errno));
		return ERR_SHMM_LOCK;
	}
  memcpy((void *)((char *)l_sAddr + p_iPos),p_pacData,p_iLen);
  
  //解锁
  if((l_iRet = OSem::iUnLock(m_iSemId)) != 0)
	{
		ErrorLog(ERR_SHMM_UNLOCK, "[%s==>%s--->%d][errno=%d, errmsg=%d]", __FILE__,__FUNCTION__,__LINE__ , errno, strerror(errno));
		return ERR_SHMM_UNLOCK;
	}
  ftime(&now);
  return 0;
}	


//功能：读共享内存数据
//参数：p_pacData,读出的数据缓冲区
//	    p_iLen 是需要读出的长度。
//			p_iPos[共享内存位置的偏移位置]
//返回：读出的长度值。[<=0]失败。[>0]成功
//      如果成功。
int OShm::iReadData(void *p_pacData,int p_iDstLen,int p_iReadLen,int p_iPos)
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
		ErrorLog(ERR_SHMM_FORMAT, "[%s==>%s--->%d][data len=%d, pos=%d]", __FILE__,__FUNCTION__,__LINE__ ,l_iLen, p_iPos);
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
		//加锁
		if((l_iRet = OSem::iLock(m_iSemId)) != 0)
		{
			ErrorLog(ERR_SHMM_LOCK, "[%s==>%s--->%d][errno=%d, errmsg=%d]", __FILE__,__FUNCTION__,__LINE__ , errno, strerror(errno));
			return ERR_SHMM_LOCK;
		}
	
		//解锁
		memcpy(p_pacData,(void *)((char *)l_sAddr + p_iPos),l_iRemLen);
		//shmdt(l_sAddr);
		if((l_iRet = OSem::iUnLock(m_iSemId)) != 0)
		{
			ErrorLog(ERR_SHMM_UNLOCK, "[%s==>%s--->%d][errno=%d, errmsg=%d]", __FILE__,__FUNCTION__,__LINE__ , errno, strerror(errno));
			return ERR_SHMM_UNLOCK;
		}
		return l_iRemLen;
	}
	else //读出部分数据
	{
		if(p_iReadLen > p_iDstLen) //目标字串大小不足
		{
			ErrorLog(ERR_SHMM_BUF_LESS, "[dest=%d, read=%d]", p_iDstLen, p_iReadLen);
			return ERR_SHMM_BUF_LESS;
		}
		//加锁
		if((l_iRet = OSem::iLock(m_iSemId)) != 0)
		{
			ErrorLog(ERR_SHMM_LOCK, "[%s==>%s--->%d][errno=%d, errmsg=%d]", __FILE__,__FUNCTION__,__LINE__ , errno, strerror(errno));
			return ERR_SHMM_LOCK;
		}

		//复制字串
		memcpy(p_pacData,(void *)((char *)l_sAddr + p_iPos),p_iReadLen);
		//解锁
  	if((l_iRet = OSem::iUnLock(m_iSemId)) != 0)
		{
			ErrorLog(ERR_SHMM_UNLOCK, "[%s==>%s--->%d][errno=%d, errmsg=%d]", __FILE__,__FUNCTION__,__LINE__ , errno, strerror(errno));
			return ERR_SHMM_UNLOCK;
		}
		return p_iReadLen;
	}		
	return -1;
}

//清空某一区域数据
int OShm::iClearData(int p_iStartpos,int p_iLen,char p_iClearChar)
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
		
	//加锁
	if((l_iRet = OSem::iLock(m_iSemId)) != 0)
	{
		ErrorLog(ERR_SHMM_LOCK, "[%s==>%s--->%d][errno=%d, errmsg=%d]", __FILE__,__FUNCTION__,__LINE__ , errno, strerror(errno));
		return ERR_SHMM_LOCK;
	}
	
	//写数据
	memset((void *)((unsigned char *)l_sAddr + p_iStartpos),p_iClearChar,l_iLen);
	//释放
	//shmdt(l_sAddr);
  //解锁
  if((l_iRet = OSem::iUnLock(m_iSemId)) != 0)
	{
		ErrorLog(ERR_SHMM_LOCK, "[%s==>%s--->%d][errno=%d, errmsg=%d]", __FILE__,__FUNCTION__,__LINE__ , errno, strerror(errno));
		return ERR_SHMM_LOCK;
	}
	return 0;
}


//删除共享内存
int OShm::iDelShm()
{
	int 	l_iRet;
	#ifdef __SOLARIS__
	l_iRet = shmctl(m_iShmId,0,(struct shmid_ds *)IPC_RMID);
	#else
	l_iRet = shmctl(m_iShmId,0,IPC_RMID);
	#endif
	if(l_iRet == -1)
	{
		ErrorLog(ERR_SHMM_CLOSE, "[%s==>%s--->%d][errno=%d, errmsg=%d]", __FILE__,__FUNCTION__,__LINE__ , errno, strerror(errno));
		return ERR_SHMM_CLOSE;
	}
	m_iShmId = INVALID_VALUE;
	m_iShmLen = INVALID_VALUE;
	m_iShmKey = INVALID_VALUE;
	return l_iRet;
}


int OShmEX::iProductorUploadFinish( int& thePos)//double lock
{
	 int l_iShmEXDataUnitSize = sizeof(struct OShmEXDataUnit); //data unit size
	 int l_iOffset = l_iShmEXDataUnitSize * thePos;
	 struct OShmEXDataUnit * l_pCurrent;
	 
	 l_pCurrent =(struct OShmEXDataUnit*) (m_pvShmAddr + l_iOffset);
	 l_pCurrent->m_uiStatus=DATA_STATUS_READY;
	 l_pCurrent->m_iUsingPID=getpid();
	 	
	 int l_iTmpFlag = 0;
	 l_iTmpFlag = OSem::iUnLock(m_iConsumSemId,0,1); //unlock consumer, adding consumer entrance privilege
	 
	 return l_iTmpFlag;
 
}

int OShmEX::iConsumerUseFinish( int& thePos)//double lock
{
	 int l_iShmEXDataUnitSize = sizeof(struct OShmEXDataUnit); //data unit size
	 int l_iOffset = l_iShmEXDataUnitSize * thePos;
	 struct OShmEXDataUnit * l_pCurrent;
	 
	 l_pCurrent =(struct OShmEXDataUnit*) (m_pvShmAddr + l_iOffset);
	 l_pCurrent->m_uiStatus=DATA_STATUS_IDEL;
	 l_pCurrent->m_iUsingPID=getpid();
	 
	 int l_iTmpFlag = 0;
	 l_iTmpFlag = OSem::iUnLock(m_iProductSemId,0,1);  //unlock productor,adding entrance privilege. when productor prepared the data , it will ask for the privilege.
	 
	 return l_iTmpFlag;
}


int OShmEX::iProductorWaitForUpload( int& thePos)//double lock
{
	//先判断 m_iProductSemId 和 m_iSemId
  //OSem::iLock(int p_iSemId, int p_iSemNum, int p_iSemOp)
  int l_iFlag_1 = -1;
  int l_iFlag_2 = -1;
  int l_iShmEXDataUnitSize = sizeof(struct OShmEXDataUnit); //data unit size
  int tmp_i = 0;
  struct OShmEXDataUnit * l_pCurrent;
  
  thePos = -1; //不成功的时候，收到的是-1
  
  l_iFlag_1 = OSem::iLock(m_iProductSemId);
	if(l_iFlag_1 != 0)
		return l_iFlag_1;
		
	l_iFlag_2 = OSem::iLock(m_iSemId);	
	if(l_iFlag_2 != 0)
		return l_iFlag_2;
   l_pCurrent = (struct OShmEXDataUnit*) (m_pvShmAddr);
  for(tmp_i = 0; tmp_i !=  this->m_iShmEXDataUnitAmount; tmp_i++)
  {
  	//l_pCurrent = (struct OShmEXDataUnit*) (m_pvShmAddr+l_iShmEXDataUnitSize);
  	if( l_pCurrent->m_uiStatus == DATA_STATUS_EMPTY ||  l_pCurrent->m_uiStatus == DATA_STATUS_IDEL)
  	{
  		l_pCurrent->m_uiStatus = DATA_STATUS_PRODUCT_USING;
  		l_pCurrent->m_iUsingPID = getpid();
  		thePos = tmp_i;
  		
  		break;
  	}
  	else
  	{
  	  l_pCurrent ++;
  	 // l_pCurrent += l_iShmEXDataUnitSize;
  	}
  	  
  }
  
  l_iFlag_2 = OSem::iUnLock(m_iSemId);	
	if(l_iFlag_2 != 0)
		return l_iFlag_2;
			
  return 0;

}

int OShmEX::iConsumerWaitForUse( int& thePos)//先进去大门，才会帮你选择给你一条数据使用
{
	int l_iFlag_1 = -1;
  int l_iFlag_2 = -1;
  int l_iShmEXDataUnitSize = sizeof(struct OShmEXDataUnit); //data unit size
  int tmp_i = 0;
  struct OShmEXDataUnit * l_pCurrent;
  
	thePos = -1; //不成功的时候，收到的是-1
	l_iFlag_1 = OSem::iLock(m_iConsumSemId);
  if(l_iFlag_1 != 0)
		return l_iFlag_1;
		
	l_iFlag_2 = OSem::iLock(m_iSemId);	
	if(l_iFlag_2 != 0)
		return l_iFlag_2;
  
   l_pCurrent = (struct OShmEXDataUnit*) (m_pvShmAddr); 
  for(tmp_i = 0; tmp_i !=  this->m_iShmEXDataUnitAmount; tmp_i++)
  {
  	
  	if( l_pCurrent->m_uiStatus == DATA_STATUS_READY )
  	{
  		l_pCurrent->m_uiStatus = DATA_STATUS_COMSUMER_USING;
  		l_pCurrent->m_uiStatus = getpid();
  		thePos = tmp_i;
  		
  		break;
  	}
  	else
  	{
  	   printf("[%X][%ld][%x]\n",l_pCurrent,l_pCurrent->m_uiStatus,l_pCurrent->m_iUsingPID);
  	  l_pCurrent ++;
  	}
  }
  
  l_iFlag_2 = OSem::iUnLock(m_iSemId);	
	if(l_iFlag_2 != 0)
		return l_iFlag_2;
		
}



int OShmEX::iCreate(int p_iShmKey,int p_iArrAmount)
{
		//p_iLen 代表有多少组 OShmEXDataUnit
		int l_iTheFlag = 0;
		int l_iTheBuffSize = 0;
		
		l_iTheBuffSize = p_iArrAmount * sizeof(struct OShmEXDataUnit);
		
		l_iTheFlag = OShm::iCreate(p_iShmKey,l_iTheBuffSize);
		if(l_iTheFlag == 0)	
		  this->m_iShmEXDataUnitAmount = p_iArrAmount;
		return l_iTheFlag;
}



int OShmEX::iSetProductSem(int p_iSemId)
{
	if(p_iSemId >= 0)//modified by hxj 2007-6-5 9:20
		m_iProductSemId = p_iSemId;//保存信号灯集
	else
		return ERR_SHM_INVALID_ID;
	return 0;

}

int OShmEX::iSetConsumeSem(int p_iSemId)
{
	if(p_iSemId >= 0)//modified by hxj 2007-6-5 9:20
		m_iConsumSemId = p_iSemId;//保存信号灯集
	else
		return ERR_SHM_INVALID_ID;
	return 0;
	
}

int OShmEX::iSetLockerSem(int p_iSemId)
{	
	return this->OShm::iSetSem(p_iSemId);
}

//sed -i 's/xx/gg/g' 
OShmEX::~OShmEX()
{
	this->OShm::~OShm();
}

