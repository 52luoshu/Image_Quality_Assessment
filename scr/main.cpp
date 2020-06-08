#include "qualitydetect.h"
#include "brisque.h"
#include <algorithm>
#include <numeric>
#include "svm.h"

int main(int argc,char** argv)
{
    /******************************* INPUT INIT ***********************************/
    if (argc < 10)
    {
        cout << "usage: ./.../originalFile width height fps delta ./.../outputFile width height fps delta\n";
        exit(1);
    }

    const char* input_file = argv[1];
    const char* output_file = argv[6];
    stringstream ss;
    int INWIDTH = 0, INHEIGHT = 0, OUTWIDTH = 0, OUTHEIGHT = 0, INDELTA = 0, OUTDELTA = 0;
    double INFPS = 0, OUTFPS = 0;
    ss << argv[2]; ss >> INWIDTH; ss.clear();
    ss << argv[3]; ss >> INHEIGHT; ss.clear();
    ss << argv[4]; ss >> INFPS; ss.clear();
    ss << argv[5]; ss >> INDELTA; ss.clear();

    ss << argv[7]; ss >> OUTWIDTH; ss.clear();
    ss << argv[8]; ss >> OUTHEIGHT; ss.clear();
    ss << argv[9]; ss >> OUTFPS; ss.clear();
    ss << argv[10]; ss >> OUTDELTA; ss.clear();

    /******************************* MODULE SWITCH ***********************************/
    bool bypass_psnr = 0;
    bool bypass_ssim = 0;
    bool bypass_nrss = 0;
    bool bypass_gssim = 0;
    bool bypass_brisque = 1;
    /******************************* INIT ***********************************/
    double NRSS = 0, SSIM = 0, PSNR = 0, GSSIM = 0, BRISQUE = 0;
    vector <double> list_nrss, list_ssim, list_psnr, list_gssim, list_brisque;
    string modelfile = "./brisque_svm_model/allmodel";
    string range_fname = "./brisque_svm_model/allrange";

    /******************************* LOG INIT ***********************************/
    char logtime[20] = { 0 };
    getTime(logtime);
    char logfile[80];
    sprintf(logfile, "%s%s%s", (char*)"./log/", logtime, (char*)".log");
    FILE* log = fopen(logfile, "a+");
    char info_log[120];

    sprintf(info_log, "[%s] [%dx%d %dfps]\n", input_file, INWIDTH, INHEIGHT, INFPS);
    writeLog(log, info_log, 1);
    sprintf(info_log, "[%s] [%dx%d %dfps]\n", output_file, OUTWIDTH, OUTHEIGHT, OUTFPS);
    writeLog(log, info_log, 1);
    sprintf(info_log, "Note: NRSS detect the input quality, others detect the output quality!\n");
    writeLog(log, info_log, 1);
    sprintf(info_log, "\n");
    writeLog(log, info_log, 0);
    /******************************* FORMAT JUDGEMENT ***********************************/
    string outfileStr(output_file);
    string suffix = outfileStr.substr(outfileStr.find_last_of('.') + 1);
    /******************************* YUV420P ***********************************/

    if (suffix == "yuv")
    {
        FILE* org_yuv;
        FILE* out_yuv;

        org_yuv = fopen(input_file, "rb");
        out_yuv = fopen(output_file, "rb");
        if (NULL == org_yuv || NULL == out_yuv)
        {
            printf("file open error\n");
            return -1;
        }

        //获取帧数、时长
        int singleSize_in = (INWIDTH * OUTHEIGHT * 3) / 2;
        int singleSize_out = (OUTWIDTH * OUTHEIGHT * 3) / 2;
        fseek(org_yuv, 0, SEEK_END);
        fseek(out_yuv, 0, SEEK_END);
        int total_in = (int)((int)ftell(org_yuv) / singleSize_in);
        int total_out = (int)((int)ftell(out_yuv) / singleSize_out);
        fseek(org_yuv, 0, SEEK_SET);
        fseek(out_yuv, 0, SEEK_SET);
        double rate_in = INFPS, rate_out = OUTFPS;
        double time_in = (int)(total_in / rate_in);
        double time_out = (int)(total_out / rate_out);

        int total = (total_in < total_out ? total_in : total_out);
        double time = (time_in < time_out ? time_in : time_out);
        bool equalsize = true;
        if (INWIDTH != OUTWIDTH || INHEIGHT != OUTHEIGHT)
        {
            equalsize = false;
        }


        unsigned char* buff_in, * buff_out;
        buff_in = (unsigned char*)malloc(singleSize_in * sizeof(unsigned char));
        buff_out = (unsigned char*)malloc(singleSize_out * sizeof(unsigned char));
        memset(buff_in, 0, singleSize_in * sizeof(unsigned char));
        memset(buff_out, 0, singleSize_out * sizeof(unsigned char));

        long long int index_in = 1, index_out = 1;
        index_out += OUTDELTA;
        Mat frame_in, frame_out;
        long offset_in = 0, offset_out = 0;
        int i = 1;
        int readsize_in = 0, readsize_out = 0;

        while (i < time + 1 && index_in < total_in && index_out < total_out)
        {
            /******************************* GET FRAME ***********************************/
            int curtime = i - 1;
            sprintf(info_log, "[%5.5d] ", curtime);
            writeLog(log, info_log, 0);

            offset_in = (curtime * rate_in + INDELTA) * singleSize_in;
            offset_out = (curtime * rate_out + OUTDELTA) * singleSize_out;

            sprintf(info_log, "[frame(in): %d frame(out): %d] ", curtime* rate_in + INDELTA, curtime* rate_out + OUTDELTA);
            writeLog(log, info_log, 0);

            fseek(org_yuv, offset_in, SEEK_SET);
            fseek(out_yuv, offset_out, SEEK_SET);


            readsize_in = fread(buff_in, 1, singleSize_in, org_yuv);
            readsize_out = fread(buff_out, 1, singleSize_out, out_yuv);

            frame_in.create(INHEIGHT * 3 / 2, INWIDTH, CV_8UC1);
            frame_out.create(OUTHEIGHT * 3 / 2, OUTWIDTH, CV_8UC1);
            memcpy(frame_in.data, buff_in, singleSize_in);
            memcpy(frame_out.data, buff_out, singleSize_out);

            Rect in_rect_Y(0, 0, INWIDTH, (frame_in.rows * 2 / 3));
            Rect out_rect_Y(0, 0, OUTWIDTH, (frame_out.rows * 2 / 3));
            Rect in_rect_U(0, (frame_in.rows * 2 / 3), INWIDTH, (frame_in.rows / 6));
            Rect out_rect_U(0, (frame_out.rows * 2 / 3), OUTWIDTH, (frame_out.rows / 6));
            Rect in_rect_V(0, (frame_in.rows * 5 / 6), INWIDTH, (frame_in.rows / 6));
            Rect out_rect_V(0, (frame_out.rows * 5 / 6), OUTWIDTH, (frame_out.rows / 6));

            Mat in_Y = frame_in(in_rect_Y), in_U = frame_in(in_rect_U), in_V = frame_in(in_rect_V);
            Mat out_Y = frame_out(out_rect_Y), out_U = frame_out(out_rect_U), out_V = frame_out(out_rect_V);
            if (!equalsize)
            {
                resize(in_Y, in_Y, Size(out_Y.cols, out_Y.rows));
                resize(in_U, in_U, Size(out_U.cols, out_U.rows));
                resize(in_V, in_V, Size(out_V.cols, out_V.rows));
            }
            /******************************* PICTURE QUALITY  DETECT ***********************************/
            if (bypass_ssim)
            {
                SSIM = getMSSIM(in_Y, out_Y);
                list_ssim.push_back(SSIM);
                sprintf(info_log, "SSIM: %1.6f ", SSIM);
                writeLog(log, info_log, 0);
            }
            if (bypass_psnr)
            {
                PSNR = getPSNR(in_Y, out_Y);
                list_psnr.push_back(PSNR);
                sprintf(info_log, "PSNR: %1.6f ", PSNR);
                writeLog(log, info_log, 0);
            }
            if (bypass_nrss)
            {
                NRSS = getNRSS(in_Y);
                list_nrss.push_back(NRSS);
                sprintf(info_log, "NRSS: %1.6f \n", NRSS);
                writeLog(log, info_log, 0);
            }
            i++;
        }
        /******************************* MIN-AVERAGE-MAX ***********************************/
        sprintf(info_log, "\n[Min] SSIM: %1.6f PSNR: %1.6f NRSS: %1.6f\n",
            (*min_element(list_ssim.begin(), list_ssim.end())),
            (*min_element(list_psnr.begin(), list_psnr.end())),
            (*min_element(list_nrss.begin(), list_nrss.end())));
        writeLog(log, info_log, 0);
        sprintf(info_log, "[Ave] SSIM: %1.6f PSNR: %1.6f NRSS: %1.6f\n",
            (accumulate(list_ssim.begin(), list_ssim.end(), 0) * 1.0 / list_ssim.size()),
            (accumulate(list_psnr.begin(), list_psnr.end(), 0) * 1.0 / list_psnr.size()),
            (accumulate(list_nrss.begin(), list_nrss.end(), 0) * 1.0 / list_nrss.size()));
        writeLog(log, info_log, 0);
        sprintf(info_log, "[Max] SSIM: %1.6f PSNR: %1.6f NRSS: %1.6f\n",
            (*max_element(list_ssim.begin(), list_ssim.end())),
            (*max_element(list_psnr.begin(), list_psnr.end())),
            (*max_element(list_nrss.begin(), list_nrss.end())));
        writeLog(log, info_log, 0);

        /******************************* BUFFER RELEASE ***********************************/
        free(buff_in);
        buff_in = NULL;
        free(buff_out);
        buff_out = NULL;
        fclose(org_yuv);
        fclose(out_yuv);
    }


    /******************************* MP4/FLV/AVI ***********************************/
    if (suffix != "yuv")
    {
        VideoCapture cap_in(input_file), cap_out(output_file);
        if (!cap_in.isOpened() || !cap_out.isOpened())
        {
            cout << "File can't open!\n";
            return -1;
        }
        //获取视频信息
        int total_in = cap_in.get(CV_CAP_PROP_FRAME_COUNT);
        double rate_in = cap_in.get(CV_CAP_PROP_FPS);
        int width_in = cap_in.get(CV_CAP_PROP_FRAME_WIDTH);
        int height_in = cap_in.get(CV_CAP_PROP_FRAME_HEIGHT);
        double time_in = (int)(total_in / rate_in);

        int total_out = cap_out.get(CV_CAP_PROP_FRAME_COUNT);
        double rate_out = cap_out.get(CV_CAP_PROP_FPS);
        int width_out = cap_out.get(CV_CAP_PROP_FRAME_WIDTH);
        int height_out = cap_out.get(CV_CAP_PROP_FRAME_HEIGHT);

        rate_in = INFPS, rate_out = OUTFPS;
        width_in = INWIDTH, width_out = OUTWIDTH;
        height_in = INHEIGHT, height_out = OUTHEIGHT;

        double time_out = (int)(total_out / rate_out);
        bool equalsize = true;
        if (width_in != width_out || height_in != height_out)
        {
            equalsize = false;
        }
        //最小帧数、最小帧率、秒数
        int total_min = (total_in < total_out ? total_in : total_out);
        double rate_min = (rate_in < rate_out ? rate_in : rate_out);
        double time = (time_in < time_out ? time_in : time_out);


        long long int index_in = 1, index_out = 1;
        index_in += INDELTA;
        index_out += OUTDELTA;
        Mat frame_in, frame_out;
        int i = 1;
        while (i < time + 1 && index_in < total_in && index_out < total_out)
        {
            clock_t startTime = clock();
            /******************************* FRAME GET ***********************************/
            int curtime = i - 1;
            sprintf(info_log, "[%d s] ", curtime);
            writeLog(log, info_log, 0);

            cap_in.set(CAP_PROP_POS_FRAMES, index_in);
            cap_out.set(CAP_PROP_POS_FRAMES, index_out);
            cap_in >> frame_in;
            rate_in = cap_in.get(CV_CAP_PROP_FPS);
            cap_out >> frame_out;
            rate_out = cap_out.get(CV_CAP_PROP_FPS);
            rate_min = (rate_in < rate_out ? rate_in : rate_out);

            sprintf(info_log, "[frame: %d vs %d fps: %2.1f vs %2.1f] ", index_in, index_out, rate_in, rate_out);
            writeLog(log, info_log, 0);

            imshow("in", frame_in);
            imshow("out", frame_out);
            waitKey(30);

            Mat img_org, img_out;
            cvtColor(frame_in, img_org, CV_BGR2YUV_I420);
            cvtColor(frame_out, img_out, CV_BGR2YUV_I420);
            if (!equalsize)
            {
                resize(img_org, img_org, Size(width_out, height_out));
            }

            Rect out_rect_Y(0, 0, width_out, (img_out.rows * 2 / 3));
            Rect out_rect_U(0, (img_out.rows * 2 / 3), width_out, (img_out.rows / 6));
            Rect out_rect_V(0, (img_out.rows * 5 / 6), width_out, (img_out.rows / 6));

            Mat in_Y = img_org(out_rect_Y); Mat  in_U = img_org(out_rect_U); Mat  in_V = img_org(out_rect_V);
            Mat out_Y = img_out(out_rect_Y); Mat  out_U = img_out(out_rect_U); Mat  out_V = img_out(out_rect_V);

            /******************************* PICTURE QUALITY DETECT ***********************************/
            if (bypass_nrss)
            {
                NRSS = getNRSS(in_Y);
                list_nrss.push_back(NRSS);
                sprintf(info_log, "NRSS: %1.6f ", NRSS);
                writeLog(log, info_log, 0);
            }
            if (bypass_psnr)
            {
                PSNR = getPSNR(in_Y, out_Y);
                list_psnr.push_back(PSNR);
                sprintf(info_log, "PSNR: %1.6f ", PSNR);
                writeLog(log, info_log, 0);
            }
            if (bypass_ssim)
            {
                SSIM = getMSSIM(in_Y, out_Y);
                list_ssim.push_back(SSIM);
                sprintf(info_log, "SSIM: %1.6f ", SSIM);
                writeLog(log, info_log, 0);
            }
            if (bypass_gssim)
            {
                GSSIM = getGSSIM(in_Y, out_Y);
                list_gssim.push_back(GSSIM);
                sprintf(info_log, "GSSIM: %1.6f ", GSSIM);
                writeLog(log, info_log, 0);
            }
            if (bypass_brisque)
            {
                BRISQUE = getBrisque(frame_out, modelfile);
                list_brisque.push_back(BRISQUE);
                sprintf(info_log, "BRISQUE: %1.6f ", BRISQUE);
                writeLog(log, info_log, 0);
            }
            sprintf(info_log, "\n");
            writeLog(log, info_log, 0);
            i += (rate_min > 1.0 ? 1.0 : 1.0 / rate_min);
            index_in += (rate_in > 1.0 ? rate_in : 1.0);
            index_out += (rate_out > 1.0 ? rate_out : 1.0);
            
            clock_t endTime = clock();
            cout << (float)(endTime - startTime) * 1000 / CLOCKS_PER_SEC << endl;
        }
        /******************************* MIN-AVERAGE-MAX ***********************************/
        sprintf(info_log, "\n");
        writeLog(log, info_log, 0);
        if (bypass_nrss)
        {
            sprintf(info_log, "[NRSS] Min: %1.6f Average: %1.6f Max: %1.6f\n",
                (*min_element(list_nrss.begin(), list_nrss.end())),
                ((accumulate(list_nrss.begin(), list_nrss.end(), 0.0) * 1.0 / list_nrss.size())),
                (*max_element(list_nrss.begin(), list_nrss.end())));
            writeLog(log, info_log, 0);
        }
        if (bypass_psnr)
        {
            sprintf(info_log, "[PSNR] Min: %1.6f Average: %1.6f Max: %1.6f\n",
                (*min_element(list_psnr.begin(), list_psnr.end())),
                ((accumulate(list_psnr.begin(), list_psnr.end(), 0.0) * 1.0 / list_psnr.size())),
                (*max_element(list_psnr.begin(), list_psnr.end())));
            writeLog(log, info_log, 0);
        }
        if (bypass_ssim)
        {
            sprintf(info_log, "[SSIM] Min: %1.6f Average: %1.6f Max: %1.6f\n",
                (*min_element(list_ssim.begin(), list_ssim.end())),
                ((accumulate(list_ssim.begin(), list_ssim.end(), 0.0) * 1.0 / list_ssim.size())),
                (*max_element(list_ssim.begin(), list_ssim.end())));
            writeLog(log, info_log, 0);
        }
        if (bypass_gssim)
        {
            sprintf(info_log, "[GSSIM] Min: %1.6f Average: %1.6f Max: %1.6f\n",
                (*min_element(list_gssim.begin(), list_gssim.end())),
                ((accumulate(list_gssim.begin(), list_gssim.end(), 0.0) * 1.0 / list_gssim.size())),
                (*max_element(list_gssim.begin(), list_gssim.end())));
            writeLog(log, info_log, 0);
        }
        if (bypass_brisque)
        {
            sprintf(info_log, "[BRISQUE] Min: %1.6f Average: %1.6f Max: %1.6f\n",
                (*min_element(list_brisque.begin(), list_brisque.end())),
                ((accumulate(list_brisque.begin(), list_brisque.end(), 0.0) * 1.0 / list_brisque.size())),
                (*max_element(list_brisque.begin(), list_brisque.end())));
            writeLog(log, info_log, 0);
        }
    }
    closeLog(log);
    system("pause");
    return 0;
    }