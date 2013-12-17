#include "BackgroundSubtractorViBe.h"
#include "DistanceUtils.h"
#include "RandUtils.h"
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

BackgroundSubtractorViBe::BackgroundSubtractorViBe(	 size_t nColorDistThreshold
													,size_t nBGSamples
													,size_t nRequiredBGSamples)
	:	 m_nBGSamples(nBGSamples)
		,m_nRequiredBGSamples(nRequiredBGSamples)
		,m_voBGImg(nBGSamples)
		,m_nColorDistThreshold(nColorDistThreshold)
		,m_bInitialized(false) {
	CV_Assert(m_nRequiredBGSamples<=m_nBGSamples);
}

BackgroundSubtractorViBe::~BackgroundSubtractorViBe() {}

cv::AlgorithmInfo* BackgroundSubtractorViBe::info() const {
	CV_Assert(false); // NOT IMPL @@@@@
	return nullptr;
}

void BackgroundSubtractorViBe::getBackgroundImage(cv::OutputArray backgroundImage) const {
	CV_DbgAssert(m_bInitialized);
	cv::Mat oAvgBGImg = cv::Mat::zeros(m_oImgSize,CV_32FC(m_voBGImg[0].channels()));
	for(size_t n=0; n<m_nBGSamples; ++n) {
		for(int y=0; y<m_oImgSize.height; ++y) {
			for(int x=0; x<m_oImgSize.width; ++x) {
				const size_t idx_uchar = m_voBGImg[n].step.p[0]*y + m_voBGImg[n].step.p[1]*x;
				const size_t idx_flt32 = idx_uchar*4;
				float* oAvgBgImgPtr = (float*)(oAvgBGImg.data+idx_flt32);
				const uchar* const oBGImgPtr = m_voBGImg[n].data+idx_uchar;
				for(size_t c=0; c<(size_t)m_voBGImg[n].channels(); ++c)
					oAvgBgImgPtr[c] += ((float)oBGImgPtr[c])/m_nBGSamples;
			}
		}
	}
	oAvgBGImg.convertTo(backgroundImage,CV_8U);
}