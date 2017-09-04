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
#define DATA_STATUS_PRODUCT_USING   1 //���ڱ��޸ģ��ֿ���ת�� finish����IDEL
#define DATA_STATUS_COMSUMER_USING   2 //���ڱ��޸ģ��ֿ���ת�� finish����IDEL
#define DATA_STATUS_READY  3
#define DATA_STATUS_IDEL    4


struct OShmEXDataUnit  //���ݽṹ(����Կ�״�)
{
	int m_uiStatus;
	int m_iUsingPID; //ʹ����
};




class OShm
{
public:
	//д����
	virtual int iWriteData(const void *p_pacData,const int p_iLen,int p_iPos = 0);
	//������
	virtual int iReadData(void *p_pacData,int p_iDstLen,int p_iReadLen = 0,int p_iPos = 0);
	//���������ڴ�
	virtual int iCreate(int p_iShmKey,int p_iLen);
	//�õ�һ�������ڴ�
	int iGetShm(int p_iShmKey);
	//���ĳһ��������
	int iClearData(int p_iStartpos,int p_iLen,char p_iClearChar = 0x00);
	//ɾ�������ڴ�
	int iDelShm();
	//�����źŵ�
	int iSetSem(int p_iSemId); 
	//�õ��󶨵�ַ
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
	
	//�õ��źŵ�
	int iGetShmId(){return m_iShmId;}
	//�õ������ڴ�ID
	int iGetSemId(){return m_iSemId;}
	//�õ������ڴ��С
	int iGetShmLen(){return m_iShmLen;}

	int m_iSemId;  //�źŵƼ�()	
	
	int m_iShmKey; //keyֵ	
	int m_iShmId;  //�����ڴ�ID
	int m_iShmLen; //�����ڴ��С
	void *m_pvShmAddr; //�󶨵�ַ
	
};

//��Ϊ2�����֣�Կ�״���,ֻ����Կ�׳���
class OShmEX: public OShm
{
public:
	~OShmEX();
	
public:
	//enum{DATA_STATUS_EMPTY,DATA_STATUS_USING,DATA_STATUS_READY,DATA_STATUS_IDEL};
	
	OShmEX():OShm(){m_iProductSemId=NULL,m_iConsumSemId=NULL; }
public:
	//�����źŵ�
	int iSetProductSem(int p_iSemId); 
	int iSetConsumeSem(int p_iSemId); 
	int iSetLockerSem(int p_iSemId); 
	
public:
	//int iFetchData(unsigned int& thePos); 
	//int iAddData(unsigned int& thePos);
public:
	//�ȴ�����
	int iProductorWaitForUpload( int& thePos); //�Ƚ�ȥ���ţ��Ż����ѡ�����һ������ʹ��
	int iConsumerWaitForUse( int& thePos);  //�Ƚ�ȥ���ţ��Ż����ѡ�����һ������ʹ��
	
	int iProductorUploadFinish( int& thePos); 
	int iConsumerUseFinish( int& thePos); 
	
public:
	//overload	
	virtual int iCreate(int p_iShmKey,int p_iLen);

private:
	int m_iProductSemId;
	int m_iConsumSemId;
	int m_iShmEXDataUnitAmount; //�ܹ��ж��ٸ����ݵ�Ԫ
	void *m_pvShmDataAddr; //�����ڴ��е����ݶεĵ�ַ
};

#endif

