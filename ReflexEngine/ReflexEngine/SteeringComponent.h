#pragma once

#include "Component.h"
#include "Utility.h"
#include "Object.h"

namespace Reflex::Systems { class SteeringSystem; }

namespace Reflex::Components
{
	// Class definition
	DefineComponent( Steering )
		friend class Reflex::Systems::SteeringSystem;

		Steering() : Component() { }

		enum class Behaviours
		{
			Seek,
			Flee,
			Arrival,
			Wander,
			Pursue,
			Evade,
			NumBehaviours,
		};

		void Seek( const sf::Vector2f& target, const float maxVelocity );
		void Flee( const sf::Vector2f& target, const float maxVelocity );
		void Arrival( const sf::Vector2f& target, const float slowingRadius, const float maxVelocity );
		void Wander( const float circleRadius, const float crcleDistance, const float angleDelta, const float maxVelocity );
		void Pursue( const Reflex::ObjectHandle& target, const float maxVelocity );
		void Evade( const Reflex::ObjectHandle& target, const float maxVelocity );
		void DisableBehaviour( const Behaviours behaviour );
		bool IsBehaviourSet( const Behaviours behaviour ) const;
		void SetMaxForce( const float force );
		void SetMass( const float newMass );

	protected:
		void SetBehaviourInternal( const Behaviours behaviour );

		std::bitset< ( size_t )Behaviours::NumBehaviours > m_behaviours;
		sf::Vector2f m_desired;
		sf::Vector2f m_steering;
		float m_mass = 1.0f;
		Reflex::ObjectHandle m_targetObject;
		sf::Vector2f m_targetPosition;

		// Generic
		float m_maxForce = 100.0f;

		// Arrival
		float m_slowingRadius = 100.0f;

		// Wander
		float m_wanderCircleRadius = 10.0f;
		float m_wanderCircleDistance = 10.0f;
		float m_wanderAngleDelta = 1.0f;
		float m_currentWanderAngle = 0.0f;
	};
}