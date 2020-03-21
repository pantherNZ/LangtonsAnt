#include "Precompiled.h"
#include "SteeringComponent.h"
#include "TransformComponent.h"

namespace Reflex::Components
{
	void Steering::Seek( const sf::Vector2f& target, const float maxVelocity )
	{
		SetBehaviourInternal( Behaviours::Seek );
		m_targetPosition = target;
		GetObject()->GetTransform()->SetMaxVelocity( maxVelocity );
	}

	void Steering::Flee( const sf::Vector2f& target, const float maxVelocity )
	{
		SetBehaviourInternal( Behaviours::Flee );
		m_targetPosition = target;
		GetObject()->GetTransform()->SetMaxVelocity( maxVelocity );
	}

	void Steering::Arrival( const sf::Vector2f& target, const float slowingRadius, const float maxVelocity )
	{
		SetBehaviourInternal( Behaviours::Arrival );
		m_targetPosition = target;
		m_slowingRadius = slowingRadius;
		GetObject()->GetTransform()->SetMaxVelocity( maxVelocity );
	}

	void Steering::Wander( const float circleRadius, const float crcleDistance, const float angleDelta, const float maxVelocity )
	{
		SetBehaviourInternal( Behaviours::Wander );
		m_wanderCircleRadius = circleRadius;
		m_wanderCircleDistance = crcleDistance;
		m_wanderAngleDelta = angleDelta;
		GetObject()->GetTransform()->SetMaxVelocity( maxVelocity );
	}

	void Steering::Pursue( const Reflex::ObjectHandle& target, const float maxVelocity )
	{
		SetBehaviourInternal( Behaviours::Pursue );
		m_targetObject = target;
		GetObject()->GetTransform()->SetMaxVelocity( maxVelocity );
	}

	void Steering::Evade( const Reflex::ObjectHandle& target, const float maxVelocity )
	{
		SetBehaviourInternal( Behaviours::Evade );
		m_targetObject = target;
		GetObject()->GetTransform()->SetMaxVelocity( maxVelocity );
	}

	void Steering::DisableBehaviour( const Behaviours behaviour )
	{ 
		if( (size_t )behaviour >= 0 && ( size_t )behaviour < ( size_t )Behaviours::NumBehaviours )
			m_behaviours.reset( ( size_t )behaviour );
	}

	void Steering::SetMaxForce( const float force ) 
	{ 
		m_maxForce = force; 
	}

	void Steering::SetMass( const float newMass )
	{
		m_mass = newMass;
	}

	void Steering::SetBehaviourInternal( const Behaviours behaviour )
	{
		m_behaviours.set( ( size_t )behaviour );
	}

	bool Steering::IsBehaviourSet( const Behaviours behaviour ) const
	{
		return m_behaviours.test( (size_t )behaviour );
	}

}