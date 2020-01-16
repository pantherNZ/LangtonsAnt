#pragma once

#include "System.h"

namespace Reflex::Systems
{
	class CameraSystem : public System
	{
	public:
		using System::System;

		void RegisterComponents() final;
		void Update( const float deltaTime ) final;
		void ProcessEvent( const sf::Event& event ) final;
	};
}