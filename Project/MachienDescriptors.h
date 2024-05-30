#pragma once

#include "MachienDevice.h"
#include <memory>
#include <unordered_map>
#include <map>
#include <vector>

namespace machien {

    class MachienDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(MachienDevice& device) : m_MachienDevice{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<MachienDescriptorSetLayout> build() const;

        private:
            MachienDevice& m_MachienDevice;
            std::map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        MachienDescriptorSetLayout(
            MachienDevice& device, std::map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~MachienDescriptorSetLayout();
        MachienDescriptorSetLayout(const MachienDescriptorSetLayout&) = delete;
        MachienDescriptorSetLayout& operator=(const MachienDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        MachienDevice& m_MachienDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class MachienDescriptorWriter;
    };

    class MachienDescriptorPool {
    public:
        class Builder {
        public:
            Builder(MachienDevice& device) : m_MachienDevice{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<MachienDescriptorPool> build() const;

        private:
            MachienDevice& m_MachienDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        MachienDescriptorPool(
            MachienDevice& MachienDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~MachienDescriptorPool();
        MachienDescriptorPool(const MachienDescriptorPool&) = delete;
        MachienDescriptorPool& operator=(const MachienDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        MachienDevice& m_MachienDevice;
        VkDescriptorPool descriptorPool;

        friend class MachienDescriptorWriter;
    };

    class MachienDescriptorWriter {
    public:
        MachienDescriptorWriter(MachienDescriptorSetLayout& setLayout, MachienDescriptorPool& pool);

        MachienDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        MachienDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        MachienDescriptorSetLayout& setLayout;
        MachienDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}