#include "Precompiled.h"
#include "SFMLObjectComponent.h"
#include "Object.h"

namespace Reflex
{
	namespace Components
	{
		SFMLObject::SFMLObject( const sf::CircleShape& shape, const sf::Color& colour )
			: m_type( SFMLObjectType::Circle )
			, m_objectData( shape )
		{
			Reflex::CenterOrigin( m_objectData.circleShape );
			m_objectData.circleShape.setFillColor( colour );
		}

		SFMLObject::SFMLObject( const sf::ConvexShape& shape, const sf::Color& colour )
			: m_type( SFMLObjectType::Convex )
			, m_objectData( shape )
		{
			Reflex::CenterOrigin( m_objectData.convexShape );
			m_objectData.convexShape.setFillColor( colour );
		}

		SFMLObject::SFMLObject( const sf::RectangleShape& shape, const sf::Color& colour )
			: m_type( SFMLObjectType::Rectangle )
			, m_objectData( shape )
		{
			Reflex::CenterOrigin( m_objectData.rectShape );
			m_objectData.rectShape.setFillColor( colour );
		}

		SFMLObject::SFMLObject( const sf::Sprite& sprite, const sf::Color& colour )
			: m_type( SFMLObjectType::Sprite )
			, m_objectData( sprite )
		{
			Reflex::CenterOrigin( m_objectData.sprite );
			m_objectData.sprite.setColor( colour );
		}

		SFMLObject::SFMLObject( const sf::Text& text, const sf::Color& colour )
			: m_type( SFMLObjectType::Text )
			, m_objectData( text )
		{
			Reflex::CenterOrigin( m_objectData.text );
			m_objectData.text.setFillColor( colour );
		}

		SFMLObject::SFMLObject( const SFMLObject& other )
			: Component( other )
			, m_type( other.m_type )
		{
			memcpy( &this->m_objectData, &other.m_objectData, sizeof( ObjectType ) );
		}

		sf::CircleShape& SFMLObject::GetCircleShape()
		{ 
			return m_objectData.circleShape;
		}

		const sf::CircleShape& SFMLObject::GetCircleShape() const
		{
			return m_objectData.circleShape;
		}

		sf::RectangleShape& SFMLObject::GetRectangleShape()
		{ 
			return m_objectData.rectShape;
		}

		const sf::RectangleShape& SFMLObject::GetRectangleShape() const
		{
			return m_objectData.rectShape;
		}

		sf::ConvexShape& SFMLObject::GetConvexShape()
		{ 
			return m_objectData.convexShape;
		}

		const sf::ConvexShape& SFMLObject::GetConvexShape() const
		{
			return m_objectData.convexShape;
		}

		sf::Sprite& SFMLObject::GetSprite()
		{ 
			return m_objectData.sprite;
		}

		const sf::Sprite& SFMLObject::GetSprite() const
		{
			return m_objectData.sprite;
		}

		sf::Text& SFMLObject::GetText()
		{
			return m_objectData.text;
		}

		const sf::Text& SFMLObject::GetText() const
		{
			return m_objectData.text;
		}

		const Reflex::Components::SFMLObjectType SFMLObject::GetType() const
		{
			return m_type;
		}
	}
}