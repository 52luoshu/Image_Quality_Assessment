#include "brisque.h"
#include <iostream>
#include "svm.h"

using namespace std;

// 提取特征向量:1x36
void ComputeBrisqueFeature(Mat& orig, vector<double>& featurevector)
{
	// 归一化的图像
	Mat orig_bw_int(orig.size(), CV_64F, 1);
	cvtColor(orig, orig_bw_int, COLOR_BGR2GRAY);
	Mat orig_bw(orig_bw_int.size(), CV_64FC1, 1);
	orig_bw_int.convertTo(orig_bw, 1.0 / 255);
	orig_bw_int.release();

	int scalenum = 2;
	for (int itr_scale = 1; itr_scale <= scalenum; itr_scale++)
	{
		// 重置图像大小
		Size dst_size(orig_bw.cols / cv::pow((double)2, itr_scale - 1), orig_bw.rows / pow((double)2, itr_scale - 1));
		Mat imdist_scaled;
		resize(orig_bw, imdist_scaled, dst_size, 0, 0, INTER_LINEAR);
		imdist_scaled.convertTo(imdist_scaled, CV_64FC1, 1.0 / 255.0);

		// 计算MSCN系数
		// 局部均值
		Mat mu(imdist_scaled.size(), CV_64FC1, 1);
		GaussianBlur(imdist_scaled, mu, Size(7, 7), 1.166);

		Mat mu_sq;
		cv::pow(mu, double(2.0), mu_sq);
		// 局部方差
		Mat sigma(imdist_scaled.size(), CV_64FC1, 1);
		cv::multiply(imdist_scaled, imdist_scaled, sigma);
		GaussianBlur(sigma, sigma, Size(7, 7), 1.166);
		cv::subtract(sigma, mu_sq, sigma);
		cv::pow(sigma, double(0.5), sigma);
		// 避免局部方差为0，因为后面计算MSCN系数要除以局部方差
		add(sigma, Scalar(1.0 / 255), sigma);
		// 计算MSCN
		Mat structdis(imdist_scaled.size(), CV_64FC1, 1);
		subtract(imdist_scaled, mu, structdis);
		divide(structdis, sigma, structdis);

		//计算AGGD，拟合MSCN
		// lsgima_best 左方差，rsigma_best右方差，gamma均值
		double lsigma_best, rsigma_best, gamma_best;

		// 非对称广义高斯分布拟合
		structdis = AGGDfit(structdis, lsigma_best, rsigma_best, gamma_best);

		// 形状参数
		featurevector.push_back(gamma_best);
		// 方差参数
		featurevector.push_back((lsigma_best * lsigma_best + rsigma_best * rsigma_best) / 2);

		// 计算两两对称参数
		int shifts[4][2] = { {0,1},{1,0},{1,1},{-1,1} };

		for (int itr_shift = 1; itr_shift <= 4; itr_shift++)
		{
			int* reqshift = shifts[itr_shift - 1];

			Mat shifted_structdis(imdist_scaled.size(), CV_64F, 1);

			BwImage OrigArr(structdis);
			BwImage ShiftArr(shifted_structdis);

			for (int i = 0; i < structdis.rows; i++)
			{
				for (int j = 0; j < structdis.cols; j++)
				{
					if (i + reqshift[0] >= 0 && i + reqshift[0] < structdis.rows && j + reqshift[1] >= 0 && j + reqshift[1] < structdis.cols)
					{
						ShiftArr[i][j] = OrigArr[i + reqshift[0]][j + reqshift[1]];
					}
					else
					{
						ShiftArr[i][j] = 0;
					}
				}
			}

			shifted_structdis = ShiftArr.equate(shifted_structdis);

			multiply(structdis, shifted_structdis, shifted_structdis);

			shifted_structdis = AGGDfit(shifted_structdis, lsigma_best, rsigma_best, gamma_best);

			double constant = sqrt(tgamma(1 / gamma_best)) / sqrt(tgamma(3 / gamma_best));
			double meanparam = (rsigma_best - lsigma_best) * (tgamma(2 / gamma_best) / tgamma(1 / gamma_best)) * constant;

			featurevector.push_back(gamma_best);
			featurevector.push_back(meanparam);
			featurevector.push_back(cv::pow(lsigma_best, 2));
			featurevector.push_back(cv::pow(rsigma_best, 2));
		}
	}
}

// 拟合非广义分布
Mat AGGDfit(Mat structdis, double& lsigma_best, double& rsigma_best, double& gamma_best)
{
	BwImage ImArr(structdis);

	long int poscount = 0, negcount = 0;
	double possqsum = 0, negsqsum = 0, abssum = 0;
	for (int i = 0; i < structdis.rows; i++)
	{
		for (int j = 0; j < structdis.cols; j++)
		{
			double pt = ImArr[i][j];
			if (pt > 0)
			{
				poscount++;
				possqsum += pt * pt;
				abssum += pt;
			}
			else if (pt < 0)
			{
				negcount++;
				negsqsum += pt * pt;
				abssum -= pt;
			}
		}
	}

	lsigma_best = cv::pow(negsqsum / negcount, 0.5);
	rsigma_best = cv::pow(possqsum / poscount, 0.5);

	double gammahat = lsigma_best / rsigma_best;
	long int totalcount = (structdis.cols) * (structdis.rows);
	double rhat = cv::pow(abssum / totalcount, static_cast<double>(2)) / ((negsqsum + possqsum) / totalcount);
	double rhatnorm = rhat * (cv::pow(gammahat, 3) + 1) * (gammahat + 1) / pow(pow(gammahat, 2) + 1, 2);

	double prevgamma = 0;
	double prevdiff = 1e10;
	float sampling = 0.001;

	for (float gam = 0.2; gam < 10; gam += sampling)
	{
		double r_gam = tgamma(2 / gam) * tgamma(2 / gam) / (tgamma(1 / gam) * tgamma(3 / gam));
		double diff = abs(r_gam - rhatnorm);
		if (diff > prevdiff) break;
		prevdiff = diff;
		prevgamma = gam;
	}
	gamma_best = prevgamma;

	return structdis.clone();
}



float getBrisque(Mat& image, string modelfile)
{
	// 从allrange文件加载预向量，用于svm预测时归一化
	float min_[36] = { 0.336999 ,0.019667 ,0.230000 ,-0.125959 ,0.000167 ,0.000616 ,0.231000 ,-0.125873 ,0.000165 ,0.000600 ,0.241000 ,-0.128814 ,0.000179 ,0.000386 ,0.243000 ,-0.133080 ,0.000182 ,0.000421 ,0.436998 ,0.016929 ,0.247000 ,-0.200231 ,0.000104 ,0.000834 ,0.257000 ,-0.200017 ,0.000112 ,0.000876 ,0.257000 ,-0.155072 ,0.000112 ,0.000356 ,0.258000 ,-0.154374 ,0.000117 ,0.000351 };
	float max_[36] = { 9.999411, 0.807472, 1.644021, 0.202917, 0.712384, 0.468672, 1.644021, 0.169548, 0.713132, 0.467896, 1.553016, 0.101368, 0.687324, 0.533087, 1.554016, 0.101000, 0.689177, 0.533133, 3.639918, 0.800955, 1.096995, 0.175286, 0.755547, 0.399270, 1.095995, 0.155928, 0.751488, 0.402398, 1.041992, 0.093209, 0.623516, 0.532925, 1.042992, 0.093714, 0.621958, 0.534484 };

	// 图像得分
	double qualityscore;
	int i;

	// 创建svm结构图
	struct svm_model* model;

	//Rect rect((int)(image.cols / 3), (int)(image.rows / 3), (int)(image.cols * 1 / 3), (int)(image.rows * 1 / 3));
	//Mat orig = image(rect);
	Mat orig = image;

	// 特征向量初始化
	vector<double> brisqueFeatures;

	//  计算特征向量
	ComputeBrisqueFeature(orig, brisqueFeatures);

	//打开svm模型
	if ((model = svm_load_model(modelfile.c_str())) == 0)
	{
		fprintf(stderr, "can't open model file allmodel\n");
		exit(1);
	}

	struct svm_node x[37];
	// 将brisqueFeatures向量从-1重新缩放到1
	// 将向量转换为svm节点数组对象
	for (i = 0; i < 36; ++i)
	{
		// 归一化
		float min = min_[i];
		float max = max_[i];

		x[i].value = -1 + (2.0 / (max - min) * (brisqueFeatures[i] - min));
		x[i].index = i + 1;
	}
	x[36].index = -1;

	// 计算内存消耗
	int nr_class = svm_get_nr_class(model);
	double* prob_estimates = (double*)malloc(nr_class * sizeof(double));

	// SVM预测
	qualityscore = svm_predict_probability(model, x, prob_estimates);

	free(prob_estimates);
	svm_free_and_destroy_model(&model);

	//qualityscore值域在[0,100]，分值越高PQ越差，这里取反
	return 100.0 - qualityscore;
}