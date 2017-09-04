/*************************************************************************
	> File Name: theMain.cpp
	> Author: abc2514671
	> Mail: abc2514671@163.com 
	> Created Time: Thu 17 Aug 2017 03:44:29 PM CST
 ************************************************************************/

#include "thepublic.h"
#include "oShmEX.h"  //相当于给数据域的锁
#include "baseShm.h" //数据域，没有锁
#include "oSem.h"
#include "filelog.h"

#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/wait.h>

static const unsigned int g_iDataAmount = 10;  //10个数据

//信号量key
const static int g_iSemKey=256;
const static int g_iSemKeyForInput=257;
const static int g_iSemKeyForOutput=258;

//共享内存key
const static int g_iShmKeyForLockers=256;
const static int g_iSemKeyForDatas=257;

static int g_iDataSemId=0;     //0,1信号量
static int g_iSemIdForInputId=0; //可以允许多少个进程取数据的信号量
static int g_iSemIdForOutputId=0; //可以允许多少个进程取数据的信号量

static int g_iChildAmount=1;
static int g_pChildArray[64];

typedef struct my_timeinfo
{
	int m_iSeriId;
	int m_iGenpid;  //生产者ID
	int m_iCuspid;  //消费者ID
	int my_numbber; //累加器
	int hour;
	int min;
	int sec;
	int usec;
	
}oMyTimeInfo;


static class OShmBase g_oShmDatas;
static class OShmEX g_oShmLockers;

static int g_iProductorSemId;
static int g_iConsumerSemId;
static int g_iLockersSemId;


const int g_iProductorAmount = 100;
unsigned int g_iProductorNums = 2;
unsigned int g_iConsumerNums = 2;

OFileLog g_oProdLogger("ForProductor"); //生产者的数据日志
OFileLog g_oUserLogger("ForConsumer"); //消费者的数据日志
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/////////////FUNCTION
/////////////FUNCTION
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


void perpareData()
{
	int l_iTmpRes = 0;
	int l_iDataAmount = sizeof(oMyTimeInfo) * g_iDataAmount;
	l_iTmpRes =   g_oShmDatas.iCreate(g_iSemKeyForDatas,l_iDataAmount);	
	if(l_iTmpRes != 0)
	{
		 //ERR_SHM_BIND
		LOGS("[inner error code is %d]:data shm create error\n",l_iTmpRes);
		exit(1);
	}
	//创建3个sem
	l_iTmpRes = OSem::iCreateSem(g_iSemKeyForInput,1,0666,g_iProductorSemId);
	if(l_iTmpRes != 0)
	{
		LOGS("[inner error code is %d]:productor sem create error\n",l_iTmpRes);
		exit(2);
	}
		l_iTmpRes = OSem::iCreateSem(g_iSemKeyForOutput,1,0666,g_iConsumerSemId);
	if(l_iTmpRes != 0)
	{
		LOGS("[inner error code is %d]:consumer sem create error\n",l_iTmpRes);
		exit(2);
	}
		l_iTmpRes = OSem::iCreateSem(g_iSemKey,1,0666,g_iLockersSemId);
	if(l_iTmpRes != 0)
	{
		LOGS("[inner error code is %d]:Locker sem create error\n",l_iTmpRes);
		exit(2);
	}	
  //创建shmex 并且初始化
  int l_iLockerDataAmount = g_iDataAmount;
  l_iTmpRes = g_oShmLockers.iCreate(g_iShmKeyForLockers,l_iLockerDataAmount);	
  if(l_iTmpRes != 0)
  {
  	LOGS("[inner error code is %d]:locker shm create error\n",l_iTmpRes);
		exit(2);
  }
 
  //
  g_oShmLockers.iSetLockerSem(g_iLockersSemId); //钥匙串 
  g_oShmLockers.iSetProductSem(g_iProductorSemId);
  g_oShmLockers.iSetConsumeSem(g_iConsumerSemId);
  //inital the sem data at beginning
  OSem::iInitSem(g_iLockersSemId,0,1);
  OSem::iInitSem(g_iProductorSemId,0,g_iDataAmount);
  OSem::iInitSem(g_iConsumerSemId,0,0);
  //INITIAL DATA
  g_oShmLockers.iClearData(0,l_iLockerDataAmount,0);
  	
	return;
}

void vWaitAllPid()
{
	int l_iTmpPid = 0;
	while((l_iTmpPid = waitpid(-1,NULL, WNOHANG)) < 1)
	{
		break;
	}	
	return;
}

//int snprintf(char *str, size_t size, const char *format, ...);
void vPrintData(FILE * l_pDest,  oMyTimeInfo * l_pShmData)
{
	  fprintf(l_pDest, "%06d \t %06d\t %02d:%02d:%02d:%06d \n",
	   l_pShmData->m_iSeriId, 
	   l_pShmData->m_iGenpid, 
	  
	   l_pShmData->hour,
	   l_pShmData->min,
	   l_pShmData->sec,
	   l_pShmData->my_numbber
	   );
	return;
}
//主进程扫描
void main_process()
{
	/*
	    static int g_pChildArray[64];
	*/
	//static class OShmBase g_oShmDatas;
	void * l_pShmData = NULL;	 

	
	int l_iTmpI = 0;
	int l_iMyTimeInfoSize = sizeof(oMyTimeInfo);
	//while(1)
	{
		l_pShmData = (void *)g_oShmDatas.iGetShmAddr();
	
	  fprintf(stdout, "====================main process poll============l_pShmData=%X================\n",l_pShmData );
	  for(l_iTmpI = 0; l_iTmpI != g_iDataAmount; l_iTmpI++,l_pShmData += l_iMyTimeInfoSize)
	  {
	  	
	  	fprintf(stdout, "[%d][addr=%016X] =====>",l_iTmpI,l_pShmData);
	  	vPrintData(stdout,(oMyTimeInfo*)l_pShmData);
	  	fflush(stdout);
	  }
	  fprintf(stdout, "=================================================================\n" );
	   {
  	
    OShmEXDataUnit * l_pTmpPtr = (OShmEXDataUnit * )g_oShmLockers.iGetShmAddr();
     oMyTimeInfo * l_pCurrent = NULL;
    for( int i = 0; i != g_iDataAmount; i++)
    {
      printf("****************************\n");
      printf("[%X][%ld][%d]",l_pTmpPtr,l_pTmpPtr->m_uiStatus,l_pTmpPtr->m_iUsingPID);
      l_pCurrent = (oMyTimeInfo*)( g_oShmDatas.iGetShmAddr() + i * sizeof(oMyTimeInfo));  
      printf("[%02d:%02d:%06d]\n",l_pCurrent->min,l_pCurrent->sec,l_pCurrent->my_numbber);
      printf("****************************\n");
      l_pTmpPtr++;
    }      
  }
  
	  vWaitAllPid();
	  sleep(1);
  }
	return;
}

void productData_1(char * l_chpRes)
{
  
  return ;
}

void productor_prepare_data(_IN_OUT oMyTimeInfo& theStrucData, int& i)
{
	struct timeval tv;	
	time_t tt;	
	struct tm strcTm;
	
	gettimeofday(&tv,NULL);
	time(&tt);
	localtime_r(&tt,&strcTm);

	theStrucData.m_iSeriId = ++i;
	theStrucData.m_iGenpid = getpid();
	srand(theStrucData.m_iCuspid);	 // SET SEED
	theStrucData.m_iCuspid = rand_r((unsigned int *)&theStrucData.m_iCuspid);
	
	theStrucData.my_numbber=tv.tv_usec;
	theStrucData.hour=strcTm.tm_hour;
	theStrucData.min=strcTm.tm_min;
	theStrucData.sec=strcTm.tm_sec;
	g_oProdLogger.iLogInfo("[pid=%04d][SeriId=%06d][random=%07d][%02d:%02d:%06d]",
	theStrucData.m_iGenpid,theStrucData.m_iSeriId,theStrucData.m_iCuspid,
	theStrucData.min,theStrucData.sec,theStrucData.my_numbber);
	fprintf(stdout,"===============producter (%d) start=====================\n", getpid());
  vPrintData(stdout,&theStrucData);
	fprintf(stdout,"===============producter (%d) end=======================\n", getpid());
	fflush(stdout);
	return;
}

void vGetData(int pos, _IN_OUT oMyTimeInfo* theStrucData)
{
  
  void * l_pCurrent = NULL;
  if(pos < g_iDataAmount)
  {
     l_pCurrent = g_oShmDatas.iGetShmAddr() + pos * sizeof(oMyTimeInfo);     
     memcpy(theStrucData,l_pCurrent,sizeof(oMyTimeInfo));
  }
	return;
}

void vInsertData(int pos, _IN_OUT oMyTimeInfo* theStrucData)
{

  void * l_pCurrent = NULL;
  if(pos < g_iDataAmount)
  {
     l_pCurrent = g_oShmDatas.iGetShmAddr() + pos * sizeof(oMyTimeInfo);     
     //memcpy(theStrucData,l_pCurrent,sizeof(oMyTimeInfo));
     fprintf(stdout,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!l_pCurrent=%X!!!!!!!!!!!!!!!!!!!!!!!sizeof(oMyTimeInfo)=%d!!!!!!!!!!!!\n",l_pCurrent,sizeof(oMyTimeInfo));
     vPrintData(stdout,theStrucData);
     memcpy(l_pCurrent,theStrucData,sizeof(oMyTimeInfo));
      fprintf(stdout,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!l_pCurrent=%X!!!!!!!!!!!!!!!!!!!!!!!sizeof(oMyTimeInfo)=%d!!!!!!!!!!!!\n",l_pCurrent,sizeof(oMyTimeInfo));
      vPrintData(stdout,(oMyTimeInfo*)l_pCurrent);
  }
	return;
}

void productor_process()
{
  oMyTimeInfo l_oTheTimeInfo;
  size_t l_iTimeInfoSize = sizeof(oMyTimeInfo);
  int l_iSerialID = 0;
  int l_iSelectedPos = 0;
   
  int l_iCounter = 0;
  int g_iProductorCounter = 0;
  while(1)
  {
    if(++l_iCounter > g_iProductorAmount)
    {
    	usleep(50000);
    	continue;
    	
    }
    l_iSelectedPos = -1;
    memset(&l_oTheTimeInfo,0,l_iTimeInfoSize);
    productor_prepare_data( l_oTheTimeInfo, l_iSerialID);

    g_oShmLockers.iProductorWaitForUpload(l_iSelectedPos);
    if(l_iSelectedPos > -1)
    {
      vInsertData( (int )l_iSelectedPos,&l_oTheTimeInfo);      
    }
    g_oShmLockers.iProductorUploadFinish(l_iSelectedPos);
    usleep(50000);
  }
	return;
}
 
void consumer_process()
{
  oMyTimeInfo l_oTheTimeInfo;
  size_t l_iTimeInfoSize = sizeof(oMyTimeInfo);
  int l_iSelectedPos = 0;
   
	while(1)
  { 
    l_iSelectedPos = -1;
    memset(&l_oTheTimeInfo,0,l_iTimeInfoSize);
    g_oShmLockers.iConsumerWaitForUse(l_iSelectedPos);
    if(l_iSelectedPos > -1)
    {
      fprintf(stdout,"===============consumer (%d) start=====================\n", getpid());
      vGetData((int)l_iSelectedPos,&l_oTheTimeInfo); 
      vPrintData( stdout, &l_oTheTimeInfo );
      fprintf(stdout,"===============consumer (%d) end=======================\n", getpid());	
    }
    g_oShmLockers.iConsumerUseFinish(l_iSelectedPos);
    
    g_oUserLogger.iLogInfo("[pid=%04d][SeriId=%06d][random=%07d][%02d:%02d:%06d]",l_oTheTimeInfo.m_iGenpid,l_oTheTimeInfo.m_iSeriId,l_oTheTimeInfo.m_iCuspid,
    l_oTheTimeInfo.min,l_oTheTimeInfo.sec,l_oTheTimeInfo.my_numbber);
    usleep(5000);
    fprintf(stdout,"===============consumer (%d) again=======================\n", getpid());	
  }
}

void testShmDt()
{

void * l_ptmp =(void*)g_oShmDatas.iGetShmAddr();
int i = 0;
int l_iSeriId = 1;
int usec = 1001;
for(i = 0; i != 10; i++)
{
  ((oMyTimeInfo*)l_ptmp)->m_iSeriId = l_iSeriId++;
   ((oMyTimeInfo*)l_ptmp)->usec = usec++;
  l_ptmp += sizeof(oMyTimeInfo);
}
l_ptmp =(void *) g_oShmDatas.iGetShmAddr();

for(i = 0; i != 10; i++)
{
   vPrintData( stdout, (oMyTimeInfo*)l_ptmp );
}

  return;
}

int main(int argc, char* argv[])
{
  int l_iTmpI = 0;
  int l_iTmpPid = 0;
  int i = 0;
  
  //OFileLog l_tmpFileLog("forErrorTXT.txt");
  g_oProdLogger.iOpenFile();
  g_oUserLogger.iOpenFile();
  //l_tmpFileLog.iLogInfo("yyyyyydddddd");
  //l_tmpFileLog.iLogInfo("yyyyyyxxxxxxxxxxxxxdddddd");

  perpareData();
// testShmDt();
  for(l_iTmpI = 0; l_iTmpI != g_iProductorNums; l_iTmpI++,i++)
  {
    l_iTmpPid = fork();
    if(l_iTmpPid == 0)
    {
      productor_process();    
      printf("1\n");    
      exit(0);
    }
    else
    {
      g_pChildArray[i] = l_iTmpPid;
    }
  }
  sleep(10);
  
 
  for(l_iTmpI = 0; l_iTmpI != g_iConsumerNums; l_iTmpI++,i++)
  {
    l_iTmpPid = fork();
    if(l_iTmpPid == 0)
    {
      consumer_process();  
      printf("2\n");    
      exit(0);      
    }
    else
    {
      g_pChildArray[i] = l_iTmpPid;
    }
  }
  printf("\n");
  printf("productor ===>");
  for(i = 0; i!= g_iProductorNums; i++)
    printf("%06d\t",g_pChildArray[i]);
  printf("\n");
  printf("consumer ===>");
  for(i = g_iProductorNums; i!= g_iProductorNums+g_iConsumerNums; i++)
    printf("%06d\t",g_pChildArray[i]);
  printf("\n");
  
  sleep(2);
  
  main_process();
  
	return 0;
}