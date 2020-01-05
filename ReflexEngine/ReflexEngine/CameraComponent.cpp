#include "Precompiled.h"
#include "CameraComponent.h"
#include "World.h"
#include "Object.h"

namespace Reflex::Components
{
	Camera::Camera( const sf::Vector2f& centre, const sf::Vector2f& size )
		: sf::View( centre, size )
	{
	}

	Camera::Camera( const sf::FloatRect& viewRect )
		: sf::View( viewRect )
	{
	}

	Camera::~Camera()
	{
	}

	bool Camera::IsActiveCamera() const
	{
		return GetObject()->GetWorld().IsActiveCamera( GetHandle() );
	}

	void Camera::SetActiveCamera()
	{
		GetObject()->GetWorld().SetActiveCamera( GetHandle() );
	}
}
