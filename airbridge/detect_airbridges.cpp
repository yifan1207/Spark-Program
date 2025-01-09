#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <iostream>

// 检测可疑的 Airbridge 区域，并返回其外接矩形 (Rect)
// 这些矩形会用来构造 ROI、在后续标注/诊断中使用
std::vector<cv::Rect> detectAirbridges(const cv::Mat& cadImage)
{
    // 1. 转灰度
    cv::Mat gray;
    if (cadImage.channels() == 3) {
        cv::cvtColor(cadImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = cadImage.clone();  // 若已是单通道则直接克隆
    }

    // 2. 这里的 120 是示意值，可能需要根据图像情况调整
    cv::Mat bin;
    cv::threshold(gray, bin, 120, 255, cv::THRESH_BINARY);

    // 3. 形态学操作
    //    先做开操作(去除噪点)，再做闭操作
    cv::Mat morph;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
    cv::morphologyEx(bin, morph, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 1);
    cv::morphologyEx(morph, morph, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 2);

    // 4. 检测轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(morph, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 5. 按照面积、长宽比等规则筛选
    std::vector<cv::Rect> candidateRects;
    for (const auto &contour : contours) {
        cv::Rect r = cv::boundingRect(contour);
        int area = r.width * r.height;
        double ratio = (double)r.width / (double)r.height;

        // 这些数值均为示例
        if (area > 50 && area < 20000 && ratio > 1.0 && ratio < 10.0) {
            candidateRects.push_back(r);
        }
    }

    return candidateRects;
}
