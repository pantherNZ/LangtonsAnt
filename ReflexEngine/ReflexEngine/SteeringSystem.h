#pragma once

#include "System.h"
#include "SteeringComponent.h"

namespace Reflex::Systems
{
	using Reflex::Components::Steering;

	class SteeringSystem : public System
	{
	public:
		using System::System;

		void RegisterComponents() final;
		void Update( const float deltaTime ) final;

	protected:
		void Integrate( const Reflex::Components::SteeringHandle& boid, const float deltaTime ) const;
		sf::Vector2f Steering( const Reflex::Components::SteeringHandle& boid ) const;

		sf::Vector2f Seek( const Reflex::Components::SteeringHandle& boid, const sf::Vector2f& target ) const;
		sf::Vector2f Flee( const Reflex::Components::SteeringHandle& boid, const sf::Vector2f& target ) const;
		sf::Vector2f Arrival( const Reflex::Components::SteeringHandle& boid, const sf::Vector2f& target ) const;
		sf::Vector2f Wander( const Reflex::Components::SteeringHandle& boid ) const;
		sf::Vector2f Pursue( const Reflex::Components::SteeringHandle& boid, const ObjectHandle& target ) const;
		sf::Vector2f Evade( const Reflex::Components::SteeringHandle& boid, const ObjectHandle& target ) const;
	};
}