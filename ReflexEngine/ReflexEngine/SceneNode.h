#pragma once

#include "Precompiled.h"

namespace Reflex::Core
{
	class SceneNode : public sf::Transformable
	{
	public:
		SceneNode();
		SceneNode( const SceneNode& other );
		~SceneNode();

		void AttachChild( const Reflex::ObjectHandle& child );
		Reflex::ObjectHandle DetachChild( const Reflex::ObjectHandle& node );

		sf::Transform GetWorldTransform() const;
		sf::Vector2f GetWorldPosition() const; 

		sf::Vector2f GetWorldTranslation() const;
		float GetWorldRotation() const;
		sf::Vector2f GetWorldScale() const;

		template< typename Func >
		void ForEachChild( Func function )
		{
			std::for_each( m_children.begin(), m_children.end(), function );
		}

		unsigned GetChildrenCount() const;
		Reflex::ObjectHandle GetChild( const unsigned index ) const;
		Reflex::ObjectHandle GetParent() const;
		void SetZOrder( const unsigned renderIndex );
		unsigned GetZOrder() const;
		void SetLayer( const unsigned layerIndex );
		unsigned GetRenderIndex() const;

	protected:
		Reflex::ObjectHandle m_owningObject;
		Reflex::ObjectHandle m_parent;
	//	Reflex::VectorSet< ObjectHandle > m_children;
		std::vector< Reflex::ObjectHandle > m_children;
		unsigned m_renderIndex = 0U;
		unsigned m_layerIndex = 0U;

		static unsigned s_nextRenderIndex;
	};
}