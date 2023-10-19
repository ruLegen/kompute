#ifndef FEATUREMATCHING_IMAGE2D_HPP
#define FEATUREMATCHING_IMAGE2D_HPP
#include <memory>
#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.h"
#include "KomputeResource.hpp"
#include "kompute/utils/Formatter.hpp"
#include <vector>
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

        template<typename T>
        std::vector<T> vector(){
            void *mappedData = nullptr;
            vmaMapMemory(mAllocator, mStagingBufferAllocation, &mappedData);
            auto aligment = mRawDataSize % sizeof(T);
            if(aligment != 0){
                throw std::runtime_error(
                        Formatter() << "mRawDataSize % sizeof(T) = "<< aligment
                                    <<"\n Cannot create vector of elements with size "
                                    << sizeof (T)
                                    << ". Cause total size is "
                                    << mRawDataSize);

            }

            std::vector<T> result ((T*)mappedData, ((T*)mappedData) + (mRawDataSize/sizeof(T)));
            vmaUnmapMemory(mAllocator,mStagingBufferAllocation);
            return  result;
        }

        void writeCopyImageToBuffer(const vk::CommandBuffer &buffer);
        void setData(void* data, size_t dataSize);

        virtual ~Image2D();
    private:
        VkBuffer createStagingBuffer(VmaAllocator allocator,VmaAllocation& outAlloc,void *data, size_t dataSize);
        bool writeDataToStaging(void *data, size_t sizeInBytes, VmaAllocator allocator,
                                VmaAllocation &stagingAllocationInfo);
        void setLastLayout(vk::ImageLayout layout);
    protected:
        vk::Format mImageFormat;
        int mWidth;
        int mHeight;
        size_t mRawDataSize;
    private:
        VmaAllocator mAllocator;
        std::shared_ptr<vk::PhysicalDevice> mPhysicalDevice;
        std::shared_ptr<vk::Device> mDevice;


        vk::Image mCreatedImage;

        VkBuffer mStagingBuffer;
        vk::ImageView mImageView;
        vk::DescriptorImageInfo mImageDescriptorInfo;
        vk::ImageLayout mLayout;
        VmaAllocation mStagingBufferAllocation;
        VmaAllocation mImageAllocation;
        bool mHasStagingBufferData;

    };
}
#endif //FEATUREMATCHING_IMAGE2D_HPP
