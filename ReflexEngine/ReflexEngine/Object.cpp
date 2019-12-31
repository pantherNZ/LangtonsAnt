#include "Precompiled.h"
#include "Object.h"
#include "World.h"
#include "Component.h"
#include "TransformComponent.h"

namespace Reflex
{
	namespace Core
	{
		Object::Object( World& world )
			: m_world( world )
			, m_cachedTransformType( Type( typeid( Reflex::Components::Transform ) ) )
		{

		}

		void Object::Destroy()
		{
			if( !m_destroyed )
			{
				m_world.DestroyObject( GetHandle() );
				m_destroyed = true;
			}
		}

		void Object::RemoveAllComponents()
		{
			for( auto& component : m_components )
				for( auto& c : component.second )
					c->OnRemoved();

			m_components.clear();
		}

		Handle< Reflex::Components::Component > Object::GetComponent( Type componentType ) const
		{
			const auto found = m_components.find( componentType );

			if( found == m_components.end() )
				return Handle< Reflex::Components::Component >();

			return Handle< Reflex::Components::Component >( found->second[0] );
		}

		//Handle< Reflex::Components::Component > Object::GetComponent( const unsigned index ) const
		//{
		//	if( index >= m_components.size() )
		//		return nullptr;
		//
		//	return m_components[index].second;
		//}

		Reflex::Components::TransformHandle Object::GetTransform() const
		{
			const auto transforms = m_components.find( m_cachedTransformType );
			return Reflex::Components::TransformHandle( transforms->second[0] );
		}

		World& Object::GetWorld() const
		{
			return m_world;
		}

		void Object::draw( sf::RenderTarget& target, sf::RenderStates states ) const
		{
			for( auto& component : m_components )
				for( auto& c : component.second )
					c->Render( target, states );
		}
	}
}