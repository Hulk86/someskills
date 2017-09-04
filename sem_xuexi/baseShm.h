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
	//д����
	int iWriteData(const void *p_pacData,const int p_iLen,int p_iPos = 0);
	//������
	int iReadData(void *p_pacData,int p_iDstLen,int p_iReadLen = 0,int p_iPos = 0);
	//���������ڴ�
	int iCreate(int p_iShmKey,int p_iLen);
	//�õ�һ�������ڴ�
	int iGetShm(int p_iShmKey);
	//���ĳһ��������
	int iClearData(int p_iStartpos,int p_iLen,char p_iClearChar = 0x00);
	//ɾ�������ڴ�
	int iDelShm();
	//�õ��󶨵�ַ
	void * iGetShmAddr(){return m_pvShmAddr;}
	OShmBase()
	{
		m_iShmId = INVALID_VALUE;
		m_iShmLen = INVALID_VALUE;
		m_iShmKey = INVALID_VALUE;
		m_pvShmAddr = NULL;
	}
	~OShmBase();
	
	//�õ��źŵ�
	int iGetShmId(){return m_iShmId;}
	//�õ������ڴ��С
	int iGetShmLen(){return m_iShmLen;}
private:
	int m_iShmId;  //�����ڴ�ID
	int m_iShmLen; //�����ڴ��С
	int m_iShmKey; //keyֵ
	void *m_pvShmAddr; //�󶨵�ַ
};

#endif