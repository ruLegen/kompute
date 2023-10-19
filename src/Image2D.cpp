#include "kompute/Image2D.hpp"
#include "kompute/logger/Logger.hpp"
#include <memory.h>
kp::Image2D::Image2D(std::shared_ptr<vk::PhysicalDevice> physicalDevice,
                     std::shared_ptr<vk::Device> device,
                     VmaAllocator allocator,
                     vk::Format imageFormat,
                     int width,
                     int height,
                     void* data,
                     size_t dataSize)
      : mAllocator(allocator),
      mPhysicalDevice(physicalDevice),
      mDevice(device),
      mStagingBuffer(VK_NULL_HANDLE),
      mStagingBufferAllocation(VK_NULL_HANDLE)
{

    rebuild(imageFormat, width, height, data, dataSize);
}

void
kp::Image2D::rebuild(vk::Format imageFormat,
                     int width,
                     int height,
                     void* data,
                     size_t dataSize)
{

    KP_LOG_DEBUG("Kompute Image2D creating buffer");

    if (!mPhysicalDevice) {
        throw std::runtime_error("Kompute Image2D phyisical device is null");
    }
    if (!mDevice) {
        throw std::runtime_error("Kompute Image2D device is null");
    }

    if (mStagingBuffer != VK_NULL_HANDLE) {
        KP_LOG_DEBUG("Kompute Image2D deleting old buffer");
        vmaDestroyBuffer(mAllocator,mStagingBuffer,mStagingBufferAllocation);
        mStagingBuffer = VK_NULL_HANDLE;
        mStagingBufferAllocation = VK_NULL_HANDLE;
    }
    mStagingBuffer = createStagingBuffer(mAllocator,mStagingBufferAllocation, data, dataSize);

    if (mCreatedImage) {
        vmaDestroyImage(mAllocator,mCreatedImage,mImageAllocation);
        mImageAllocation = VK_NULL_HANDLE;
        mCreatedImage = vk::Image();
    }
    auto imageCreateInfo = (VkImageCreateInfo)vk::ImageCreateInfo(
      {},
      vk::ImageType::e2D,
      imageFormat,
      vk::Extent3D(width, height, 1),
      1,
      1,
      vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferDst |vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage,
      vk::SharingMode::eExclusive,
      {},
      vk::ImageLayout::ePreinitialized);
    VkImage createdImage;
    VmaAllocationInfo allocInfo;
    VmaAllocationCreateInfo imageAllocCreateInfo{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };
    if (vmaCreateImage(mAllocator,
                       &imageCreateInfo,
                       &imageAllocCreateInfo,
                       &createdImage,
                       &mImageAllocation,
                       &allocInfo) != VK_SUCCESS) {
        throw "Cannot create Image";
    }
    setLastLayout(vk::ImageLayout::ePreinitialized);
    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(
      mAllocator, mImageAllocation, &memPropFlags);
    if (!(memPropFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw "Image created not in GPU";
    }
    mCreatedImage = vk::Image(createdImage);
    auto imageViewCreateInfo = vk::ImageViewCreateInfo(
      {},
      mCreatedImage,
      vk::ImageViewType::e2D,
      imageFormat,
      {},
      vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    if(mImageView){
       mDevice->destroyImageView(mImageView, {});
        mImageView = vk::ImageView();
    }
    mImageView = mDevice->createImageView(imageViewCreateInfo);
    mImageDescriptorInfo =
      vk::DescriptorImageInfo(nullptr, mImageView);


    // if all success save image info
    mImageFormat=imageFormat;
    mWidth= width;
    mHeight=height;
    mRawDataSize=dataSize;
}


VkBuffer
kp::Image2D::createStagingBuffer(VmaAllocator allocator,
                                 VmaAllocation& outAlloc,
                                 void* data,
                                 size_t dataSize)
{

    auto bufferCreateInfo = (VkBufferCreateInfo)vk::BufferCreateInfo(
      {},
      dataSize,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::SharingMode::eExclusive);
    VkBuffer stagingBuffer;
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VmaAllocationInfo allocInfo;
    if (vmaCreateBuffer(allocator,
                        &bufferCreateInfo,
                        &allocCreateInfo,
                        &stagingBuffer,
                        &outAlloc,
                        &allocInfo) != VK_SUCCESS)
        throw std::runtime_error("Cannot create staging buffer");

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(allocator, outAlloc, &memPropFlags);
    if (!(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        throw std::runtime_error("Cannot map staging buffer");

    if(writeDataToStaging(data,dataSize,allocator,outAlloc)){
        mHasStagingBufferData = true;
    };
    return stagingBuffer;
}

vk::WriteDescriptorSet
kp::Image2D::createWriteDescriptorSet(vk::DescriptorSet dst,
                                      int bindingposition)
{
    return vk::WriteDescriptorSet(
      dst,
      bindingposition, // destination binding
      0,               // destination array element
      1,               // descriptor count
      vk::DescriptorType::eStorageImage,
      &mImageDescriptorInfo, // descriptor image info
      nullptr);
}

void
kp::Image2D::recordPrimaryBufferMemoryBarrier(
  const vk::CommandBuffer& commandBuffer,
  vk::AccessFlagBits srcAccessMask,
  vk::AccessFlagBits dstAccessMask,
  vk::PipelineStageFlagBits srcStageMask,
  vk::PipelineStageFlagBits dstStageMask)
{
    KP_LOG_DEBUG("Kompute image2d recording buffer memory barrier");
    vk::ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.oldLayout = mLayout;
    imageMemoryBarrier.newLayout = vk::ImageLayout::eGeneral;
    imageMemoryBarrier.image = mCreatedImage;
    imageMemoryBarrier.srcAccessMask = srcAccessMask;
    imageMemoryBarrier.dstAccessMask = dstAccessMask;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    commandBuffer.pipelineBarrier(srcStageMask,
                                  dstStageMask,
                                  vk::DependencyFlags(),
                                  nullptr,
                                  nullptr,
                                  imageMemoryBarrier);
    setLastLayout(vk::ImageLayout::eGeneral);
}

vk::DescriptorType
kp::Image2D::getDescriptorType()
{
    return vk::DescriptorType::eStorageImage;
}

void
kp::Image2D::recordStagingBufferCopyToImage(
  const vk::CommandBuffer& commandBuffer)
{
    // If we don't have data
    if(!mHasStagingBufferData)
        return;
    vk::BufferImageCopy region(
      0,
      0,
      0,
      vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
      { 0, 0, 0 },
      { static_cast<uint32_t>(mWidth),
        static_cast<uint32_t>(mHeight),
        1 });

    commandBuffer.copyBufferToImage(mStagingBuffer,
                                    mCreatedImage,
                                    vk::ImageLayout::eTransferDstOptimal,
                                    1,
                                    &region);
}

void
kp::Image2D::recordImageTransitionLayout(const vk::CommandBuffer& commandBuffer,
                                         vk::ImageLayout oldLayout,
                                         vk::ImageLayout newLayout)
{

    vk::ImageMemoryBarrier bufferMemoryBarrier;
    bufferMemoryBarrier.oldLayout = oldLayout;
    bufferMemoryBarrier.newLayout = newLayout;
    bufferMemoryBarrier.image = mCreatedImage;

    bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vk::PipelineStageFlagBits sourceStage =
      vk::PipelineStageFlagBits::eTopOfPipe;
    vk::PipelineStageFlagBits destinationStage =
      vk::PipelineStageFlagBits::eAllCommands;

    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {
        bufferMemoryBarrier.srcAccessMask = (vk::AccessFlagBits)0;
        bufferMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        bufferMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        bufferMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eComputeShader;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    commandBuffer.pipelineBarrier(sourceStage,
                                  destinationStage,
                                  vk::DependencyFlags(),
                                  nullptr,
                                  nullptr,
                                  bufferMemoryBarrier);

    setLastLayout(newLayout);
}

void
kp::Image2D::writeCopyImageToBuffer(const vk::CommandBuffer& commandBuffer)
{
    // Change image layout to VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    vk::ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.oldLayout = mLayout;
    imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
    imageMemoryBarrier.image = mCreatedImage;

    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
    vk::PipelineStageFlagBits sourceStage = vk::PipelineStageFlagBits::eComputeShader;
    vk::PipelineStageFlagBits destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;

    commandBuffer.pipelineBarrier(sourceStage,
                                  destinationStage,
                                  vk::DependencyFlags(),
                                  nullptr,
                                  nullptr,
                                  imageMemoryBarrier);
    setLastLayout(vk::ImageLayout::eTransferSrcOptimal);


    // Copy image to buffer
    vk::BufferImageCopy imageCopy(
      0,
      0,
      0,
      vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
      { 0, 0, 0 },
      { static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight), 1 });

    // Copy image to buffer
    commandBuffer.copyImageToBuffer(mCreatedImage,
                                    vk::ImageLayout::eTransferDstOptimal,
                                    mStagingBuffer,
                                    imageCopy);

    vk::BufferMemoryBarrier bufferMemoryBarrier(
    vk::AccessFlagBits::eTransferWrite,
    vk::AccessFlagBits::eHostRead,
     VK_QUEUE_FAMILY_IGNORED,
     VK_QUEUE_FAMILY_IGNORED,
     mStagingBuffer,
     0,
     mRawDataSize);

    // create buffer barrier
    commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eHost,
            vk::DependencyFlags(),
            nullptr,
            bufferMemoryBarrier,
            nullptr);

}

void kp::Image2D::setLastLayout(vk::ImageLayout layout) {
    mLayout = layout;
}
vk::Format
kp::Image2D::imageFormat()
{
    return mImageFormat;
}

int
kp::Image2D::width()
{
    return mWidth;
}

int
kp::Image2D::height()
{
    return mHeight;
}

void
kp::Image2D::destroy()
{
    if(mImageView){
        mDevice->destroyImageView(mImageView, nullptr);
        mImageView = vk::ImageView();
    }
    if(mCreatedImage){
        vmaDestroyImage(mAllocator,mCreatedImage,mImageAllocation);
        mImageAllocation = VK_NULL_HANDLE;
        mCreatedImage = vk::Image();
    }
    if(mStagingBuffer != VK_NULL_HANDLE){
        vmaDestroyBuffer(mAllocator,mStagingBuffer,mStagingBufferAllocation);
        mStagingBufferAllocation= VK_NULL_HANDLE;
        mStagingBuffer = VK_NULL_HANDLE;
    }
}

kp::Image2D::~Image2D()
{
    destroy();
}

bool
kp::Image2D::isInit()
{
    return mDevice && mHeight > 0 && mWidth > 0;
}

void kp::Image2D::setData(void *data, size_t dataSize)
{
    if(dataSize != mRawDataSize)
        throw std::runtime_error(Formatter()<<"Old size="<<mRawDataSize<< "!= newSize="<<dataSize);

    if(writeDataToStaging(data,dataSize,mAllocator,mStagingBufferAllocation)){
        mHasStagingBufferData = true;
    }
}

bool kp::Image2D::writeDataToStaging(void *data, size_t sizeInBytes, VmaAllocator allocator,
                                     VmaAllocation &stagingAllocationInfo)
{
    if (data != nullptr) {
        void *mappedData = nullptr;
        vmaMapMemory(allocator, stagingAllocationInfo, &mappedData);
        memcpy(mappedData, data, sizeInBytes);
        vmaFlushAllocation(allocator, stagingAllocationInfo, 0, sizeInBytes);
        vmaUnmapMemory(allocator,stagingAllocationInfo);
        return true;
    }else
        return false;


}
