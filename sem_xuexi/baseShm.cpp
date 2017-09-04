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
	//�Ƴ���ַ
	shmdt(m_pvShmAddr);
}

//���ܣ����������ڴ�
//������
//����ֵ��
int OShmBase::iCreate(int p_iShmKey,int p_iLen)
{
	int l_iRet;
	if(m_iShmId != INVALID_VALUE)
	{	
		//����Ѿ����˹����ڴ�,ɾ���ɵģ������µġ�
		shmctl(m_iShmId,IPC_RMID,NULL);
		m_iShmId = INVALID_VALUE;
	}
	m_iShmId = shmget(p_iShmKey, p_iLen, IPC_CREAT | 0666 | IPC_EXCL);
	if(m_iShmId == -1)
	{
		m_iShmId = INVALID_VALUE;
		if(errno == EEXIST)
		{	
			//�Ѿ�����,ɾ��
			//�Ѿ����ڣ��õ�����źŵƼ�
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
	//�󶨵�ַ
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

//�õ�һ�������ڴ�
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
  //�õ������ڴ�ĳ���
  struct shmid_ds l_shm;
  
  if(shmctl(m_iShmId,IPC_STAT,&l_shm) == -1)
  {
  	ErrorLog(ERR_SHM_STATUS, "[key=%d, errno=%d, errmsg=%s]", p_iShmKey, errno, strerror(errno));
  	return ERR_SHM_STATUS;
	}
	m_iShmLen = l_shm.shm_segsz;
	//�󶨵�ַ
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


//���ܣ�д����
//������p_iPos [�����ڴ��е�ƫ��λ��]
//���أ�
int OShmBase::iWriteData(const void *p_pacData,const int p_iLen,int p_iPos)
{
	void *l_sAddr = NULL;
	int l_iRet;
	//added by hxj
	l_sAddr = m_pvShmAddr;
	//end add

	//printf("start %d,%d\n",(int)now.time,(int)now.millitm);
	//�жϹ����ڴ�Ĵ�С
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


//���ܣ��������ڴ�����
//������p_pacData,���������ݻ�����
//	    p_iLen ����Ҫ�����ĳ��ȡ�
//			p_iPos[�����ڴ�λ�õ�ƫ��λ��]
//���أ������ĳ���ֵ��[<=0]ʧ�ܡ�[>0]�ɹ�
//      ����ɹ���
int OShmBase::iReadData(void *p_pacData,int p_iDstLen,int p_iReadLen,int p_iPos)
{
	int l_iLen = p_iReadLen + p_iPos;//ƫ�Ƴ��ȺͶ���������֮��
	int l_iRemLen = m_iShmLen - p_iPos; //ʣ��ĺ�
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
	//����ʣ��ȫ������
	{
		if(p_iDstLen <= l_iRemLen) //Ŀ���ִ���С����
		{
			ErrorLog(ERR_SHMM_BUF_LESS, "[dest=%d, read=%d]", p_iDstLen, l_iRemLen);
			return ERR_SHMM_BUF_LESS;
		}
	
		//����
		memcpy(p_pacData,(void *)((char *)l_sAddr + p_iPos),l_iRemLen);
	
		return l_iRemLen;
	}
	else //������������
	{
		if(p_iReadLen > p_iDstLen) //Ŀ���ִ���С����
		{
			ErrorLog(ERR_SHMM_BUF_LESS, "[dest=%d, read=%d]", p_iDstLen, p_iReadLen);
			return ERR_SHMM_BUF_LESS;
		}

		//�����ִ�
		memcpy(p_pacData,(void *)((char *)l_sAddr + p_iPos),p_iReadLen);
		
		return p_iReadLen;
	}		
	return -1;
}

//���ĳһ��������
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
	
	if(p_iLen <= 0) //����ȫ�����
	{
		l_iLen = m_iShmLen - p_iStartpos;
	}
	if(l_iLen + p_iStartpos > m_iShmLen)
		l_iLen = m_iShmLen - p_iStartpos;
	
	//д����
	memset((void *)((unsigned char *)l_sAddr + p_iStartpos),p_iClearChar,l_iLen);

	return 0;
}


//ɾ�������ڴ�
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
