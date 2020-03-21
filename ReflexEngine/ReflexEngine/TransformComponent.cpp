#include "Precompiled.h"
#include "TransformComponent.h"
#include "Object.h"
#include "World.h"

namespace Reflex
{
	namespace Components
	{
		Transform::Transform( const sf::Vector2f& position /*= sf::Vector2f()*/, const float rotation /*= 0.0f*/, const sf::Vector2f& scale /*= sf::Vector2f( 1.0f, 1.0f )*/ )
			: Reflex::Core::SceneNode()
			, Component()
		{
			
			sf::Transformable::setPosition( position );
			setRotation( rotation );
			setScale( scale );
		}

		Transform::Transform( const Transform& other )
			: SceneNode( other )
			, Component( other )
			, m_rotateDegreesPerSec( other.m_rotateDegreesPerSec )
			, m_rotateDurationSec( other.m_rotateDurationSec )
			, m_finishedRotationCallback( other.m_finishedRotationCallback )
		{

		}

		void Transform::OnConstructionComplete()
		{
			m_owningObject = GetObject();
			//m_object->GetWorld().GetTileMap().Insert( m_object, sf::FloatRect( GetWorldPosition(), sf::Vector2f( 0.0f, 0.0f ) ) );
		}

		void Transform::setPosition( float x, float y )
		{
			Transform::setPosition( sf::Vector2f( x, y ) );
		}

		void Transform::setPosition( const sf::Vector2f& position )
		{
			//auto& tileMap = m_object->GetWorld().GetTileMap();
			//const auto previousID = tileMap.GetID( m_object );

			sf::Transformable::setPosition( position );

			//const auto newID = tileMap.GetID( m_object );
			//
			//if( previousID != newID )
			//{
			//	tileMap.RemoveByID( m_object, previousID );
			//	tileMap.Insert( m_object );
			//}
		}

		void Transform::move( float offsetX, float offsetY )
		{
			Transform::move( sf::Vector2f( offsetX, offsetY ) );
		}

		void Transform::move( const sf::Vector2f& offset )
		{
			setPosition( getPosition() + offset );
		}

		void Transform::RotateForDuration( const float degrees, const float durationSec )
		{
			m_rotateDegreesPerSec = degrees / durationSec;
			m_rotateDurationSec = durationSec;
			m_finishedRotationCallback = nullptr;
		}

		void Transform::RotateForDuration( const float degrees, const float durationSec, std::function< void( const TransformHandle& ) > finishedRotationCallback )
		{
			m_rotateDegreesPerSec = degrees / durationSec;
			m_rotateDurationSec = durationSec;
			m_finishedRotationCallback = finishedRotationCallback;
		}

		void Transform::StopRotation()
		{
			m_rotateDegreesPerSec = 0.0f;
			m_rotateDurationSec = 0.0f;
			m_finishedRotationCallback = nullptr;
		}

		void Transform::SetVelocity( const sf::Vector2f velocity )
		{
			m_velocity = Reflex::Truncate( velocity, GetMaxVelocity() );
		}

	}
}