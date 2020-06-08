//log.cpp -- д��־��غ���

#include "qualitydetect.h"

int getLogTime(char* out, int fmt)// ��ȡ��ǰϵͳʱ��
{
    if (out == NULL)
        return -1;

    time_t t;
    struct tm* tp;
    t = time(NULL);

    tp = localtime(&t);
    if (fmt == 0)
        sprintf(out, "%2.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
    else if (fmt == 1)
        sprintf(out, "%2.2d-%2.2d-%2.2d", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday);
    else if (fmt == 2)
        sprintf(out, "%2.2d:%2.2d:%2.2d", tp->tm_hour, tp->tm_min, tp->tm_sec);
    return 0;
}

int getTime(char* out)//��ȡϵͳʱ�䣬�����Զ�����log�ļ�
{
    if (out == NULL)
        return -1;

    time_t t;
    struct tm* tp;
    t = time(NULL);

    tp = localtime(&t);
    sprintf(out, "%2.2d%2.2d%2.2d%2.2d%2.2d%2.2d", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
    return 0;
}


FILE* openFile(const char* fileName, const char* mode) // ���ı��ļ�
{
    FILE* fp = fopen(fileName, mode);
    return fp;
}


int writeLog(FILE* fp, const char* str, bool bLog)// д�ַ������ļ�,bLog�����Ƿ�Ϊ��־�ļ�
{
    assert(fp != NULL && str != NULL);
    char curTime[100] = { 0 };
    int ret = -1;
    if (bLog) // ��ȡ��ǰϵͳʱ��
    {
        getLogTime(curTime, 0);
        ret = fprintf(fp, "[%s] %s", curTime, str);
    }
    else
        ret = fprintf(fp, "%s", str);

    if (ret >= 0)
    {
        fflush(fp);
        return 0; // д�ļ��ɹ�
    }
    else
        return -1;
}

int closeLog(FILE* fp)//�ر���־�ļ�
{
    return fclose(fp);
}