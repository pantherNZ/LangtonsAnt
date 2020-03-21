#include "Precompiled.h"
#include "World.h"
#include "InteractableSystem.h"
#include "InteractableComponent.h"
#include "SFMLObjectComponent.h"

using namespace Reflex::Components;

namespace Reflex
{
	namespace Systems
	{
		void MovementSystem::RegisterComponents()
		{
			RequiresComponent( Transform );
		}

		void MovementSystem::Update( const float deltaTime )
		{
			ForEachSystemComponent< Transform >(
				[&]( const TransformHandle& transform )
			{
				if( transform->GetVelocity().x != 0.0f || transform->GetVelocity().y != 0.0f )
				{
					const auto newPos = Reflex::WrapAround( transform->getPosition() + transform->GetVelocity() * deltaTime, GetWorld().GetBounds() );
					transform->setPosition( newPos );
				}

				if( transform->m_rotateDurationSec > 0.0f )
				{
					const float step = std::min( transform->m_rotateDurationSec, deltaTime );
					transform->m_rotateDurationSec = std::max( 0.0f, transform->m_rotateDurationSec - deltaTime );

					transform->rotate( transform->m_rotateDegreesPerSec * step );

					if( transform->m_rotateDurationSec == 0.0f && transform->m_finishedRotationCallback )
						transform->m_finishedRotationCallback( transform );
				}
			} );
		}
	}
}