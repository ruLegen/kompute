#ifndef FEATUREMATCHING_IMAGE2D_HPP
#define FEATUREMATCHING_IMAGE2D_HPP
#include <memory>
#include "vulkan/vulkan.hpp"

namespace kp {
    class Image2D {

    public:
        Image2D(std::shared_ptr<vk::PhysicalDevice> physicalDevice,
                std::shared_ptr<vk::Device> device,
                vk::Format imageFormat,
                int width,
                int height,
                void *data);

        void rebuild(vk::Format type, int width, int height, void *data);

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
    private:
        std::shared_ptr<vk::PhysicalDevice> mPhysicalDevice;
        std::shared_ptr<vk::Device> mDevice;


        uint32_t findMemoryIndex(vk::PhysicalDeviceMemoryProperties memoryProperties,
                                 vk::MemoryRequirements memoryRequirements,
                                 vk::MemoryPropertyFlags desiredMemoryTypes);

        vk::Image mCreatedImage;
        vk::DeviceMemory mBindedMemory;
    };
}
#endif //FEATUREMATCHING_IMAGE2D_HPP
