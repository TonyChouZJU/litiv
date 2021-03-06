
// This file is part of the LITIV framework; visit the original repository at
// https://github.com/plstcharles/litiv for more information.
//
// Copyright 2016 Pierre-Luc St-Charles; pierre-luc.st-charles<at>polymtl.ca
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "litiv/utils/opencv.hpp"
#include "litiv/utils/math.hpp"
#include <opencv2/features2d.hpp>

#define DASC_DEFAULT_RF_SIGMAS (2.0f)
#define DASC_DEFAULT_RF_SIGMAR (0.2f)
#define DASC_DEFAULT_RF_ITERS  (size_t(1))
#define DASC_DEFAULT_GF_RADIUS (size_t(2))
#define DASC_DEFAULT_GF_EPS    (0.09f)
#define DASC_DEFAULT_GF_SUBSPL (size_t(1))
#define DASC_DEFAULT_PREPROCESS (true)

/**
    Dense Adaptive Self-Correlation (DASC) feature extractor

    For more details on the different parameters, see S. Kim et al., "DASC: Robust Dense
    Descriptor for Multi-modal and Multi-spectral Correspondence Estimation", in TPAMI2016.
*/
class DASC : public cv::DescriptorExtractor {
public:
    /// constructor for recursive filtering-based DASC feature descriptor extractor (see original paper for parameter info)
    explicit DASC(float fSigma_s/*=DASC_DEFAULT_RF_SIGMAS*/,
                  float fSigma_r/*=DASC_DEFAULT_RF_SIGMAR*/,
                  size_t nIters=DASC_DEFAULT_RF_ITERS,
                  bool bPreProcess=DASC_DEFAULT_PREPROCESS);
    /// constructor for guided filtering-based DASC feature descriptor extractor (see original paper for parameter info)
    explicit DASC(size_t nRadius/*=DASC_DEFAULT_GF_RADIUS*/,
                  float fEpsilon/*=DASC_DEFAULT_GF_EPS*/,
                  size_t nSubSamplFrac=DASC_DEFAULT_GF_SUBSPL,
                  bool bPreProcess=DASC_DEFAULT_PREPROCESS);
    /// loads extractor params from the specified file node @@@@ not impl
    virtual void read(const cv::FileNode&) override;
    /// writes extractor params to the specified file storage @@@@ not impl
    virtual void write(cv::FileStorage&) const override;
    /// returns the window size that will be used around each keypoint (also gives the minimum image size required for description)
    virtual cv::Size windowSize() const;
    /// returns the border size required around each keypoint in x or y direction (also gives the invalid descriptor border size for output maps to ignore)
    virtual int borderSize(int nDim=0) const; // typically equal to windowSize().width/2
    /// returns the expected dense descriptor matrix output info, for a given input matrix size/type
    virtual lv::MatInfo getOutputInfo(const lv::MatInfo& oInputInfo) const;
    /// returns the current descriptor size, in bytes (overrides cv::DescriptorExtractor's)
    virtual int descriptorSize() const override;
    /// returns the current descriptor data type (overrides cv::DescriptorExtractor's)
    virtual int descriptorType() const override;
    /// returns the default norm type to use with this descriptor (overrides cv::DescriptorExtractor's)
    virtual int defaultNorm() const override;
    /// return true if detector object is empty (overrides cv::DescriptorExtractor's)
    virtual bool empty() const override;

    /// returns whether this extractor will use the recursive filtering approach (true) or the guided filtering approach (false) for description
    bool isUsingRF() const;
    /// returns whether input images will be preprocessed using a gaussian filter or not
    bool isPreProcessing() const;

    /// similar to DescriptorExtractor::compute(const cv::Mat& image, ...), but in this case, the descriptors matrix has the same shape as the input matrix, and all image points are described (note: descriptors close to borders will be invalid)
    void compute2(const cv::Mat& oImage, cv::Mat& oDescMap);
    /// similar to DescriptorExtractor::compute(const cv::Mat& image, ...), but in this case, the descriptors matrix has the same shape as the input matrix, and all image points are described (note: descriptors close to borders will be invalid)
    void compute2(const cv::Mat& oImage, cv::Mat_<float>& oDescMap);
    /// similar to DescriptorExtractor::compute(const cv::Mat& image, ...), but in this case, the descriptors matrix has the same shape as the input matrix
    void compute2(const cv::Mat& oImage, std::vector<cv::KeyPoint>& voKeypoints, cv::Mat_<float>& oDescMap);
    /// batch version of LBSP::compute2(const cv::Mat& image, ...)
    void compute2(const std::vector<cv::Mat>& voImageCollection, std::vector<cv::Mat_<float>>& voDescMapCollection);
    /// batch version of LBSP::compute2(const cv::Mat& image, ...)
    void compute2(const std::vector<cv::Mat>& voImageCollection, std::vector<std::vector<cv::KeyPoint> >& vvoPointCollection, std::vector<cv::Mat_<float>>& voDescMapCollection);

    /// utility function, used to reshape a descriptors matrix to its input image size (assumes fully-dense keypoints over input)
    static void reshapeDesc(cv::Size oSize, cv::Mat& oDescriptors);
    /// utility function, used to filter out bad keypoints that would trigger out of bounds error because they're too close to the image border
    static void validateKeyPoints(std::vector<cv::KeyPoint>& voKeypoints, cv::Size oImgSize);
    /// utility function, used to filter out bad pixels in a ROI that would trigger out of bounds error because they're too close to the image border
    static void validateROI(cv::Mat& oROI);
    /// utility function, used to calculate the L2 distance between two individual descriptors
    inline double calcDistance(const float* aDescriptor1, const float* aDescriptor2) const {
        const cv::Mat_<float> oDesc1(1,int(m_nLUTSize),const_cast<float*>(aDescriptor1));
        const cv::Mat_<float> oDesc2(1,int(m_nLUTSize),const_cast<float*>(aDescriptor2));
        return cv::norm(oDesc1,oDesc2,cv::NORM_L2);
    }
    /// utility function, used to calculate the L2 distance between two individual descriptors
    inline double calcDistance(const cv::Mat_<float>& oDescriptor1, const cv::Mat_<float>& oDescriptor2) {
        lvAssert_(oDescriptor1.dims==oDescriptor2.dims && oDescriptor1.size==oDescriptor2.size,"descriptor mat sizes mismatch");
        lvAssert_(oDescriptor1.dims==2 || oDescriptor1.dims==3,"unexpected descriptor matrix dim count");
        lvAssert_(oDescriptor1.dims!=2 || oDescriptor1.total()==m_nLUTSize,"unexpected descriptor size");
        lvAssert_(oDescriptor1.dims!=3 || (oDescriptor1.size[0]==1 && oDescriptor1.size[1]==1 && oDescriptor1.size[2]==int(m_nLUTSize)),"unexpected descriptor size");
        return calcDistance(oDescriptor1.ptr<float>(0),oDescriptor2.ptr<float>(0));
    }
    /// utility function, used to calculate per-desc L2 distance between two descriptor sets/maps
    void calcDistances(const cv::Mat_<float>& oDescriptors1, const cv::Mat_<float>& oDescriptors2, cv::Mat_<float>& oDistances);

protected:
    /// hides default keypoint detection impl (this class is a descriptor extractor only)
    using cv::DescriptorExtractor::detect;
    /// classic 'compute' implementation, based on DescriptorExtractor's arguments & expected output
    virtual void detectAndCompute(cv::InputArray oImage, cv::InputArray oMask, std::vector<cv::KeyPoint>& voKeypoints, cv::OutputArray oDescriptors, bool bUseProvidedKeypoints=false) override;
    /// defines whether this extractor will use the recursive filtering approach (true) or the guided filtering approach (false) for description
    const bool m_bUsingRF;
    /// defines whether input images should be preprocessed using a gaussian filter or not
    const bool m_bPreProcess;
    /// parameter unique to recursive-filtering-based approach
    const float m_fSigma_s;
    /// parameter unique to recursive-filtering-based approach
    const float m_fSigma_r;
    /// parameter unique to recursive-filtering-based approach
    const size_t m_nIters;
    /// parameter unique to guided-filtering-based approach
    const size_t m_nRadius;
    /// parameter unique to guided-filtering-based approach
    const float m_fEpsilon;
    /// parameter unique to guided-filtering-based approach
    const size_t m_nSubSamplFrac;
    /// defines the size of the internal pre-trained descriptor LUT
    const size_t m_nLUTSize;

private:
    /// helper/util function for recursive filtering
    void recursFilter(const cv::Mat_<float>& oImage, const cv::Mat_<float>& oRef_V_dHdx, const cv::Mat_<float>& oRef_V_dVdy_t, cv::Mat_<float>& oOutput);
    /// dense recursive filtering description approach impl
    void dasc_rf_impl(const cv::Mat& oImage, cv::Mat_<float>& oDescriptors);
    /// helper/util function for dense guided filtering
    void guidedFilter(const cv::Mat_<float>& oImage, const cv::Mat_<float>& oRef, cv::Mat_<float>& oOutput);
    /// dense guided filtering description approach impl
    void dasc_gf_impl(const cv::Mat& oImage, cv::Mat_<float>& oDescriptors);

    // helper variables for internal impl (helps avoid continuous mem realloc)
    cv::Mat_<float> m_oTempTransp,m_oImageLocalDiff_Y,m_oImageLocalDiff_X;
    cv::Mat_<float> m_oRef_dVdy,m_oRef_dHdx,m_oRef_V_dHdx,m_oRef_V_dVdy_t;
    cv::Mat_<float> m_oImage_AdaptiveMean,m_oImage_AdaptiveMeanSqr;
    cv::Mat_<float> m_oLookupImage,m_oLookupImage_Sqr,m_oLookupImage_Mix;
    cv::Mat_<float> m_oLookupImage_AdaptiveMean,m_oLookupImage_AdaptiveMeanSqr,m_oLookupImage_AdaptiveMeanMix;
    cv::Mat_<float> m_oImage_SubSampl,m_oImage_SubSamplBlur,m_oImage_SubSamplVar,m_oImage_SubSamplBlurSqr;
    cv::Mat_<float> m_oRef_SubSampl,m_oRef_SubSamplCross,m_oRef_SubSamplBlur,m_oRef_SubSamplCrossBlur;
    cv::Mat_<float> m_oNormVarDiff,m_oNormVarDiff_SubSampl,m_oNormVarDiff_SubSamplBlur;
    cv::Mat_<float> m_oNormVar,m_oNormVar_SubSampl,m_oNormVar_SubSamplBlur;
    cv::Size m_oImageSize,m_oSubSamplSize,m_oBlurKernelSize;
};