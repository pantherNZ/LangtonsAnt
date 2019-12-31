#pragma once

#include "Component.h"
#include "SceneNode.h"
#include "MovementSystem.h"

namespace Reflex::Components
{
	class Grid;
	typedef Handle< class Transform > TransformHandle;

	class Transform : public Component, public Reflex::Core::SceneNode
	{
	public:
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

	protected:
		float m_rotateDegreesPerSec = 0.0f;
		float m_rotateDurationSec = 0.0f;
		std::function< void( const TransformHandle& ) > m_finishedRotationCallback;
	};
}