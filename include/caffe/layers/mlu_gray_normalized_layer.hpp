/*
All modification made by Cambricon Corporation: © 2018-2019 Cambricon Corporation
All rights reserved.
All other contributions:
Copyright (c) 2014--2019, the respective contributors
All rights reserved.
For the list of contributors go to https://github.com/BVLC/caffe/blob/master/CONTRIBUTORS.md
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INCLUDE_CAFFE_LAYERS_MLU_YUVTONORMALIZEDGRAY_LAYER_HPP_
#define INCLUDE_CAFFE_LAYERS_MLU_YUVTONORMALIZEDGRAY_LAYER_HPP_
#ifdef USE_MLU

#include <vector>
#include "caffe/blob.hpp"
#include "caffe/layer.hpp"
#include "caffe/proto/caffe.pb.h"

namespace caffe {

/**
 * @brief MLU acceleration of MLUGrayNormalizedLayer
 *
 */

template <typename Dtype>
class MLUGrayNormalizedLayer : public Layer<Dtype> {
  public:
    explicit MLUGrayNormalizedLayer(const LayerParameter& param)
      : Layer<Dtype>(param), yuv2gray_op_ptr_(nullptr) {}
    virtual ~MLUGrayNormalizedLayer() { MLUDestroyOp(); }
    virtual void Reshape_tensor(const vector<Blob<Dtype>*>& bottom,
                                const vector<Blob<Dtype>*>& top);
    virtual void Reshape(const vector<Blob<Dtype>*>& bottom,
                         const vector<Blob<Dtype>*>& top) {
       Reshape_tensor(bottom, top);
    }
    virtual void Forward_cpu(const vector<Blob<Dtype>*>& bottom,
                             const vector<Blob<Dtype>*>& top) {
      Dtype* top_data = top[0]->mutable_cpu_data();
      uint8_t* bottom_data = (uint8_t*)bottom[0]->mutable_cpu_data();
      for (int i=0; i<bottom[0]->count(); i++) {
        top_data[i] = (int)bottom_data[i] / 255. - 0.5;
      }
    }

    virtual void Backward_cpu(const vector<Blob<Dtype>*>& top,
                              const vector<bool>& propagate_down,
                              const vector<Blob<Dtype>*>& bottom) {}
    virtual inline bool mfus_supported() { return true; }
    virtual void fuse(MFusion<Dtype>* fuser);
    virtual inline const char* type() const { return "MLUGrayNormalized"; }


  protected:
    virtual void Forward_mlu(const vector<Blob<Dtype>*>& bottom,
                             const vector<Blob<Dtype>*>& top);
    virtual void MLUDestroyOp();
    virtual void MLUCreateOpBindData(const vector<Blob<Dtype>*>& bottom,
                                     const vector<Blob<Dtype>*>& top);
    virtual void MLUCompileOp();

    cnmlBaseOp_t yuv2gray_op_ptr_;
    cnmlYuvType_t yuv_type;
};

}  // namespace caffe
#endif  // USE_MLU
#endif  // INCLUDE_CAFFE_LAYERS_MLU_YUVTONORMALIZEDGRAY_LAYER_HPP_
