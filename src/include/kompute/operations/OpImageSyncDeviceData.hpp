#ifndef FEATUREMATCHING_OPIMAGESYNCDEVICEDATA_HPP
#define FEATUREMATCHING_OPIMAGESYNCDEVICEDATA_HPP

#include "kompute/operations/OpBase.hpp"
#include "kompute/Image2D.hpp"

namespace kp {

    class OpImageSyncDeviceData : public OpBase {

    public:
        OpImageSyncDeviceData(const std::vector<std::shared_ptr<kp::Image2D>>& mImages);

    private:
        std::vector<std::shared_ptr<kp::Image2D>> mImages;
    public:
        void record(const vk::CommandBuffer &commandBuffer) override;

        void preEval(const vk::CommandBuffer &commandBuffer) override;

        void postEval(const vk::CommandBuffer &commandBuffer) override;
    };
}


#endif //FEATUREMATCHING_OPIMAGESYNCDEVICEDATA_HPP
