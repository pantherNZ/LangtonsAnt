#include "Precompiled.h"
#include "Camera.h"

namespace Reflex::Core
{
	Camera::Camera( const sf::Vector2f& centre, const sf::Vector2f& size, Context context )
		: m_view( centre, size )
	{
		context.window.setView( m_view );
	}

	Camera::Camera( const sf::FloatRect& viewRect, Context context )
		: m_view( viewRect )
	{
		context.window.setView( m_view );
	}

	Camera::~Camera()
	{

	}
}