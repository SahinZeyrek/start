#pragma once
#include "MachienWindow.h"
#include "MachienDevice.h"
#include "MachienRenderer.h"
#include "MachienObject.h"
#include "MachienObject2D.h"
#include "MachienDescriptors.h"

#include <memory>
#include <vector>

namespace machien
{
	class MachienApp final
	{
	public:
		static constexpr int Width = 800;
		static constexpr int Height = 600;
		MachienApp();
		~MachienApp();


		MachienApp(const MachienApp&) = delete;
		MachienApp(MachienApp&&) = delete;
		MachienApp& operator=(const MachienApp&) = delete;
		MachienApp& operator=(MachienApp&&) = delete;

		void Run();
	private:
		void LoadObjects();
		MachienWindow m_Window{ Width,Height,"2DAE15 Sahin Zeyrek - Vulkan Milestone" };
		MachienDevice m_Device{ m_Window };
		MachienRenderer m_Renderer{ m_Window,m_Device };

		std::unique_ptr<MachienDescriptorPool> m_DescriptorPool{};
		std::vector<MachienObject> m_Objects;
		std::vector<MachienObject2D> m_Objects2D;

	};

}
