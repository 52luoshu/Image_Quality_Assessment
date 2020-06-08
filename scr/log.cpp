//log.cpp -- 写日志相关函数

#include "qualitydetect.h"

int getLogTime(char* out, int fmt)// 获取当前系统时间
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

int getTime(char* out)//获取系统时间，用于自动命名log文件
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


FILE* openFile(const char* fileName, const char* mode) // 打开文本文件
{
    FILE* fp = fopen(fileName, mode);
    return fp;
}


int writeLog(FILE* fp, const char* str, bool bLog)// 写字符串到文件,bLog表明是否为日志文件
{
    assert(fp != NULL && str != NULL);
    char curTime[100] = { 0 };
    int ret = -1;
    if (bLog) // 获取当前系统时间
    {
        getLogTime(curTime, 0);
        ret = fprintf(fp, "[%s] %s", curTime, str);
    }
    else
        ret = fprintf(fp, "%s", str);

    if (ret >= 0)
    {
        fflush(fp);
        return 0; // 写文件成功
    }
    else
        return -1;
}

int closeLog(FILE* fp)//关闭日志文件
{
    return fclose(fp);
}