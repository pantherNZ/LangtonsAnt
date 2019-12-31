#pragma once

#include "System.h"
#include "TransformComponent.h"

namespace Reflex::Systems
{
	class InteractableSystem : public System
	{
	public:
		using System::System;

		void RegisterComponents() final;
		void Update( const float deltaTime ) final;
		void Render( sf::RenderTarget& target, sf::RenderStates states ) const final;
		void ProcessEvent( const sf::Event& event ) final;
		void OnComponentAdded() final;
		void OnSystemStartup() final {}
		void OnSystemShutdown() final {}

	protected:
		bool CheckCollision( const Reflex::Components::TransformHandle& transform, const sf::FloatRect& localBounds, const sf::Vector2f& mousePosition ) const;

	protected:
		bool m_mousePressed = false;
		bool m_mouseReleased = false;
	};
}