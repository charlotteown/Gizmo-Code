// Gizmo -- face tracking + OSC out (builds on "Sample Face Track.cpp").
// YuNet detects the nearest face; its centre is normalized to [-1,1] and sent to Bela
// as /cv/person <present> <x> <y>. No external OSC dependency (see OscSender.h).

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include "OscSender.h"

using namespace cv;
using namespace std;

// ---- config ----
static const char*    kBelaIp    = "192.168.7.2";  // TODO: Bela's IP on the shared link
static const uint16_t kBelaPort  = 7562;           // must match cfg::kOscReceivePort on Bela
static const bool     kShowWindow = true;          // debug preview window

int main(int argc, char** argv) {
    OscSender osc;
    if (!osc.open(argc > 1 ? argv[1] : kBelaIp, kBelaPort)) { cerr << "OSC socket failed\n"; return 1; }

    // YuNet face detection (unchanged from the sample: score 0.9, nms 0.3, top_k 5000).
    String modelPath = "face_detection_yunet_2026may.onnx";
    Ptr<FaceDetectorYN> detector =
        FaceDetectorYN::create(modelPath, "", Size(320, 320), 0.9f, 0.3f, 5000);

    VideoCapture cam(0);   // built-in webcam for now
    if (!cam.isOpened()) { cerr << "camera 0 failed\n"; return 1; }

    Mat frame, faces;
    while (true) {
        if (!cam.read(frame) || frame.empty()) continue;

        detector->setInputSize(frame.size());
        detector->detect(frame, faces);

        int   present = 0;
        float nx = 0.0f, ny = 0.0f;

        if (faces.rows > 0) {
            // nearest face == largest bounding box
            int best = 0; float bestArea = -1.0f;
            for (int i = 0; i < faces.rows; i++) {
                float w = faces.at<float>(i, 2), h = faces.at<float>(i, 3);
                if (w * h > bestArea) { bestArea = w * h; best = i; }
            }
            float x = faces.at<float>(best, 0), y = faces.at<float>(best, 1);
            float w = faces.at<float>(best, 2), h = faces.at<float>(best, 3);
            float cx = x + w * 0.5f, cy = y + h * 0.5f;

            nx = (cx / frame.cols) * 2.0f - 1.0f;   // [-1,1], +x = right
            ny = (cy / frame.rows) * 2.0f - 1.0f;   // [-1,1], +y = down (Bela inverts for tilt)
            present = 1;

            if (kShowWindow)
                rectangle(frame, Rect2i((int)x, (int)y, (int)w, (int)h), Scalar(0, 255, 0), 2);
        }

        osc.sendPerson(present, nx, ny);   // sent every frame (~30 Hz)

        if (kShowWindow) {
            imshow("Gizmo face track", frame);
            if (waitKey(1) == 27) break;   // Esc to quit
        }
    }

    osc.close();
    return 0;
}
