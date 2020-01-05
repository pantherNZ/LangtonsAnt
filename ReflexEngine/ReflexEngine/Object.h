#pragma once

#include "Precompiled.h"
#include "Component.h"
#include "TransformComponent.h"
#include "World.h"
#include "SceneNode.h"

namespace Reflex
{
	typedef Handle< class Reflex::Core::Object > ObjectHandle;
	typedef Handle< const class Reflex::Core::Object > ObjectHandleConst;
}

namespace Reflex::Core
{
	class Object : public SceneNode, private sf::NonCopyable, public sf::Drawable, public std::enable_shared_from_this< Object >
	{
	public:
		friend class World;

		Object( World& world );
		virtual ~Object() { }

		void Destroy();

		// Creates and adds a new component of the template type and returns a handle to it
		template< class T, typename... Args >
		Handle< T > AddComponent( Args&&... args );

		// Removes all components
		void RemoveAllComponents();

		// Removes all components matching the template type
		template< class T >
		bool RemoveComponents();

		// Removes first component matching the template type
		template< class T >
		bool RemoveComponent();

		// Removes component by handle
		template< class T >
		bool RemoveComponent( Handle< T > handle );

		// Checks if this object has a component of template type
		template< class T >
		bool HasComponent() const;

		// Returns a component handle of template type if this object has one (index is the nth number of the component looking for)
		template< class T >
		Handle< T > GetComponent( const unsigned index = 0U ) const;

		Handle< Reflex::Components::Component > GetComponent( Type componentType ) const;
		//Handle< Reflex::Components::Component > GetComponent( const unsigned index ) const;

		template< class T >
		std::vector< Handle< T > >  GetComponents() const;

		// Copy components from another object to this object
		template< typename... Args >
		void CopyComponentsFrom( const ObjectHandle& other );

		Reflex::Components::TransformHandle GetTransform() const;
		ObjectHandle GetHandle() { return ObjectHandle( shared_from_this() ); }
		ObjectHandleConst GetHandle() const { return ObjectHandleConst( shared_from_this() ); }

		World& GetWorld() const;

	protected:
		void Update( const float deltaTime ) {}
		virtual void ProcessEvent( const sf::Event& event ) {}

		template< typename T, typename... Args >
		void CopyComponentsFromInternal( const ObjectHandle& other );

		// Remove the base case where there is 0 argument types (above function calls itself recursively until we reach 0 template arguments)
		template< typename... Args >
		typename std::enable_if< sizeof...( Args ) == 0 >::type CopyComponentsFromInternal( const ObjectHandle& other ) { LOG_INFO( "Copy components complete" ); }

	private:
		Object() = delete;
		void draw( sf::RenderTarget& target, sf::RenderStates states ) const final;

	protected:
		World& m_world;
		bool m_destroyed = false;
		typedef std::vector< std::shared_ptr< Reflex::Components::Component > > ComponentVector;
		std::unordered_map< Type, ComponentVector > m_components;

	private:
		Type m_cachedTransformType;
	};

	// Template definitions
	template< class T, typename... Args >
	Handle< T > Object::AddComponent( Args&&... args )
	{
		const auto componentType = Type( typeid( T ) );
		auto found = m_components.find( componentType );

		if( found == m_components.end() )
			found = m_components.insert( std::make_pair( componentType, ComponentVector() ) ).first;

		auto component = std::make_shared< T >( std::forward< Args >( args )... );
		component->SetOwningObject( GetHandle() );
		component->OnConstructionComplete();
		found->second.emplace_back( component );
		m_world.AddComponentToSystems( GetHandle(), component->GetHandle(), componentType );
		return Handle< T >( component );
	}

	template< class T >
	bool Object::RemoveComponents()
	{
		const auto componentType = Type( typeid( T ) );
		auto found = m_components.find( componentType );

		if( found == m_components.end() || found->second.empty() )
			return false;

		for( auto& component : found->second )
			component.OnRemoved();

		found->second.clear();
		return true;
	}

	template< class T >
	bool Object::RemoveComponent()
	{
		const auto componentType = Type( typeid( T ) );
		auto found = m_components.find( componentType );

		if( found == m_components.end() || found->second.empty() )
			return false;

		found->second.begin()->OnRemoved();
		found->second.erase( found->second.begin() );
		return true;
	}

	template< class T >
	bool Object::RemoveComponent( Handle< T > handle )
	{
		if( handle.expired() )
			return false;

		const auto componentType = Type( typeid( T ) );
		auto found = m_components.find( componentType );

		if( found == m_components.end() || found->second.empty() )
			return false;

		auto ptr = handle.lock();

		auto match = std::find( found->second.begin(), found->second.end(), ptr );

		if( match == found->second.end() )
			return false;

		( *match )->OnRemoved();
		found->second.erase( match );
		return true;
	}

	template< class T >
	bool Object::HasComponent() const
	{
		return GetComponent< T >().IsValid();
	}

	template< class T >
	Handle< T > Object::GetComponent( const unsigned index /*= 0U*/ ) const
	{
		const auto componentType = Type( typeid( T ) );

		auto found = m_components.find( componentType );

		if( found == m_components.end() || found->second.size() > index )
			return Handle< T >();

		return Handle< T >( std::static_pointer_cast< T >( found->second[index] ) );
	}

	template< class T >
	std::vector< Handle< T > > Object::GetComponents() const
	{
		const auto componentType = Type( typeid( T ) );

		auto found = m_components.find( componentType );
		std::vector< Handle< T > > results;

		if( found == m_components.end() || found->second.empty() )
			return results;

		for( auto& component : found->second )
		{
			if( !component )
				continue;

			results.push_back( Handle< T >( std::static_pointer_cast< T >( component ) ) );
		}

		return results;
	}

	template< typename... Args >
	void Object::CopyComponentsFrom( const ObjectHandle& other )
	{
		CopyComponentsFromInternal< Args... >( other );
	}

	template< typename T, typename... Args >
	void Object::CopyComponentsFromInternal( const ObjectHandle& other )
	{
		RemoveAllComponents();

		auto component = other->GetComponent< T >();

		if( component )
			AddComponent< T >( *component.Get() );

		// Recursively pop arguments off the variadic template args and continue
		CopyComponentsFrom< Args... >( other );
	}
}