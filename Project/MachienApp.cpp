#include "MachienApp.h"
#include "MachienRenderSystem.h"
#include "MachienCamera.h"
#include "MachienKeyboard.h"
#include "MachienBuffer.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


#include <stdexcept>
#include <array>
#include <iterator>
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <chrono>

namespace machien
{
	struct GlobalUBO
	{
		glm::mat4 ProjectionView{ 1.f };
		glm::vec3 LightDirection = glm::normalize(glm::vec3{ 1.f,-3.f,1.f });
	};
	MachienApp::MachienApp()
	{
		m_DescriptorPool = MachienDescriptorPool::Builder(m_Device)
						   .setMaxSets(MachienSwapChain::MAX_FRAMES_IN_FLIGHT)
						   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MachienSwapChain::MAX_FRAMES_IN_FLIGHT)
						   .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MachienSwapChain::MAX_FRAMES_IN_FLIGHT)
						   .build();
		LoadTextures();
		LoadObjects();
		// Load Texture function 

	}
	MachienApp::~MachienApp()
	{

	}
	void MachienApp::Run()
	{
		std::vector<std::unique_ptr<MachienBuffer>> uboBuffers(MachienSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<MachienBuffer>(m_Device, sizeof(GlobalUBO),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}
		
		auto allPurposeSetLayout = MachienDescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		std::vector<VkDescriptorSet> descriptorSets{ MachienSwapChain::MAX_FRAMES_IN_FLIGHT };

		for (size_t i = 0; i < descriptorSets.size(); i++)
		{
			VkDescriptorImageInfo albedoInfo
			{
			.sampler = m_AlbedoTexture->GetTextureSampler(),
			.imageView = m_AlbedoTexture->GetTextureImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			};
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			MachienDescriptorWriter(*allPurposeSetLayout,*m_DescriptorPool)
								    .writeBuffer(0,&bufferInfo)
									.writeImage(1,&albedoInfo)
								    .build(descriptorSets[i]);
		}

		MachienRenderSystem renderSys{ 
			m_Device,m_Renderer.GetSwapChainRenderPass() ,
			allPurposeSetLayout->getDescriptorSetLayout()};
        MachienCamera camera{};
        camera.SetViewDirection(glm::vec3( 0.f ), glm::vec3( 0.5f, 0.f, 1.f ));

        //camera.SetViewTarget(glm::vec3{ -1.f,-2.f,2.f }, glm::vec3{ 0.f,0.f,2.5f });

        auto cameraObject = MachienObject::CreateObject();
        MachienKeyboard cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!m_Window.IsClosed())
		{
			glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.MoveInPlaneXZ(m_Window.GetGLFWWindow(), frameTime, cameraObject);
            camera.SetViewYXZ(cameraObject.Transform.Translation, cameraObject.Transform.RadRotation);
            float aspectRatio = m_Renderer.GetAspectRatio();
           // camera.SetOrtoGraphProjection(-aspectRatio, aspectRatio, -1.f, 1.f, -1.f, 1.f);
            camera.SetPerspectiveProjection(glm::radians(50.f), aspectRatio, 0.1f, 10.f);
			if (auto commandBuffer = m_Renderer.BeginFrame())
			{
				int frameIndex = m_Renderer.GetFrameIndex();
				FrameInfo frameInfo{
					frameIndex,frameTime,commandBuffer,camera,descriptorSets[frameIndex]
				};
				//update
				GlobalUBO ubo{};
				ubo.ProjectionView = camera.GetProjection() * camera.GetView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();
				//uboBuffers[frameIndex]->flushIndex(frameIndex);
				// render
				m_Renderer.BeginSwapChainRenderPass(commandBuffer);
				renderSys.DrawObjects(frameInfo, m_Objects);
				m_Renderer.EndSwapChainRenderPass(commandBuffer);
				m_Renderer.EndFrame();
			}
		}
		vkDeviceWaitIdle(m_Device.device());
	}

	void MachienApp::LoadTextures()
	{
		m_AlbedoTexture = std::make_unique < MachienTexture>(m_Device,"resources/vehicle_diffuse.png");
	}

	void MachienApp::LoadObjects()
	{
		std::shared_ptr<MachienModel> catModel = MachienModel::CreateModelFromFile(m_Device, "resources/vehicle.obj");
		
		auto cat = MachienObject::CreateObject();
		cat.Model = catModel;
		cat.Transform.Translation = { .0f,2.0f,2.5f };
		cat.Transform.Scale = glm::vec3{ -0.1f };
		// Rotation angle in degrees (converted to radians)
		float angle = glm::radians(180.0f);

		// Apply rotation around the X-axis (pitch)
		cat.Transform.RadRotation = glm::vec3{ angle, 0.0f, 0.0f };

		m_Objects.push_back(std::move(cat));
		
		
		std::shared_ptr<MachienModel> carModel = MachienModel::CreateModelFromFile(m_Device, "resources/colored_cube.obj");
		
		auto car = MachienObject::CreateObject();
		car.Model = carModel;
		car.Transform.Translation = { 0.0f,0.0f,1.5f };
		car.Transform.Scale = glm::vec3{ -0.1f };
		m_Objects.push_back(std::move(car));

		//std::shared_ptr<MachienModel> squareModel = MachienModel::CreateSquare(m_Device, glm::vec2{ 0.5f,0.5f },0.3f, 0.3f);
		//auto square = MachienObject2D::CreateObject();
		//square.Model = squareModel;
		//square.Tranform.Translation = { 0.5f,0.5f,1.f };
		//square.Tranform.Scale = glm::vec3{ 1 };
		//square.Color = { 1.f,1.f,1.f };
		//m_Objects2D.push_back(std::move(square));
		//
		//std::shared_ptr<MachienModel> squareModel2 = MachienModel::CreateSquare(m_Device, glm::vec2{-0.2f,-0.2f}, 0.2f, 0.2f);
		//auto square2 = MachienObject2D::CreateObject();
		//square2.Model = squareModel2;
		//square2.Tranform.Translation = { -0.2f,-0.2f,1.f };
		//square2.Tranform.Scale = glm::vec3{ 1 };
		//square2.Color = { 1.f,1.f,1.f };
		//m_Objects2D.push_back(std::move(square2));
	}
   


}
