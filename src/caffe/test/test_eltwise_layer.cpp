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

#include <algorithm>
#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/filler.hpp"
#include "caffe/layers/eltwise_layer.hpp"
#include "caffe/layers/mlu_eltwise_layer.hpp"

#include "caffe/test/test_caffe_main.hpp"
#include "caffe/test/test_gradient_check_util.hpp"

namespace caffe {

template <typename TypeParam>
class EltwiseLayerTest : public MultiDeviceTest<TypeParam> {
  typedef typename TypeParam::Dtype Dtype;

  protected:
  EltwiseLayerTest()
      : blob_bottom_a_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_bottom_b_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_bottom_c_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_top_(new Blob<Dtype>()) {
    // fill the values
    Caffe::set_random_seed(1701);
    FillerParameter filler_param;
    UniformFiller<Dtype> filler(filler_param);
    filler.Fill(this->blob_bottom_a_);
    filler.Fill(this->blob_bottom_b_);
    filler.Fill(this->blob_bottom_c_);
    blob_bottom_vec_.push_back(blob_bottom_a_);
    blob_bottom_vec_.push_back(blob_bottom_b_);
    blob_bottom_vec_.push_back(blob_bottom_c_);
    blob_top_vec_.push_back(blob_top_);
  }
  virtual ~EltwiseLayerTest() {
    delete blob_bottom_a_;
    delete blob_bottom_b_;
    delete blob_bottom_c_;
    delete blob_top_;
  }
  Blob<Dtype>* const blob_bottom_a_;
  Blob<Dtype>* const blob_bottom_b_;
  Blob<Dtype>* const blob_bottom_c_;
  Blob<Dtype>* const blob_top_;
  vector<Blob<Dtype>*> blob_bottom_vec_;
  vector<Blob<Dtype>*> blob_top_vec_;
};

TYPED_TEST_CASE(EltwiseLayerTest, TestDtypesAndDevices);

TYPED_TEST(EltwiseLayerTest, TestSetUp) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  shared_ptr<EltwiseLayer<Dtype> > layer(new EltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  EXPECT_EQ(this->blob_top_->num(), 2);
  EXPECT_EQ(this->blob_top_->channels(), 3);
  EXPECT_EQ(this->blob_top_->height(), 4);
  EXPECT_EQ(this->blob_top_->width(), 5);
}

TYPED_TEST(EltwiseLayerTest, TestProd) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  shared_ptr<EltwiseLayer<Dtype> > layer(new EltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  for (int i = 0; i < count; ++i) {
    EXPECT_NEAR(data[i], in_data_a[i] * in_data_b[i] * in_data_c[i], 1e-4);
  }
}

TYPED_TEST(EltwiseLayerTest, TestSum) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  shared_ptr<EltwiseLayer<Dtype> > layer(new EltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  for (int i = 0; i < count; ++i) {
    EXPECT_NEAR(data[i], in_data_a[i] + in_data_b[i] + in_data_c[i], 1e-4);
  }
}

TYPED_TEST(EltwiseLayerTest, TestSumCoeff) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  eltwise_param->add_coeff(1);
  eltwise_param->add_coeff(-0.5);
  eltwise_param->add_coeff(2);
  shared_ptr<EltwiseLayer<Dtype> > layer(new EltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  for (int i = 0; i < count; ++i) {
    EXPECT_NEAR(data[i], in_data_a[i] - 0.5 * in_data_b[i] + 2 * in_data_c[i],
                1e-4);
  }
}

TYPED_TEST(EltwiseLayerTest, TestStableProdGradient) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  eltwise_param->set_stable_prod_grad(true);
  EltwiseLayer<Dtype> layer(layer_param);
  GradientChecker<Dtype> checker(1e-2, 1e-3);
  checker.CheckGradientEltwise(&layer, this->blob_bottom_vec_,
                               this->blob_top_vec_);
}

TYPED_TEST(EltwiseLayerTest, TestUnstableProdGradient) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  eltwise_param->set_stable_prod_grad(false);
  EltwiseLayer<Dtype> layer(layer_param);
  GradientChecker<Dtype> checker(1e-2, 1e-3);
  checker.CheckGradientEltwise(&layer, this->blob_bottom_vec_,
                               this->blob_top_vec_);
}

TYPED_TEST(EltwiseLayerTest, TestSumGradient) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  EltwiseLayer<Dtype> layer(layer_param);
  GradientChecker<Dtype> checker(1e-2, 1e-3);
  checker.CheckGradientEltwise(&layer, this->blob_bottom_vec_,
                               this->blob_top_vec_);
}

TYPED_TEST(EltwiseLayerTest, TestSumCoeffGradient) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  eltwise_param->add_coeff(1);
  eltwise_param->add_coeff(-0.5);
  eltwise_param->add_coeff(2);
  EltwiseLayer<Dtype> layer(layer_param);
  GradientChecker<Dtype> checker(1e-2, 1e-3);
  checker.CheckGradientEltwise(&layer, this->blob_bottom_vec_,
                               this->blob_top_vec_);
}

TYPED_TEST(EltwiseLayerTest, TestMax) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_MAX);
  shared_ptr<EltwiseLayer<Dtype> > layer(new EltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  for (int i = 0; i < count; ++i) {
    EXPECT_EQ(data[i],
              std::max(in_data_a[i], std::max(in_data_b[i], in_data_c[i])));
  }
}

TYPED_TEST(EltwiseLayerTest, TestMaxGradient) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_MAX);
  EltwiseLayer<Dtype> layer(layer_param);
  GradientChecker<Dtype> checker(1e-4, 1e-3);
  checker.CheckGradientEltwise(&layer, this->blob_bottom_vec_,
                               this->blob_top_vec_);
}

#ifdef USE_MLU

template <typename TypeParam>
class MLUEltwiseLayerTest : public MLUDeviceTest<TypeParam> {
  typedef typename TypeParam::Dtype Dtype;

  protected:
  MLUEltwiseLayerTest()
      : blob_bottom_a_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_bottom_b_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_bottom_c_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_bottom_d_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_top_(new Blob<Dtype>()) {
    // fill the values
    Caffe::set_random_seed(1701);
    FillerParameter filler_param;
    UniformFiller<Dtype> filler(filler_param);
    filler.Fill(this->blob_bottom_a_);
    filler.Fill(this->blob_bottom_b_);
    filler.Fill(this->blob_bottom_c_);
    filler.Fill(this->blob_bottom_d_);
    blob_bottom_vec_.push_back(blob_bottom_a_);
    blob_bottom_vec_.push_back(blob_bottom_b_);
    blob_bottom_vec_.push_back(blob_bottom_c_);
    blob_bottom_vec_.push_back(blob_bottom_d_);
    blob_top_vec_.push_back(blob_top_);
  }
  virtual ~MLUEltwiseLayerTest() {
    delete blob_bottom_a_;
    delete blob_bottom_b_;
    delete blob_bottom_c_;
    delete blob_bottom_d_;
    delete blob_top_;
  }
  Blob<Dtype>* const blob_bottom_a_;
  Blob<Dtype>* const blob_bottom_b_;
  Blob<Dtype>* const blob_bottom_c_;
  Blob<Dtype>* const blob_bottom_d_;

  Blob<Dtype>* const blob_top_;
  vector<Blob<Dtype>*> blob_bottom_vec_;
  vector<Blob<Dtype>*> blob_top_vec_;
};

TYPED_TEST_CASE(MLUEltwiseLayerTest, TestMLUDevices);

TYPED_TEST(MLUEltwiseLayerTest, TestSetUp) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  EXPECT_EQ(this->blob_top_->num(), 2);
  EXPECT_EQ(this->blob_top_->channels(), 3);
  EXPECT_EQ(this->blob_top_->height(), 4);
  EXPECT_EQ(this->blob_top_->width(), 5);
}

TYPED_TEST(MLUEltwiseLayerTest, TestProd) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  const Dtype* in_data_d = this->blob_bottom_d_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = in_data_a[i] * in_data_b[i] * in_data_c[i] * in_data_d[i];
    EXPECT_NEAR(data[i], top_data, 1e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
         << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  EXPECT_LE(err_sum / sum, 1e-2);
  ERR_RATE(err_sum/sum);
  PARAM(param);
  EVENT_TIME(layer->get_event_time());
}

TYPED_TEST(MLUEltwiseLayerTest, TestSum) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  const Dtype* in_data_d = this->blob_bottom_d_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = in_data_a[i] + in_data_b[i] + in_data_c[i] + in_data_d[i];
    EXPECT_NEAR(data[i], top_data, 5e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  EXPECT_LE(err_sum / sum, 1e-3);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(layer->get_event_time());
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  PARAM(param);
  BOTTOM(stream);
}

TYPED_TEST(MLUEltwiseLayerTest, TestSumCoeff) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  eltwise_param->add_coeff(1);
  eltwise_param->add_coeff(-0.5);
  eltwise_param->add_coeff(-1);
  eltwise_param->add_coeff(2);

  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  const Dtype* in_data_d = this->blob_bottom_d_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data =
        in_data_a[i] - 0.5 * in_data_b[i] - in_data_c[i] + 2 * in_data_d[i];
    EXPECT_NEAR(data[i], top_data, 3e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  EXPECT_LE(err_sum / sum, 5e-4);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(layer->get_event_time());
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  PARAM(param);
}
TYPED_TEST(MLUEltwiseLayerTest, TestProdTwo) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_bottom_a_);
  this->blob_bottom_vec_.push_back(this->blob_bottom_b_);

  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = in_data_a[i] * in_data_b[i];
    EXPECT_NEAR(data[i], top_data, 1e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  EXPECT_LE(err_sum / sum, 1e-2);
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  PARAM(param);
  EXPECT_LE(err_sum / sum, 1e-2);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(layer->get_event_time());
}

TYPED_TEST(MLUEltwiseLayerTest, TestSumTwo) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_bottom_a_);
  this->blob_bottom_vec_.push_back(this->blob_bottom_b_);

  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = in_data_a[i] + in_data_b[i];
    EXPECT_NEAR(data[i], top_data, 5e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  EXPECT_LE(err_sum / sum, 1e-3);
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  EXPECT_LE(err_sum / sum, 1e-2);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(layer->get_event_time());
}

TYPED_TEST(MLUEltwiseLayerTest, TestSumCoeffTwo) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  eltwise_param->add_coeff(2);
  eltwise_param->add_coeff(-0.5);

  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_bottom_a_);
  this->blob_bottom_vec_.push_back(this->blob_bottom_b_);

  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->Forward(this->blob_bottom_vec_, this->blob_top_vec_);
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = 2 * in_data_a[i] - 0.5 * in_data_b[i];
    EXPECT_NEAR(data[i], top_data, 3e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  EXPECT_LE(err_sum / sum, 5e-4);
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  PARAM(param);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(layer->get_event_time());
}

template <typename TypeParam>
class MFUSEltwiseLayerTest : public MFUSDeviceTest<TypeParam> {
  typedef typename TypeParam::Dtype Dtype;

  protected:
  MFUSEltwiseLayerTest()
      : blob_bottom_a_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_bottom_b_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_bottom_c_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_bottom_d_(new Blob<Dtype>(2, 3, 4, 5)),
        blob_top_(new Blob<Dtype>()) {
    // fill the values
    Caffe::set_random_seed(1701);
    FillerParameter filler_param;
    UniformFiller<Dtype> filler(filler_param);
    filler.Fill(this->blob_bottom_a_);
    filler.Fill(this->blob_bottom_b_);
    filler.Fill(this->blob_bottom_c_);
    filler.Fill(this->blob_bottom_d_);
    blob_bottom_vec_.push_back(blob_bottom_a_);
    blob_bottom_vec_.push_back(blob_bottom_b_);
    blob_bottom_vec_.push_back(blob_bottom_c_);
    blob_bottom_vec_.push_back(blob_bottom_d_);
    blob_top_vec_.push_back(blob_top_);
  }
  virtual ~MFUSEltwiseLayerTest() {
    delete blob_bottom_a_;
    delete blob_bottom_b_;
    delete blob_bottom_c_;
    delete blob_bottom_d_;
    delete blob_top_;
  }
  Blob<Dtype>* const blob_bottom_a_;
  Blob<Dtype>* const blob_bottom_b_;
  Blob<Dtype>* const blob_bottom_c_;
  Blob<Dtype>* const blob_bottom_d_;

  Blob<Dtype>* const blob_top_;
  vector<Blob<Dtype>*> blob_bottom_vec_;
  vector<Blob<Dtype>*> blob_top_vec_;
};

TYPED_TEST_CASE(MFUSEltwiseLayerTest, TestMFUSDevices);

TYPED_TEST(MFUSEltwiseLayerTest, TestSetUp) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  EXPECT_EQ(this->blob_top_->num(), 2);
  EXPECT_EQ(this->blob_top_->channels(), 3);
  EXPECT_EQ(this->blob_top_->height(), 4);
  EXPECT_EQ(this->blob_top_->width(), 5);
}

TYPED_TEST(MFUSEltwiseLayerTest, TestProd) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  ASSERT_TRUE(layer->mfus_supported());

  MFusion<Dtype> fuser;
  fuser.reset();
  fuser.addInputs(this->blob_bottom_vec_);
  fuser.addOutputs(this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->fuse(&fuser);
  fuser.compile();
  fuser.forward();
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  const Dtype* in_data_d = this->blob_bottom_d_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = in_data_a[i] * in_data_b[i] * in_data_c[i] * in_data_d[i];
    EXPECT_NEAR(data[i], top_data, 1e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  EXPECT_LE(err_sum / sum, 1e-2);
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  PARAM(param);
  EXPECT_LE(err_sum / sum, 1e-2);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(fuser.get_event_time());
}

TYPED_TEST(MFUSEltwiseLayerTest, TestSum) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  ASSERT_TRUE(layer->mfus_supported());

  MFusion<Dtype> fuser;
  fuser.reset();
  fuser.addInputs(this->blob_bottom_vec_);
  fuser.addOutputs(this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->fuse(&fuser);
  fuser.compile();
  fuser.forward();
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  const Dtype* in_data_d = this->blob_bottom_d_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = in_data_a[i] + in_data_b[i] + in_data_c[i] + in_data_d[i];
    EXPECT_NEAR(data[i], top_data, 5e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  EXPECT_LE(err_sum / sum, 1e-3);
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  PARAM(param);
  EXPECT_LE(err_sum / sum, 1e-2);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(fuser.get_event_time());
}

TYPED_TEST(MFUSEltwiseLayerTest, TestSumCoeff) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  eltwise_param->add_coeff(1);
  eltwise_param->add_coeff(-0.5);
  eltwise_param->add_coeff(-1);
  eltwise_param->add_coeff(2);

  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  ASSERT_TRUE(layer->mfus_supported());

  MFusion<Dtype> fuser;
  fuser.reset();
  fuser.addInputs(this->blob_bottom_vec_);
  fuser.addOutputs(this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->fuse(&fuser);
  fuser.compile();
  fuser.forward();
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();
  const Dtype* in_data_c = this->blob_bottom_c_->cpu_data();
  const Dtype* in_data_d = this->blob_bottom_d_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data =
        in_data_a[i] - 0.5 * in_data_b[i] - in_data_c[i] + 2 * in_data_d[i];
    EXPECT_NEAR(data[i], top_data, 3e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  EXPECT_LE(err_sum / sum, 5e-4);
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  PARAM(param);
  EXPECT_LE(err_sum / sum, 1e-2);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(fuser.get_event_time());
}

TYPED_TEST(MFUSEltwiseLayerTest, TestProdTwo) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_PROD);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_bottom_a_);
  this->blob_bottom_vec_.push_back(this->blob_bottom_b_);

  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  ASSERT_TRUE(layer->mfus_supported());

  MFusion<Dtype> fuser;
  fuser.reset();
  fuser.addInputs(this->blob_bottom_vec_);
  fuser.addOutputs(this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->fuse(&fuser);
  fuser.compile();
  fuser.forward();
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = in_data_a[i] * in_data_b[i];
    EXPECT_NEAR(data[i], top_data, 1e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  PARAM(param);
  EXPECT_LE(err_sum / sum, 1e-2);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(fuser.get_event_time());
}

TYPED_TEST(MFUSEltwiseLayerTest, TestSumTwo) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_bottom_a_);
  this->blob_bottom_vec_.push_back(this->blob_bottom_b_);

  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  ASSERT_TRUE(layer->mfus_supported());

  MFusion<Dtype> fuser;
  fuser.reset();
  fuser.addInputs(this->blob_bottom_vec_);
  fuser.addOutputs(this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->fuse(&fuser);
  fuser.compile();
  fuser.forward();
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = in_data_a[i] + in_data_b[i];
    EXPECT_NEAR(data[i], top_data, 5e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  std::ostringstream stream, param;
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  PARAM(param);
  EXPECT_LE(err_sum / sum, 1e-2);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(fuser.get_event_time());
}

TYPED_TEST(MFUSEltwiseLayerTest, TestSumCoeffTwo) {
  typedef typename TypeParam::Dtype Dtype;
  LayerParameter layer_param;
  EltwiseParameter* eltwise_param = layer_param.mutable_eltwise_param();
  eltwise_param->set_operation(EltwiseParameter_EltwiseOp_SUM);
  eltwise_param->add_coeff(2);
  eltwise_param->add_coeff(-0.5);

  shared_ptr<MLUEltwiseLayer<Dtype> > layer(
      new MLUEltwiseLayer<Dtype>(layer_param));
  this->blob_bottom_vec_.clear();
  this->blob_bottom_vec_.push_back(this->blob_bottom_a_);
  this->blob_bottom_vec_.push_back(this->blob_bottom_b_);

  layer->SetUp(this->blob_bottom_vec_, this->blob_top_vec_);
  ASSERT_TRUE(layer->mfus_supported());

  MFusion<Dtype> fuser;
  fuser.reset();
  fuser.addInputs(this->blob_bottom_vec_);
  fuser.addOutputs(this->blob_top_vec_);
  layer->Reshape_dispatch(this->blob_bottom_vec_, this->blob_top_vec_);
  layer->fuse(&fuser);
  fuser.compile();
  fuser.forward();
  const Dtype* data = this->blob_top_->cpu_data();
  const int count = this->blob_top_->count();
  const Dtype* in_data_a = this->blob_bottom_a_->cpu_data();
  const Dtype* in_data_b = this->blob_bottom_b_->cpu_data();

  float err_sum = 0, sum = 0;
  for (int i = 0; i < count; ++i) {
    Dtype top_data = 2 * in_data_a[i] - 0.5 * in_data_b[i];
    EXPECT_NEAR(data[i], top_data, 3e-3);
    err_sum += std::abs(top_data - data[i]);
    sum += top_data;
  }
  std::ostringstream stream, param;
  EVENT_TIME(fuser.get_event_time());
  stream << "bottom1:" << this->blob_bottom_a_->shape_string().c_str() << "\t"
    << "bottom2:" << this->blob_bottom_b_->shape_string().c_str();
  param << "operation:" << eltwise_param->operation();
  BOTTOM(stream);
  PARAM(param);
  EXPECT_LE(err_sum / sum, 1e-2);
  ERR_RATE(err_sum/sum);
  EVENT_TIME(fuser.get_event_time());
}

#endif

}  // namespace caffe
