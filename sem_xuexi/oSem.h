/*************************************************************************
	> File Name: oSem.h
	> Author: abc2514671
	> Mail: abc2514671@163.com 
	> Created Time: Thu 17 Aug 2017 01:43:00 PM CST
 ************************************************************************/
#ifndef _O_SEM_H_
#define _O_SEM_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>

/* –≈∫≈µ∆¿‡*/   
union semun_
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};


class OSem //interface
{
	public:
		static int iCreateSem(long p_lSemKey, int p_iSems,int p_iSemRight, int &p_iSemId);
		static int iGetSem(long p_lSemKey, int p_iSemNum,int p_iSemRight, int &p_iSemId);
		static int iDelSem(int m_iSemId);
		static int iInitSem(int p_iSemId, int p_iSemNum,int p_iSemVal = 1);     //
		static int iLock(int p_iSemId, int p_iSenNum = 0, int p_iSemOp = 1);	  //0,1
		static int iUnLock(int p_iSemId, int p_iSenNum = 0 , int p_iSemOp = 1); //0,1–Õ∫≈µ∆
};


#endif
