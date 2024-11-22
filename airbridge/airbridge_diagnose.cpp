#include "airbridge_diagnose.h"
void AirbridgeDiagnose::diag(map<int, cv::Mat>& images, vector<BridgeSamples*>& bridge_samples)
{
    dbg_printf("%zu bridge samples need to process.", bridge_samples.size());

    // 定义矩形的线宽
    int thickness = 1;
    // 在图像上绘制矩形
    int index = 0;
    const int SAMPLE_LINE_LENGTH = 49;
    map<int, cv::Vec3b> m_sample_line_raw_accum_data;
    cv::Vec3b v(0, 0, 0);
    for (int i = 0; i < SAMPLE_LINE_LENGTH; i++)
        m_sample_line_raw_accum_data.insert(make_pair(i, v));
    for (auto& sample : bridge_samples)
    {
        //TODO: process airbridge images to generate a quality number.
        cv:imwrite("samples/sample" + std::to_string(++index) + ".png", sample->image);

        
        
        cv::Size size = sample->image.size();
        if (size.width != SAMPLE_LINE_LENGTH || size.height != 4)
            continue;
        for (int i = 0; i < SAMPLE_LINE_LENGTH; i++)
            m_sample_line_raw_accum_data[i] += sample->image.at<cv::Vec3b>(i, 1);// we use the second sample line to accumulate raw data.
    }
    int sample_count = m_sample_line_raw_accum_data.size();
    for (int i = 0; i < SAMPLE_LINE_LENGTH; i++)
        m_sample_line_raw_accum_data[i] /= (double)sample_count;
    cv::Scalar color(0, 255, 0); // 绿色
    for (auto& sample : bridge_samples)
    { 
        cv::Size size = sample->image.size();
        if (size.width != SAMPLE_LINE_LENGTH || size.height != 4)
        {
            cv::rectangle(images[sample->group_index], sample->pt1, sample->pt2, color, thickness);
            continue;
        }
        cv::Vec3i variance(0, 0, 0);

        for (int i = 0; i < SAMPLE_LINE_LENGTH; i++)
        {
            cv::Vec3b diff = sample->image.at<cv::Vec3b>(i, 1) - m_sample_line_raw_accum_data[i];
            for (int s = 0; s < 3; s++)
                if (diff[s] > 2.0)
                    variance[s] += diff[s];
        }
        cv::Scalar color_error = cv::Scalar(0, 0, 0);
        variance /= SAMPLE_LINE_LENGTH;
        dbg_printf("variance is %d, %d, %d.\n", variance[0], variance[1], variance[2]);
        
        
        color_error = cv::Scalar(255 - variance[0], 255 - variance[1], 255 - variance[2]);

        cv::rectangle(images[sample->group_index], sample->pt1, sample->pt2, color_error, -1);

    }
    for (auto& item : images)
    {
        cv::imwrite("result" + std::to_string(item.first) + ".png", item.second);
    }
}

/*
int main(void)
{
    return 0;
}
*/