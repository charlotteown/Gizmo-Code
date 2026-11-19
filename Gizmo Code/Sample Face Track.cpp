#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/objdetect/face.hpp>
#include <iostream>


using namespace cv;
using namespace std;

int main() {

	// YuNet face detection. Deep neural net -- quite accurate

	String modelPath = "face_detection_yunet_2026may.onnx";
	Ptr<FaceDetectorYN> detector = FaceDetectorYN::create(modelPath, "", Size(320, 320), 0.9, 0.3, 5000);


	/*
	////// image //////

	Mat image = imread("Resources/test.png");

	detector->setInputSize(image.size()); //setting network input size

	Mat faces;
	detector->detect(image, faces);

	for (int i = 0; i < faces.rows; i++) {

		//from yunet example code:
		rectangle(image, Rect2i(int(faces.at<float>(i, 0)), int(faces.at<float>(i, 1)), int(faces.at<float>(i, 2)), int(faces.at<float>(i, 3))), Scalar(0, 255, 0), 2);
	}

	cv::imshow("Face Detection", image);
	cv::waitKey(0);
	*/




	////// webcam! //////
	cv::VideoCapture cam(0); // this takes the ID of our camera. Default 0
	cv::Mat frame;

	while (true) {
		cam.read(frame);

		detector->setInputSize(frame.size()); //setting network input size

		Mat Videofaces;
		detector->detect(frame, Videofaces);

		for (int i = 0; i < Videofaces.rows; i++) {

			//from yunet example code:
			rectangle(frame, Rect2i(int(Videofaces.at<float>(i, 0)), int(Videofaces.at<float>(i, 1)), int(Videofaces.at<float>(i, 2)), int(Videofaces.at<float>(i, 3))), Scalar(0, 255, 0), 2);
		}

		cv::imshow("Face Detection", frame);
		cv::waitKey(1);
	}



	return 0;
}