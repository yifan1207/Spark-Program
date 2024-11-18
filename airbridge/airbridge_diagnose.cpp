#include "airbridge_diagnose.h"
void AirbridgeDiagnose::diag(map<int, cv::Mat>& images, vector<BridgeSamples*>& bridge_samples)
{
    cv::Scalar color(0, 255, 0); // 绿色

    // 定义矩形的线宽
    int thickness = 1;
    // 在图像上绘制矩形
    int index = 0;
    for (auto& sample : bridge_samples)
    {
        //TODO: process airbridge images to generate a quality number.
        cv:imwrite("samples/sample" + std::to_string(++index) + ".png", sample->image);

        cv::rectangle(images[sample->group_index], sample->pt1, sample->pt2, color, thickness);
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