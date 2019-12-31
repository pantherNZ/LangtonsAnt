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
		void InteractableSystem::RegisterComponents()
		{
			RequiresComponent( Transform );
			RequiresComponent( Interactable );
			RequiresComponent( SFMLObject );
		}

		bool InteractableSystem::CheckCollision( const Reflex::Components::TransformHandle& transform, const sf::FloatRect& localBounds, const sf::Vector2f& mousePosition ) const
		{
			sf::Transform transformFinal;
			transformFinal.scale( transform->GetWorldScale() ).translate( transform->GetWorldTranslation() );
			auto globalBounds = transformFinal.transformRect( localBounds );
			globalBounds.left -= localBounds.width / 2.0f;
			globalBounds.top -= localBounds.height / 2.0f;
			return Reflex::BoundingBox( globalBounds, transform->GetWorldRotation() ).contains( mousePosition );
		}

		void InteractableSystem::Update( const float deltaTime )
		{
			const auto& window = GetWorld().GetWindow();
			const auto mousePosition = window.mapPixelToCoords( sf::Mouse::getPosition( window ) );

			ForEachSystemComponent< Transform, Interactable, SFMLObject >(
				[&]( const TransformHandle& transform, InteractableHandle& interactable, const SFMLObjectHandle& sfmlObj )
			{
				auto* ptr = interactable.Get();

				bool collision = false;
				
				// Collision with bounds
				switch( sfmlObj->GetType() )
				{
				case SFMLObjectType::Circle:
					collision = Reflex::Circle( transform->GetWorldPosition(), sfmlObj->GetCircleShape().getRadius() ).Contains( mousePosition );
				break;
				case SFMLObjectType::Rectangle:
					collision = CheckCollision( transform, sfmlObj->GetRectangleShape().getLocalBounds(), mousePosition );
				break;
				case SFMLObjectType::Convex:
					collision = CheckCollision( transform, sfmlObj->GetConvexShape().getLocalBounds(), mousePosition );
				break;
				case SFMLObjectType::Sprite:
					collision = CheckCollision( transform, sfmlObj->GetSprite().getLocalBounds(), mousePosition );
				break;
				case SFMLObjectType::Text:
					collision = CheckCollision( transform, sfmlObj->GetText().getLocalBounds(), mousePosition );
				break;
				}

				// Focus / highlighting
				if( ptr->isFocussed != collision )
				{
					ptr->isFocussed = collision;

					if( !collision && ptr->focusChangedCallback )
						ptr->focusChangedCallback( *ptr, false );
					else if( collision && ptr->focusChangedCallback )
						ptr->focusChangedCallback( *ptr, true );

					// Lost highlight, then we also unselect
					if( !collision && !ptr->selectionIsToggle && ptr->unselectIfLostFocus )
						ptr->Deselect();
				}

				// Selection (or can be deselection for toggle mode)
				if( ptr->isFocussed && m_mousePressed )
				{
					ptr->isSelected && ptr->selectionIsToggle ? ptr->Deselect() : ptr->Select();
					m_mousePressed = false;
				}

				// Un-selection
				if( m_mouseReleased && !ptr->selectionIsToggle )
					ptr->Deselect();
			} );

			m_mouseReleased = false;
		}

		void InteractableSystem::Render( sf::RenderTarget& target, sf::RenderStates states ) const
		{
			sf::RenderStates copied_states( states );

			//ForEachSystemComponent< Transform, Interactable, SFMLObject >(
			//	[&]( const TransformHandle& transform, InteractableHandle& interactable, const SFMLObjectHandle& sfmlObj )
			//{
			//	if( interactable->m_renderCollisionBounds )
			//	{
			//		copied_states.transform = states.transform * transform->getTransform();
			//
			//		switch( sfmlObj->GetType() )
			//		{
			//		case Components::Rectangle:
			//			target.draw( object->GetRectangleShape(), copied_states );
			//		break;
			//		case Components::Convex:
			//			target.draw( object->GetConvexShape(), copied_states );
			//		break;
			//		case Components::Circle:
			//			target.draw( object->GetCircleShape(), copied_states );
			//		break;
			//		case Components::Sprite:
			//			target.draw( object->GetSprite(), copied_states );
			//		break;
			//		}
			//	}
			//} );
		}

		void InteractableSystem::OnComponentAdded()
		{
			const auto newComponent = GetSystemComponent< Interactable >( m_components.back() );

			if( newComponent->m_replaceCollisionObject )
			{
				for( auto& set : m_components )
				{
					const auto obj = Handle< SFMLObject >( set[2] );

					if( obj->GetObject() == newComponent->m_replaceCollisionObject->GetObject() )
					{
						set[2] = newComponent->m_replaceCollisionObject;
						return;
					}
				}

				newComponent->m_replaceCollisionObject = SFMLObjectHandle();
			}
		}

		void InteractableSystem::ProcessEvent( const sf::Event& event )
		{
			if( event.type == sf::Event::MouseButtonPressed )
			{
				m_mousePressed = true;
			}
			else if( event.type == sf::Event::MouseButtonReleased )
			{
				m_mousePressed = false;
				m_mouseReleased = true;
			}
		}
	}
}