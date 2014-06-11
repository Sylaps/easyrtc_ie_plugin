
#include "stdafx.h"

#include <math.h>

#include "talk/base/common.h"
#include "talk/base/logging.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/base/win32socketserver.h"
#include "talk/base/json.h"
#include "defaults.h"
#include "talk/media/webrtc/webrtcvideocapturer.h"

#include "easy_rtc_video_renderer.h"
#include "javascript_callback.h"
#include "WebRTCAPI.h"
#include "device_controller.h"


EasyRTCVideoRenderer::EasyRTCVideoRenderer(DeviceController* cb, std::string easyrtcid, int width, int height,
	webrtc::VideoTrackInterface* track_to_render)
	: callback_(cb), easyrtcid_(easyrtcid), rendered_track_(track_to_render) {

	::InitializeCriticalSection(&buffer_lock_);
	ZeroMemory(&bmi_, sizeof(bmi_));
	bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi_.bmiHeader.biPlanes = 1;
	bmi_.bmiHeader.biBitCount = 32;
	bmi_.bmiHeader.biCompression = BI_RGB;
	bmi_.bmiHeader.biWidth = width;
	bmi_.bmiHeader.biHeight = -height;
	bmi_.bmiHeader.biSizeImage = width * height * (bmi_.bmiHeader.biBitCount >> 3);
	rendered_track_->AddRenderer(this);
}

EasyRTCVideoRenderer::~EasyRTCVideoRenderer() {
	rendered_track_->RemoveRenderer(this);
	::DeleteCriticalSection(&buffer_lock_);
}

void EasyRTCVideoRenderer::SetSize(int width, int height) {
	AutoLock<EasyRTCVideoRenderer> lock(this);

	bmi_.bmiHeader.biWidth = width;
	bmi_.bmiHeader.biHeight = -height;
	bmi_.bmiHeader.biSizeImage = width * height *
		(bmi_.bmiHeader.biBitCount >> 3);
	image_.reset(new uint8[bmi_.bmiHeader.biSizeImage]);
}

void EasyRTCVideoRenderer::RenderFrame(const cricket::VideoFrame* frame) {
	if (!frame)
		return;

	AutoLock<EasyRTCVideoRenderer> lock(this);

	ASSERT(image_.get() != NULL);
	frame->ConvertToRgbBuffer(cricket::FOURCC_ARGB,
		image_.get(),
		bmi_.bmiHeader.biSizeImage,
		bmi_.bmiHeader.biWidth *
		bmi_.bmiHeader.biBitCount / 8);

	std::stringstream stream;
	std::string* base64bitmap = encodeImage(image_.get(), bmi_);

	// Optimized json construction for frame
	if (base64bitmap && *base64bitmap != "") {
		stream << "{\"pluginMessage\":{\"data\":\"data:image/png;base64,"
			<< *base64bitmap << "\", \"message\":\"frame\", \"easyrtcid\":\""
			<< this->easyrtcid_ <<"\"}}";
		std::string *data = new std::string(stream.str());
		callback_->QueueUIThreadCallback(easyrtcid_, DeviceController::SEND_MESSAGE_TO_BROWSER, data);
		delete base64bitmap;
	}

}

