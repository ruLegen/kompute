#ifndef FEATUREMATCHING_IMAGE2D_HPP
#define FEATUREMATCHING_IMAGE2D_HPP
#include <memory>
#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.h"
#include "KomputeResource.hpp"

namespace kp {
    class Image2D : public KomputeResource {

    public:
        Image2D(std::shared_ptr<vk::PhysicalDevice> physicalDevice,
                std::shared_ptr<vk::Device> device,
                VmaAllocator allocator,
                vk::Format imageFormat,
                int width,
                int height,
                void *data,size_t dataSize);

        vk::WriteDescriptorSet createWriteDescriptorSet(vk::DescriptorSet dst, int bindingPosition) override;
        void recordPrimaryBufferMemoryBarrier(const vk::CommandBuffer& commandBuffer,
                                                       vk::AccessFlagBits srcAccessMask,
                                                       vk::AccessFlagBits dstAccessMask,
                                                       vk::PipelineStageFlagBits srcStageMask,
                                                       vk::PipelineStageFlagBits dstStageMask) override;

        void recordStagingBufferCopyToImage(const vk::CommandBuffer& commandBuffer);
        void recordImageTransitionLayout(const vk::CommandBuffer& commandBuffer,vk::ImageLayout old,vk::ImageLayout dst);

        vk::DescriptorType getDescriptorType() override;

        void rebuild(vk::Format type, int width, int height, void *data,size_t dataSize);

        vk::Format imageFormat();
        int width();
        int height();
        bool isInit();
        void destroy();

        virtual ~Image2D();

    protected:
        vk::MemoryPropertyFlags getStagingMemoryPropertyFlags();

    protected:
        vk::Format mImageFormat;
        int mWidth;
        int mHeight;
        void* mRawData;
        size_t mRawDataSize;
    private:
        VmaAllocator mAllocator;
        std::shared_ptr<vk::PhysicalDevice> mPhysicalDevice;
        std::shared_ptr<vk::Device> mDevice;


        uint32_t findMemoryIndex(vk::PhysicalDeviceMemoryProperties memoryProperties,
                                 vk::MemoryRequirements memoryRequirements,
                                 vk::MemoryPropertyFlags desiredMemoryTypes);

        vk::Image mCreatedImage;

        VkBuffer createStagingBuffer(VmaAllocator allocator,void *data, size_t dataSize);
        VkBuffer mStagingBuffer;
        vk::ImageView mImageView;
        vk::DescriptorImageInfo mImageDescriptorInfo;
    };
}
#endif //FEATUREMATCHING_IMAGE2D_HPP
