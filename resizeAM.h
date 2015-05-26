#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int resizeManual(std::string input,std::string output, cv::Point size, cv::Point featured1, cv::Point featured2);
int resizeAutomatic(std::string input, std::string output, cv::Point size);
int clip(int n, int lower, int upper);