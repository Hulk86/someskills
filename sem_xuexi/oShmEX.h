/*************************************************************************
	> File Name: oShmEX.h
	> Author: abc2514671
	> Mail: abc2514671@163.com 
	> Created Time: Thu 17 Aug 2017 01:36:56 PM CST
 ************************************************************************/

#ifndef _OSHMEX_H_
#define _OSHMEX_H_


#include "thepublic.h"
#include <unistd.h>

#define DATA_STATUS_EMPTY   0
#define DATA_STATUS_PRODUCT_USING   1 //正在被修改，又可能转到 finish或者IDEL
#define DATA_STATUS_COMSUMER_USING   2 //正在被修改，又可能转到 finish或者IDEL
#define DATA_STATUS_READY  3
#define DATA_STATUS_IDEL    4


struct OShmEXDataUnit  //数据结构(类似钥匙串)
{
	int m_uiStatus;
	int m_iUsingPID; //使用者
};




class OShm
{
public:
	//写数据
	virtual int iWriteData(const void *p_pacData,const int p_iLen,int p_iPos = 0);
	//读数据
	virtual int iReadData(void *p_pacData,int p_iDstLen,int p_iReadLen = 0,int p_iPos = 0);
	//创建共享内存
	virtual int iCreate(int p_iShmKey,int p_iLen);
	//得到一个共享内存
	int iGetShm(int p_iShmKey);
	//清空某一区域数据
	int iClearData(int p_iStartpos,int p_iLen,char p_iClearChar = 0x00);
	//删除共享内存
	int iDelShm();
	//设置信号灯
	int iSetSem(int p_iSemId); 
	//得到绑定地址
	void * iGetShmAddr(){return m_pvShmAddr;}
	OShm()
	{
		m_iSemId = INVALID_VALUE; 
		m_iShmId = INVALID_VALUE;
		m_iShmLen = INVALID_VALUE;
		m_iShmKey = INVALID_VALUE;
		m_pvShmAddr = NULL;
	}
	virtual ~OShm();
	
	//得到信号灯
	int iGetShmId(){return m_iShmId;}
	//得到共享内存ID
	int iGetSemId(){return m_iSemId;}
	//得到共享内存大小
	int iGetShmLen(){return m_iShmLen;}

	int m_iSemId;  //信号灯集()	
	
	int m_iShmKey; //key值	
	int m_iShmId;  //共享内存ID
	int m_iShmLen; //共享内存大小
	void *m_pvShmAddr; //绑定地址
	
};

//分为2个部分，钥匙串域,只管理钥匙池子
class OShmEX: public OShm
{
public:
	~OShmEX();
	
public:
	//enum{DATA_STATUS_EMPTY,DATA_STATUS_USING,DATA_STATUS_READY,DATA_STATUS_IDEL};
	
	OShmEX():OShm(){m_iProductSemId=NULL,m_iConsumSemId=NULL; }
public:
	//设置信号灯
	int iSetProductSem(int p_iSemId); 
	int iSetConsumeSem(int p_iSemId); 
	int iSetLockerSem(int p_iSemId); 
	
public:
	//int iFetchData(unsigned int& thePos); 
	//int iAddData(unsigned int& thePos);
public:
	//等待函数
	int iProductorWaitForUpload( int& thePos); //先进去大门，才会帮你选择给你一条数据使用
	int iConsumerWaitForUse( int& thePos);  //先进去大门，才会帮你选择给你一条数据使用
	
	int iProductorUploadFinish( int& thePos); 
	int iConsumerUseFinish( int& thePos); 
	
public:
	//overload	
	virtual int iCreate(int p_iShmKey,int p_iLen);

private:
	int m_iProductSemId;
	int m_iConsumSemId;
	int m_iShmEXDataUnitAmount; //总共有多少个数据单元
	void *m_pvShmDataAddr; //共享内存中的数据段的地址
};

#endif

