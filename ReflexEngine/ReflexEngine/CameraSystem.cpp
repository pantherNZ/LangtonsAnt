#include "Precompiled.h"
#include "CameraSystem.h"
#include "CameraComponent.h"
#include "Object.h"
#include "World.h"

namespace Reflex::Systems
{
	using namespace Reflex::Components;

	void CameraSystem::RegisterComponents()
	{
		RequiresComponent( Reflex::Components::Camera );
	}

	void CameraSystem::Update( const float deltaTime )
	{
		if( auto camera = GetWorld().GetActiveCamera() )
		{
			auto transform = camera->GetObject()->GetTransform();

			if( camera->followTarget )
			{
				if( camera->followInterpSpeed == 0.0f )
					transform->setPosition( camera->followTarget->GetTransform()->getPosition() );
				else
					transform->move( ( camera->followTarget->GetTransform()->getPosition() - transform->getPosition() ) * deltaTime * camera->followInterpSpeed );
			}
			else
			{
				float vertical = 0.0f;
				float horizontal = 0.0f;

				if( camera->flags[Camera::Flags::WASDPanning] )
				{
					horizontal += ( (float )sf::Keyboard::isKeyPressed( sf::Keyboard::D ) - (float )sf::Keyboard::isKeyPressed( sf::Keyboard::A ) ) * camera->panSpeed.x;
					vertical += ( (float )sf::Keyboard::isKeyPressed( sf::Keyboard::S ) - (float )sf::Keyboard::isKeyPressed( sf::Keyboard::W ) ) * camera->panSpeed.y;
				}

				if( camera->flags[Camera::Flags::ArrowPanning] )
				{
					horizontal += ( (float )sf::Keyboard::isKeyPressed( sf::Keyboard::Right ) - (float )sf::Keyboard::isKeyPressed( sf::Keyboard::Left ) ) * camera->panSpeed.x;
					vertical += ( (float )sf::Keyboard::isKeyPressed( sf::Keyboard::Down ) - (float )sf::Keyboard::isKeyPressed( sf::Keyboard::Up ) ) * camera->panSpeed.y;
				}

				if( camera->flags[Camera::Flags::MousePanning] )
				{
					const auto mousePosPixel = sf::Mouse::getPosition( GetWorld().GetWindow() );
					horizontal += ( (float )( mousePosPixel.x >= GetWorld().GetWindow().getSize().x - camera->panMouseMargin.x ) - (float )( mousePosPixel.x <= camera->panMouseMargin.x ) ) * camera->panSpeed.x;
					vertical += ( (float )( mousePosPixel.y >= GetWorld().GetWindow().getSize().y - camera->panMouseMargin.y ) - (float )( mousePosPixel.y <= camera->panMouseMargin.y ) ) * camera->panSpeed.y;
				}

				if( vertical || horizontal )
				{
					if( !camera->flags[Camera::Flags::AdditivePanning] )
					{
						horizontal = std::min( horizontal, camera->panSpeed.x );
						vertical = std::min( vertical, camera->panSpeed.y );
					}

					if( camera->flags[Camera::Flags::NormaliseDiagonalPanning] )
					{
						if( horizontal && vertical )
						{
							horizontal /= SQRT2;
							vertical /= SQRT2;
						}
					}

					transform->move( sf::Vector2f( horizontal, vertical ) * deltaTime );
				}
			}

			camera->setCenter( camera->GetObject()->GetTransform()->getPosition() );
		}
	}

	void CameraSystem::ProcessEvent( const sf::Event& event )
	{
		if( event.type == sf::Event::MouseWheelScrolled )
		{
			if( auto camera = GetWorld().GetActiveCamera() )
			{
				const auto prevMousePos = GetWorld().GetMousePosition( camera );
				camera->zoom( event.mouseWheelScroll.delta < 0 ? camera->zoomScaleFactor : 1.0f / camera->zoomScaleFactor );

				if( camera->flags[Camera::Flags::ZoomCentreOnMouse] )
				{
					const auto afterMousePos = GetWorld().GetMousePosition( camera );
					camera->GetObject()->GetTransform()->move( prevMousePos - afterMousePos );
				}
			}
		}
	}
}