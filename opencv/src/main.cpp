#ifdef _WIN32
#include "windows.h"
#endif

#include <locale>
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <filesystem>

#include <opencv2/opencv.hpp>
#define CVUI_DISABLE_COMPILATION_NOTICES
#define CVUI_IMPLEMENTATION
#include <cvui.h>

#include "version.h"

#ifdef _WIN32
#define _setenv(k,v) _putenv_s(k,v)
#else
#define _setenv(k,v) std::setenv(k,v,1)
#endif

namespace fs = std::filesystem;

#define WINDOW_NAME "opencv"

int main(int argc, char *argv[]) {

#ifdef _WIN32
    // ロケールの設定
    std::locale::global(std::locale(".UTF8"));
    // 実行ファイルのパスを取得
    char c_exec_path[MAX_PATH];
    ::GetModuleFileNameA(NULL, c_exec_path, MAX_PATH);
    fs::path exec_path = fs::path(c_exec_path);
    if (exec_path.parent_path().filename() == "bin") {
        FreeConsole(); // デバッグ以外はコンソールを閉じる
    }
#else
    // ロケールの設定
    std::locale::global(std::locale("ja_JP.utf8"));
    // 実行ファイルのパスを取得
    fs::path exec_path = fs::canonical("/proc/self/exe");
#endif
    // GStreamerのプラグインパスを設定
    fs::path gst_plugin_path = exec_path.parent_path() / "gstreamer-1.0";
    _setenv("GST_PLUGIN_PATH", gst_plugin_path.string().c_str());

    // CVUIの初期化
    cvui::init(WINDOW_NAME);

    // VideoCaptureの初期化
    cv::VideoCapture  cap;
    cap.open("videotestsrc ! clockoverlay auto-resize=false time-format=%Y-%m-%d%H:%M:%S ! videoconvert ! appsink", cv::CAP_GSTREAMER);
    if (! cap.isOpened()) {
        return 1;
    }
    cv::Mat frame = cv::Mat(400, 600, CV_8UC3);
    cv::Mat image;
    cv::Mat processed;
    int contrast = 100; // 100 = original
    int hue = 0;        // 0 = original

    while(cap.read(image)) {
        // 描画クリア
        frame = cv::Scalar(0, 0, 0);
        // スライダー表示
        cvui::text(frame, 20, 20, "Contrast");
        cvui::trackbar(frame, 20, 40, 200, &contrast, 50, 150);

        cvui::text(frame, 20, 100, "Hue");
        cvui::trackbar(frame, 20, 120, 200, &hue, -100, 100);

        // 画像処理
        image.convertTo(processed, -1, contrast / 100.0, 0);
        cv::Mat hsv;
        cv::cvtColor(processed, hsv, cv::COLOR_BGR2HSV);
        for (int y = 0; y < hsv.rows; ++y) {
            for (int x = 0; x < hsv.cols; ++x) {
                hsv.at<cv::Vec3b>(y, x)[0] = (hsv.at<cv::Vec3b>(y, x)[0] + hue + 180) % 180;
            }
        }
        cv::cvtColor(hsv, processed, cv::COLOR_HSV2BGR);
        cv::Rect roi(250, 40, processed.cols, processed.rows);
        processed.copyTo(frame(roi));
        cvui::update();

        cvui::imshow(WINDOW_NAME, frame);
        const int key = cv::waitKey(1);
        if(key == 'q'){
            break;
        }
    }
    cv::destroyAllWindows();
    return 0;
}
