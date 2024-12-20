#include "airbridge_diagnose.h"
#include <opencv2/opencv.hpp>
#include <map>
#include <vector>
#include <string>
#include <iostream>

// Structure representing a bridge sample
struct BridgeSamples {
    cv::Mat image;
    int group_index;
    cv::Point pt1;
    cv::Point pt2;
};

class AirbridgeDiagnose {
public:
    void diag(const std::map<int, cv::Mat>& images, const std::vector<BridgeSamples*>& bridge_samples);
};

void AirbridgeDiagnose::diag(const std::map<int, cv::Mat>& images, const std::vector<BridgeSamples*>& bridge_samples)
{
    std::cout << bridge_samples.size() << " bridge samples need to be processed." << std::endl;

    const int thickness = 1;
    constexpr int SAMPLE_LINE_LENGTH = 49;
    constexpr int REQUIRED_HEIGHT = 4;
    constexpr int ACCUMULATION_ROW_INDEX = 1;
    const cv::Scalar RECTANGLE_COLOR_GREEN(0, 255, 0);

    std::vector<cv::Vec3d> accumulated_data(SAMPLE_LINE_LENGTH, cv::Vec3d(0, 0, 0));
    int sample_index = 0;

    for (const auto& sample : bridge_samples)
    {
        if (!sample || sample->image.empty()) {
            std::cerr << "Warning: Encountered a null or empty sample. Skipping." << std::endl;
            continue;
        }

        std::string sample_filename = "samples/sample" + std::to_string(++sample_index) + ".png";
        if (!cv::imwrite(sample_filename, sample->image)) {
            std::cerr << "Error: Failed to write image " << sample_filename << std::endl;
            continue;
        }

        const cv::Size image_size = sample->image.size();
        if (image_size.width != SAMPLE_LINE_LENGTH || image_size.height != REQUIRED_HEIGHT) {
            std::cerr << "Warning: Sample " << sample_index << " has invalid dimensions (" 
                      << image_size.width << "x" << image_size.height << "). Skipping accumulation." << std::endl;
            continue;
        }

        for (int i = 0; i < SAMPLE_LINE_LENGTH; ++i) {
            const cv::Vec3b& pixel = sample->image.at<cv::Vec3b>(ACCUMULATION_ROW_INDEX, i);
            accumulated_data[i][0] += static_cast<double>(pixel[0]);
            accumulated_data[i][1] += static_cast<double>(pixel[1]);
            accumulated_data[i][2] += static_cast<double>(pixel[2]);
        }
    }

    const size_t valid_sample_count = bridge_samples.size();
    if (valid_sample_count == 0) {
        std::cerr << "Error: No valid bridge samples to process." << std::endl;
        return;
    }

    for (auto& pixel_sum : accumulated_data) {
        pixel_sum[0] /= static_cast<double>(valid_sample_count);
        pixel_sum[1] /= static_cast<double>(valid_sample_count);
        pixel_sum[2] /= static_cast<double>(valid_sample_count);
    }

    for (const auto& sample : bridge_samples)
    {
        if (!sample || images.find(sample->group_index) == images.end()) {
            std::cerr << "Warning: Sample or corresponding image not found. Skipping." << std::endl;
            continue;
        }

        const cv::Mat& target_image = images.at(sample->group_index);
        cv::Mat annotated_image = target_image.clone();

        const cv::Size image_size = sample->image.size();
        if (image_size.width != SAMPLE_LINE_LENGTH || image_size.height != REQUIRED_HEIGHT) {
            cv::rectangle(annotated_image, sample->pt1, sample->pt2, RECTANGLE_COLOR_GREEN, thickness);
        }
        else {
            cv::Vec3i variance(0, 0, 0);
            for (int i = 0; i < SAMPLE_LINE_LENGTH; ++i) {
                const cv::Vec3b& pixel = sample->image.at<cv::Vec3b>(ACCUMULATION_ROW_INDEX, i);
                for (int c = 0; c < 3; ++c) {
                    uchar diff = (pixel[c] > accumulated_data[i][c] + 2) ? pixel[c] - static_cast<uchar>(accumulated_data[i][c]) : 0;
                    variance[c] += diff;
                }
            }

            variance[0] /= SAMPLE_LINE_LENGTH;
            variance[1] /= SAMPLE_LINE_LENGTH;
            variance[2] /= SAMPLE_LINE_LENGTH;

            std::cout << "Sample Variance - R: " << variance[2]
                      << ", G: " << variance[1]
                      << ", B: " << variance[0] << std::endl;

            cv::Scalar error_color(
                std::max(0, 255 - variance[2]),
                std::max(0, 255 - variance[1]),
                std::max(0, 255 - variance[0])
            );

            cv::rectangle(annotated_image, sample->pt1, sample->pt2, error_color, cv::FILLED);
        }

        const_cast<std::map<int, cv::Mat>&>(images).at(sample->group_index) = annotated_image;
    }

    for (const auto& [group_index, annotated_image] : images)
    {
        std::string result_filename = "result" + std::to_string(group_index) + ".png";
        if (!cv::imwrite(result_filename, annotated_image)) {
            std::cerr << "Error: Failed to write annotated image " << result_filename << std::endl;
        }
    }
}
