#pragma once

#include "System.h"

namespace Reflex::Systems
{
	class MovementSystem : public System
	{
	public:
		using System::System;

		void RegisterComponents() final;
		void Update( const float deltaTime ) final;
		void Render( sf::RenderTarget& target, sf::RenderStates states ) const final { }
		void ProcessEvent( const sf::Event& event ) final { }
		void OnSystemStartup() final { }
		void OnSystemShutdown() final { }
	};
}