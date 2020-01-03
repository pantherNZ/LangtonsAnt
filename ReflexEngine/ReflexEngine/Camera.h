#pragma once

// Includes
#include "Precompiled.h"
#include "Context.h"

namespace Reflex::Core
{
	class Camera
	{
	public:
		Camera( const sf::Vector2f& centre, const sf::Vector2f& size, Context context );
		Camera( const sf::FloatRect& viewRect, Context context );
		~Camera();

		sf::View& GetView() { return m_view; }
		void Update( const float deltaTime ) { }

	protected:

	protected:
		// Core window
		sf::View m_view;
	};
}