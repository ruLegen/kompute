#ifndef FEATUREMATCHING_KOMPUTERESOURCE_HPP
#define FEATUREMATCHING_KOMPUTERESOURCE_HPP

#include <vulkan/vulkan.hpp>

namespace kp {

    class KomputeResource {
    public:
        virtual vk::WriteDescriptorSet createWriteDescriptorSet(vk::DescriptorSet dst, int bindingPosition) = 0;
        virtual  void recordPrimaryBufferMemoryBarrier(const vk::CommandBuffer& commandBuffer,
                                                  vk::AccessFlagBits srcAccessMask,
                                                  vk::AccessFlagBits dstAccessMask,
                                                  vk::PipelineStageFlagBits srcStageMask,
                                                  vk::PipelineStageFlagBits dstStageMask) = 0;
        virtual vk::DescriptorType getDescriptorType()=0;

        virtual ~KomputeResource(){}
    };
}
#endif //FEATUREMATCHING_KOMPUTERESOURCE_HPP
