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
///功		能：	获得信号量
///输		入：	p_lSemKey信号量KEY值
///			p_iSemRight信号量权限
///输		出：	p_iSemId信号量ID
///返 回 	值：	成功0，失败-1
///操 作 表：	无
///操作字段：	无
///操作文件：	无
	
	int		l_iRet;
	int		l_iSemRight;
	
	l_iSemRight = p_iSemRight;
	
	l_iRet = semget(p_lSemKey,p_iSemNum, l_iSemRight);//缺省只有一个信号灯集里只有一个信号灯。
	
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
///功		能：	创建信号量
///输		入：	p_lSemKey信号量KEY值
///						p_iSems信号集中的信号灯数
///			p_iSemRight信号量权限
///输		出：	p_iSemId信号量ID
///返 回 	值：	成功0，失败-1
///操 作 表：	无
///操作字段：	无
///操作文件：	无
	
	int		l_iRet;
	int		l_iSemRight;
	
	l_iSemRight = p_iSemRight|IPC_CREAT|IPC_EXCL;
	//创建一个信号灯集
	l_iRet = semget(p_lSemKey, p_iSems, l_iSemRight);
	
	if (l_iRet < 0)
	{
		if (EEXIST == errno)
		{	//已经存在，得到这个信号灯集
			l_iRet = semget(p_lSemKey, 0, p_iSemRight);
			if (l_iRet < 0)
			{
				ErrorLog(ERR_PMUTEX_CREATE, "[key=%d, errno=%d, errmsg=%s]", p_lSemKey, errno, strerror(errno));
				return ERR_PMUTEX_CREATE;
			}
			//删除信号灯集
			if (semctl(l_iRet, 0, IPC_RMID) < 0)
			{
				ErrorLog(ERR_PMUTEX_CLOSE, "[key=%d, errno=%d, errmsg=%s]", p_lSemKey, errno, strerror(errno));
				return ERR_PMUTEX_CLOSE;
			}
			//生成新的
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
///功		能：	初始化信号量值
///输		入：	p_iSemId信号量ID
///						p_iSemNum,集合中的一个成员下标
///			p_iSemVal信号量值
///输		出：	无
///返 回 	值：	成功0，失败-1
///操 作 表：	无
///操作字段：	无
///操作文件：	无
	
	union semun_	 l_unArg;
	
	l_unArg.val = p_iSemVal;
	//初始化集合中的一个信号灯
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
///功		能：	请求资源
///输		入：	p_iSemId信号量ID
///			p_iSenNum信号量在信号集中的下标
///			p_iSemOp请求资源数
///输		出：	无
///返 回 	值：	成功0，失败-1
///操 作 表：	无
///操作字段：	无
///操作文件：	无
		
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
///功		能：	释放资源
///输		入：	p_iSemId信号量ID
///			p_iSenNum信号量
///			p_iSemOp释放资源数
///输		出：	无
///返 回 	值：	成功0，失败-1
///操 作 表：	无
///操作字段：	无
///操作文件：	无
	
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