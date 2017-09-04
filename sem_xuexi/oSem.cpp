/*************************************************************************
	> File Name: oSem.cpp
	> Author: abc2514671
	> Mail: abc2514671@163.com 
	> Created Time: Thu 17 Aug 2017 01:43:06 PM CST
 ************************************************************************/

#include "oSem.h"
#include "thepublic.h"
#include <errno.h>
#include <string.h>

//gcc -wAll -m64 -g3 -fPIC -c oSem.cpp -o oSem.o -I./

int OSem::iGetSem(long p_lSemKey, int p_iSemNum,int p_iSemRight, int &p_iSemId)
{
///��		�ܣ�	����ź���
///��		�룺	p_lSemKey�ź���KEYֵ
///			p_iSemRight�ź���Ȩ��
///��		����	p_iSemId�ź���ID
///�� �� 	ֵ��	�ɹ�0��ʧ��-1
///�� �� ��	��
///�����ֶΣ�	��
///�����ļ���	��
	
	int		l_iRet;
	int		l_iSemRight;
	
	l_iSemRight = p_iSemRight;
	
	l_iRet = semget(p_lSemKey,p_iSemNum, l_iSemRight);//ȱʡֻ��һ���źŵƼ���ֻ��һ���źŵơ�
	
	if (l_iRet < 0)
	{
		ErrorLog(ERR_PMUTEX_CREATE, "[key=%d, errno=%d, errmsg=%s]", p_lSemKey, errno, strerror(errno));
		return ERR_PMUTEX_CREATE;
	}
	p_iSemId = l_iRet;
	return 0;
}

int OSem::iCreateSem(long p_lSemKey, int p_iSems,int p_iSemRight, int &p_iSemId)
{
///��		�ܣ�	�����ź���
///��		�룺	p_lSemKey�ź���KEYֵ
///						p_iSems�źż��е��źŵ���
///			p_iSemRight�ź���Ȩ��
///��		����	p_iSemId�ź���ID
///�� �� 	ֵ��	�ɹ�0��ʧ��-1
///�� �� ��	��
///�����ֶΣ�	��
///�����ļ���	��
	
	int		l_iRet;
	int		l_iSemRight;
	
	l_iSemRight = p_iSemRight|IPC_CREAT|IPC_EXCL;
	//����һ���źŵƼ�
	l_iRet = semget(p_lSemKey, p_iSems, l_iSemRight);
	
	if (l_iRet < 0)
	{
		if (EEXIST == errno)
		{	//�Ѿ����ڣ��õ�����źŵƼ�
			l_iRet = semget(p_lSemKey, 0, p_iSemRight);
			if (l_iRet < 0)
			{
				ErrorLog(ERR_PMUTEX_CREATE, "[key=%d, errno=%d, errmsg=%s]", p_lSemKey, errno, strerror(errno));
				return ERR_PMUTEX_CREATE;
			}
			//ɾ���źŵƼ�
			if (semctl(l_iRet, 0, IPC_RMID) < 0)
			{
				ErrorLog(ERR_PMUTEX_CLOSE, "[key=%d, errno=%d, errmsg=%s]", p_lSemKey, errno, strerror(errno));
				return ERR_PMUTEX_CLOSE;
			}
			//�����µ�
			l_iRet = semget(p_lSemKey, p_iSems, l_iSemRight);
		}
		else
		{
			ErrorLog(ERR_PMUTEX_CREATE, "[key=%d, errno=%d, errmsg=%s]", p_lSemKey, errno, strerror(errno));
			return ERR_PMUTEX_CREATE;
		}
	}
	p_iSemId = l_iRet;
	return 0;
}

int OSem::iInitSem(int p_iSemId, int p_iSemNum,int p_iSemVal)
{
///��		�ܣ�	��ʼ���ź���ֵ
///��		�룺	p_iSemId�ź���ID
///						p_iSemNum,�����е�һ����Ա�±�
///			p_iSemVal�ź���ֵ
///��		����	��
///�� �� 	ֵ��	�ɹ�0��ʧ��-1
///�� �� ��	��
///�����ֶΣ�	��
///�����ļ���	��
	
	union semun_	 l_unArg;
	
	l_unArg.val = p_iSemVal;
	//��ʼ�������е�һ���źŵ�
	if(-1 == semctl(p_iSemId, p_iSemNum, SETVAL,l_unArg))
	{
		ErrorLog(ERR_PMUTEX_INIT, "[key=%d, errno=%d, errmsg=%s]", p_iSemId, errno, strerror(errno));
		return ERR_PMUTEX_INIT;
	}
		#ifdef _DEBUG
	{
		//union semun_ l_unionSem;
		int l_iTmpVal;
		l_iTmpVal = semctl(p_iSemId,p_iSemNum,GETVAL);
		printf("l_iTmpVal=%d\n",l_iTmpVal);
	}
	#endif
	return 0;
}

int OSem::iLock(int p_iSemId, int p_iSemNum, int p_iSemOp)
{
///��		�ܣ�	������Դ
///��		�룺	p_iSemId�ź���ID
///			p_iSenNum�ź������źż��е��±�
///			p_iSemOp������Դ��
///��		����	��
///�� �� 	ֵ��	�ɹ�0��ʧ��-1
///�� �� ��	��
///�����ֶΣ�	��
///�����ļ���	��
		
	struct sembuf l_sLock;
	
	memset(&l_sLock, 0x00, sizeof(l_sLock));
	
	#ifdef _DEBUG
	{
		//union semun_ l_unionSem;
		int l_iTmpVal;
		l_iTmpVal = semctl(p_iSemId,p_iSemNum,GETVAL);
		printf("l_iTmpVal=%d\n",l_iTmpVal);
	}
	#endif
	
	l_sLock.sem_num = p_iSemNum;
	l_sLock.sem_op = -1 * p_iSemOp;
	l_sLock.sem_flg = SEM_UNDO;
		
	if (-1 == semop(p_iSemId,&l_sLock, 1))
	{
		ErrorLog(ERR_PMUTEX_LOCK, "[key=%d, errno=%d, errmsg=%s]", p_iSemId, errno, strerror(errno));
		return ERR_PMUTEX_LOCK;
	}
	else
	{
		return 0;
	}
}

int OSem::iUnLock(int p_iSemId, int p_iSenNum, int p_iSemOp)
{
///��		�ܣ�	�ͷ���Դ
///��		�룺	p_iSemId�ź���ID
///			p_iSenNum�ź���
///			p_iSemOp�ͷ���Դ��
///��		����	��
///�� �� 	ֵ��	�ɹ�0��ʧ��-1
///�� �� ��	��
///�����ֶΣ�	��
///�����ļ���	��
	
	struct sembuf l_sUnlock;
	
	memset(&l_sUnlock, 0x00, sizeof(l_sUnlock));
	
	l_sUnlock.sem_num = p_iSenNum;
	l_sUnlock.sem_op = p_iSemOp;
	l_sUnlock.sem_flg = SEM_UNDO;
	
	if (-1 == semop(p_iSemId,&l_sUnlock, 1))
	{
		ErrorLog(ERR_PMUTEX_UNLOCK, "[key=%d,errno=%d, errmsg=%s]", p_iSemId, errno, strerror(errno));
		return ERR_PMUTEX_UNLOCK;
	}
	else
	{
		return 0;
	}
}

int OSem::iDelSem(int p_iSemId)
{
	int l_iRet;
	l_iRet = semctl(p_iSemId,0,IPC_RMID);
	if(l_iRet == -1)
	{
		ErrorLog(ERR_PMUTEX_CLOSE, "[key=%d, errno=%d, errmsg=%s]", p_iSemId, errno, strerror(errno));
		return ERR_PMUTEX_CLOSE;
	}
	return l_iRet;
}