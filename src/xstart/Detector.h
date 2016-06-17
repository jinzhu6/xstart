#ifndef _DETECTOR_H_
#define _DETECTOR_H_


#include "ScriptObject.h"
#include "Rect.h"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>


class Detector : public ScriptObject {
public:

	Detector() : ScriptObject() {
		id = "Detector";
		ctor = "((optional) {string} file)";
		help = "Haar-Cascade detector. Can detect objects, persons, faces, etc. on a given bitmap image.";

		BindFunction("loadHaarCascade", (SCRIPT_FUNCTION)&Detector::gm_loadHaarCascade, "{int} loadHaarCascade({sting} file)", "Loads a haar-cascade file.");
		BindFunction("detect", (SCRIPT_FUNCTION)&Detector::gm_detect, "{int} detect([Bitmap] bitmap, {int} minSize)", "Run haar-cascade detection on the given bitmap. Returns the number of findings in the bitmap.");
		BindFunction("getFinding", (SCRIPT_FUNCTION)&Detector::gm_getFinding, "[Rect] getFinding({int} index)", "Gets the finding with the given index. Returns a [Rect] object with the coordinates on the source bitmap.");
	};

	~Detector() {
	};

	virtual int Initialize(gmThread* a_thread) {
		if(a_thread->GetNumParams() == 1) {
			GM_CHECK_STRING_PARAM(file, 0);
			loadHaarCascade(file);
		}
		return GM_OK;
	}

	bool loadHaarCascade(const char* file) {
		if(!cascade.load(_FILE(file))) {
			Log(LOG_ERROR, "Error while loading haar-cascade file '%s'!", file);
			return false;
		}
		return true;
	}
	int gm_loadHaarCascade(gmThread* a_thread) {
		GM_CHECK_STRING_PARAM(file, 0);
		a_thread->PushInt(loadHaarCascade(file));
		return GM_OK;
	}

	int detect(Bitmap* bmp, int minSize) {
		if(!bmp) { return 0; }
		IMAGE* im = bmp->image;
		if(!im) { return 0; }

		// OpenCV needs image as cv:Mat matrix object
		cv::Mat mat(im->height, im->width, CV_8UC4, im->data);
		if(mat.empty()) {
			Log(LOG_ERROR, "Error while detecting object, invalid Mat conversion!");
			return 0;
		}

		// Convert image to grayscale and equalize histrogram for better detection
		cv::Mat mt_gray;
		cv::cvtColor(mat, mt_gray, CV_BGRA2GRAY);
		equalizeHist(mt_gray, mt_gray);

		// Detect and return number of findings
		cascade.detectMultiScale(mt_gray, findings, 1.1, 2, CV_HAAR_DO_CANNY_PRUNING|CV_HAAR_SCALE_IMAGE, cv::Size(minSize,minSize), cv::Size(im->width, im->height));
		return findings.size();
	}
	int gm_detect(gmThread* a_thread) {
		GM_CHECK_USER_PARAM(Bitmap*, GM_TYPE_OBJECT, bitmap, 0);
		GM_CHECK_FLOAT_OR_INT_PARAM(minSize, 1);
		a_thread->PushInt(detect(bitmap, (int)minSize));
		return GM_OK;
	}

	int gm_getFinding(gmThread* a_thread) {
		GM_CHECK_INT_PARAM(index, 0);
		if(findings.size() <= index) { return ReturnNull(a_thread); }
		Rect* rect = new Rect();
		rect->left = findings[index].x;
		rect->top = findings[index].y;
		rect->right = findings[index].x + findings[index].width;
		rect->bottom = findings[index].y + findings[index].height;
		return rect->ReturnThis(a_thread);
	}


public:
	cv::CascadeClassifier cascade;
	std::vector<cv::Rect> findings;
};


#endif
