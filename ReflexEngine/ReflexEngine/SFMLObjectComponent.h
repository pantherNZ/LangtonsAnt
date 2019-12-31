#pragma once

#include "Component.h"

namespace Reflex::Components
{
	typedef Handle< class SFMLObject > SFMLObjectHandle;

	enum class SFMLObjectType : char
	{
		Invalid,
		Circle,
		Rectangle,
		Convex,
		Sprite,
		Text,
	};

	// Class definition
	class SFMLObject : public Component
	{
	public:
		SFMLObject( const sf::CircleShape& shape, const sf::Color& colour = sf::Color::White );
		SFMLObject( const sf::ConvexShape& shape, const sf::Color& colour = sf::Color::White );
		SFMLObject( const sf::RectangleShape& shape, const sf::Color& colour = sf::Color::White );
		SFMLObject( const sf::Sprite& spriteconst, const sf::Color& colour = sf::Color::White );
		SFMLObject( const sf::Text& text, const sf::Color& colour = sf::Color::White );
		SFMLObject( const SFMLObject& other );
		~SFMLObject() { }

		// Get functions
		sf::CircleShape& GetCircleShape();
		const sf::CircleShape& GetCircleShape() const;

		sf::RectangleShape& GetRectangleShape();
		const sf::RectangleShape& GetRectangleShape() const;
			
		sf::ConvexShape& GetConvexShape();
		const sf::ConvexShape& GetConvexShape() const;

		sf::Sprite& GetSprite();
		const sf::Sprite& GetSprite() const;

		sf::Text& GetText();
		const sf::Text& GetText() const;

		const SFMLObjectType GetType() const;

	private:
		union ObjectType
		{
			sf::CircleShape circleShape;			// 292 bytes
			sf::RectangleShape rectShape;			// 292 bytes
			sf::ConvexShape convexShape;			// 300 bytes
			sf::Sprite sprite;						// 272 bytes
			sf::Text text;							// 280 bytes

			ObjectType() {}
			ObjectType( const sf::CircleShape& shape ) : circleShape( shape ) { }
			ObjectType( const sf::RectangleShape& shape ) : rectShape( shape ) { }
			ObjectType( const sf::ConvexShape& shape ) : convexShape( shape ) { }
			ObjectType( const sf::Sprite& sprite ) : sprite( sprite ) { }
			ObjectType( const sf::Text& text ) : text( text ) { }
			~ObjectType() {}
		};

		ObjectType m_objectData;
		SFMLObjectType m_type = SFMLObjectType::Invalid;
	};
}