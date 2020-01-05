#pragma once

#include "Precompiled.h"

#undef GetObject

namespace Reflex::Core { class Object; }

namespace Reflex::Components
{
	typedef Handle< class Component > ComponentHandle;
	typedef Handle< const class Component > ComponentHandleConst;

	class Component : public std::enable_shared_from_this< Component >
	{
	public:
		friend class Reflex::Core::Object;

		ObjectHandle GetObject() const { return m_object; }
		ComponentHandle GetHandle() { return ComponentHandle( shared_from_this() ); }
		ComponentHandleConst GetHandle() const { return ComponentHandleConst( shared_from_this() ); }
		virtual void SetOwningObject( const ObjectHandle& owner ) { m_object = owner; }

	protected:
		virtual void OnConstructionComplete() {}
		virtual void OnRemoved() {}
		virtual void Render( sf::RenderTarget& target, sf::RenderStates states ) {}

		Component() { }
		virtual ~Component() { }

	protected:
		ObjectHandle m_object;
	};
}