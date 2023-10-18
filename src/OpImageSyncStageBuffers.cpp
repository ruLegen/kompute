
#include "kompute/operations/OpImageSyncStageBuffers.hpp"
#include "include/kompute/Core.hpp"
#include "include/kompute/Image2D.hpp"
kp::OpImageSyncStageBuffers::OpImageSyncStageBuffers(const std::vector<std::shared_ptr<Image2D>> &images) {
    KP_LOG_DEBUG("Kompute OpImageSyncStageBuffers constructor with params");

    if (images.size() < 1) {
        throw std::runtime_error(
                "Kompute OpImageSyncStageBuffers called with less than 1 tensor");
    }

    this->mImages = images;
}
kp::OpImageSyncStageBuffers::~OpImageSyncStageBuffers() {
    KP_LOG_DEBUG("Kompute OpImageSyncStageBuffers destructor started");
    this->mImages.clear();
}

void kp::OpImageSyncStageBuffers::record(const vk::CommandBuffer &commandBuffer) {
    for (auto&& image: this->mImages) {
        image->recordImageTransitionLayout(commandBuffer,vk::ImageLayout::eUndefined,vk::ImageLayout::eTransferDstOptimal);
        image->setLastLayout(vk::ImageLayout::eTransferDstOptimal);
    }
    for (auto&& image: this->mImages) {
        image->recordStagingBufferCopyToImage(commandBuffer);
    }
}

void kp::OpImageSyncStageBuffers::preEval(const vk::CommandBuffer &commandBuffer) {

}

void kp::OpImageSyncStageBuffers::postEval(const vk::CommandBuffer &commandBuffer) {

}


