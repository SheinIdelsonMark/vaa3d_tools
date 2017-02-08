#include <classification.h>

#include <caffe/caffe.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "../../../../released_plugins/v3d_plugins/istitch/y_imglib.h"

Classifier::Classifier(const string& model_file,
                       const string& trained_file,
                       const string& mean_file) {

    /* Load the network. */
    net_.reset(new Net<float>(model_file, TEST));
    net_->CopyTrainedLayersFrom(trained_file);
    CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
    CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";
    Blob<float>* input_layer = net_->input_blobs()[0];
    num_channels_ = input_layer->channels();
    CHECK(num_channels_ == 3 || num_channels_ == 1)
            << "Input layer should have 1 or 3 channels.";
    input_geometry_ = cv::Size(input_layer->width(), input_layer->height());
    /* Load the binaryproto mean file. */
    SetMean(mean_file);
    /* Load labels. */
}

/* Load the mean file in binaryproto format. */
void Classifier::SetMean(const string& mean_file) {
    BlobProto blob_proto;
    ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);
    /* Convert from BlobProto to Blob<float> */
    Blob<float> mean_blob;
    mean_blob.FromProto(blob_proto);
    CHECK_EQ(mean_blob.channels(), num_channels_)
            << "Number of channels of mean file doesn't match input layer.";
    /* The format of the mean file is planar 32-bit float BGR or grayscale. */
    std::vector<cv::Mat> channels;
    float* data = mean_blob.mutable_cpu_data();
    for (int i = 0; i < num_channels_; ++i) {
        /* Extract an individual channel. */
        cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
        channels.push_back(channel);
        data += mean_blob.height() * mean_blob.width();
    }
    /* Merge the separate channels into a single image. */
    cv::Mat mean;
    cv::merge(channels, mean);
    mean_ = mean;
//    cv::imwrite("/local4/Data/IVSCC_test/comparison/Caffe_testing_3nd/train/test_cpp/mean.tif",mean_);
//    /* Compute the global mean pixel value and create a mean image
//* filled with this value. */
//    cv::Scalar channel_mean = cv::mean(mean);
//    mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
//   // cv::imwrite("/local4/Data/IVSCC_test/comparison/Caffe_testing_3nd/train/test_cpp/mean.tif",mean_);
}
std::vector<std::vector<float> > Classifier::Predict(const std::vector<cv::Mat>& imgs) {
    Blob<float>* input_layer = net_->input_blobs()[0];
    input_layer->Reshape(imgs.size(), num_channels_,
                         input_geometry_.height, input_geometry_.width);
    /* Forward dimension change to all layers. */
    net_->Reshape();
    for (int i = 0; i < imgs.size(); ++i) {
        std::vector<cv::Mat> input_channels;
        WrapInputLayer(&input_channels, i);
        Preprocess(imgs[i], &input_channels);
    }
    net_->ForwardPrefilled();
    std::vector<std::vector<float> > outputs;
    Blob<float>* output_layer = net_->output_blobs()[0];
    for (int i = 0; i < output_layer->num(); ++i) {
        const float* begin = output_layer->cpu_data() + i * output_layer->channels();
        const float* end = begin + output_layer->channels();
        /* Copy the output layer to a std::vector */
        outputs.push_back(std::vector<float>(begin, end));
    }
    return outputs;
}
/* Wrap the input layer of the network in separate cv::Mat objects
* (one per channel). This way we save one memcpy operation and we
* don't need to rely on cudaMemcpy2D. The last preprocessing
* operation will write the separate channels directly to the input
* layer. */
void Classifier::WrapInputLayer(std::vector<cv::Mat>* input_channels, int n) {
    Blob<float>* input_layer = net_->input_blobs()[0];
    int width = input_layer->width();
    int height = input_layer->height();
    int channels = input_layer->channels();
    float* input_data = input_layer->mutable_cpu_data() + n * width * height * channels;
    for (int i = 0; i < channels; ++i) {
        cv::Mat channel(height, width, CV_32FC1, input_data);
        input_channels->push_back(channel);
        input_data += width * height;
    }
}
void Classifier::Preprocess(const cv::Mat& img,
                            std::vector<cv::Mat>* input_channels) {
    /* Convert the input image to the input image format of the network. */
    cv::Mat sample;
    if (img.channels() == 3 && num_channels_ == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
    else if (img.channels() == 4 && num_channels_ == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
    else if (img.channels() == 4 && num_channels_ == 3)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
    else if (img.channels() == 1 && num_channels_ == 3)
        cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
    else
        sample = img;
    cv::Mat sample_resized;
    if (sample.size() != input_geometry_)
        cv::resize(sample, sample_resized, input_geometry_);
    else
        sample_resized = sample;
    cv::Mat sample_float;
    if (num_channels_ == 3)
        sample_resized.convertTo(sample_float, CV_32FC3);
    else
        sample_resized.convertTo(sample_float, CV_32FC1);
    cv::Mat sample_normalized;
    cv::subtract(sample_float, mean_, sample_normalized);
    /* This operation will write the separate BGR planes directly to the
* input layer of the network because it is wrapped by the cv::Mat
* objects in input_channels. */
    cv::split(sample_normalized, *input_channels);
    /*
CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
== net_->input_blobs()[0]->cpu_data())
<< "Input channels are not wrapping the input layer of the network.";
*/
}


QStringList importSeriesFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.tif"<<"*.raw"<<"*.v3draw"<<"*.lsm"
            <<"*.TIF"<<"*.RAW"<<"*.V3DRAW"<<"*.LSM"
           <<"*.jpg"<<"*.JPG";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}
