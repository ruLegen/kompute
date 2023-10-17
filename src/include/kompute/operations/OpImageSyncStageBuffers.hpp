#ifndef FEATUREMATCHING_OPIMAGESYNCSTAGEBUFFERS_HPP
#define FEATUREMATCHING_OPIMAGESYNCSTAGEBUFFERS_HPP

#include "OpBase.hpp"
#include "kompute/Image2D.hpp"
namespace kp {

    class OpImageSyncStageBuffers : public OpBase
    {
    public:
        OpImageSyncStageBuffers(const std::vector<std::shared_ptr<Image2D>>& images);

        void record(const vk::CommandBuffer &commandBuffer) override;
        void preEval(const vk::CommandBuffer &commandBuffer) override;
        void postEval(const vk::CommandBuffer &commandBuffer) override;
        ~OpImageSyncStageBuffers() override;

    private:
        std::vector<std::shared_ptr<Image2D>> mImages;
    };

}

#endif // FEATUREMATCHING_OPIMAGESYNCSTAGEBUFFERS_HPP
