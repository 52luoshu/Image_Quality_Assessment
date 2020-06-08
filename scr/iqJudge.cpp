//iqJudge.cpp -- 三段阈值法对当前结果进行判定
//需要有准确的阈值作前提

#include "qualitydetect.h"

int iqJudge(double result, double threshold_up, double threshold_down)
{
	if(result<threshold_down)
	{
		return 0;	//必须处理
	}
	if ((result >= threshold_down) && (result < threshold_up))
	{
		return 1;	//质量差，预警
	}
	if (result >= threshold_up)
	{
		return 2;	//PASS
	}
}