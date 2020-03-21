#pragma once

#include "Component.h"
#include "SceneNode.h"
#include "MovementSystem.h"

namespace Reflex::Components
{
	class Grid;

	DefineComponentWithInheritence( Transform, Reflex::Core::SceneNode )
		friend class Reflex::Systems::MovementSystem;
		friend class Grid;

		Transform( const sf::Vector2f& position = sf::Vector2f(), const float rotation = 0.0f, const sf::Vector2f & scale = sf::Vector2f( 1.0f, 1.0f ) );
		Transform( const Transform& other );

		void OnConstructionComplete() final;

		void setPosition( float x, float y );
		void setPosition( const sf::Vector2f& position );

		void move( float offsetX, float offsetY );
		void move( const sf::Vector2f& offset );

		void RotateForDuration( const float degrees, const float durationSec );
		void RotateForDuration( const float degrees, const float durationSec, std::function< void( const TransformHandle& ) > finishedRotationCallback );
		void StopRotation();

		void SetVelocity( const sf::Vector2f velocity );
		void ModifyVelocity( const sf::Vector2f velocity ) { SetVelocity( GetVelocity() + velocity ); }
		sf::Vector2f GetVelocity() const { return m_velocity; }

		void SetMaxVelocity( const float maxVelocity ) { m_maxVelocity = maxVelocity; }
		void ModifyMaxVelocity( const float maxVelocity ) { SetMaxVelocity( GetMaxVelocity() + maxVelocity ); }
		float GetMaxVelocity() const { return m_maxVelocity; }

	protected:
		float m_rotateDegreesPerSec = 0.0f;
		float m_rotateDurationSec = 0.0f;
		std::function< void( const TransformHandle& ) > m_finishedRotationCallback;
		sf::Vector2f m_velocity = sf::Vector2f( 0.0f, 0.0f );
		float m_maxVelocity = std::numeric_limits< float >::infinity();
	};
}