
#include "kompute/operations/OpImageSyncDeviceData.hpp"

kp::OpImageSyncDeviceData::OpImageSyncDeviceData(const std::vector<std::shared_ptr<kp::Image2D>>& images)
{
    KP_LOG_DEBUG("Kompute OpImageSyncDeviceData constructor with params");

    if (images.size() < 1) {
        throw std::runtime_error(
                "Kompute OpImageSyncDeviceData called with less than 1 tensor");
    }

    this->mImages = images;
}


void kp::OpImageSyncDeviceData::record(const vk::CommandBuffer &commandBuffer) {
    for(auto&& image : mImages){
        image->writeCopyImageToBuffer(commandBuffer);
    }
}

void kp::OpImageSyncDeviceData::preEval(const vk::CommandBuffer &commandBuffer) {

}

void kp::OpImageSyncDeviceData::postEval(const vk::CommandBuffer &commandBuffer) {

}
