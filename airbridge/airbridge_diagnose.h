#ifndef _AIRBRIDGE_DIAGNOSE_H_
#define _AIRBRIDGE_DIAGNOSE_H_
#include <map>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace std;
typedef struct _BridgeSamples
{
    cv::Mat image;
    int group_index;
    cv::Point2d pt1;
    cv::Point2d pt2;
} BridgeSamples;

class AirbridgeDiagnose
{
public:
    void diag(map<int, cv::Mat>& images, vector<BridgeSamples*>& bridge_samples);
};

#endif