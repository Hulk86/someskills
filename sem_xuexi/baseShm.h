/*************************************************************************
	> File Name: baseShm.h
	> Author: abc2514671
	> Mail: abc2514671@163.com 
	> Created Time: Thu 17 Aug 2017 11:48:36 AM CST
 ************************************************************************/

#ifndef _BASESHM_H_
#define _BASESHM_H_

#include<unistd.h>
#include<sys/types.h>
#include "thepublic.h"

class OShmBase
{
public:
	//写数据
	int iWriteData(const void *p_pacData,const int p_iLen,int p_iPos = 0);
	//读数据
	int iReadData(void *p_pacData,int p_iDstLen,int p_iReadLen = 0,int p_iPos = 0);
	//创建共享内存
	int iCreate(int p_iShmKey,int p_iLen);
	//得到一个共享内存
	int iGetShm(int p_iShmKey);
	//清空某一区域数据
	int iClearData(int p_iStartpos,int p_iLen,char p_iClearChar = 0x00);
	//删除共享内存
	int iDelShm();
	//得到绑定地址
	void * iGetShmAddr(){return m_pvShmAddr;}
	OShmBase()
	{
		m_iShmId = INVALID_VALUE;
		m_iShmLen = INVALID_VALUE;
		m_iShmKey = INVALID_VALUE;
		m_pvShmAddr = NULL;
	}
	~OShmBase();
	
	//得到信号灯
	int iGetShmId(){return m_iShmId;}
	//得到共享内存大小
	int iGetShmLen(){return m_iShmLen;}
private:
	int m_iShmId;  //共享内存ID
	int m_iShmLen; //共享内存大小
	int m_iShmKey; //key值
	void *m_pvShmAddr; //绑定地址
};

#endif