#include "kompute/Image2D.hpp"
#include "kompute/logger/Logger.hpp"

kp::Image2D::Image2D(std::shared_ptr<vk::PhysicalDevice> physicalDevice,std::shared_ptr<vk::Device> device, vk::Format imageFormat, int width,int height, void *data):
                     mImageFormat(imageFormat),
                     mWidth(width),
                     mHeight(height),
                     mRawData(data),
                     mPhysicalDevice(physicalDevice),
                     mDevice(device)
{

    this->rebuild(mImageFormat,mWidth,mHeight,mRawData);
}

void kp::Image2D::rebuild(vk::Format imageFormat, int width, int height, void *data) {
    KP_LOG_DEBUG("Kompute Image2D creating buffer");
    if(data == nullptr)
        return;

    if (!this->mPhysicalDevice) {
        throw std::runtime_error("Kompute Image2D phyisical device is null");
    }
    if (!this->mDevice) {
        throw std::runtime_error("Kompute Image2D device is null");
    }

    auto imageCreateInfo = vk::ImageCreateInfo({},
                                               vk::ImageType::e2D,
                                               imageFormat,
                                               vk::Extent3D(width,height,1),
                                               1,
                                               1,
                                               vk::SampleCountFlagBits::e1,
                                               vk::ImageTiling::eLinear,
                                               vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                               vk::SharingMode::eExclusive);
    auto createdImage =  this->mDevice->createImage(imageCreateInfo);
    auto imageMemoryRequirements = this->mDevice->getImageMemoryRequirements(createdImage);

    auto deviceMemoryProps = this->mPhysicalDevice->getMemoryProperties();
    auto memoryIndex = this->findMemoryIndex(deviceMemoryProps,imageMemoryRequirements, getStagingMemoryPropertyFlags());

    auto allocateInfo = vk::MemoryAllocateInfo(imageMemoryRequirements.size,memoryIndex);

    auto imageMemory = this->mDevice->allocateMemory(allocateInfo);
    this->mDevice->bindImageMemory(createdImage,imageMemory,0);

    mCreatedImage = createdImage;
    mBindedMemory = imageMemory;


    /*
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }
        vkBindImageMemory(device, textureImage, textureImageMemory, 0);
     */
}

vk::Format kp::Image2D::imageFormat() {
    return mImageFormat;
}

int kp::Image2D::width() {
    return mWidth;
}

int kp::Image2D::height() {
    return mHeight;
}

void kp::Image2D::destroy() {

}

kp::Image2D::~Image2D() {
    destroy();
}

bool kp::Image2D::isInit() {
        return mDevice &&  mHeight>0 && mWidth >0  && this->mRawData;
}

vk::MemoryPropertyFlags kp::Image2D::getStagingMemoryPropertyFlags() {
    return vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
}

uint32_t kp::Image2D::findMemoryIndex(vk::PhysicalDeviceMemoryProperties memoryProperties,
                                     vk::MemoryRequirements memoryRequirements,
                                     vk::MemoryPropertyFlags memoryPropertyFlags) {

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if (memoryRequirements.memoryTypeBits & (1 << i)
        && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}
